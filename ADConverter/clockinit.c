/*
 * clockinit.c
 *
 * Created: 18.09.2017 22:39:42
 *  Author: mburger
 */ 
 #include <avr/io.h>

 void writeCCP(volatile register8_t* address, uint8_t value) {
	 volatile uint8_t* tmpAddr = address;
	 asm volatile("movw r30, %0"  "\n\t"
	 "LDI r16,0xD8"  "\n\t"
	 "out %2,r16"    "\n\t"
	 "st Z, %1"    "\n\t"
	 :
	 : "r" (tmpAddr), "r" (value), "i"(&CCP)
	 : "r16", "r30", "r31"
	 );
 }
 void clockInit(void) {
	 OSC.XOSCCTRL = OSC_FRQRANGE_2TO9_gc | OSC_XOSCSEL_XTAL_256CLK_gc | OSC_XOSCPWR_bm;
	 OSC.CTRL |= OSC_XOSCEN_bm;
	 while(!(OSC.STATUS & OSC_XOSCRDY_bm));
	 //OSC.CTRL |= OSC_RC32MEN_bm;
	 //while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	 OSC.PLLCTRL = (OSC_PLLSRC_XOSC_gc | 4); //PLL Multiplication Factor
	 OSC.CTRL |= OSC_PLLEN_bm;
	 while(!(OSC.STATUS & OSC_PLLRDY_bm));
	 writeCCP(&CLK.CTRL, CLK_SCLKSEL_PLL_gc);
	 //writeCCP(&CLK.CTRL, CLK_SCLKSEL_RC32M_gc);
 }