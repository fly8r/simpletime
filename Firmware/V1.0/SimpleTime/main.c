/*
 * SimpleTime.c
 *
 * Created: 11.12.2017 13:28:29
 * Author : v.bandura
 */
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "config.h"
#include "rtos.h"
#include "drvHD44780.h"
#include "utils.h"


/************************************************************************/
/* VARS                                                                 */
/************************************************************************/
enum MODE_ENUM
{
	MODE_NORMAL,				// Default normal working mode
	MODE_SET_TIMER_SECONDS,		// Setup seconds
	MODE_SET_TIMER_MINUTES,		// Setup minutes
	MODE_SET_TIMER_HOURS		// Setup hours
};

enum BUTTON_STATE_ENUM
{
	BUTTON_STATE_UP,		// Button not pressed
	BUTTON_STATE_DN,		// Button pressed short
	BUTTON_STATE_AL			// Button pressed
};

enum BUTTON_EVENTS_ENUM
{
	BUTTON_EVENT_NOT_PRESSED,   // No events
	BUTTON_EVENT_SHORT_PRESS,   // Short press event
	BUTTON_EVENT_LONG_PRESS     // Long press event
};

struct ENCODER_STRUCT
{
	int8_t			prev_state,		// Previous state line in zero by default
					value;			// Encoder pulse counter
	struct {
		enum        BUTTON_EVENTS_ENUM  event; // Key pressed event type
		enum		BUTTON_STATE_ENUM	state; // Current button state
		uint8_t		time;	// Button pressed counter
	} button;
};

struct TIMER_STRUCT
{
	int8_t			time[3];	// Time counter
	enum		    MODE_ENUM			mode;
};

struct FLAGS_STRUCT
{
    uint8_t         led_blink,      //
                    buzzer_blink;   //
};

// Max time values:                                 h,  m,  s
const	uint8_t		max_time_values[3] PROGMEM = { 47, 59, 59 };

/* Timer vars */
struct				TIMER_STRUCT		timer;

/* Buzzer cycle counter */
uint8_t             buzzer_cycle=(BUZZER_BEEP_COUNT * 2);

/* Encoder state vars */
struct				ENCODER_STRUCT		encoder;

/*  */
struct              FLAGS_STRUCT        flags;

/* Saved timer value */
uint8_t		EEMEM	EE_timer_value[3];


//------------------------------ Toggling LED and BEZZER indicators
void AUTO_ToggleOutputs(void)
{
	// Control LED IO with LED flag state
    if(!flags.led_blink) {
        TICK_LED_OFF();
    }

	// Control BUZZER IO with BUZZER flag state and BUZZER cycle counter
    if(flags.buzzer_blink && buzzer_cycle--) {
        BUZZER_TOGGLE();
    } else {
        BUZZER_OFF();
		// Reload BUZZER cycle counter
        buzzer_cycle=(BUZZER_BEEP_COUNT * 2);
		// Flush BUZZER flag
        flags.buzzer_blink=0;
    }
	// Run this task every ~500ms
    RTOS_SetTimerTask(AUTO_ToggleOutputs, 500);
}

//------------------------------ Key code processing
void keyProcessing(void)
{
	// Processing button events
	if(encoder.button.event == BUTTON_EVENT_SHORT_PRESS) {
		// Event: short click detected
		// Processing with mode
		if(timer.mode != MODE_NORMAL) {
			//> Mode is setting time
			// Set next mode
			switch(timer.mode) {
				case MODE_SET_TIMER_SECONDS: timer.mode = MODE_SET_TIMER_MINUTES; break;
				case MODE_SET_TIMER_MINUTES: timer.mode = MODE_SET_TIMER_HOURS; break;
				default: timer.mode = MODE_NORMAL; break;
			}
		} else {
			//> Mode is NORMAL
            // Check Timer time more than zero
            uint8_t t = (timer.time[SECONDS] + timer.time[MINUTES] + timer.time[HOURS]);
            if(t) {
                // Start timer tick
                //TIMER_TICK_INTERRUPT_TOGGLE();
				TIMER_TICK_TOGGLE();
                //
                flags.led_blink ^= 0x1;
				// Relay switch ON
				RELAY_TOGGLE();
            }
		}
	} else {
		// Event: long click detected
		// Set next action
		switch(timer.mode) {
			// Go to seconds setup mode
			case MODE_NORMAL:
				if(!TIMER_TICK_CHECK) {
					timer.mode = MODE_SET_TIMER_SECONDS;
				}
				break;
			// Go to loading timer value from EEPROM
            case MODE_SET_TIMER_SECONDS:
                // Waiting for EEPROM ready
                while(!eeprom_is_ready());
                // Read data block
                eeprom_read_block(&timer.time, &EE_timer_value, sizeof(timer.time));
                break;
			// Go to saving timer value in EEPROM
            case MODE_SET_TIMER_HOURS:
                // Waiting for EEPROM ready
                while(!eeprom_is_ready());
                // Write data block
                eeprom_write_block(&timer.time, &EE_timer_value, sizeof(timer.time));
                break;
			// Nothing to do
			default: break;
		}
	}
	// Flush button event
	encoder.button.event = BUTTON_EVENT_NOT_PRESSED;
}

