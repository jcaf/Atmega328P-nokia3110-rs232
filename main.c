/*
 * main.c
 *
 * ok
 *  Created on: May 7, 2021
 *      Author: jcaf
 *
 *      Atmega328P
 *
 *      16MHz, boden off,
 *      [jcaf@firwar ~]$ avrdude -c usbasp -B5 -p m328P -U lfuse:w:0xff:m -U hfuse:w:0xd9:m -U efuse:w:0xfd:m
 *
 *
 *      octave:4> dec2hex(sum(int8('@N512F1023R256')))
 *
 *      http://carlosefr.github.io/pcd8544/
 *
 * OK SABADO 26 JUNIO 2021
 */

#include "system.h"
#include "types.h"
#include "main.h"
#include "SPI/SPI.h"
#include "PCD8544/PCD8544.h"
#include "usart/usart.h"
#include "serial/serial.h"
#include "pinGetLevel/pinGetLevel.h"
#include "NTC10K/NTC10K.h"
#include "adc/adc.h"

char bin_to_asciihex(char c)//nibbleBin_to_asciiHex
{
    if (c < 10)
        return c+'0';
    else
        return (c-10) + 'A';
}

int16_t ADRESHL_NTC10K = 0;
int16_t ADRESHL_RESISTANCE1 = 0;
int16_t ADRESHL_RESISTANCE2 = 0;

//Activacion del buzzer dentro del rango
//R1: 0..200 OHMs
#define R1_RANGE_MIN 98.0f
#define R1_RANGE_MAX 104.0f

//R2: 0..100 OHMs
#define R2_RANGE_MIN 50.0f
#define R2_RANGE_MAX 60.0f

float temperature;
float resistance1;
float resistance2;
//float batteryvoltage;
float battery_porcent;
//6V -> 0%
//9>= ->100%
#define BATTERYVOLT_MIN 6//VDC
#define BATTERYVOLT_MAX 9//VDC
#define BATTERYVOLT_EQ_PENDIENTE_M (100.0f/ (BATTERYVOLT_MAX-BATTERYVOLT_MIN))
#define BATTERYVOLT_EQ_B -(BATTERYVOLT_EQ_PENDIENTE_M*BATTERYVOLT_MIN)

char buff_out[SCIRBUF_BUFF_SIZE];

void wINTRODUCCION(void)
{
	 gotoXY (0,0);
	 LcdString("WETRE INDRE.");
	 gotoXY (10,2);
	 LcdString("SISTEMA");
	 gotoXY (10,3);
	 LcdString("MEDICION");
	 gotoXY (15,4);
	 LcdString("T 300");
}

void wVENTA1(void)
{
	 gotoXY (0,0);
	 LcdString("WETRE INDRE.");

	 gotoXY (20,2);
	 LcdString("RESIST");//resistencia
	 gotoXY((84-33)/2, 3);
	 glc_write_symbol_audio();
}
void wVENTA2(void)
{
	 gotoXY (0,0);
	 LcdString("WETRE INDRE.");
	 gotoXY (7,2);
	 LcdString("PRESION-OK");
	 gotoXY((84-33)/2, 3);
	 glc_write_symbol_audio();
}
void wVENTA3(void)
{
	 gotoXY (0,0);
	 LcdString("WETRE INDRE.");
	 gotoXY (3,1);
	 LcdString("TEMPERATURA");
	 gotoXY ( (84-10)/2,2);
	 glc_write_symbol_grados();
	 LcdString("C");
}
void wVENTA4(void)
{
	 gotoXY (0,0);
	 LcdString("WETRE INDRE.");
	 gotoXY (15,1);
	 LcdString("BATERIA");
	 gotoXY((84-5)/2,2);
	 LcdString("%");
}

volatile struct _isr_flag
{
    unsigned f1ms :1;
    unsigned __a :7;
} isr_flag;

struct _main_flag
{
    unsigned f1ms :1;
    unsigned changeWindow:1;
    unsigned __a:6;

}main_flag = { 0,0 };


#define NUM_WINDOWS 4

uint8_t checksum(char *str, uint8_t length);

void rx_trama(void);

