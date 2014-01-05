#ifndef _LIGHT_H__
#define _LIGHT_H__
//#include <stdint.h>
//#include "platform.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus  

#define TURN_ON 1
#define TURN_OFF 0
#define LIGHT_TURN_ON_OFF  1
#define LIGHT_R_NUM 1
#define LIGHT_G_NUM 2
#define LIGHT_B_NUM 3
#define LIGHT_LEVEL   14
#define LIGHT_TIMER_SWTH 15

 
#define LIGHT_REGISTER 1



#define TIMER1_INIT()   \
   do {                 \
      T1CTL  = 0x00;    \
      T1CCTL0 = 0x00;   \
      T1CCTL1 = 0x00;   \
      T1CCTL2 = 0x00;   \
      TIMIF &= ~0x40;   \
   } while (0)
// Macro for enabling or disabling overflow interrupts of timer 1.
#define TIMER1_ENABLE_OVERFLOW_INT(val) \
   (TIMIF =  (val) ? TIMIF | 0x40 : TIMIF & ~0x40)

#define PWM_Period_Low 0xFF
#define PWM_Period_Hight 0x00

#define TIMER34_INIT(timer)   \
   do {                       \
      T##timer##CTL   = 0x06; \
      T##timer##CCTL0 = 0x00; \
      T##timer##CC0   = 0x00; \
      T##timer##CCTL1 = 0x00; \
      T##timer##CC1   = 0x00; \
   } while (0)

#define TIMER1_SET_PWM_MODLE(channel,val)   \
do {                       \
      T1CCTL##channel##   = val; \
} while (0)


#define TIMER1_SET_PWM_LENGTH(channel,val)   \
do {                       \
      T1CC##channel##L   = val; \
      T1CC##channel##H   = val>>8; \
} while (0)

#define TIMER1_SET_PWM_INTERRUPT_PERIOD(val)   \
do {                       \
      T1CC0L   = val; \
      T1CC0H   = val>>8; \
} while (0)

#define TIMER_SET_COUNTER_REGISTER(val)   \
do {                       \
      T1CNTL   = val; \
      T1CNTH   = val>>8; \
} while (0)
  
#endif

void Init_T1_PWM(void);
unsigned int RGB_PWM_FF_00(unsigned int *v);
void TIMER1_SET_PWM_LENGTH_RGB(unsigned int *v);