//------------------------------ Change time value in position(seconds, minutes, hours)
void changeValueInPosition(uint8_t p)
{
	timer.time[p] += (encoder.value >> 2);
	uint8_t max_value = pgm_read_byte(max_time_values + p);
	if(timer.time[p] > max_value) timer.time[p] = 0;
	if(timer.time[p] < 0) timer.time[p] = max_value;
}

//------------------------------ Encoder value processing
void encProcessing(void)
{
	// Processing value change with mode
	switch(timer.mode) {
        // Change value in SECONDS position
		case MODE_SET_TIMER_SECONDS: changeValueInPosition(SECONDS); break;
        // Change value in MINUTES position
		case MODE_SET_TIMER_MINUTES: changeValueInPosition(MINUTES); break;
        // Change value in HOURS position
		case MODE_SET_TIMER_HOURS: changeValueInPosition(HOURS); break;
        // Other
		default: break;
	}
    // Flush encoder value after processing
	encoder.value=0;
}

//------------------------------ Display update function
void AUTO_DisplayUpdater(void)
{
	char buffer[4];

	// Moving cursor to second string begin
	hd44780_GoToXY(1, 0);
    // Update data on display in all time positions
    uint8_t i=0;
    do {
        // Update value with conversation
        hd44780_Puts(utoa_cycle_sub(timer.time[i], buffer));
        // If the current position is not the end, print char ":"
        if(i++ != 2) hd44780_Puts(":");
    } while(i<3);

    // Cursor visibility rule
	if(timer.mode != MODE_NORMAL) {
        // Show squared cursor in SET TIMER modes
		hd44780_SendCmd(HD44780_OPT_DISPLAY_ENABLE | HD44780_OPT_CURSOR_VISIBLE | HD44780_OPT_CURSOR_IS_SQUARE);
        // Cursor position selector by modes
        uint8_t position=1;
        switch(timer.mode) {
            ////////////
            //00:00:00//
            // |: |: |//
            //01234567// <- Position index
            ////////////
            case MODE_SET_TIMER_SECONDS: position=7; break;
            case MODE_SET_TIMER_MINUTES: position=4; break;
            default: break;
        }
        hd44780_GoToXY(1, position);
	} else {
        // Hide cursor in NORMAL mode
		hd44780_SendCmd(HD44780_OPT_DISPLAY_ENABLE | HD44780_OPT_CURSOR_INVISIBLE);
	}
    // Run this function every ~200ms
	RTOS_SetTimerTask(AUTO_DisplayUpdater, 100);
}

//------------------------------ Check button state
void AUTO_KeyScan(void)
{
    // Processing button state NOT PRESSED(UP)
	if(encoder.button.state == BUTTON_STATE_UP) {
		// Check key state
		if(BTN_PRESSED) {
			// Button is pressed
			// Set current state to PRESSED(DN)
			encoder.button.state = BUTTON_STATE_DN;
			// Flush pressed state timer
            encoder.button.time = 0;
            // If key pressed event was not processed
            if(!encoder.button.event) {
                // Set flag to SHORT click
                encoder.button.event = BUTTON_EVENT_SHORT_PRESS;
            }
		} else if(encoder.button.event) {
            // If key not pressed, but set event
			// Run task with key processing
			RTOS_SetTask(keyProcessing);
		}
	// Processing button state PRESSED(DN)
	} else if(encoder.button.state == BUTTON_STATE_DN) {
		// Check key state
		if(BTN_PRESSED) {
			// If timer long click not tick yet
			if(encoder.button.time < 30) {
				// Increment timer
                encoder.button.time++;
			    // Else
			} else {
				// Flush long click
                encoder.button.time = 0;
				// Set current state to LONG PRESSED(AL)
				encoder.button.event = BUTTON_STATE_AL;
			}
		// If key not pressed yet
		} else {
			// Set current state to NOT PRESSED(UP)
            encoder.button.state = BUTTON_STATE_UP;
			// Flush pressed state timer
            encoder.button.time = 0;
		}
	// Processing button state LONG PRESSED(AL)
	} else if(encoder.button.state == BUTTON_STATE_AL) {
		// Waiting while key will not be pressed
		if(!BTN_PRESSED) {
			// Set current state to NOT PRESSED(UP)
            encoder.button.state = BUTTON_STATE_UP;
			// Flush pressed state timer
            encoder.button.time = 0;
			// Set flag to LONG click
            encoder.button.event = BUTTON_EVENT_LONG_PRESS;
		}
	}
	// Run this process with periodic ~20ms
	RTOS_SetTimerTask(AUTO_KeyScan, 20);
}

