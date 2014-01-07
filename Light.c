#include <string.h>
#include <ioCC2530.h>
#include "platform.h"
#include "light.h"

void Init_T1_PWM(void);
/*****************************************
//T1 as pwm interrupt period
*****************************************/
void Init_T1_PWM(void)
{

    TIMER1_INIT();                  ////initial T1
    //TIMER1_ENABLE_OVERFLOW_INT(1);  // open T1 interrupt 
    //EA = 1;//Enbale all interrupt 
    T1IE   = 1;//Enbale timer1 interrupt 
    PERCFG = 0x03;  //To have all Timer 1 channels visible in alternative 1 location, move both USART 0 and USART 1 to
    P0SEL  |= 0x78; //0 : input 1: output
    P1SEL  |= 0xF0; //P1.7--RX P1.6--TX P1.5--RT P1.4--CT
    P0DIR  |= 0X78; //Set P0_3/P0_4/P0_5/P0_6 as output
    P1DIR  |= 0x70; //P1.7--RX P1.6--TX P1.5--RT P1.4--CT  1:output 0:input
    P2DIR  =  0X80;  //P0 as TIMER1()1st priority: Timer 1 channels 0-1,uart0 uart1 as 2
    /*******************************************
    P2DIR :BIT7-6
    00 USART0 USART1 TIMER1
    01 USART1 USART0 TIMER1
    10 TIMER1(channels0-1) USART1 USART0 TIMER1(channels2-3)
    11 TIMER1(channels2-3) USART0 USART1 TIMER1(channels0-1)

    ********************************************/
  
    TIMER_SET_COUNTER_REGISTER(0x00);

    TIMER1_SET_PWM_INTERRUPT_PERIOD(0xFF);
    //P0.3
    TIMER1_SET_PWM_MODLE(1,0x24);
    TIMER1_SET_PWM_LENGTH(1,0xFF);
    //P0.4
    TIMER1_SET_PWM_MODLE(2,0x24);
    TIMER1_SET_PWM_LENGTH(2,0xFF); 
    ////P0.5
    TIMER1_SET_PWM_MODLE(3,0x24);
    TIMER1_SET_PWM_LENGTH(3,0xFF); 
    ////P0.6
    TIMER1_SET_PWM_MODLE(4,0x24);
    TIMER1_SET_PWM_LENGTH(4,0xFF); 
    T1CTL =0x00;//CLEAR TIMER1 CONTROL REGISTER
    T1CTL |=0x0c; // divider=128
    T1CTL |= 0x03; //UP AND DOWN MODULE
    

};
void TIMER1_SET_PWM_LENGTH_RGB(unsigned int *v){
     TIMER1_SET_PWM_LENGTH(1,*v);     // p0.3
     TIMER1_SET_PWM_LENGTH(2,*(v+1)); //  p0.4
     TIMER1_SET_PWM_LENGTH(3,*(v+2)); //  p0.5

}
     
unsigned int  RGB_PWM_FF_00(unsigned int  *v){
    unsigned int *rgb = NULL;
    int i;
    rgb = v;
    for(i = 0; i < 3 ; i++)
    {
        if((*rgb & 0xFF) == 0xFF)  
        {
            *rgb = 0x00;
        }
        else if ((*rgb & 0xFF) == 0x00) 
        {
            *rgb = 0xFF;
        }
        rgb++;
    }
    return 0;
}