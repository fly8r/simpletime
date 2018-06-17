/*
 * drvHD44780.c
 *
 * Created: 11.12.2017 13:31:19
 *  Author: v.bandura
 */
#include "config.h"

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdarg.h>

#include "drvHD44780.h"

/************************************************************************/
/* MACROS                                                               */
/************************************************************************/
#define HD44780_RS_HIGH()				{HD44780_IO_PIN_RS_PORT |= HD44780_IO_PIN_RS_MASK;}
#define HD44780_RS_LOW()				{HD44780_IO_PIN_RS_PORT &= ~(HD44780_IO_PIN_RS_MASK);}
#define HD44780_RW_HIGH()				{HD44780_IO_PIN_RW_PORT |= HD44780_IO_PIN_RW_MASK;}
#define HD44780_RW_LOW()				{HD44780_IO_PIN_RW_PORT &= ~(HD44780_IO_PIN_RW_MASK);}
#define HD44780_E_HIGH()				{HD44780_IO_PIN_E_PORT |= HD44780_IO_PIN_E_MASK;}
#define HD44780_E_LOW()					{HD44780_IO_PIN_E_PORT &= ~(HD44780_IO_PIN_E_MASK);}
#if (HD44780_BL_CTRL)
	#define HD44780_BL_HIGH()			HD44780_IO_PIN_BL_PORT |= HD44780_IO_PIN_BL_MASK
	#define HD44780_BL_LOW()			HD44780_IO_PIN_BL_PORT &= ~HD44780_IO_PIN_BL_MASK
#endif
#define HD44780_SetDATA_4bit(val)		{HD44780_IO_DATA_PORT &= ~(0xF << HD44780_IO_DATA_SHIFT); HD44780_IO_DATA_PORT |= ((val) << HD44780_IO_DATA_SHIFT);}
#define HD44780_SetDATA_8bit(val)		{HD44780_IO_DATA_PORT = val;}
#define HD44780_GetDATA_4bit()			((HD44780_IO_DATA_PIN >> HD44780_IO_DATA_SHIFT) & 0x0F)
#define HD44780_GetDATA_8bit()			HD44780_IO_DATA_PIN
#define HD44780_SetDATA_PinMode_IN(mask)	{ HD44780_IO_DATA_DDR &= ~(mask); HD44780_IO_DATA_PORT |= mask; }
#define HD44780_SetDATA_PinMode_OUT(mask)	{ HD44780_IO_DATA_DDR |= mask; }

/************************************************************************/
/* VARS                                                                 */
/************************************************************************/
volatile uint8_t LastRow=0;
char hd44780_Buffer[HD44780_COLS * HD44780_ROWS+8];


/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
//------------------------------ Display write byte function
void hd44780_SendByte(uint8_t data, char dt)
{
	// Set data type for display
	if(dt == HD44780_COMMAND) {
		HD44780_RS_LOW();
	} else {
		HD44780_RS_HIGH();
	}

#if (HD44780_4bit_MODE)
	// Set data pin to output mode
	HD44780_SetDATA_PinMode_OUT(0x0F << HD44780_IO_DATA_SHIFT);
	// Send High half of byte
	HD44780_SetDATA_4bit(data >> 4);
	HD44780_E_HIGH();
	_delay_us(HD44780_TIME_SHORT_DELAY);
	HD44780_E_LOW();
	// Timeout before half of byte send
	_delay_us(HD44780_TIME_SHORT_DELAY);
	// Send Low half of byte
	HD44780_SetDATA_4bit(data & 0x0F);
	HD44780_E_HIGH();
	_delay_us(HD44780_TIME_SHORT_DELAY);
	HD44780_E_LOW();
	// Set data pin to input mode
	HD44780_SetDATA_PinMode_IN(0x0F << HD44780_IO_DATA_SHIFT);
#else
	// Set data pin to output mode
	HD44780_SetDATA_PinMode_OUT(0xFF);
	// Send byte
	HD44780_SetDATA_8bit(data);
	HD44780_E_HIGH();
	_delay_us(HD44780_TIME_SHORT_DELAY);
	HD44780_E_LOW();
	// Set data pin to input mode
	HD44780_SetDATA_PinMode_IN(0xFF);
#endif
}

