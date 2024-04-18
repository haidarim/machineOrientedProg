//INSTRUKTIONER

//Detta spela är inspirerat av Googles offlinespel där man är en dinosauri som hoppar över kaktusar/träd.
//Endast ettan på keypaden används i detta spel. 
//
//Spelet går ut på att hoppa över träden. Om dinosaurien nuddar trädet är spelet 
//slut. Knapp 1 används för att hoppa. Om knapp 1 är aktiv kommer dinosaurien att 
//fortsätta hoppa, och därför kan det vara smart att dubbelklicka på 1 för att 
//endast göra ett hopp.
//
//På den nedre displayen visas en poängräknare. När dinosaurien springer in i  ett
//träd så stoppas poängräknaren. 




#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define SYSTICK 0xE000E010
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))  
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))  
#define STK_VAL  ((volatile unsigned int *)(0xE000E018))
#define STK_CALIB ((volatile unsigned int*) (SYSTICK+0xC))

//D-Portar
#define GPIO_D 0x40020C00
#define GPIO_MODER  ((volatile unsigned int *) (GPIO_D)) 
#define GPIO_OTYPER  ((volatile unsigned short *) (GPIO_D+0x4)) 
#define GPIO_PUPDR ((volatile unsigned int *) (GPIO_D+0xC)) 
#define GPIO_IDR_LOW ((volatile unsigned char *) (GPIO_D+0x10)) 
#define GPIO_IDR_HIGH  ((volatile unsigned char *) (GPIO_D+0x11)) 
#define GPIO_ODR_LOW ((volatile unsigned char *) (GPIO_D+0x14)) 
#define GPIO_ODR_HIGH ((volatile unsigned char *) (GPIO_D+0x15))

//E-Portar
#define PORT_BASE 0x40021000
#define GPIO_E_MODER ((volatile unsigned int*) (PORT_BASE))
#define GPIO_E_OTYPER ((volatile unsigned int*) (PORT_BASE+0x4))
#define GPIO_E_OSPEEDR ((volatile unsigned int*) (PORT_BASE+0x8))
#define GPIO_E_PUPDR ((volatile unsigned int*) (PORT_BASE+0xC))
#define GPIO_E_IDRLOW ((volatile unsigned char*) (PORT_BASE+0x10))
#define GPIO_E_IDRHIGH ((volatile unsigned char*) (PORT_BASE+0x11))
#define GPIO_E_ODRLOW ((volatile unsigned char*) (PORT_BASE+0x14))
#define GPIO_E_ODRHIGH ((volatile unsigned char*) (PORT_BASE+0x14+1))

// Styrregister bitar
#define B_E 0x40 	// Enable
#define B_SELECT 4  // Select ASCII-display
#define B_RW 2 		// 0=Write, 1=Read
#define B_RS 1 		// 0=Control, 1=Data

    
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void ) {
    __asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n");					/* call main */
    __asm__ volatile(".L1: B .L1\n");				/* never return */
}

__attribute__((naked))
void graphic_initalize(void){
	__asm volatile (" .HWORD 0xDFF0 \n");
	__asm volatile ("BX LR\n");
}

__attribute__((naked))
	void graphic_clear_screen(void){
	__asm volatile(" .HWORD 0xDFF1 \n");
	__asm volatile("BX LR \n");
}
	
__attribute__((naked))
void graphic_pixel_set(int x, int y){
	__asm volatile(" .HWORD 0xDFF2 \n");
	__asm volatile("BX LR \n");
}
	
__attribute__((naked))
	void graphic_pixel_clear(int x, int y){
	__asm volatile(" .HWORD 0xDFF3 \n");
	__asm volatile("BX LR \n");
}
	
