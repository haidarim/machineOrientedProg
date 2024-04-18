void kbdActivate( unsigned int row );
void app_init(void);
void startup ( void );
void out7seg(unsigned char c);
unsigned short keyb_alt_ctrl(void);
unsigned char is_numeric(unsigned short keyb_status);


#define GPIO_D 0x40020C00
#define GPIO_MODER  ((volatile unsigned int *) (GPIO_D)) 
#define GPIO_OTYPER  ((volatile unsigned short *) (GPIO_D+0x4)) 
#define GPIO_PUPDR ((volatile unsigned int *) (GPIO_D+0xC)) 
#define GPIO_IDR_LOW ((volatile unsigned char *) (GPIO_D+0x10)) 
#define GPIO_IDR_HIGH  ((volatile unsigned char *) (GPIO_D+0x11)) 
#define GPIO_ODR_LOW ((volatile unsigned char *) (GPIO_D+0x14)) 
#define GPIO_ODR_HIGH ((volatile unsigned char *) (GPIO_D+0x15))   

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");              /* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");                                 /* call main */
__asm__ volatile(".L1: B .L1\n");                               /* never return */
}

void main(void)
{
   unsigned short keyb_status;
   unsigned char c;
   app_init();

   while( 1 )
   {
       keyb_status = keyb_alt_ctrl();
       c = is_numeric(keyb_status);
       if(c != 0xFF)
           out7seg(c);
   }
}

void app_init(void)
{

*((volatile unsigned int*) GPIO_MODER) = 0x55005555;
*((volatile unsigned short*) GPIO_OTYPER) = 0x0000;
*((volatile unsigned int*) GPIO_PUPDR) = 0xAAAA0000;

}
void kbdActivate( unsigned int row )
{ /* Aktivera angiven rad hos tangentbordet, eller
* deaktivera samtliga */
switch( row )
{
case 1: *GPIO_ODR_HIGH = 0x10; break;
case 2: *GPIO_ODR_HIGH = 0x20; break;
case 3: *GPIO_ODR_HIGH = 0x40; break;
case 4: *GPIO_ODR_HIGH = 0x80; break;

}
}

unsigned short keyb_alt_ctrl(void)
{
unsigned short keyb_state = 0;

    int p=1;
    for(int i=4;i>=1;i--){
        kbdActivate(i);
        unsigned short temp=*GPIO_IDR_HIGH * p;
        keyb_state=keyb_state+temp;

        p=p*2;
        p=p*2;
        p=p*2;
        p=p*2;

    }
    return keyb_state;
}           


void out7seg(unsigned char c){
    char segcodes[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
    if(c<10){
        *GPIO_ODR_LOW=segcodes[c];
    }
    else{
        *GPIO_ODR_LOW=0;
    }
}

unsigned char is_numeric(unsigned short keyb_status){
    if(keyb_status & 0x2) return 0;
    if(keyb_status & 0x1000) return 1;
    if(keyb_status & 0x2000) return 2;
    if(keyb_status & 0x4000) return 3;
    if(keyb_status & 0x100) return 4;
    if(keyb_status & 0x200) return 5;
    if(keyb_status & 0x400) return 6;
    if(keyb_status & 0x10) return 7;
    if(keyb_status & 0x20) return 8;
    if(keyb_status & 0x40) return 9;
return 0xFF;
}
