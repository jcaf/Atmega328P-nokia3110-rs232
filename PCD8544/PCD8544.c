/*
 * PCD8544.c
 *
 * 48 row, 84 column outputs
 * Display data RAM 48 × 84 bits
 * Serial interface maximum 4.0 Mbits/s

 *  Created on: Jun 25, 2021
 *      Author: jcaf
 */
#include "../system.h"
#include "../SPI/SPI.h"
#include "PCD8544.h"


#define LCD_C 0//LOW
#define LCD_D 1//HIGH

#define LCD_X 84
#define LCD_Y 48
#define LCD_CMD 0

static int _x,_y;

int pixels[84][6] = {{0}};

const unsigned char GLCD_SYMBOL_GRADOS[5] PROGMEM =
{
		0x00, 0x02, 0x05, 0x02, 0x0
};
const unsigned char WETRE_INDRERIGHT[16][4] PROGMEM =
{
	//W
	{0x1f, 0x08, 0x1f, 0x00},
	//E
	{0x1f, 0x15, 0x11, 0x00},
	//T
	{0x01, 0x1f, 0x01, 0x00},
	//R
	{0x1f, 0x05, 0x1b, 0x00},
	//E
	{0x1f, 0x15, 0x11, 0x00},
	//blank
	{0x00, 0x00, 0x00, 0x00},
	//
	//I
	{0x00, 0x1f, 0x00, 0x00},
	//N+1 mas
	{0x1f, 0x02, 0x04, 0x1f},
	//D
	{0x1f, 0x11, 0x0e, 0x00},
	//R
	{0x1f, 0x05, 0x1b, 0x00},
	//E
	{0x1f, 0x15, 0x11, 0x00},
	//R
	{0x1f, 0x05, 0x1b, 0x00},
	//I
	{0x00, 0x1f, 0x00, 0x00},
	//G
	{0x1f, 0x15, 0x1d, 0x00},
	//H
	{0x1f, 0x04, 0x1f, 0x00},
	//T
	{0x01, 0x1f, 0x01, 0x00},

};

const unsigned char GLCD_SYMBOL_AUDIO[3][33] PROGMEM =
{
		{
0x00,
0xC0,
0x60,
0x30,
0x18,
0x8C,
0xC6,
0x62,
0B00110000,
0B00011000,
0B10001000,
0B11000000,
0B01100000,
0B00100000,
0B00000000,
0B10000000,
0B10000000,//MITAD
0B10000000,
0B00000000,
0B00100000,
0B01100000,
0B11000000,
0B10001000,
0B00011000,
0B00110000,
0x62,
0xC6,
0x8C,
0x18,
0x30,
0x60,
0xC0,
0x00,
},
{
0B11111111,
0B11111111,
0B00000000,
0B00000000,
0B11111110,
0B11111111,
0B00000000,
0B00000000,
0B01111100,
0B11111111,
0B00000001,
0B00000000,
0B01111100,
0B11111110,
0B10000011,
0B00000001,
0B00000001,//MITAD
0B00000001,
0B10000011,
0B11111110,
0B01111100,
0B00000000,
0B00000001,
0B11111111,
0B01111100,
0B00000000,
0B00000000,
0B11111111,
0B11111110,
0B00000000,
0B00000000,
0B11111111,
0B11111111,

},
{
0B00000001,
0B00000111,
0B00001100,
0B00011000,
0B00110000,
0B01100011,
0B11000110,
0B10001100,
0B00011000,
0B00110001,
0B00100011,
0B00000110,
0B00001100,
0B00001000,
0B00000001,
0B00000011,
0B00000011,//MITAD
0B00000011,
0B00000001,
0B00001000,
0B00001100,
0B00000110,
0B00100011,
0B00110001,
0B00011000,
0B10001100,
0B11000110,
0B01100011,
0B00110000,
0B00001100,
0B00000111,
0B00000001,
}

};


