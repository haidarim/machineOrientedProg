/*
* Delay
*
*/
#include <stdio.h>
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))
#define GPIO_E 0x40021000 // MD407 port E
#define GPIO_E_ODR ((volatile unsigned char*) (0x40021014))
#define SIMULATOR
void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );
void startup ( void )
{
__asm volatile(
" LDR R0,=0x2001C000\n" /* set stack */
" MOV SP,R0\n"
" BL main\n" /* call main */
"_exit: B .\n" /* never return */
) ;
}

void init_app(){
	* ((unsigned long*) GPIO_E) = 0x55555555;
}

void delay_250ns( void ) {
	
	*STK_CTRL = 0;
	*STK_LOAD = ( (1680000/4) -1 );
	*STK_VAL = 0;
	*STK_CTRL = 5;

	while( (*STK_CTRL & 0x10000 )== 0 );
	*STK_CTRL = 0;
}
	
void delay_micro(unsigned int us) {
	#ifdef SIMULATOR
	us = us / 1000;
	us++;
	#endif
	
	while( us > 0 ) {
	delay_250ns();
	delay_250ns();
	delay_250ns();
	delay_250ns();
	us--;
	}
}

void delay_milli( unsigned int ms ) {
	
	#ifdef SIMULATOR
	ms = ms/1000;
	ms++;
	#endif*
	while( ms-- )
	delay_micro(1000); //Varför 2000? En ms = 1000 ns
	
}

void main(void) {
	
	init_app();
	
	while(1){
	*GPIO_E_ODR = 0;
	delay_milli(500);
	*GPIO_E_ODR =0xFF;
	delay_milli(500);
	}
}