//PIC16F88
#define ADC_TOP 1023
#define RPU 221.0f //Ohms 1206 1% 1/4Watss

float calculate_resistance(int16_t ADC_HL, int16_t adcTop)
{
	return (ADC_HL*RPU*-1.0f)/(ADC_HL-adcTop);
}


/*
 * ADC CHANNEL 1
 * ADC calibration, Medir en ckto
 * AREF Nominal(5V), Medido = 5V
 * R1 Nominal(10k), Medido = 9.98K
 * R2 Nominal(20k), Medido = 19.89K
 *
 * INTERNAL AREF NOMINAL(5)
 *
 * VCC
 * | (19.89K)
 * R2
 * |
 * +---->
 * |
 * R1 (9.98K)
 * |
 * GND
 *ADC_init(ADC_MODE_SINGLE_END);
 */
#define ADCH1_R1 9.98E3//100E3	//MEDIR 99.3
#define ADCH1_R2 19.89E3	//510E3MEDIR
#define ADC_AREF 5		//5V MEDIR
#define ADC_TOP 1023	//KTE

float VoltBatt_get(void)//Blocking
{
	return (ADC_read(ADC_CH_0) * ADC_AREF * (ADCH1_R1 + ADCH1_R2) )/ (ADC_TOP * ADCH1_R1);
}

int main(void)
{
	int8_t kb_counter = 0;
	int8_t sm0 =0;
	char str[20];

	//PinTo1(PORTWxLED1, PINxLED1);
	//ConfigOutputPin(CONFIGIOxLED1, PINxLED1);

	//glcd init
	LcdInitialise();
	LcdClear();
	clearPixels();

//	gotoXY((84-33)/2, 3);
//	glc_write_symbol_audio();
//gotoXY(20, 3);
//LCD_writeString_megaFont("31.2");
//while (1);
	//
	pinGetLevel_init();
	//
	PinTo0(PORTWxBUZZER, PINxBUZZER);
	ConfigOutputPin(CONFIGIOxBUZZER, PINxBUZZER);

	PinTo0(PORTWxLED1, PINxLED1);
	ConfigOutputPin(CONFIGIOxLED1, PINxLED1);

	USART_Init ( (int)MYUBRR ); //1200 baud

	ADC_init(ADC_MODE_SINGLE_END);

	//Atmega328P TCNT0 CTC mode
    TCNT0 = 0x0000;
    TCCR0A = (1 << WGM01);

    //x16MHZ...ok
    //TCCR0B = (0 << CS02) | (1 << CS01) | (1 << CS00); //CTC PRES=64
    //OCR0A = CTC_SET_OCR_BYTIME(1E-3,64);//249

    //x11.05MHz es mejor PRESCALER
    TCCR0B = (1 << CS02) | (0 << CS01) | (0 << CS00); //CTC PRES=256
	OCR0A = CTC_SET_OCR_BYTIME(1E-3,256);//42.2

    TIMSK0 |= (1 << OCIE0A);
    sei();

    //
    wINTRODUCCION ();
    __delay_ms(2000);
    LcdClear();

    main_flag.changeWindow = 1;

    while (1)
	{
		if (isr_flag.f1ms)
		{
			isr_flag.f1ms = 0;
			main_flag.f1ms = 1;
		}
		//----------------------
		if (main_flag.f1ms)
		{
			if (++kb_counter >= 20)
			{
				kb_counter = 0;
				//
				pinGetLevel_job();
				if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_SW0))
				{
					pinGetLevel_clearChange(PGLEVEL_LYOUT_SW0);
					if (pinGetLevel_level(PGLEVEL_LYOUT_SW0) == 0)
					{
						main_flag.changeWindow = 1;
						if (++sm0 >= NUM_WINDOWS)
							{sm0 = 0;}
					}
				}
				//---------------------------
				if (main_flag.changeWindow)
				{
					main_flag.changeWindow = 0;

					LcdClear();
					PinTo0(PORTWxBUZZER, PINxBUZZER);

					if (sm0 == 0)
					{
						wVENTA1();//RESISTENCIA
					}
					else if (sm0 == 1)
					{
						wVENTA2();//PRESION
					}
					else if (sm0 == 2)
					{
						wVENTA3();//TEMPERATURA
					}
					else if (sm0 == 3)
					{
						wVENTA4();//BATERIA
					}
				}
				//---------------------------
				if (sm0 == 0)
				{
					//itoa(ADRESHL_RESISTANCE1, str, 10);
					dtostrf(resistance1, 0, 1, str);
//					gotoXY (20,4);
//					LcdString(str);//resistencia

					if ( (resistance1 >= R1_RANGE_MIN) && (resistance1  <= R1_RANGE_MAX) )
					{
						PinTo1(PORTWxBUZZER, PINxBUZZER);
						PinTo1(PORTWxLED1, PINxLED1);
						//Buzzer ON
					}
					else
					{
						PinTo0(PORTWxBUZZER, PINxBUZZER);
						PinTo0(PORTWxLED1, PINxLED1);
					}
				}
				else if (sm0 == 1)
				{
					//itoa(ADRESHL_RESISTANCE2, str, 10);
					dtostrf(resistance1, 0, 1, str);
//					gotoXY (20,4);
//					LcdString(str);

					if ( (resistance1 >= R2_RANGE_MIN) && (resistance1  <= R2_RANGE_MAX) )
					{
						//Buzzer ON
						PinTo1(PORTWxBUZZER, PINxBUZZER);
						PinTo1(PORTWxLED1, PINxLED1);
					}
					else
					{
						PinTo0(PORTWxBUZZER, PINxBUZZER);
						PinTo0(PORTWxLED1, PINxLED1);
					}
				}
				else if (sm0 == 2)
				{
					//itoa(ADRESHL_NTC10K, str, 10);
					dtostrf(temperature, 0, 2, str);
					gotoXY (10,3);
					//LcdString(str);
					LCD_writeString_megaFont(str);
				}
				else if (sm0 == 3)
				{
					battery_porcent = ( BATTERYVOLT_EQ_PENDIENTE_M*VoltBatt_get() ) + BATTERYVOLT_EQ_B;
					if (battery_porcent < 0)
					{
						battery_porcent = 0.0f;
					}
					else if (battery_porcent >100.0)
					{
						battery_porcent = 100.0f;
					}

					dtostrf(battery_porcent, 0, 0, str);
					gotoXY (20,3);
					LCD_writeString_megaFont(str);

				}
			//
				rx_trama();
			}
		}


		main_flag.f1ms = 0;
	}//End while
	return 0;
}