static const unsigned char ASCII[][5]  PROGMEM =
{
	{0x00, 0x00, 0x00, 0x00, 0x00} // 20
  , {0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
  , {0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
  , {0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
  , {0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
  , {0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
  , {0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
  , {0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
  , {0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
  , {0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
  , {0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
  , {0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
  , {0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
  , {0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
  , {0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
  , {0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
  , {0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
  , {0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
  , {0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
  , {0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
  , {0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
  , {0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
  , {0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
  , {0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
  , {0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
  , {0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
  , {0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
  , {0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
  , {0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
  , {0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
  , {0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
  , {0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
  , {0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
  , {0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
  , {0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
  , {0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
  , {0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
  , {0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
  , {0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
  , {0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
  , {0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
  , {0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
  , {0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
  , {0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
  , {0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
  , {0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
  , {0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
  , {0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
  , {0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
  , {0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
  , {0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
  , {0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
  , {0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
  , {0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
  , {0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
  , {0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
  , {0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
  , {0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
  , {0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
  , {0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
  , {0x02, 0x04, 0x08, 0x10, 0x20} // 5c ¥
  , {0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
  , {0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
  , {0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
  , {0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
  , {0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
  , {0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
  , {0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
  , {0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
  , {0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
  , {0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
  , {0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
  , {0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
  , {0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
  , {0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j
  , {0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
  , {0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
  , {0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
  , {0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
  , {0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
  , {0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
  , {0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
  , {0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
  , {0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
  , {0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
  , {0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
  , {0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
  , {0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
  , {0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
  , {0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
  , {0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
  , {0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
  , {0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
  , {0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
  , {0x10, 0x08, 0x08, 0x10, 0x08} // 7e ←
  , {0x00, 0x06, 0x09, 0x09, 0x06} // 7f →
  , {0x00, 0x00, 0x7D, 0x00, 0x00} // 80 ¡
};


/*
 * los primeros 16 elementos se escriben en la fila f, luego los 2 subsiguientes en f+1, f+2
 */
static const unsigned char number[13][3][16] PROGMEM =
{

{{0,128,192,224,224,96,224,224,  //'0'
192,128,0,0,0,0,0,0}
,
{112,255,255,1,0,0,0,0,
255,255,254,0,0,0,0,0}
,
{0,15,31,60,56,48,56,56,
31,15,3,0,0,0,0,0}
},
{
//{0xff,0,0,0,128,224,224,0, 		   //'1'
//0,0,0,0,0,0,0,0x01}

{0x00,0,0,0,128,224,224,0, 		   //'1'
0,0,0,0,0,0,0,0x00}
,
{0,0,3,3,3,255,255,0,
0,0,0,0,0,0,0,0}
,
{0,0,56,56,56,63,63,56,
56,56,0,0,0,0,0,0}
},
{
{0,192,192,224,96,96,224,224,   //'2'
192,128,0,0,0,0,0,0}
,
{0,1,0,0,128,192,224,249,
63,31,0,0,0,0,0,0}
,
{0,60,62,63,63,59,57,56,
56,56,56,0,0,0,0,0}
},
{
{0,192,224,224,96,96,224,224,   //'3'
192,192,0,0,0,0,0,0}
,
{0,1,0,0,48,48,56,125,
239,207,0,0,0,0,0,0}
,
{0,28,56,56,48,48,56,60,
31,15,1,0,0,0,0,0}
},
{
{0,0,0,0,0,128,192,224, 		   //'4'
224,0,0,0,0,0,0,0}
,
{224,240,248,222,207,199,193,255,
255,192,192,0,0,0,0,0}
,
{0,0,0,0,0,0,0,63,
63,0,0,0,0,0,0,0}
},
{
{0,224,224,224,224,224,224,224,	//'5'
224,224,224,0,0,0,0,0}
,
{0,63,63,63,56,56,48,112,
240,224,0,0,0,0,0,0}
,
{0,28,56,56,48,48,56,60,
31,15,1,0,0,0,0,0}
},
{
{0,0,128,192,192,224,96,96,		//'6'
224,224,0,0,0,0,0,0}
,
{224,254,255,55,57,24,24,56,
240,240,192,0,0,0,0,0}
,
{0,15,31,28,56,48,48,56,
31,15,7,0,0,0,0,0}
},
{
{0,224,224,224,224,224,224,224,		 //'7'
224,224,224,0,0,0,0,0}
,
{0,0,0,0,128,224,248,126,
31,7,1,0,0,0,0,0}
,
{0,0,56,62,31,7,1,0,
0,0,0,0,0,0,0,0}
},
{
{0,128,192,224,224,96,96,224,		 //'8'
192,192,0,0,0,0,0,0}
,
{0,207,255,127,56,48,112,112,
255,239,199,0,0,0,0,0}
,
{3,15,31,60,56,48,48,56,
31,31,15,0,0,0,0,0}
},
{
{0,128,192,224,224,96,224,224,		 //'9'
192,128,0,0,0,0,0,0}
,
{12,63,127,241,224,192,192,225,
255,255,254,0,0,0,0,0}
,
{0,0,56,48,48,56,56,30,
15,7,0,0,0,0,0,0}
},
{

{0,0,0,0,0,0,0,0,	  		  		 //'.'
0,0,0,0,0,0,0,0}
,
{0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0}
,
{60,60,60,0,0,0,0,0,
0,0,0,0,0,0,0,0}
},
{
{0,0,0,0,0,0,0,0,   					 //'+'
0,0,0,0,0,0,0,0}
,
{0,0,64,64,64,64,64,254,
254,64,64,64,64,64,0,0}
,
{0,0,0,0,0,0,0,15,
15,0,0,0,0,0,0,0}
},
{
{0,0,0,0,0,0,0,0, 	   				 //'-'
0,0,0,0,0,0,0,0}
,
{0,64,64,64,64,64,64,0,
0,0,0,0,0,0,0,0}
,
{0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0}
}};

void LcdWrite(int8_t dc, unsigned char data)
{

  if (dc==1)
	  PinTo1(PORTWxPCD8544_DC, PIN_PCD8544_DC);
  else
	  PinTo0(PORTWxPCD8544_DC, PIN_PCD8544_DC);

  __delay_us(1);

  PinTo0(PORTWxPCD8544_SCE, PIN_PCD8544_SCE);
  __delay_us(1);

  SPI_MSTR_ExchangeData(data);
  //shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);

  __delay_us(1);
  PinTo1(PORTWxPCD8544_SCE, PIN_PCD8544_SCE);
}

void LcdCharacter(char character)
{
  LcdWrite(LCD_D, 0x00);
  for (int index = 0; index < 5; index++)
  {
    LcdWrite(LCD_D, pgm_read_byte(&ASCII[character - 0x20][index]));
    //LcdWrite(LCD_D, ASCII[character - 0x20][index]);
  }
  LcdWrite(LCD_D, 0x00);
}

void LcdClear(void)
{
  for (int index = 0; index < LCD_X * LCD_Y / 8; index++)
  {
    LcdWrite(LCD_D, 0x00);
  }
}
/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeChar_megaFont
  Description  :  Displays a character in large fonts, used here for displatying temperature
		  (for displaying '.', '+', '-', and '0' to '9', stored
		  in 3310_routines.h as three dimensional array, number[][][])
  Argument(s)  :  ch   -> Character to write.
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeChar_megaFont (unsigned char ch)
{
	unsigned char i, j;
	if(ch == '.')
		{ch = 10;}
	else if (ch == '+')
		{ch = 11;}
	else if (ch == '-')
		{ch = 12;}
	else
		{ch = ch & 0x0f;}

	for(i=0; i<3; i++)
	{
		LcdWrite( 0, 0x80 + _x); // Column.
		LcdWrite( 0, 0x40 + _y+i); // Row.
		for(j=0; j<16; j++)
		{
			//lcd_buffer[cursor_row][cursor_col + j] |=  pgm_read_byte(&(number[ch][i][j]));
			//LCD_writeData(lcd_buffer[cursor_row][cursor_col + j]);
			LcdWrite(LCD_D, pgm_read_byte(&(number[ch][i][j])) );
		}
	}

	if(ch == 10)
	{
		_x += 5;
	}
	else
	{
		_x += 12;
	}
}


void glc_write_symbol_audio(void)
{
	for (int8_t row=0; row<3; row++)
	{
		LcdWrite( 0, 0x80 + _x);
		LcdWrite( 0, 0x40 + _y+row);

		for (int8_t col=0; col<33; col++)
		{
			LcdWrite(LCD_D, pgm_read_byte(&GLCD_SYMBOL_AUDIO[row][col]) );
		}
	}
}
void glc_write_WETRE_INDRERIGHT(void)
{
	for (int8_t f=0; f<16; f++)
	{
		for (int8_t col=0; col<4; col++)
		{
			LcdWrite(LCD_D, pgm_read_byte(&WETRE_INDRERIGHT[f][col]) );
		}
		LcdWrite(LCD_D, 0x00 );
	}
}

void glc_write_symbol_grados(void)
{
	for (int8_t col=0; col<5; col++)
	{
		LcdWrite(LCD_D, pgm_read_byte(&GLCD_SYMBOL_GRADOS[col]) );
	}
}

void bigfont2after_blanks(void)
{
	for(int i=0; i<3; i++)
	{
		LcdWrite( 0, 0x80 + _x); // Column.
		LcdWrite( 0, 0x40 + _y+i); // Row.

		for(int j=0; j<16; j++)
		{
			LcdWrite(LCD_D, 0xff);
		}
	}
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LCD_writeString_megaFont
  Description  :  Displays a string at current location, in large fonts
  Argument(s)  :  string -> Pointer to ASCII string (stored in RAM)
  Return value :  None.
--------------------------------------------------------------------------------------------------*/
void LCD_writeString_megaFont ( char *string )
{

    while ( *string )
    {
        LCD_writeChar_megaFont( *string++ );
    }

    bigfont2after_blanks();

}

void LcdInitialise(void)
{
	//+- Proteus compatibility
	PinTo1(PORTWxPCD8544_DC, PIN_PCD8544_DC);
	PinTo1(PORTWxPCD8544_SCE, PIN_PCD8544_SCE);
	PinTo1(PORTWxPCD8544_RES, PIN_PCD8544_RES);
	PinTo1(PORTWxPCD8544_SDIN, PIN_PCD8544_SDIN);
	PinTo1(PORTWxPCD8544_SCK, PIN_PCD8544_SCK);
	//-+
	PinTo0(PORTWxPCD8544_BKLIGHT, PIN_PCD8544_BKLIGHT);//active NPN to ground
	ConfigOutputPin(CONFIGIOxPCD8544_BKLIGHT, PIN_PCD8544_BKLIGHT);

	__delay_ms(1);
	ConfigOutputPin(CONFIGIOxPCD8544_DC, PIN_PCD8544_DC);
	ConfigOutputPin(CONFIGIOxPCD8544_SCE, PIN_PCD8544_SCE);
	ConfigOutputPin(CONFIGIOxPCD8544_RES, PIN_PCD8544_RES);
	ConfigOutputPin(CONFIGIOxPCD8544_SDIN, PIN_PCD8544_SDIN);
	ConfigOutputPin(CONFIGIOxPCD8544_SCK, PIN_PCD8544_SCK);


	InitSPI_MASTER();//NECESITA QUE TODOS LOS PINS ESTEN EN ALTO PARA ASEGURAR UNA CORRECTA ASERCION DEL NIVEL BAJO
	//CPOL = CPHA = 0 SAMPLE IN RISING!!!! CLAVE!
	//SPCR = (1<<SPE) | (0<< DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (SPR1SPR0_SCK_FOSC_4);
	__delay_ms(50);

	PinTo0(PORTWxPCD8544_SCE, PIN_PCD8544_SCE);
	__delay_ms(1);
		PinTo0(PORTWxPCD8544_RES, PIN_PCD8544_RES);
		__delay_ms(1);
		PinTo1(PORTWxPCD8544_RES, PIN_PCD8544_RES);
	__delay_ms(1);
	PinTo1(PORTWxPCD8544_SCE, PIN_PCD8544_SCE);

	LcdWrite(LCD_CMD, 0x21); 	//LCD Extended Commands for Vop, TEMP Coef, bias mode
	LcdWrite(LCD_CMD, 0xC0); 	//Set LCD Vop (Contrast). //B1
	LcdWrite(LCD_CMD, 0x04); 	//Set Temp coefficent. //0x04
	LcdWrite(LCD_CMD, 0x14); 	//LCD bias mode 1:48.
	LcdWrite(LCD_CMD, 0x20);	//Return to standar for address X, Y
	LcdWrite(LCD_CMD, 0x0C);	//LCD in 0x0C normal mode. 0x0d for inverse
}

void LcdString(char *characters)
{
  while (*characters)
  {
    LcdCharacter(*characters++);
  }
}


// gotoXY routine to position cursor
// x - range: 0 to 84
// y - range: 0 to 5

void gotoXY(int x, int y)
{
//  LcdWrite( 0, 0x80 | x); // Column.
//  LcdWrite( 0, 0x40 | y); // Row.

  LcdWrite( 0, 0x80 + x); // Column.
  LcdWrite( 0, 0x40 + y); // Row.

  _x = x;
  _y = y;
}

// ***************
// Enable or disable a specific pixel
// x: 0 to 84, y: 0 to 48
void setPixel(int x, int y, int d)
{
  if (x > 84 || y > 48)
  {
    return;
  }

  // The LCD has 6 rows, with 8 pixels per row.
  // 'y_mod' is the row that the pixel is in.
  // 'y_pix' is the pixel in that row we want to enable/disable

  int y_mod = (int)(y >> 3); 		// >>3 divides by 8
  int y_pix = (y - (y_mod << 3)); 	// << 3 multiplies by 8
  int val = 1 << y_pix;

  /// We have to keep track of which pixels are on/off in order to
  // write the correct character out to the LCD.
  if (d)
  {
    pixels[x][y_mod] |= val;
  }
  else
  {
    pixels[x][y_mod] &= ~val;
  }

  // Write the updated pixel out to the LCD
  // TODO Check if the pixel is already in the state requested,
  // if so, don't write to LCD.
  gotoXY(x, y_mod);
  LcdWrite (1, pixels[x][y_mod]);
}

void drawLine(int x1, int y1, int x2, int y2)
{
}


// Draw a horizontal line between x1 and x2 at row y
void drawHorizontalLineXY(int x1, int x2, int y)
{
  for (int i = x1; i <= x2; i++)
  {
    setPixel(i, y, 1);
  }
}
// Draw a horizontal line of width w from x,y
void drawHorizontalLine(int x, int y, int w)
{
  drawHorizontalLineXY(x, x + w, y);
}

// Draw a vertical line from y1 to y2 on column x
void drawVerticalLineXY(int y1, int y2, int x)
{
  for (int i = y1; i <= y2; i++)
  {
    setPixel(x, i, 1);
  }
}
// Draw a vertical line of height h from x,y
void drawVerticalLine(int x, int y, int h)
{
  drawVerticalLineXY(y, y + h, x);
}

//#define LCD_X 84
//#define LCD_Y 48
void clearPixels(void)
{
  for (int x = 0; x < 83; x++)
  {
    for (int y = 0; y < 6; y++)
    {
      pixels[x][y] = 0;
    }
  }
}
// Draw a rectangle of width w and height h from x,y
void drawRect(int x, int y, int w, int h)
{
  drawHorizontalLineXY(x, x + w, y);
  drawHorizontalLineXY(x, x + w, y + h);
  drawVerticalLineXY(y, y + h, x);
  drawVerticalLineXY(y, y + h, x + w);
}

