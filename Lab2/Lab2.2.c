

// ASCII-DISPLAY 


// Register för systemklockan (SYSTICK)
#define SYSTICK 0xE000E010
#define STK_CTRL ((volatile unsigned int*) (SYSTICK))
#define STK_LOAD ((volatile unsigned int*) (SYSTICK+0x4))
#define STK_VAL ((volatile unsigned int*) (SYSTICK+0x8))
#define STK_CALIB ((volatile unsigned int*) (SYSTICK+0xC))

// Word adresser för initieringar
#define PORT_BASE 0x40021000
#define GPIO_E_MODER ((volatile unsigned int*) (PORT_BASE))
#define GPIO_E_OTYPER ((volatile unsigned int*) (PORT_BASE+0x4))
#define GPIO_E_OSPEEDR ((volatile unsigned int*) (PORT_BASE+0x8))
#define GPIO_E_PUPDR ((volatile unsigned int*) (PORT_BASE+0xC))

// Byte adresser för data & styrregister
#define GPIO_E_IDRLOW ((volatile unsigned char*) (PORT_BASE+0x10))
#define GPIO_E_IDRHIGH ((volatile unsigned char*) (PORT_BASE+0x11))
#define GPIO_E_ODRLOW ((volatile unsigned char*) (PORT_BASE+0x14))
#define GPIO_E_ODRHIGH ((volatile unsigned char*) (PORT_BASE+0x14+1))

// Styrregister bitar
#define B_E 0x40 // Enable
#define B_SELECT 4 // Select ASCII-display
#define B_RW 2 // 0=Write, 1=Read
#define B_RS 1 // 0=Control, 1=Data

void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

// Skriv data
void ascii_write_data(unsigned char data){
	ascii_ctrl_bit_set(B_RS);    	 //RS=1
	ascii_ctrl_bit_clear(B_RW);  	 //RW=0
	ascii_write_controller(data);  	// Skriver data till controller (?)
}
	
// Skriv kommando
void ascii_write_cmd (unsigned char command){
	ascii_ctrl_bit_clear(B_RS);		 // RS=0
	ascii_ctrl_bit_clear(B_RW); 	 // RW=0
	ascii_write_controller(command); // Skriv kommando till controller (?)
}

unsigned char ascii_read_data(void){
	unsigned char c;

	*GPIO_E_MODER=0x00005555;		//E15-8=iport
	ascii_ctrl_bit_set(B_RS);		//RS=1
	ascii_ctrl_bit_set(B_RW);		//RW=1
	c=ascii_read_controller();		//rv=ascii read controller
	*GPIO_E_MODER=0x55555555;		//E15-8=outport
	return c; 						//return rv
}
	
unsigned char ascii_read_status(void){
	unsigned char c;
	*GPIO_E_MODER=0x00005555;		//E15-8 = inport // 7-0 portar (?)
	ascii_ctrl_bit_set(B_RW); 		//RW=1
	ascii_ctrl_bit_clear(B_RS);		 //RS=0
	c=ascii_read_controller(); 		//rv=ascii_read_controller
	*GPIO_E_MODER=0x55555555;		//E15-8 = outport
	return c; 						//return rv
}
	
void ascii_read_controller(void){
	unsigned char c;
	ascii_ctrl_bit_set(B_E);		// E=1
	delay_250ns(); 					// Delay 360 nanosekunder
	delay_250ns();					// (250+250=500. 500>360. Td=360)
	c=*GPIO_E_IDRHIGH; 				// rv=DATA_REGISTER
	ascii_ctrl_bit_clear(B_E);		// E=0
	return c;						// Return rv
}
	
void ascii_write_controller(unsigned char c){
	ascii_ctrl_bit_set(B_E);		 // E=1
	*GPIO_E_ODRHIGH=c; 			// DATA_REGISTER = input
	ascii_ctrl_bit_clear(B_E); 		// E=0
	delay_250ns(); 				// Delay
}