//------------------------------ Check encoder state
void AUTO_EncoderScan(void)
{
    // Getting current encoder pin state
    uint8_t curr_state = ENC_STATE;
    // Processing state
    switch(encoder.prev_state) {
        case 0:
            if(curr_state == 2) encoder.value++;
            if(curr_state == 1) encoder.value--;
            break;

        case 1:
            if(curr_state == 0) encoder.value++;
            if(curr_state == 3) encoder.value--;
            break;

        case 2:
            if(curr_state == 3) encoder.value++;
            if(curr_state == 0) encoder.value--;
            break;

        case 3:
            if(curr_state == 1) encoder.value++;
            if(curr_state == 2) encoder.value--;
            break;
    }
    // Save last state of encoder pin
    encoder.prev_state = curr_state;
    // If counter not null
    //if(encoder.value != 0) {
	if(encoder.value > 3 || encoder.value < -3) {
        RTOS_SetTask(encProcessing);
    }
    // Set timer task to autostart this scan procedure every 1ms
    RTOS_SetTimerTask(AUTO_EncoderScan, 1);
}

//------------------------------ Initialize MCU peripheral
inline void MCU_Init(void)
{
	// Initialize RELAY IO
	RELAY_INIT();
	// Initialize SYSTICK timer for RTOS
	SYSTICK_TIMER_INIT();
	SYSTICK_INTERRUPT_ENABLE();
	// Initialize timer for timer TICK
	TIMER_TICK_INIT();
	TIMER_TICK_INTERRUPT_ENABLE();
	// Initialize ENCODER IO
	ENC_INIT();
	// Initialize BUTTON IO
	BTN_INIT();
	// Initialize TICK LED IO
	TICK_LED_INIT();
	// Initialize BUZZER IO
	BUZZER_INIT();
}

//------------------------------ Interrupt timer for RTOS
ISR(TIMER0_COMPA_vect)
{
	RTOS_TimerService();
}

ISR(TIMER1_COMPA_vect)
{
	// Toggle TICK led
	TICK_LED_TOGGLE();

    // Time processing
	if(timer.time[SECONDS] == 0) {
		if(timer.time[MINUTES] == 0) {
			if(timer.time[HOURS] > 0) {
				timer.time[HOURS]--;
                timer.time[MINUTES] = 59;
                timer.time[SECONDS] = 59;
			}
		} else {
            timer.time[MINUTES]--;
            timer.time[SECONDS] = 59;
		}
	} else {
        timer.time[SECONDS]--;
	}

	// Detecting last second
	uint8_t t = (timer.time[SECONDS] + timer.time[MINUTES] + timer.time[HOURS]);
	if(!t) {
		// Relay switch OFF
		RELAY_OFF();
		// Set disable led flag
		flags.led_blink = 0;
		// Set enable buzzer flag
		flags.buzzer_blink = 1;
		// Stop timer tick
		TIMER_TICK_STOP();
	}


}

//------------------------------ MAIN WORK CYCLE
int main(void)
{
	MCU_Init();
	RTOS_Init();
	hd44780_Init();

    sei();

    hd44780_Clear();
    hd44780_Puts(" Timer:");

	// Run cycle encoder scan
	RTOS_SetTask(AUTO_EncoderScan);
	// Run cycle button scan
	RTOS_SetTask(AUTO_KeyScan);
    // Run cylcle update LED and RELAY states
    RTOS_SetTask(AUTO_ToggleOutputs);
	// Run cycle display updater
    RTOS_SetTask(AUTO_DisplayUpdater);

    while (1) {
		RTOS_TaskManager();
    }
}

