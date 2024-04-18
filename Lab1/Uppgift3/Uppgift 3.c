/*
 * 	startup.c
 *
 */
 
#include <stdio.h>
#include <stdbool.h>
#define gpio 0x40020C00
#define GPIO_MODER (volatile unsigned int *)(gpio)
#define GPIO_OTYPER (volatile unsigned short *)(gpio+0x04)
#define GPIO_PUPDR (volatile unsigned int *)(gpio+0x0C)
#define GPIO_IDR_HIGH (volatile unsigned char *)(gpio+0x11)
#define GDPIO_IDR_LOW (volatile unsigned char *)(gpio+0x10)
#define GPIO_ODR_HIGH (volatile unsigned char *)(gpio+0x15)
#define GPIO_ODR_LOW (volatile unsigned char *)(gpio+0x14)
 
__attribute__((naked)) 
__attribute__((section (".start_section")) )

void startup ( void ) {
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

void kbdActivate(unsigned int row){
/*aktivera angiven rad hos tangentbordet, eller deaktivera samtliga*/
switch( row ){
	case 1: *GPIO_ODR_HIGH = 0x10; break;
	case 2: *GPIO_ODR_HIGH = 0x20; break;
	case 3: *GPIO_ODR_HIGH = 0x40; break;
	case 4: *GPIO_ODR_HIGH = 0x80; break;
	case 0: *GPIO_ODR_HIGH = 0x00; break;
}
}

int kbdGetCol (void) {
	unsigned char c;
	c = *GPIO_IDR_HIGH;
	if (c & 0x8) return 4;
	if (c & 0x4) return 3;
	if (c & 0x2) return 2;
	if (c & 0x1) return 1;
	return 0;
}

void app_init(void){

	*GPIO_MODER = 0x55005555;

}



unsigned char keyb (void) {
	unsigned char key[] = {1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD};
	int row, col;
	for (row=1; row <= 4; row++) {
		kbdActivate( row );
		if ((col = kbdGetCol ())) {
			kbdActivate (0);
			return key [4*(row-1)+(col-1)];
		}
	}
	kbdActivate(0);
	return 0xFF;
}

void out7seg (unsigned char c) {
	int output;
	switch (c) {
		case 0: output = 0x3F;
		break;
		case 1: output = 0x30;
		break;
		case 2: output = 0x5B;
		break;
		case 3: output = 0x4F;
		break;
		case 4: output = 0x66;
		break;
		case 5: output = 0x6D;
		break;
		case 6: output = 0x7D;
		break;
		case 7: output = 0x03;
		break;
		case 8: output = 0x7F;
		break;
		case 9: output = 0x6F;
		break;
		case 0xA: output = 0x77;
		break;
		case 0xB: output = 0x7C;
		break;
		case 0xC: output = 0x38;
		break;
		case 0xE: output = 0x79;
		break;
		case 0xF: output = 0x71;
		break;
		case 0xD: output = 0x5E;
		break;
	}
	*GPIO_ODR_LOW = output;	
}

unsigned char keyb_enhanced(void) {		
	bool keyb_state = true;				 // 1(true) = initialtillstånd, 0(false) = väntetillstånd

	if (keyb_state == false) {
		//AKTIVERA SAMTLIGA RADER????
		if (keyb() != 0xFF) {
			keyb_state = false;
		}
		else {
			keyb_state = true;
		}
		return 0xFF;
	}
	if (keyb_state == true) {
		if (keyb() != 0xFF) {
			keyb_state = false;
			return keyb();
		}
	}
}

void main (void) {
	unsigned char c;
	
	app_init();
	
	while(1) {
		c = keyb_enhanced ();
		if (c != 0xFF) {
		out7seg(c);
		}
	}
	
}