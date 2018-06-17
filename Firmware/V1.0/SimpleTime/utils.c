/*
 * utils.c
 *
 * Created: 14.12.2017 9:38:26
 *  Author: v.bandura
 */
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

/************************************************************************/
/* VARS                                                                 */
/************************************************************************/
//-> Hexidecimal table convertor
const       char        Hexidecimal[]   PROGMEM = { "0123456789ABCDEF" };
//-> Utoa convert table
const		uint8_t		pow3Table8[]	PROGMEM	= { 100ul, 10ul, 1ul };


/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
char * hex_to_ascii(uint8_t number, char * buffer)
{
	buffer[0] = pgm_read_byte(Hexidecimal + (number >> 4));
	buffer[1] = pgm_read_byte(Hexidecimal + (number & 0x0F));
	buffer[2] = '\0';
	return buffer;
}

char * utoa_cycle_sub(uint8_t value, char *buffer)
{
	if(value == 0)
	{
		buffer[0] = '0';
		buffer[1] = '0';
		buffer[2] = 0;
		return buffer;
	}
	char *ptr = buffer;
	uint8_t j = (value > 9) ? 0 : 1;
	uint8_t i = 0;
	do
	{
		uint8_t pow3 = pgm_read_dword(pow3Table8 + (i++));
		uint8_t count = 0;
		while(value >= pow3)
		{
			count ++;
			value -= pow3;
		}
		*ptr++ = count + '0';
	} while(i < 3);
	*ptr = 0;
	
	while(buffer[j] == '0') ++buffer;
	return buffer;
}