// Addressera ASCII-display och ettställ bitar som är 1 i x
void ascii_ctrl_bit_set(unsigned char x){
	unsigned char c;
	c= *GPIO_E_ODRLOW;
	c |= (B_SELECT | x);
	*GPIO_E_ODRLOW=c;
}

// Addressera ASCII_display och nollställ bitar som är 1 i x
void ascii_ctrl_bit_clear(unsigned char x){
	unsigned char c;
	c=*GPIO_E_ODRLOW;
	c &= (B_SELECT | ~x);
	*GPIO_E_ODRLOW=c;
}

// Fördröjer en mikrosekund
void delay_mikro(unsigned int us){
	//#ifdef SIMULATOR
	us=us/1000;
	us++;
	//#endif
	while (us>0) {
		delay_250ns();
		delay_250ns();
		delay_250ns();
		delay_250ns();
		us--;
	}
}

// Fördröjer en milisekund
void delay_milli(unsigned int ms){
	delay_mikro(1000*ms);
	}

// Fördröjer 250 nanosekunder
void delay_250ns(void){
	*STK_CTRL=0; 		// Nollställ STK_CTRL
	*STK_LOAD=((168/4)-1); 		// Load laddas med räknevärde
	*STK_VAL=0;		 // Nollställ STK_VAL
	*STK_CTRL=5; 		// Styrord till STK_CTRL
	while ((*STK_CTRL & 0x10000)==0); 		// Paus tills COUNTFLAG = 1
	*STK_CTRL=0; 		// Nollställ STK_CTRL
}

// Initierar DISPLAY för 2 rader, 5x8 punkters tecken, TÄND display, TÄND markör, konstant visning. 

// Adressering med "increment". Ingen skift av adressbussen.
void ascii_init(void){
	while((ascii_read_status() & 0x80)==0x80){} 		// Vänta tills statusflaggan = 0
	delay_mikro(8);		 	// Latency tid för kommando
	ascii_write_cmd(0x38); 			// 2 rader, 5x8 punkter (0011 1000)
	delay_mikro(40);		 // Fördröj 40 us
	ascii_write_cmd(0x0E);		 // Tänd display,Tänd markör,ingen blink (0000 1110)
	delay_mikro(40);		 // Fördröj 40 us
	ascii_write_cmd(0x04);		 // Increment (ID) = 0, Skift (SH) = 0 (0000 0100)
	delay_mikro(40); 		// Fördröj 40 us
} 

void clear_display(void){
	while((ascii_read_status() & 0x80)==0x80){} 		// Vänta tills statusflaggan = 0
	delay_mikro(8); 		// Latency tid för kommando
	ascii_write_cmd(1); 		// Clear display
	delay_milli(2); // delay 2 milisekunder
}

void ascii_write_char(unsigned char c){
	while((ascii_read_status() & 0x80)==0x80){} 		// Vänta tills statusflaggan = 0
	delay_mikro(8); 		// Latency tid för kommando
	ascii_write_data(c); 		// Skriver char till display
	delay_mikro(45);		 // Fördröj 45 
	
	//mikrosekunder (43 us);
}

void ascii_gotoxy(int row,int column){
	unsigned char adress;
	adress = row - 1;
	if (column==2){
	adress = adress + 0x40;
	}
	ascii_write_cmd(0x80 | adress);
}

void init_app(void){ 
	*GPIO_E_MODER=0x55555555;
}

void startup ( void ) {
	__asm volatile(
	" LDR R0,=0x2001C000\n" /* set stack */
	" MOV SP,R0\n"
	" BL main\n" /* call main */
	"_exit: B .\n" /* never return */
	) ;
}


void main(void) {
	char *s;
	char test1[] = "Alfanumerisk ";
	char test2[] = "Display - test";

	init_app();
	ascii_init();

	ascii_gotoxy(1,1);
	s=test1;
	while(*s){
		ascii_write_char(*s++);
	}
	
	ascii_gotoxy(1,2);
	s=test2;
	while(*s){
		ascii_write_char(*s++);
	}
	
	return 0;
}