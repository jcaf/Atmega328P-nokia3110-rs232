/*
 * PCD8544.h
 *
 *  Created on: Jun 25, 2021
 *      Author: jcaf
 */

#ifndef PCD8544_PCD8544_H_
#define PCD8544_PCD8544_H_


	#define PORTWxPCD8544_DC 	PORTB
	#define PORTRxPCD8544_DC  	PINB
	#define CONFIGIOxPCD8544_DC DDRB
	#define PIN_PCD8544_DC    	2

	#define PORTWxPCD8544_SCE 	PORTB
	#define PORTRxPCD8544_SCE  	PINB
	#define CONFIGIOxPCD8544_SCE DDRB
	#define PIN_PCD8544_SCE    	1

	#define PORTWxPCD8544_RES 	PORTB
	#define PORTRxPCD8544_RES  	PINB
	#define CONFIGIOxPCD8544_RES DDRB
	#define PIN_PCD8544_RES    	0

	#define PORTWxPCD8544_SDIN 	PORTB
	#define PORTRxPCD8544_SDIN  	PINB
	#define CONFIGIOxPCD8544_SDIN DDRB
	#define PIN_PCD8544_SDIN    	3

	#define PORTWxPCD8544_SCK 	PORTB
	#define PORTRxPCD8544_SCK 	PINB
	#define CONFIGIOxPCD8544_SCK DDRB
	#define PIN_PCD8544_SCK    	5

	#define PORTWxPCD8544_BKLIGHT 	PORTC
	#define PORTRxPCD8544_BKLIGHT  	PINC
	#define CONFIGIOxPCD8544_BKLIGHT DDRC
	#define PIN_PCD8544_BKLIGHT    	1

	//
	void LcdInitialise(void);
	void LcdClear(void);
	void clearPixels(void);
	void gotoXY(int x, int y);
	void LcdString(char *characters);
	//
	void LCD_writeChar_megaFont (unsigned char ch);
	void LCD_writeString_megaFont ( char *string );


	void glc_write_symbol_audio(void);
	void glc_write_symbol_grados(void);
#endif /* PCD8544_PCD8544_H_ */
