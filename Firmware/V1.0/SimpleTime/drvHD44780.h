/*
 * drvHD44780.h
 *
 * Created: 11.12.2017 13:31:33
 *  Author: v.bandura
 */
#ifndef DRVHD44780_H
#define DRVHD44780_H

#include <stdio.h>
#include <avr/io.h>

//------------------------------ Display size
#define HD44780_ROWS							2			// Rows count
#define HD44780_COLS							16			// Cols count

//------------------------------ Display timings configuration
#define HD44780_TIME_SHORT_DELAY				1			// Strobe E length in uS
#define	HD44780_TIME_BUSY_DELAY					50			// Wait busy flag timeout

//------------------------------
#define HD44780_COMMAND							0
#define HD44780_DATA							1

//------------------------------ Display configuration flags
//>
#define	HD44780_CMD_CLEAR_DISPLAY				0x01
#define HD44780_CMD_FLUSH_DDRAM_ADDR			0x02
//>
#define HD44780_CMD_DISPLAY_SHIFT_CURSOR		0x04
#define HD44780_OPT_ADDRESS_DECREMENT			(HD44780_CMD_DISPLAY_SHIFT_CURSOR & ~(1<<1))		// 0 - decrement address
#define HD44780_OPT_ADDRESS_INCREMENT			(HD44780_CMD_DISPLAY_SHIFT_CURSOR | (1<<1))			// 1 - increment address
#define HD44780_OPT_LINE_SHIFT_DISABLE			(HD44780_CMD_DISPLAY_SHIFT_CURSOR & ~(1<<0))		// 0 - no display shift
#define HD44780_OPT_LINE_SHIFT_ENABLE			(HD44780_CMD_DISPLAY_SHIFT_CURSOR |	(1<<0))			// 1 - shift display to the COUNTER_DIRECTION
//>
#define HD44780_CMD_DISPLAY_MODE				0x08
#define HD44780_OPT_DISPLAY_DISABLE				(HD44780_CMD_DISPLAY_MODE & ~(1<<2))				// 0 - hidden all data
#define HD44780_OPT_DISPLAY_ENABLE				(HD44780_CMD_DISPLAY_MODE | (1<<2))					// 1 - show data
#define HD44780_OPT_CURSOR_INVISIBLE			(HD44780_CMD_DISPLAY_MODE & ~(1<<1))				// 0 - invisible
#define HD44780_OPT_CURSOR_VISIBLE				(HD44780_CMD_DISPLAY_MODE | (1<<1))					// 1 - visible
#define HD44780_OPT_CURSOR_IS_UNDERLINE			(HD44780_CMD_DISPLAY_MODE & ~(1<<0))				// 0 - cursor is underline
#define HD44780_OPT_CURSOR_IS_SQUARE			(HD44780_CMD_DISPLAY_MODE | (1<<0))					// 1 - cursor is black square
//>
#define HD44780_CMD_SHIFT_MODE					0x10
#define HD44780_OPT_SHIFT_CURSOR				(HD44780_CMD_SHIFT_MODE & ~(1<<3))					// 0 - shift cursor
#define HD44780_OPT_SHIFT_DISPLAY				(HD44780_CMD_SHIFT_MODE | (1<<3))					// 1 - shift display
#define HD44780_OPT_SHIFT_LEFT					(HD44780_CMD_SHIFT_MODE & ~(1<<2))					// 0 - to the left
#define HD44780_OPT_SHIFT_RIGHT					(HD44780_CMD_SHIFT_MODE | (1<<2))					// 1 - to the right
//>
#define HD44780_CMD_DATA_MODE					0x20
#define HD44780_OPT_4BIT_MODE					(HD44780_CMD_DATA_MODE & ~(1<<4))					// 0 - 4-bit mode
#define HD44780_OPT_8BIT_MODE					(HD44780_CMD_DATA_MODE | (1<<4))					// 1 - 8-bit mode
#define HD44780_OPT_ONE_LINE_MODE				(HD44780_CMD_DATA_MODE & ~(1<<3))					// 0 - one string
#define HD44780_OPT_TWO_LINE_MODE				(HD44780_CMD_DATA_MODE | (1<<3))					// 1 - two string
#define HD44780_OPT_CHAR_SIZE_5X8				(HD44780_CMD_DATA_MODE & ~(1<<2))					// 0 - 5x8 points
#define HD44780_OPT_CHAR_SIZE_5x10				(HD44780_CMD_DATA_MODE | (1<<2))					// 1 - 5x10 points
//>
#define HD44780_CMD_CGRAM_ADDR					0x40
#define HD44780_CMD_DDRAM_ADDR					0x80




/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
extern	void hd44780_Init(void);										// Display initialization function
extern	void hd44780_SendCmd(uint8_t data);								// Display send command
extern	void hd44780_SendData(uint8_t data);							// Display send data
extern	void hd44780_Clear(void);										// Clear display function
extern	void hd44780_GoToXY(uint8_t Row, uint8_t Col);					// Set cursor to position of X,Y
extern	void hd44780_WriteBuff(char * pBuff, uint8_t len);				// Send to display buffer string
extern	void hd44780_Puts(char *str);									// Send string to current cursor position
extern  void hd44780_PutsF(const char * str);							// Send string from flash to current cursor position
extern  void hd44780_CreateCharacter(char code, char * pattern);		// The function is create new char from pattern
extern	void hd44780_CreateCharacterF(char code, const char * pattern);	// The function is create new char from pattern from flash
extern	void hd44780_Printf(const char * args, ...);					// Formatted print from current position

#endif
