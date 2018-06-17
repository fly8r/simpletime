/*
 * config.h
 *
 * Created: 11.12.2017 13:32:51
 *  Author: v.bandura
 */
#include "drvHD44780.h"

#define F_CPU							8000000UL	// Core CPU Frequency

//------------------------------ Macro definitions
#define SECONDS                         2
#define MINUTES                         1
#define HOURS                           0
#define BUZZER_BEEP_COUNT				5

//------------------------------ System timer configuration for RTOS
#define SYSTICK_TIME_MS					1
#define SYSTICK_PRESCALER				64L
#define SYSTICK_OCR_CONST				((F_CPU*SYSTICK_TIME_MS) / (SYSTICK_PRESCALER*1000))
#define SYSTICK_TIMER_COUNTER           TCNT0
#define SYSTICK_TIMER_OCR               OCR0A
#define SYSTICK_TIMER_INIT()            { TCCR0A=1<<WGM01; TCCR0B=1<<CS01|1<<CS00; OCR0A=SYSTICK_OCR_CONST; /*SYSTICK_TIMER_COUNTER=0;*/ }
#define SYSTICK_INTERRUPT_ENABLE()      { TIMSK |= 1<<OCIE0A; }
#define SYSTICK_INTERRUPT_DISABLE()     { TIMSK &= ~(1<<OCIE0A); }

//------------------------------ RTOS configuration
#define RTOS_TASK_QUEUE_SIZE            5
#define RTOS_TIMER_TASK_QUEUE_SIZE      5

//------------------------------ Timer configuration
#define TIMER_TICK_TIME_MS				1000UL
#define TIMER_TICK_PRESCALER			1024UL
#define TIMER_TICK_OCR_CONST			((F_CPU / TIMER_TICK_PRESCALER) * (TIMER_TICK_TIME_MS / 1000UL)) //7812  //((TIMER_TIME_MS * 1000UL * TIMER_PRESCALER) / F_CPU)
#define	TIMER_TICK_COUNTER_REG			TCNT1
#define TIMER_TICK_OCR_REG				OCR1A
#define TIMER_TICK_INIT()				{ TCCR1A=0; TCCR1B=0; TIMER_TICK_OCR_REG=TIMER_TICK_OCR_CONST; }
#define TIMER_TICK_START()				{ TIMER_TICK_COUNTER_REG=0; TCCR1B=1<<WGM12|1<<CS12|1<<CS10; }
#define TIMER_TICK_STOP()				{ TCCR1B &= ~(1<<WGM12|1<<CS12|1<<CS10); }
#define TIMER_TICK_TOGGLE()				{ TCCR1B ^= (1<<WGM12|1<<CS12|1<<CS10); }
#define TIMER_TICK_CHECK				( TCCR1B & (1<<WGM12|1<<CS12|1<<CS10) )
#define TIMER_TICK_INTERRUPT_ENABLE()	{ TIMER_TICK_COUNTER_REG=0; TIMSK |= 1<<OCIE1A; }
#define TIMER_TICK_INTERRUPT_DISABLE()	{ TIMSK &= ~(1<<OCIE1A); }
#define TIMER_TICK_INTERRUPT_TOGGLE()   { TIMSK ^= (1<<OCIE1A); }
#define TIMER_TICK_CHECK_INTERRUPT		(TIMSK & (1<<OCIE1A))


//------------------------------ Display configuration
#define HD44780_4bit_MODE				1					// 0 - 8bit mode, 1 - 4bit mode
#define HD44780_IO_DATA_SHIFT			4					// Shift to the left by port pins in 4bit mode
#define HD44780_BL_CTRL					0					// Use for control back light
#define HD44780_WAIT_BUSY_FLAG			0					// Check LCD busy flag
// Parallel ports settings
#define HD44780_IO_DATA_DDR				DDRB
#define HD44780_IO_DATA_PIN				PINB
#define HD44780_IO_DATA_PORT			PORTB
// Signal E settings
#define HD44780_IO_PIN_E_DDR			DDRB
#define HD44780_IO_PIN_E_PORT			PORTB
#define HD44780_IO_PIN_E_MASK			(1<<3)				// 3bit of port
// Signal RW settings
#define HD44780_IO_PIN_RW_DDR			DDRB
#define HD44780_IO_PIN_RW_PORT			PORTB
#define HD44780_IO_PIN_RW_MASK			(1<<2)				// 2bit of port
// Signal RS settings
#define HD44780_IO_PIN_RS_DDR			DDRB
#define HD44780_IO_PIN_RS_PORT			PORTB
#define HD44780_IO_PIN_RS_MASK			(1<<1)				// 1bit of port
#if (HD44780_BL_CTRL)
// Signal back light control
	#define HD44780_IO_PIN_BL_DDR		DDRB
	#define HD44780_IO_PIN_BL_PORT		PORTB
	#define HD44780_IO_PIN_BL_MASK		(1<<0)				// 0bit of port
#endif



//------------------------------ Encoder configuration
#define ENC_DDR							DDRD
#define ENC_PIN							PIND
#define ENC_MASK						(1<<1|1<<0)
#define ENC_INIT()						{ ENC_DDR &= ~(ENC_MASK); }
#define ENC_STATE						(ENC_PIN & ENC_MASK)

//------------------------------ Encoder button configuration
#define BTN_DDR							DDRD
#define BTN_PIN							PIND
#define BTN_MASK						(1<<2)
#define BTN_INIT()						{ BTN_DDR &= ~(BTN_MASK); }
#define BTN_PRESSED						(!(BTN_PIN & BTN_MASK))


//------------------------------ IO relay configuration
#define RELAY_DDR						DDRD
#define RELAY_PORT						PORTD
#define RELAY_MASK						(1<<4)
#define RELAY_ON()						{ RELAY_PORT |= RELAY_MASK; }
#define RELAY_OFF()						{ RELAY_PORT &= ~RELAY_MASK; }
#define RELAY_TOGGLE()					{ RELAY_PORT ^= RELAY_MASK; }
#define RELAY_INIT()					{ RELAY_DDR |= RELAY_MASK; RELAY_OFF(); }

//------------------------------ IO system LED configuration
#define TICK_LED_DDR					DDRD
#define TICK_LED_PORT					PORTD
#define TICK_LED_MASK					(1<<3)
#define TICK_LED_TOGGLE()				{ TICK_LED_PORT ^= TICK_LED_MASK; }
#define TICK_LED_ON()					{ TICK_LED_PORT |= TICK_LED_MASK; }
#define TICK_LED_OFF()					{ TICK_LED_PORT &= ~TICK_LED_MASK; }
#define TICK_LED_INIT()					{ TICK_LED_DDR |= TICK_LED_MASK; TICK_LED_OFF(); }


//------------------------------ IO buzzer configuration
#define BUZZER_DDR						DDRD
#define BUZZER_PORT						PORTD
#define BUZZER_MASK						(1<<5)
#define BUZZER_TOGGLE()					{ BUZZER_PORT ^= BUZZER_MASK; }
#define BUZZER_ON()						{ BUZZER_PORT |= BUZZER_MASK; }
#define BUZZER_OFF()					{ BUZZER_PORT &= ~BUZZER_MASK; }
#define BUZZER_INIT()					{ BUZZER_DDR |= BUZZER_MASK; }