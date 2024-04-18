/*
 * startup.c
 *
 */
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");  /* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n"); /* call main */
__asm__ volatile(".L1: B .L1\n"); /* never return */
}


#define STK_CTRL ((volatile unsigned int *)(0xE000E010)) 
#define STK_LOAD ((volatile unsigned int *)(0xE000E014)) 
#define STK_VAL  ((volatile unsigned int *)(0xE000E018)) 
#define SIMULATOR

void graphic_initalize(void);
void graphic_pixel_set(int x, int y);
void graphic_pixel_clear(int x, int y);
void graphic_clear_screen(void);


__attribute__((naked))
void graphic_initalize(void)
{
    __asm volatile (" .HWORD 0xDFF0\n");
    __asm volatile (" BX LR\n");

}

__attribute__((naked))
void graphic_pixel_set(int x, int y)
{
    __asm volatile (" .HWORD 0xDFF2\n");
    __asm volatile (" BX LR\n");

}

__attribute__((naked))
void graphic_clear_screen(void)
{
    __asm volatile (" .HWORD 0xDFF1\n");
    __asm volatile (" BX LR\n");

}

__attribute__((naked))
void graphic_pixel_clear(int x, int y)
{
    __asm volatile (" .HWORD 0xDFF3\n");
    __asm volatile (" BX LR\n");

}

#define GPIO_D 0x40020C00
#define GPIO_MODER  ((volatile unsigned int *) (GPIO_D)) 
#define GPIO_OTYPER  ((volatile unsigned short *) (GPIO_D+0x4)) 
#define GPIO_PUPDR ((volatile unsigned int *) (GPIO_D+0xC)) 
#define GPIO_IDR_LOW ((volatile unsigned char *) (GPIO_D+0x10)) 
#define GPIO_IDR_HIGH  ((volatile unsigned char *) (GPIO_D+0x11)) 
#define GPIO_ODR_LOW ((volatile unsigned char *) (GPIO_D+0x14)) 
#define GPIO_ODR_HIGH ((volatile unsigned char *) (GPIO_D+0x15))

void delay_250ns( void );
void delay_micro(unsigned int us);
void delay_milli(unsigned int ms);
void app_init(void);unsigned char keyb(void);
int kbdGetCol ( void );
void kbdActivate( unsigned int row );

/*------------STRUCTS--------------------------------*/

#define MAX_POINTS 30

typedef struct {
	char x,y;
	}POINT, *PPOINT;

typedef struct {
    int numpoints;
    int sizex;
    int sizey;
    POINT px[ MAX_POINTS ];
	}GEOMETRY, *PGEOMETRY;

typedef struct tObj{
    PGEOMETRY geo;
    int dirx,diry;
    int posx,posy;
    void (* draw) (struct tObj *);
    void (*clear) (struct tObj *);
    void (* move) (struct tObj *);
    void (* set_speed) (struct tObj *, int, int);
	}OBJECT, *POBJECT;

void draw_ballobject(POBJECT o);
void clear_ballobject(POBJECT o);
void move_ballobject(POBJECT o);
void set_ballobject_speed(POBJECT o, int speedx, int speedy);
int objects_overlap(POBJECT o1, POBJECT o2);
int objects_contact(POBJECT o1, POBJECT o2);

GEOMETRY ball_geometry=
{
    12,    /*numpoints*/
    4,4,    /*sizex,sizey*/
    {
            /*  px[1,2,3 .....]  */
    {101,30},{102,30},{100,31},{101,31},{102,31},{103,31},{100,32},{101,32},{102,32},{103,32},{101,33},{102,33}
    }
};

static OBJECT ball=
{
    &ball_geometry,
    0,0,
    4,4,
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_ballobject_speed,
};

