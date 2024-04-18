/*
 *      startup.c
 *
 */
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");              /* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");                                 /* call main */
__asm__ volatile(".L1: B .L1\n");                               /* never return */
}
#define STK_CTRL ((volatile unsigned int *)(0xE000E010)) 
#define STK_LOAD ((volatile unsigned int *)(0xE000E014)) 
#define STK_VAL  ((volatile unsigned int *)(0xE000E018)) 



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
#define SIMULATOR

#define LEFT_DOWN 1
#define LEFT_UP 4096
#define RIGHT_DOWN 8
#define RIGHT_UP 32768

void delay_250ns( void );
void delay_micro(unsigned int us);
void delay_milli(unsigned int ms);
void app_init(void);
int kbdGetCol ( void );
void kbdActivate( unsigned int row );
unsigned short keyb_alt_ctrl(void);

/*------------STRUCTS--------------------------------*/

#define MAX_POINTS 30


typedef struct
{
    char x,y;
}POINT, *PPOINT;

typedef struct
{
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
int objects_contact(POBJECT o1, POBJECT o2);

GEOMETRY ball_geometry=
{
    12,    /*numpoints*/
    4,4,    /*sizex,sizey*/
    {
            /*  px[1,2,3 .....]  */
    {0,1},{0,2},{1,0},{1,1},{1,2},{1,3},{2,0},{2,1},{2,2},{2,3},{3,1},{3,2}
    }
};

static OBJECT ball1=
{
    &ball_geometry,
    0,0,
    20,30,
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_ballobject_speed,
};

GEOMETRY pad1_geometry=
{
    27,    /*numpoints*/
    5,9,    /*sizex,sizey*/
    {
            /*  px[1,2,3 .....]  */
    {0,0},{1,0},{2,0},{3,0},{4,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{1,8},{2,8},{3,8},{2,5},{2,3},{2,4}
    }
};

static OBJECT pad1=
{
    &pad1_geometry,
    0,0,
    0,32,
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_ballobject_speed,
};

GEOMETRY pad2_geometry=
{
    27,    /*numpoints*/
    5,9,    /*sizex,sizey*/
    {
            /*  px[1,2,3 .....]  */
    {0,0},{1,0},{2,0},{3,0},{4,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{1,8},{2,8},{3,8},{2,5},{2,3},{2,4}
    }
};

static OBJECT pad2=
{
    &pad2_geometry,
    0,0,
    124,32,
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_ballobject_speed,
};

int main() {
  unsigned short s;
  POBJECT ball = &ball1;
  POBJECT paddle1 = &pad1;
  POBJECT paddle2 = &pad2;

  app_init();
  graphic_initalize();
  graphic_clear_screen();

  ball->set_speed(ball, 4, 1);

  while (1) {
    ball->move(ball);
    paddle1->move(paddle1);
    paddle2->move(paddle2);
    s = keyb_alt_ctrl();

    if ((s &(LEFT_UP | LEFT_DOWN)) == (LEFT_UP | LEFT_DOWN))
      paddle1->set_speed(paddle1, 0, 0);
    else if (s &LEFT_UP)
      paddle1->set_speed(paddle1, 0, -4);
    else if (s &LEFT_DOWN)
      paddle1->set_speed(paddle1, 0, 4);
    else
      paddle1->set_speed(paddle1, 0, 0);

    if ((s &(RIGHT_UP | RIGHT_DOWN)) == (RIGHT_UP | RIGHT_DOWN))
      paddle2->set_speed(paddle2, 0, 0);
    else if (s &RIGHT_UP)
      paddle2->set_speed(paddle2, 0, -4);
    else if (s &RIGHT_DOWN)
      paddle2->set_speed(paddle2, 0, 4);
    else
      paddle2->set_speed(paddle2, 0, 0);

    if (objects_contact(ball, paddle1)) {
      ball->clear(ball);
      ball->dirx = ball->dirx*-1;
      ball->draw(ball);
    } else if (ball->posx < (1 + ball->geo->sizex)) {
      break;
    }

    if (objects_contact(ball, paddle2)) {
      ball->clear(ball);
      ball->dirx = ball->dirx*-1;
      ball->draw(ball);
    } else if (ball->posx > (128 - ball->geo->sizex)) {
      break;
    }
    delay_micro(20000);
  }
}

/* ---------------------- FUNKTIONER ------------------------*/

int objects_contact(POBJECT o1, POBJECT o2)
{
  int offset1x = o1->posx;
  int offset1y = o1->posy;
  int offset2x = o2->posx;
  int offset2y = o2->posy;
  for (int i = 0; i < o1->geo->numpoints; i++) {
    for (int j = 0; j < o2-> geo->numpoints; j++)
      if ((offset1x + o1->geo->px[i].x == offset2x + o2->geo->px[j].x) &&
        (offset1y + o1->geo->px[i].y == offset2y + o2->geo->px[j].y)) return 1;
  }
  return 0;
}

void draw_ballobject(POBJECT o){
            char p= o->geo->numpoints;
            for(int i=0; i < p ;i++){
                graphic_pixel_set(o->geo->px[i].x + o->posx,o->geo->px[i].y + o->posy);
                }
        }
void clear_ballobject(POBJECT o){
            char p= o->geo->numpoints;
            for(int i=0;i<p;i++){
                graphic_pixel_clear(o->geo->px[i].x + o->posx,o->geo->px[i].y + o->posy);
                }
}

void move_ballobject(POBJECT o){

	clear_ballobject(o);

    if(o->posx < 1 || o->posx + o->geo->sizex > 128) {
	o->dirx *= -1;
	}
    if(o->posy < 1 || o->posy + o->geo->sizey > 64) {
	o->diry *= -1;
	}
	

	o->posx += o->dirx;
	o->posy += o->diry;
    draw_ballobject(o);
    } 

void set_ballobject_speed(POBJECT o, int speedx, int speedy)
{
    o->dirx=speedx;
    o->diry=speedy;
}

void app_init(void)
{

*((volatile unsigned int*) GPIO_MODER) = 0x55005555;
*((volatile unsigned short*) GPIO_OTYPER) = 0x0000;
*((volatile unsigned int*) GPIO_PUPDR) = 0x00AA0000;

}

/*--------------------KEYB----------------------------*/
void kbdActivate( unsigned int row )
{ /* Aktivera angiven rad hos tangentbordet, eller
* deaktivera samtliga */
switch( row )
{
case 1: *GPIO_ODR_HIGH = 0x10; break;
case 2: *GPIO_ODR_HIGH = 0x20; break;
case 3: *GPIO_ODR_HIGH = 0x40; break;
case 4: *GPIO_ODR_HIGH = 0x80; break;
case 5: *GPIO_ODR_HIGH = 0xF0; break;
case 0: *GPIO_ODR_HIGH = 0x00; break;
}
}


int kbdGetCol ( void )
{ /* Om någon tangent (i aktiverad rad)
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

unsigned short keyb_alt_ctrl(void)
{
unsigned char key[]={1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD};
unsigned short keyb_state = 0;
int row, col;

    int up=1;
    for(int i=4;i>=1;i--){
        kbdActivate(i);
        unsigned short temp=*GPIO_IDR_HIGH * up;
        keyb_state=keyb_state+temp;

        up=up*2;
        up=up*2;
        up=up*2;
        up=up*2;

    }
    return keyb_state;
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