/*---------------Fördröjningsrutinerna----------------------*/
void delay_250ns( void ) {
	*STK_CTRL = 0;
	*STK_LOAD = ( (168/4) -1 );
	*STK_VAL = 0;
	*STK_CTRL = 5;

	while( (*STK_CTRL & 0x10000 )== 0 );
	*STK_CTRL = 0;
}
void delay_micro(unsigned int us) {
	//#ifdef SIMULATOR
	us = us / 1000;
	us++;
	//#endif
	
	while( us > 0 ) {
	delay_250ns();
	delay_250ns();
	delay_250ns();
	delay_250ns();
	us--;
	}
}
void delay_milli( unsigned int ms ) {
	delay_micro(1000); 
}




/*----------------------------APP_INIT----------------------*/
void app_init(void) {
	*((volatile unsigned int* ) GPIO_E_MODER) = 0x55555555;
	*((volatile unsigned int*) GPIO_MODER) = 0x55005555;
	*((volatile unsigned short*) GPIO_OTYPER) = 0x0000;
	*((volatile unsigned int*) GPIO_PUPDR) = 0x00AA0000;
}




/*----------Keyboard declarations-----------------------------*/

int kbdGetCol ( void );
void kbdActivate( unsigned int row );
unsigned char keyb(void);

/*----------------Alfanumerisk display--------------------------*/
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
// Adressering med "increment". Ingen skift av adressbussen.
void ascii_init(void){
	while((ascii_read_status() & 0x80)==0x80){} 		// VÃ¤nta tills statusflaggan = 0
	delay_micro(8);		 	// Latency tid fÃ¶r kommando
	ascii_write_cmd(0x38); 			// 2 rader, 5x8 punkter (0011 1000)
	delay_micro(40);		 // FÃ¶rdrÃ¶j 40 us
	ascii_write_cmd(0x0E);		 // TÃ¤nd display,TÃ¤nd markÃ¶r,ingen blink (0000 1110)
	delay_micro(40);		 // FÃ¶rdrÃ¶j 40 us
	ascii_write_cmd(0x04);		 // Increment (ID) = 0, Skift (SH) = 0 (0000 0100)
	delay_micro(40); 		// FÃ¶rdrÃ¶j 40 us
} 
//Clear display
void clear_display(void){
	while((ascii_read_status() & 0x80)==0x80){} 		// VÃ¤nta tills statusflaggan = 0
	delay_micro(8); 		// Latency tid fÃ¶r kommando
	ascii_write_cmd(1); 		// Clear display
	delay_milli(2); // delay 2 milisekunder
}

void ascii_write_char(unsigned char c){
	while((ascii_read_status() & 0x80)==0x80){} 		// Vänta tills statusflaggan = 0
	delay_micro(8); 		// Latency tid för kommando
	ascii_write_data(c); 		// Skriver char till display
	delay_micro(45);		 // Fördröj 45 
}

void ascii_gotoxy(int row,int column){
	unsigned char adress;
	adress = row - 1;
	if (column==2){
	adress = adress + 0x40;
	}
	ascii_write_cmd(0x80 | adress);
}



/*------------STRUCTS--------------------------------*/

#define MAX_POINTS 400

typedef struct {
    char x,y;
}POINT, *PPOINT;

typedef struct {
    int numpoints;
    int sizex;
    int sizey;
    POINT px[ MAX_POINTS ];
}GEOMETRY, *PGEOMETRY;

typedef struct tObj {
    PGEOMETRY geo;
    int dirx,diry;
    int posx,posy;
    void (* draw) (struct tObj *);
    void (*clear) (struct tObj *);
    void (* move) (struct tObj *);
    void (* set_speed) (struct tObj *, int, int);
}OBJECT, *POBJECT;

void draw_object(POBJECT o);
void clear_object(POBJECT o);
void move_object(POBJECT o);
void move_dino(POBJECT o);
void set_object_speed(POBJECT o, int speedx, int speedy);
void move_back(POBJECT o);
void objects_contact(POBJECT o1, POBJECT o2);