//------------------------------ Display read byte function
uint8_t hd44780_ReadByte(char dt)
{
	volatile uint8_t data=0;

	// Set data type for display
	if(dt == HD44780_COMMAND) {
		HD44780_RS_LOW();
	} else {
		HD44780_RS_HIGH();
	}
	// Set RW line to READ mode
	HD44780_RW_HIGH();

#if (HD44780_4bit_MODE)
	// Read high half of byte
	HD44780_E_HIGH();
	_delay_us(HD44780_TIME_SHORT_DELAY);
    //
	data = HD44780_GetDATA_4bit() << 4;
	HD44780_E_LOW();
	// Timeout before half of byte read
	_delay_us(HD44780_TIME_SHORT_DELAY);
	// Read low half of byte
	HD44780_E_HIGH();
	_delay_us(HD44780_TIME_SHORT_DELAY);
    //
	data |= (HD44780_GetDATA_4bit() & 0x0F);
	HD44780_E_LOW();
#else
	// Read byte
	HD44780_E_HIGH();
	_delay_us(HD44780_TIME_SHORT_DELAY);
	data = HD44780_GetDATA_8bit();
	HD44780_E_LOW();
#endif
	// Set RW line to WRITE mode
	HD44780_RW_LOW();
	// Return data
	return data;
}

//------------------------------ Check display busy flag function
#if (HD44780_WAIT_BUSY_FLAG)
uint8_t hd44780_IsBusy(void)
{
	if(hd44780_ReadByte(HD44780_COMMAND) & (1<<7))
		return 0xFF;
	else
		return 0x00;
}
#endif

//------------------------------ Display send command
void hd44780_SendCmd(uint8_t data)
{
#if (HD44780_WAIT_BUSY_FLAG)
	while(hd44780_IsBusy()){};
#endif
	hd44780_SendByte(data,HD44780_COMMAND);
#if (!HD44780_WAIT_BUSY_FLAG)
	_delay_us(HD44780_TIME_BUSY_DELAY);
#endif
}

//------------------------------ Display send data
void hd44780_SendData(uint8_t data)
{
#if (HD44780_WAIT_BUSY_FLAG)
	while(hd44780_IsBusy()){};
#endif
	hd44780_SendByte(data, HD44780_DATA);
#if (!HD44780_WAIT_BUSY_FLAG)
	_delay_us(HD44780_TIME_BUSY_DELAY);
#endif
}

//------------------------------ Clear display function
void hd44780_Clear(void)
{
	hd44780_SendCmd(HD44780_CMD_CLEAR_DISPLAY);
	LastRow=0;

#if (!HD44780_WAIT_BUSY_FLAG)
	_delay_ms(2);
#endif
}

//------------------------------ Display initialization function
inline void hd44780_Init(void)
{
	// Initialize MCU IO pins
	HD44780_IO_PIN_E_DDR |= HD44780_IO_PIN_E_MASK;
	HD44780_IO_PIN_RS_DDR |= HD44780_IO_PIN_RS_MASK;
    HD44780_IO_PIN_RW_DDR |= HD44780_IO_PIN_RW_MASK;

#if (HD44780_BL_CTRL)
	// If back light control used
	HD44780_IO_PIN_BL_DDR |= HD44780_IO_PIN_BL_MASK;
#endif

	// Wait ~20 ms before trying to initialize display
	_delay_ms(20);
	// Set display mode
#if (HD44780_4bit_MODE)
	hd44780_SendByte(HD44780_OPT_4BIT_MODE, HD44780_COMMAND);
	_delay_ms(5);
	hd44780_SendByte(HD44780_OPT_4BIT_MODE, HD44780_COMMAND);
	_delay_us(100);
	#if (HD44780_ROWS > 1)
		hd44780_SendCmd(HD44780_OPT_4BIT_MODE | HD44780_OPT_TWO_LINE_MODE | HD44780_OPT_CHAR_SIZE_5X8);
	#else
		hd44780_SendCmd(HD44780_OPT_4BIT_MODE | HD44780_OPT_ONE_LINE_MODE | HD44780_OPT_CHAR_SIZE_5X8);
	#endif
#else
	hd44780_SendByte(HD44780_OPT_8BIT_MODE, HD44780_COMMAND);
	_delay_ms(5);
	hd44780_SendByte(HD44780_OPT_8BIT_MODE, HD44780_COMMAND);
	_delay_us(100);
	#if (HD44780_ROWS > 1)
		hd44780_SendCmd(HD44780_OPT_8BIT_MODE | HD44780_OPT_TWO_LINE_MODE | HD44780_OPT_CHAR_SIZE_5X8);
	#else
		hd44780_SendCmd(HD44780_OPT_8BIT_MODE | HD44780_OPT_ONE_LINE_MODE | HD44780_OPT_CHAR_SIZE_5X8);
	#endif
#endif

	// Technology timeout
	_delay_ms(10);
	// Enable display and disable cursor
	hd44780_SendCmd(HD44780_OPT_DISPLAY_ENABLE | HD44780_OPT_CURSOR_INVISIBLE);
	// Technology timeout
	_delay_ms(10);
	// Set address counter direction
	hd44780_SendCmd(HD44780_OPT_ADDRESS_INCREMENT | HD44780_OPT_LINE_SHIFT_DISABLE);
}