GEOMETRY spider_geometry=
{
    22,    /*numpoints*/
    6,8,    /*sizex,sizey*/
    {
            /*  px[1,2,3 .....]  */
    {2,0},{3,0},{1,1},{4,1},{0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{0,3},{2,3},{3,3},{5,3},{1,4},{4,4},{2,5},{3,5},{1,6},{4,6},{0,7},{5,7}
    }
};

static OBJECT spider=
{
    &spider_geometry,
    0,0,
    6,8,
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_ballobject_speed,
};
/*--------------------MAIN--------------------------*/


int main(void)
{
   char c;
   POBJECT victim = &ball;
   POBJECT creture = &spider;
   app_init();
   graphic_initalize();
   graphic_clear_screen();
   victim->set_speed(victim, 4, 1);

   while(1)
   {
       victim->move(victim);
       creture->move(creture);
       c=keyb();
       switch(c)
       {
           case 6: creture->set_speed(creture, 2, 0); break;
           case 4: creture->set_speed(creture, -2, 0); break;
           default: creture->set_speed(creture, 0, 0); break;
           case 2: creture->set_speed(creture, 0, -2); break;
           case 8: creture->set_speed(creture, 0, 2); break;
       }
		if (objects_contact(victim, creture)){
			if(objects_overlap(victim, creture)) {
				//game over
				break;
			}
			delay_micro(500);
		}
   }
}

/* ---------------------- FUNKTIONER ------------------------*/

int objects_contact(POBJECT o1, POBJECT o2) {
	if ((o2->posx) + 4 >= o1->posx &&  o2->posx <= ((o1->posx)+6) && (o2->posy) + 4 >= o1->posy &&  o2->posy <= ((o1->posy)+8)) {
		return 1;
	} else {
		return 0;
	}
}

int objects_overlap(POBJECT o1, POBJECT o2) {
	{
	for(int i=0;i<o1->geo->numpoints;i++) {
		for(int j=0;j<o2->geo->numpoints;j++) {
			if(o1->geo->px[i].x == o2->geo->px[j].x  && o1->geo->px[i].y == o2->geo->px[j].y) {
				return 1;
			}
		}
	}
	}

    return 0;
}

void draw_ballobject(POBJECT o) {
	for(int i=0;i<(o->geo->numpoints);i++) {
        graphic_pixel_set(o->geo->px[i].x + o->posx, o->geo->px[i].y + o->posy);
    }
}

void clear_ballobject(POBJECT o) {
	for(int i=0;i<(o->geo->numpoints);i++) {
		graphic_pixel_clear(o->geo->px[i].x + o->posx, o->geo->px[i].y + o->posy);
	}
}

void move_ballobject(POBJECT o) {
    clear_ballobject(o);
    for(int i=0;i<(o->geo->numpoints);i++) {
		if(o->geo->px[i].x+o->dirx<1) {
		o->dirx=o->dirx*(-1);
		}
    
		if(o->geo->px[i].x+o->dirx>124) {
			o->dirx=o->dirx*(-1);
		}
    
		if(o->geo->px[i].y+o->diry<1) {
			o->diry=o->diry*(-1);
		}
    
		if(o->geo->px[i].y+o->diry>60) {
			o->diry=o->diry*(-1);
		}
    }
	
    for(int i=0;i<(o->geo->numpoints);i++)
    {
        o->geo->px[i].x=o->geo->px[i].x+o->dirx;
        o->geo->px[i].y=o->geo->px[i].y+o->diry;
    }

    draw_ballobject(o);

}

void set_ballobject_speed(POBJECT o, int speedx, int speedy) {
    o->dirx=speedx;
    o->diry=speedy;
}


void app_init(void) {
	*((volatile unsigned int*) GPIO_MODER) = 0x55005555;
	*((volatile unsigned short*) GPIO_OTYPER) = 0x0000;
	*((volatile unsigned int*) GPIO_PUPDR) = 0xAAAA0000;
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



/*--------------------DELAYS----------------------------*/
void delay_250ns( void )
{
/* SystemCoreClock = 168000000 */
*STK_CTRL = 0;
*STK_LOAD = ( (168/4) -1 );
*STK_VAL = 0;
*STK_CTRL = 5;
while( (*STK_CTRL & 0x10000 )== 0 );
*STK_CTRL = 0;
}
void delay_micro(unsigned int us)
{
#ifdef SIMULATOR
us = us / 1000;
us++;
#endif
while( us > 0 )
{
delay_250ns();
delay_250ns();
delay_250ns();
delay_250ns();
us--;
}
}
void delay_milli(unsigned int ms)
{
#ifdef SIMULATOR
ms = ms / 1000;
ms++;
#endif
while( ms > 0 )
{

for(int i = 0; i < 1000; i++){
delay_250ns();
delay_250ns();
delay_250ns();
delay_250ns();
}
ms--;
}
}