ISR(TIMER0_COMPA_vect)
{
    isr_flag.f1ms = 1;
    //PinToggle(PORTWxLED1, PINxLED1);
}



uint8_t checksum(char *str, uint8_t length)
{
    uint16_t acc = 0;
    for (int i=0; i< length; i++)
    {
        acc += str[i];
    }
    return (uint8_t)(acc);
}



/*
 * //Construir payload data + checksum
 @NxxxxFxxxxRxxxxCcc'\r\n'
 1234546789........64

 */
#define RX_BUFFER_MAXSIZE 32
struct _rx
{
	int8_t sm0;
	char buffer[RX_BUFFER_MAXSIZE];
}rx;


/* recorre todo el array
 * 1: success
 * 0: fail
 */

int8_t str_trimlr(char *str_in, char *str_out, char l, char r)
{
	int8_t counter = 0;
	int8_t idx = 0;
	int8_t length = strlen(str_in);

	for (counter = 0; counter < length; counter ++)
	{
		if (str_in[counter] == l)
		{
			counter ++;//sale dejando apuntando al siguiente byte
			//@N512F
			//copy
			for (;counter < length; counter++)
			{

				if (str_in[counter] == r)
				{
					//ok, hasta aqui nomas, procede
					str_out[idx] = '\0';//fin de cadena trimmed
					return 1;
				}

				str_out[idx++] = str_in[counter];
			}
		}
	}

	return 0;
}

/*
 * la busqueda en el buffer circular es cada "x" ms
 * no puede ser directa porque perderia mucho tiempo hasta que se complete la trama completa
 *
 * octave:7>  dec2hex(sum(int8('@N512F1023R256')))
	ans = 321 -> el resultado esta en HEX, solo me quedo con el byte menor = 0x21
 *
 * 	@N512F1023R256C21
	@N512F1023R257C22
 */