//Dinosaurie
GEOMETRY dino_geometry=
{
    157,    /*numpoints*/
    20,20,    /*sizex,sizey*/
    {
            /*  px[1,2,3 .....]  */
    {0,6},{0,7},{0,8},{0,9},{0,10},{0,11}, //x=0
	{1,8},{1,9},{1,10},{1,11},{1,12},		//x=1
	{2,9},{2,10},{2,11},{2,12},{2,13},		//x=2...
	{3,10},{3,11},{3,12},{3,13},{3,14},	
	{4,10},{4,11},{4,12},{4,13},{4,14},{4,15},
	{5,9},{5,10},{5,11},{5,12},{5,13},{5,14},{5,15},{5,16},{5,17},{5,18},{5,19},
	{6,8},{6,9},{6,10},{6,11},{6,12},{6,13},{6,14},{6,15},{6,16},{6,17},{6,19},
	{7,8},{7,9},{7,10},{7,11},{7,12},{7,13},{7,14},{7,15},{7,16},
	{8,7},{8,8},{8,9},{8,10},{8,11},{8,12},{8,13},{8,14},{8,15},
	{9,7},{9,8},{9,9},{9,10},{9,11},{9,12},{9,13},{9,14},{9,15},{9,16},
	{10,6},{10,7},{10,8},{10,9},{10,10},{10,11},{10,12},{10,13},{10,14},{10,15},{10,16},{10,17},{10,18},{10,19},
	{11,1},{11,2},{11,3},{11,4},{11,5},{11,6},{11,7},{11,8},{11,9},{11,10},{11,11},{11,12},{11,13},{11,14},{11,19},
	{12,0},{12,1},{12,2},{12,3},{12,4},{12,5},{12,6},{12,7},{12,8},{12,9},{12,10},{12,11},{12,12},{12,13},
	{13,0},{13,2},{13,3},{13,4},{13,5},{13,6},{13,7},{13,8},{13,9},{13,10},{13,11},
	{14,0},{14,1},{14,2},{14,3},{14,4},{14,7},
	{15,0},{15,1},{15,2},{15,4},{15,7},{15,8},
	{16,0},{16,1},{16,2},{16,4},
	{17,0},{17,1},{17,2},{17,4},
	{18,0},{18,1},{18,2},{18,4},
	{19,1},{19,2},
    }
};

