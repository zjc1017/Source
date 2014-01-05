struct TEST_FLAG{       // bits   description
    uint16_t  flag1:1;     // 0   KEY2 FLAG
    uint16_t  flag2:1;     // 1    SEQ2 state
    uint16_t  flag3:1;     // 2      reserved
    uint16_t  flag4:1;     // 3  Sequencing counter status 
    uint16_t  rsvd:11;         //4--9  reserved  
}testf1g;
struct light_flag
{
    uint16_t flg1: 1;
    uint16_t flg2: 1;
    uint16_t rsvd: 14;
}light_flg;
uint8_t Serial_Test1[]     = "Match is ok";
uint8_t Serial_Test2[]     = "Match is not ok";
uint16_t func_pamter[64];
//LIGHT PARAMETER
uint16_t  R_PWM_VALUE;
uint16_t  G_PWM_VALUE;
uint16_t  B_PWM_VALUE;
uint16_t func_r_backup;
uint16_t func_g_backup;
uint16_t func_b_backup;
//uint16_t rgb_backup[4] = {0,0,0,0};
uint16_t rgb[4] = {0,0,0};
uint8_t serial_test[32];

//SYSTEM PARAMETER
uint16_t timer_counter ;// unit second

#define SERIALAPP_LIGHT_EVT 0x0800
#define SERIALAPP_TIMER_EVT 0x1000
#define SERIALAPP_HEARTBEAT_EVT 0X2000
#define SERIALAPP_LIGHT_TaskID 0x01
#define SERIALAPP_TIMER_TaskID 0x02

#define SERIALAPP_LIGHT_INTERVAL  10
#define SERIALAPP_TIMER_INTERVAL  1000  //1000ms

#define ten_sec 10   //10 second interval


uint8_t zb_internet_state;//connected and unconnected
#define zb_internet_connected 1
#define zb_internet_unconnected 0



//#define router_model   1