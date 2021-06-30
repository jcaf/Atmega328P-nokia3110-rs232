    /**************************************************************************************
    InitSPI_MASTER:

    - Las líneas MOSI, MISO, CLK son configuradas a sus respectivos salidas/entradas en esta perspectiva

    ****************************************************************************************************/
    #include <avr/io.h>

	#include "SPI.h"
    #include "../system.h"
	void InitSPI_MASTER(void)
	{
		//ConfigOutputPin(CONFIGIOxSPI_MOSI, PINxKB_SPI_MOSI);
		//ConfigOutputPin(CONFIGIOxSPI_SCK, PINxKB_SPI_SCK);
		//ConfigOutputPin(CONFIGIOxSPI_CS, PINxKB_SPI_CS);
        //DDRB |= (1<<7) | (1<< 4) | (1<< 5);
        //DDRB = 0xFF;

        //SPCR = (1<<SPE) | (0<< DORD) | (1<<MSTR) | (1<<CPOL) | (0<<CPHA) | (SPR1SPR0_SCK_FOSC_4);


        //SPCR = (1<<SPE) | (0<< DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (SPR1SPR0_SCK_FOSC_16);
        SPCR = (1<<SPE) | (0<< DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (SPR1SPR0_SCK_FOSC_64);

        //SPCR = 0x58;
        //01011000
       // SPSR = DOUBLE_SPI_SPEED_BIT << SPI2X;

	}//End fx()

    uint8_t SPI_MSTR_ExchangeData(uint8_t DATASPI)
    {
        SPDR = DATASPI;

        //Esperar por data transmitida
        while ( !(SPSR & (1<<SPIF) ) )   //1º Read Status register
        {;}

        return  SPDR;               //2º leer SPDR -> Clear flag
    }

