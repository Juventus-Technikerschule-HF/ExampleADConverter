/*
 * ADConverter.c
 *
 * Created: 21.11.2017 19:34:18
 * Author : chaos
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "clockinit.h"
#include "ButtonHandler.h"
#include "NHD0420Driver.h"
#include "NumberStringConverter.h"

uint16_t adResult0;
uint16_t adResult1;
uint16_t adTempRaw;
uint16_t calibrationValue;

uint8_t read_calibration_byte( uint8_t index )
{
    uint8_t result;
    /* Load the NVM Command register to read the calibration row. */
    NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
    result = pgm_read_byte(index);
    /* Clean up NVM Command register. */
    NVM_CMD = NVM_CMD_NO_OPERATION_gc;
    return( result );
}

void initTimer() {
	TCC0.CTRLA = TC_CLKSEL_DIV64_gc ;										// Prescaler: 32MHz / 64 = 500kHz
	TCC0.CTRLB = 0x00;														// Mode = Normal. No CompareEnable
	TCC0.INTCTRLA = 0x02;													// Overflow Interrupt Level: Medium
	TCC0.PER = 500;
}
void initGPIO() {
	
}
void initInterrupt() {
	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
	sei();
}

void initADC() {
	ADCA.CTRLA = 0x01; //Enable ADCA;
	ADCA.CTRLB = 0x00;
	ADCA.REFCTRL = 0x12;
	ADCA.PRESCALER = 0x03;
	ADCA.CH0.CTRL = 0x01; //Startbit at MSB
	ADCA.CH0.MUXCTRL = (8 << 3) | (7 << 0);
	ADCA.CH0.INTCTRL = 0x01;
	ADCA.CH1.CTRL = 0x01; //Startbit at MSB
	ADCA.CH1.MUXCTRL = (9 << 3) | (5 << 0);
	ADCA.CH1.INTCTRL = 0x01;
	
	//Load Calibration-Data from Factory-Signature Memory
	ADCB.CALL = read_calibration_byte(0x24);
	ADCB.CALH = read_calibration_byte(0x25);
	calibrationValue = read_calibration_byte(0x2E);
	calibrationValue += (read_calibration_byte(0x2F)<<8);

	ADCB.CTRLA = 0x01; //Enable ADC0;
	ADCB.CTRLB = 0x00;
	ADCB.REFCTRL = 0x03; //Enable Internal 1V Reference and Temperature-Sensor
	ADCB.PRESCALER = 0x03;
	ADCB.CH0.CTRL = 0x00; //Inputmode: Internal. For Temperature-Sensor
	ADCB.CH0.MUXCTRL = (0 << 3) | (5 << 0); //Select Temperature-Sensor as Input
	ADCB.CH0.INTCTRL = 0x01;	
}

int main(void)
{
    clockInit();
	initButtons();
	displayInit();
	initTimer();
	initGPIO();
	initADC();
	initInterrupt();
	while (1) 
    {
    }
}

ISR(TCC0_OVF_vect) {
	updateButtons();
	displayBufferClear();
	char numberString[8];
	//Print Potentiometer Values
	displayBufferWriteStringAtPos(0,0,"POTI1:");
	convert_uint_string(adResult0, numberString);
	displayBufferWriteStringAtPos(0,6, numberString);
	displayBufferWriteStringAtPos(1,0,"POTI2:");
	convert_uint_string(adResult1, numberString);
	displayBufferWriteStringAtPos(1,6, numberString);

	//Print RAW Temperature Value
	displayBufferWriteStringAtPos(2,0,"TEMPRaw:");
	convert_uint_string(adTempRaw, numberString);
	displayBufferWriteStringAtPos(2,8, numberString);
                           
	//Calculate Temperature from measured ad-value
	float temperature = (float)(((273.15+85) / (float)calibrationValue) * (float)(adTempRaw-190))-(float)(273);

	//Print Temperature
	displayBufferWriteStringAtPos(3,0,"Temperature:");
	convert_float_string(temperature, numberString, 1);
	displayBufferWriteStringAtPos(3,12, numberString);

	displayUpdateWorker();	
	ADCA.CH0.CTRL |= 0x80; //ADC Start Poti1
	ADCA.CH1.CTRL |= 0x80; //ADC Start Poti2
	ADCB.CH0.CTRL |= 0x80; //ADC Start Temp-Sensor
}

ISR(ADCA_CH0_vect) {
	adResult0 = ADCA.CH0.RES;
}
ISR(ADCA_CH1_vect) {
	adResult1 = ADCA.CH1.RES;
}
ISR(ADCB_CH0_vect) {
	adTempRaw = ADCB.CH0.RES;
}