void rx_trama(void)
{
	//
	uint8_t checks = 0;
	char *pb = rx.buffer;
	uint8_t counter = 0;
	char str[4];
	char buff_temp[10];
	int8_t idxtokens = 0;
	int8_t idx_base = 0;
	uint8_t bytes_available;

	static char Cstr[64];//todos los bytes se inicializan a 0

	if (rx.sm0 == 0)
	{
		//busqueda en buffer circular
		bytes_available = scirbuf_bytes_available();
		if (bytes_available>0)
		{
			//PinToggle(PORTWxLED1, PINxLED1);

			scirbuf_read_nbytes((uint8_t*)buff_out, bytes_available); //hago la copia desde el buffer circular hacia el de salida temporal
			//
			buff_out[bytes_available] = '\0';//convertir en c_str
			strcat(Cstr,buff_out);

			//Ahora analizo por toda la trama completa
			//@NxxxxFxxxxRxxxxCcc'\r\n'

			idxtokens = 0;
			#define TOKENS_NUMMAX 6
			//const char tokens[TOKENS_NUMMAX] = {'@','N','F','R','C','\n'};
			const char tokens[TOKENS_NUMMAX] = {'@','N','F','R','C',0x0D};//Enter x proteus


			char c;
			idx_base = 0;
			for (int8_t i=0; i<strlen(Cstr); i++)
			{
				c = Cstr[i];
				if (  c == tokens[idxtokens] )
				{
					idxtokens++;
				}
				//
				if (idxtokens > 0) //osea se encontró al menos el primer token '@'
					{rx.buffer[idx_base++] = c;}

				if (idxtokens >= TOKENS_NUMMAX)
				{
					//Todos los tokens fueron encontrados de principio a fin
					rx.buffer[idx_base] = '\0';//fin, convierte a Cstring
					strcpy(Cstr, "");//reset Cstr;
					//
					rx.sm0++;


					break;
				}
			}
		}
	}
	if (rx.sm0 == 1)
	{
		//en este punto tengo la probabilidad de tener el buffer correctamente copiado desde el Buffer circular
		//Necesito tener si o si toda la trama desde @ hasta \r\n.. OK
		//@NxxxxFxxxxRxxxxCcc'\r\n'
		counter = 0x00;
		pb = &rx.buffer[0];

		while (*pb++ != 'C')//Si recorre todo array y no encuentra C, entonces resetear algoritmo de busqueda
		{
			counter++;
		}
		checks = checksum(rx.buffer, counter);//checksum desde @....C, nada mas

		//PinTo1(PORTWxLED1, PINxLED1);
		//itoa(checks, str, 10);
		//sprintf(str,"%d",checks);

		//0x ASCII-HEX CODED
//		if (rx.buffer[counter+1] == str[0])
//		{
//			if (rx.buffer[counter+2] == str[1])
		if (rx.buffer[counter+1] == bin_to_asciihex(checks>>4))
		{
			if (rx.buffer[counter+2] == bin_to_asciihex(checks & 0x0F))
			{
				//ok, ambos checksum son iguales
				//-> se usa toda la data integra
				//Convertir de ASCII a integer, separar la data
				if (str_trimlr(rx.buffer, buff_temp, 'N', 'F' ))
				{
					ADRESHL_NTC10K = atoi(buff_temp);
					temperature = ntc10k_st(ADRESHL_NTC10K, 1023);

				}
				if (str_trimlr(rx.buffer, buff_temp, 'F', 'R' ))
				{
					ADRESHL_RESISTANCE2 = atoi(buff_temp);
					resistance2 = calculate_resistance(ADRESHL_RESISTANCE2, ADC_TOP);
				}
				if (str_trimlr(rx.buffer, buff_temp, 'R', 'C' ))
				{
					ADRESHL_RESISTANCE1 = atoi(buff_temp);
					resistance1 = calculate_resistance(ADRESHL_RESISTANCE1, ADC_TOP);
				}

				rx.sm0 = 0x00;

			}
		}
	}
}

