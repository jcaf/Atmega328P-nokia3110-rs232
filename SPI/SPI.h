#ifndef _SPI_H_
#define _SPI_H_

    /*************************************************************************************
    Seria ideal seguir haciendo esta asignaci�n pin a pin, lo malo es que el compilador
    lo traduce a aprox 5 instrucciones de carga indirecta... :P
    *************************************************************************************/

    //#define MOSI_PIN_ ()
    //#define MISO_
    //#define SCK ()
    //#define         ()

	#define PORTWxSPI_MOSI 		PORTB
	#define PORTRxSPI_MOSI 		PINB
	#define CONFIGIOxSPI_MOSI 	DDRB
	#define PINxKB_SPI_MOSI		5

	#define PORTWxSPI_MISO 		PORTB
	#define PORTRxSPI_MISO 		PINB
	#define CONFIGIOxSPI_MISO 	DDRB
	#define PINxKB_SPI_MISO		6

	#define PORTWxSPI_SCK 		PORTB
	#define PORTRxSPI_SCK 		PINB
	#define CONFIGIOxSPI_SCK 	DDRB
	#define PINxKB_SPI_SCK		7

	//Application specific
	#define PORTWxSPI_CS 	PORTB
	#define PORTRxSPI_CS 	PINB
	#define CONFIGIOxSPI_CS 	DDRB
	#define PINxKB_SPI_CS	4

    /****************************************************************************************
                                Definir el orden LSB / MSB
	****************************************************************************************/
	//Data Order SPI
	#define MSB_FIRST (0)
	#define LSB_FIRST (1)
    //
	#define MSB_LSB_FIRST (MSB_FIRST)
	//#define MSB_LSB_FIRST (LSB_FIRST)

    /****************************************************************************************
        Relationship Between SCK and the Oscillator Frequency SPI2X SPR1SPR0_SCK Frequency

	****************************************************************************************/
	//#define DOUBLE_SPI_SPEED_BIT 1
	#define DOUBLE_SPI_SPEED_BIT 0

	#if DOUBLE_SPI_SPEED_BIT == 0
		//000 fosc/4
		//001 fosc/16
		//010 fosc/64
		//011 fosc/128
		#define SPR1SPR0_SCK_FOSC_4	    (0x00)
		#define SPR1SPR0_SCK_FOSC_16	(0x01)
		#define SPR1SPR0_SCK_FOSC_64	(0x02)
		#define SPR1SPR0_SCK_FOSC_128	(0x03)

	#else
		//100 fosc/2
		//101 fosc/8
		//110 fosc/32
		//111 fosc/64
		#define SPR1SPR0_SCK_FOSC_2	    (0x04)
		#define SPR1SPR0_SCK_FOSC_8 	(0x05)
		#define SPR1SPR0_SCK_FOSC_32	(0x06)
		#define SPR1SPR0_SCK_FOSC_64	(0x07)

	#endif

    /************************************************************************************************
                            Dummy Byte by exchange data
    ************************************************************************************************/

    #define    DUMMY_BYTE_BY_EXCHANGE   (0x00)

    /************************************************************************************************

    ************************************************************************************************/
    void InitSPI_MASTER(void);
    uint8_t SPI_MSTR_ExchangeData(uint8_t DATASPI);

#endif