//------------------------------ Set cursor to position of X,Y
void hd44780_GoToXY(uint8_t Row, uint8_t Col)
{
	uint8_t address=0;
	if(Row & 0x01) {				// Odd row address set
		address = 0x40;				// Odd row is similarly has address 0x40
	}
	if(Row > 1) {					// For 4 rows display and row > 1
		address += HD44780_COLS;	// add address shift
	}
	// Calculate address with Col shift
	address += Col;
	// Set address to display DRAM
	hd44780_SendCmd(HD44780_CMD_DDRAM_ADDR | address);
	// Store row value
	LastRow = Row;
}

//------------------------------ Send to display buffer string
void hd44780_WriteBuff(char * pBuff, uint8_t len)
{
	while(len--)
	{
		hd44780_SendData(*(pBuff++));
	}
}

//------------------------------ Send string to current cursor position
void hd44780_Puts(char * str)
{
	while(*str != '\0')
	{
		switch(*str) {
			// Goto new line
			case '\n':
				LastRow++;
				hd44780_GoToXY(LastRow, 0);
				break;

			// Flush cursor position in row
			case '\r':
				hd44780_GoToXY(LastRow, 0);
				break;

			// Tabulation replace with 4 space char
			/*case '\t':
				for(uint8_t i=0; i<4; i++)
				{
					hd44780_SendData(0x20);
				}
				break;*/

			default:
				hd44780_SendData(*str);
				break;
		}
		str++;
	}
}

//------------------------------ Send string from flash to current cursor position
void hd44780_PutsF(const char * str)
{
	while(pgm_read_byte(str) != '\0')
	{
		char b = pgm_read_byte(str);
		switch(b) {
			// Goto new line
			case '\n':
				LastRow++;
				hd44780_GoToXY(LastRow, 0);
				break;

			// Flush cursor position in row
			case '\r':
				hd44780_GoToXY(LastRow, 0);
				break;

			// Tabulation replace with 4 space char
			/*case '\t':
				for(uint8_t i=0; i<4; i++)
				{
					hd44780_SendData(0x20);
				}
				break;*/

			default:
				hd44780_SendData(b);
				break;
		}
		str++;
	}

}

//------------------------------ The function is create new char from pattern
void hd44780_CreateCharacter(char code, char * pattern)
{
	// Set CGRAM address pointer shift
	hd44780_SendCmd(HD44780_CMD_CGRAM_ADDR | (code << 3));
	// Load pattern in CGRAM 8 bytes line by line
	for(uint8_t i=0; i<8; i++) {
		hd44780_SendData(*pattern++);
	}
	// Return to DDRAM in position 0,0
	hd44780_SendCmd(HD44780_CMD_DDRAM_ADDR);
}

//------------------------------ The function is create new char from pattern from flash
void hd44780_CreateCharacterF(char code, const char * pattern)
{
	// Set CGRAM address pointer shift
	hd44780_SendCmd(HD44780_CMD_CGRAM_ADDR | (code << 3));
	// Load pattern in CGRAM 8 bytes line by line
	for(uint8_t i=0; i<8; i++) {
		hd44780_SendData(pgm_read_byte(pattern++));
	}
	// Return to DDRAM in position 0,0
	hd44780_SendCmd(HD44780_CMD_DDRAM_ADDR);
}


//------------------------------ Formatted print from current position
void hd44780_Printf(const char * args, ...)
{
	va_list pArg;
	va_start(pArg, args);
	vsnprintf(hd44780_Buffer, sizeof(hd44780_Buffer), args, pArg);
	va_end(pArg);

	hd44780_Puts(hd44780_Buffer);
}