static OBJECT dino=
{
    &dino_geometry,
    0,0,
    20,30,
    draw_object,
    clear_object,
    move_dino,
    set_object_speed,
}; 
//Träd/Kaktus
GEOMETRY tree_geometry=
{
    103,    /*numpoints*/
    16,13,    /*sizex,sizey*/
    {
            /*  px[1,2,3 .....]  */
    {0,5},{0,6},{0,7},{0,8},
	{1,4},{1,5},{1,6},{1,7},{1,8},{1,9},
	{2,5},{2,6},{2,7},{2,8},{2,9},{2,10},
	{3,8},{3,9},{3,10},
	{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{4,9},{4,10},{4,11},{4,12},{4,13},{4,14},{4,15},
	{5,0},{5,1},{5,2},{5,3},{5,4},{5,5},{5,6},{5,7},{5,8},{5,9},{5,10},{5,11},{5,12},{5,13},{5,14},{5,15},
	{6,0},{6,1},{6,2},{6,3},{6,4},{6,5},{6,6},{6,7},{6,8},{6,9},{6,10},{6,11},{6,12},{6,13},{6,14},{6,15},
	{7,1},{7,2},{7,3},{7,4},{7,5},{7,6},{7,7},{7,8},{7,9},{7,10},{7,11},{7,12},{7,13},{7,14},{7,15},
	{8,6},{8,7},{8,8},
	{9,6},{9,7},{9,8},
	{10,3},{10,4},{10,5},{10,6},{10,7},{10,8},
	{11,2},{11,3},{11,4},{11,5},{11,6},{11,7},
	{12,3},{12,4},{12,5},{12,6},
    }
};

static OBJECT tree=
{
    &tree_geometry,
    -8,0,
    128,32,
    draw_object,
    clear_object,
    move_object,
    set_object_speed,
};

/*------------------Globals--------------------------------*/
int tree_dead = 0;		//Om trädet har åkt utanför displayen (x>1) så räknas därför trädet som "dött" och ett nytt genereras då tree_dead = 1.
int game_over = 0;		//När dinosaurien nuddar ett träd så s'tts game_over = 1, vilket stoppar while-loopen som kör spelet.

char jump_up = 3;		//Dessa används för att få dinosuriens hopp att forma en båge, för att 
char wait = 0;			// få en känsla av gravitation, itsället för att dinosaurien flyger
char jump_down = 0;		//upp och ner med en konstant hastighet.
char done = 1;			// Mer om det i funktionen move_dino.

/*----------------------------------------------------------*/

void main(void)
{
	POBJECT dinosaur = &dino;
	POBJECT the_tree = &tree;
	
	graphic_initalize();
	graphic_clear_screen();
	
	
	char *s;
	char t1[] = "Score: ";
	char t2[] = "00001"; //48-57

	app_init();
	ascii_init();
	
	ascii_gotoxy(1,1);
	s=t1;
	while(*s){
		ascii_write_char(*s++);
	}
	dinosaur->draw(dinosaur);
	
	
	while (game_over == 0) {
		
		//Öka pängräknaren med 1.
		ascii_gotoxy(1,2);
		s=t2;
		while (*s) {
			ascii_write_char(*s++);
		}
		
		//Kolla om dinousaurien ska hoppa
		dinosaur->move(dinosaur);
		
		//Flytta trädet
		the_tree->move(the_tree);
		
		//Sök efter kontakt
		objects_contact(dinosaur, the_tree);
		
		//Om trädet är utanför skärmen, flytta tillbaka det
		if(tree_dead == 1) {
			move_back(the_tree);
			tree_dead = 0;
		}
		
		
		//Funktion för att öka poängen med ascii-kod.
		if (t2[4] == 57) {
			t2[3]++;
			t2[4] = 48;
			if (t2[3] == 57) {
				t2[2]++;
				t2[3] = 48;
				if (t2[2] == 57) {
					t2[1]++;
					t2[2] = 48;
					if (t2[1] == 57) {
						t2[0]++;
						t2[1] = 48;
					}	
				}
			}
		} 
		else {
			t2[4]++;
		}
			
			
	}
}


//----------------------Instruktioner för object------------------------

//Ritar objectet
void draw_object(POBJECT o){
            char p= o->geo->numpoints;
            for(int i=0; i < p ;i++){
                graphic_pixel_set(o->geo->px[i].x + o->posx,o->geo->px[i].y + o->posy);
                }
        }

//Suddar objectet
void clear_object(POBJECT o){
            char p= o->geo->numpoints;
            for(int i=0;i<p;i++){
                graphic_pixel_clear(o->geo->px[i].x + o->posx,o->geo->px[i].y + o->posy);
                }
}

//Sätter snabbheten på hur objectet rör sig.
void set_object_speed(POBJECT o, int speedx, int speedy) {
    o->dirx=speedx;
    o->diry=speedy;
}

// Endast trädet använder denna funktion, dinosaurien använder istället move_dino, eftersom trädet och dinosaurien rör sig på olika sätt.
void move_object(POBJECT o){

	clear_object(o);
	
	if (o->posx >= 1) {
		o->posx += o->dirx;
		o->posy += o->diry;
		draw_object(o);
	}else{
		tree_dead = 1;
	}
} 

//Denna funktion gör så att dinosaurien hoppar. Funktionen kollar om 1 är nedtryckt på keypaden samt om tidigare hopp-sekvens är klar. Om
//båda dessa krav är uppfyllda påbörjas ett nytt hopp. 
void move_dino(POBJECT o){
	char c;
	c = keyb();
	if (c == 1 || done == 0) {	//Är 1 nedtryckt eller om dinosaurien inte är nere på marken igen? Om ja: påbörja ett nytt hopp/fortsätt på tidigare hopp.
		done = 0;	
		if (jump_up > 0) { //Jump_up är en global varibel som sedan tidigare är satt till 3. Så dinosaurien kommer att hoppa upp 3 gånger. 21 pixlar första gången, sedan
			clear_object(o);// 14, och sist 7. Detta ger hopppet en form av en båga, istället för att hoppa rakt upp.
			o->posy = o->posy - (7 * jump_up);
			draw_object(o);
			if (jump_up = 1) {	//efter sista "delen" i hoppet sätts den globala variabeln wait till 3. Detta görs för att göra spelet lite lättare.
				wait = 3;		//Dinosaurien stannar alltså i hoppets högsta punkt en liten stund (tre varv i while loopen).
			}
			jump_up --;
		}else if (wait > 0) {	//När väntan är klar sätts den globala variabeln jump_down till 3. 
			if (wait == 1){
			jump_down = 3;
			}
			wait --;
		} else if (jump_down > 0) {	//Nu upprepas samma process som i jump_up, fast nedåt.
			clear_object(o);
			o->posy = o->posy + (7 * jump_down);
			draw_object (o);
			if (jump_down = 1) {
				jump_up = 3;
				done = 1;		//Done sätts till 1, vilket gör så att ett nytt hopp inte påbörjas förutom om kepaden står på 1.
			}
			jump_down --;
		}
		}
	}
	
//Flyttar tillbaka trädet så att det ser ut som att det kommer nya träd.
void move_back(POBJECT o) {
	o->posx = 128;
	o->posy = 32;
}


void objects_contact(POBJECT o1, POBJECT o2) {
  int offset1x = o1->posx;
  int offset1y = o1->posy;
  int offset2x = o2->posx;
  int offset2y = o2->posy;
	//Nedan gjorde vi så att endast en pixel på dinosaurien (specifikt dinosauriens klo) triggar en kollision med trädet. Annars hade det tagit 150 gånger så lång tid, då dinosaurien består av 150 pixlar. 
    //Därför kan det ibland se ut som att dinosaurien borde dör, men överlever ändå. Detta är eftersom endasst klon triggar kollision.
	for (int j = 0; j < o2-> geo->numpoints; j++)
      if ((offset1x + o1->geo->px[142].x == offset2x + o2->geo->px[j].x) &&
        (offset1y + o1->geo->px[142].y == offset2y + o2->geo->px[j].y)) {
			game_over = 1;
		}
}


/*--------------------KEYB----------------------------*/

void kbdActivate( unsigned int row ) { /* Aktivera angiven rad hos tangentbordet, eller
* deaktivera samtliga */
	switch( row ) {
		case 1: *GPIO_ODR_HIGH = 0x10; break;
		case 2: *GPIO_ODR_HIGH = 0x20; break;
		case 3: *GPIO_ODR_HIGH = 0x40; break;
		case 4: *GPIO_ODR_HIGH = 0x80; break;
		case 0: *GPIO_ODR_HIGH = 0x00; break;
	}
}
int kbdGetCol ( void ) { /* Om någon tangent (i aktiverad rad)
* är nedtryckt, returnera dess kolumnnummer,
* annars, returnera 0 */
	unsigned char c;
	c = *GPIO_IDR_HIGH;
	if ( c & 0x8 ) return 4;
	if ( c & 0x4 ) return 3;
	if ( c & 0x2 ) return 2;
	if ( c & 0x1 ) return 1;
	return 0;
}
unsigned char keyb(void) {
	unsigned char key[]={1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD};
    int row, col;
	for (row=1; row <=4 ; row++ ) {
		kbdActivate( row );
		if( (col = kbdGetCol () ) ) {
			kbdActivate( 0 );
			return key [4*(row-1)+(col-1) ];
        }
    }
    kbdActivate( 0 );
    return 0xFF;
}