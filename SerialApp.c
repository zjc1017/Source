/**************************************************************************************************
  Filename:       SerialApp.c
  Revised:        $Date: 2009-03-29 10:51:47 -0700 (Sun, 29 Mar 2009) $
  Revision:       $Revision: 19585 $

  Description -   Serial Transfer Application (no Profile).


  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 鈥淎S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

/*********************************************************************
  This sample application is basically a cable replacement
  and it should be customized for your application. A PC
  (or other device) sends data via the serial port to this
  application's device.  This device transmits the message
  to another device with the same application running. The
  other device receives the over-the-air message and sends
  it to a PC (or other device) connected to its serial port.
				
  This application doesn't have a profile, so it handles everything directly.

  Key control:
    SW1:
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "SerialApp.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"

/* myself code */
#include "zb_msg.h"
#include "SerialApp.h"
#include "Sys_Test.h"
#include "string.h"
#include "light.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#if !defined( SERIAL_APP_PORT )
#define SERIAL_APP_PORT  0
#endif

#if !defined( SERIAL_APP_BAUD )
#define SERIAL_APP_BAUD  HAL_UART_BR_38400
//#define SERIAL_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_APP_THRESH )
#define SERIAL_APP_THRESH  64
#endif

#if !defined( SERIAL_APP_RX_SZ )
#define SERIAL_APP_RX_SZ  128
#endif

#if !defined( SERIAL_APP_TX_SZ )
#define SERIAL_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_APP_IDLE )
#define SERIAL_APP_IDLE  6
#endif

// Loopback Rx bytes to Tx for throughput testing.
#if !defined( SERIAL_APP_LOOPBACK )
#define SERIAL_APP_LOOPBACK  FALSE
#endif

// This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX )
#define SERIAL_APP_TX_MAX  80
#endif

#define SERIAL_APP_RSP_CNT  4

// This list should be filled with Application specific Cluster IDs.
const cId_t SerialApp_ClusterList[SERIALAPP_MAX_CLUSTERS] =
{
  SERIALAPP_CLUSTERID1,
  SERIALAPP_CLUSTERID2
};

const SimpleDescriptionFormat_t SerialApp_SimpleDesc =
{
  SERIALAPP_ENDPOINT,              //  int   Endpoint;
  SERIALAPP_PROFID,                //  uint16 AppProfId[2];
  SERIALAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SERIALAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SERIALAPP_FLAGS,                 //  int   AppFlags:4;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SerialApp_ClusterList,  //  byte *pAppInClusterList;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumOutClusters;
  (cId_t *)SerialApp_ClusterList   //  byte *pAppOutClusterList;
};

const endPointDesc_t SerialApp_epDesc =
{
  SERIALAPP_ENDPOINT,
 &SerialApp_TaskID,
  (SimpleDescriptionFormat_t *)&SerialApp_SimpleDesc,
  noLatencyReqs
};

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8 SerialApp_TaskID;    // Task ID for internal task/event processing.

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 SerialApp_MsgID;

static afAddrType_t SerialApp_TxAddr;
//static uint8 SerialApp_TxSeq;
static uint8 SerialApp_TxBuf[SERIAL_APP_TX_MAX+1];
static uint8 SerialApp_TxLen;

static afAddrType_t SerialApp_RxAddr;
//static uint8 SerialApp_RxSeq;
static uint8 SerialApp_RspBuf[SERIAL_APP_RSP_CNT];

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SerialApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
static void SerialApp_HandleKeys( uint8 shift, uint8 keys );
static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
static void SerialApp_Send(void);
static void SerialApp_Resp(void);
static void SerialApp_CallBack(uint8 port, uint8 event);
/*********************************************************************
 * @fn      SerialApp_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
void SerialApp_Init( uint8 task_id )
{
  halUARTCfg_t uartConfig;

  SerialApp_TaskID = task_id;
  //SerialApp_RxSeq = 0xC3;

  afRegister( (endPointDesc_t *)&SerialApp_epDesc );

  RegisterForKeys( task_id );

  uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.baudRate             = SERIAL_APP_BAUD;
  uartConfig.flowControl          = TRUE;
  uartConfig.flowControlThreshold = SERIAL_APP_THRESH; // 2x30 don't care - see uart driver.
  uartConfig.rx.maxBufSize        = SERIAL_APP_RX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.tx.maxBufSize        = SERIAL_APP_TX_SZ;  // 2x30 don't care - see uart driver.
  uartConfig.idleTimeout          = SERIAL_APP_IDLE;   // 2x30 don't care - see uart driver.
  uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
  uartConfig.callBackFunc         = SerialApp_CallBack;
  HalUARTOpen (SERIAL_APP_PORT, &uartConfig);

  //#if defined ( LCD_SUPPORTED )
  //HalLcdWriteString( "SerialApp", HAL_LCD_LINE_2 );
  //#endif
  
  ZDO_RegisterForZDOMsg( SerialApp_TaskID, End_Device_Bind_rsp );
  ZDO_RegisterForZDOMsg( SerialApp_TaskID, Match_Desc_rsp );
  #ifdef DEVICE_BUILD_ROUTER 
  Init_T1_PWM();//Initial PWM
  #endif
  RGB_PWM_FF_00((unsigned int *) rgb);
  TIMER1_SET_PWM_LENGTH_RGB((unsigned int *)rgb);
  osal_start_timerEx( SerialApp_TaskID,
                      SERIALAPP_TIMER_EVT,
                     SERIALAPP_TIMER_INTERVAL);
  //Device Register 

 // osal_set_event(SerialApp_TaskID, SERIALAPP_TIMER_EVT);
  
}

/*********************************************************************
 * @fn      SerialApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events   - Bit map of events to process.
 *
 * @return  Event flags of all unprocessed events.
 */
UINT16 SerialApp_ProcessEvent( uint8 task_id, UINT16 events )
{
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    afIncomingMSGPacket_t *MSGpkt;

    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SerialApp_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
      case ZDO_CB_MSG:
        SerialApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
        break;
          
      case KEY_CHANGE:
        SerialApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
        break;

      case AF_INCOMING_MSG_CMD:
        SerialApp_ProcessMSGCmd( MSGpkt );
        break;

      default:
        break;
      }

      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & SERIALAPP_SEND_EVT )
  {
    SerialApp_Send();
    return ( events ^ SERIALAPP_SEND_EVT );
  }

  if ( events & SERIALAPP_RESP_EVT )
  {
    SerialApp_Resp();
    return ( events ^ SERIALAPP_RESP_EVT );
  }
  if ( events & SERIALAPP_LIGHT_EVT )// 0x0002
  {

      //uint8_t len ;
      //len = 3 * sizeof(uint16_t);
      osal_memcpy(rgb, &func_pamter[LIGHT_R_NUM],3 * sizeof(uint16_t));//*osal_memcpy( void *dst, const void GENERIC *src, unsigned int len )
      rgb[3] = '\0';
      RGB_PWM_FF_00((unsigned int *) rgb);
      TIMER1_SET_PWM_LENGTH_RGB((unsigned int *)rgb);
      //osal_memcpy(rgb_backup, &func_pamter[LIGHT_R_NUM], 3 * sizeof(uint16_t));
      //rgb_backup[3] = '\0';
      osal_memcpy( serial_test, rgb, 4 * sizeof(uint16_t));
      //osal_memcpy( serial_test + 4 * sizeof(uint16_t), rgb_backup, 4 * sizeof(uint16_t));
      //osal_memcpy(serial_test + sizeof(rgb),rgb_backup,sizeof(rgb_backup));
      HalUARTWrite( SERIAL_APP_PORT,serial_test,4 * sizeof(uint16_t));
      //HalUARTWrite( SERIAL_APP_PORT,rgb,4);//Array format test;
      //HalUARTWrite( SERIAL_APP_PORT,rgb_backup,4);//Array format test;
      /*osal_start_timerEx( SerialApp_TaskID,
                        SERIALAPP_LIGHT_EVT,
                        SERIALAPP_LIGHT_INTERVAL);*/
   // }
       return (events ^ SERIALAPP_LIGHT_EVT);
    }
  if ( events & SERIALAPP_TIMER_EVT )// 0x0003 ----register
  {
      
      zAddrType_t txAddr;
      txAddr.addrMode = AddrBroadcast;
      txAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &txAddr, NWK_BROADCAST_SHORTADDR,
                        SERIALAPP_PROFID,
                        SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
                        SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
                        FALSE );   
    /*osal_start_timerEx( SerialApp_TaskID,
                        SERIALAPP_TIMER_EVT,
                        SERIALAPP_TIMER_INTERVAL);*/
       return (events ^ SERIALAPP_TIMER_EVT);
  }

  return ( 0 );  // Discard unknown events.
}

/*********************************************************************
 * @fn      SerialApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
static void SerialApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined(BLINK_LEDS)
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;
      
    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            SerialApp_TxAddr.addrMode = (afAddrMode_t)Addr16Bit;
            SerialApp_TxAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            SerialApp_TxAddr.endPoint = pRsp->epList[0];
            
            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          {  
            #ifdef DEVICE_BUILD_ROUTER
            struct zb_reg_req reg_req;
            int rv;
            char buf[30];
            reg_req.hdr.cmd = ZB_ID_REG_REQ;
            reg_req.hdr.addr = NLME_GetShortAddr();
            reg_req.dev_type = 0x0001;//light
            osal_memcpy(reg_req.ieee_addr, aExtendedAddress, sizeof(aExtendedAddress));
            rv = zb_encode_reg_req(&reg_req, buf, sizeof(reg_req) );
            SerialApp_TxAddr.addrMode = Addr16Bit;
            SerialApp_TxAddr.addr.shortAddr = 0x0000; // Send message to Coordinator
            if ( AF_DataRequest(&SerialApp_TxAddr,(endPointDesc_t *)&SerialApp_epDesc,
                       SERIALAPP_CLUSTERID1,
                       rv, (byte *)buf,
                       &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
            {
             // Successfully requested to be sent.
              testf1g.flag3 = 1;
              //HalUARTWrite( SERIAL_APP_PORT,Serial_Test1,sizeof(Serial_Test1));//match is ok;
            }
            else
           {
           //HalUARTWrite( SERIAL_APP_PORT,Serial_Test2,sizeof(Serial_Test2));//match isn't ok;
           }
           
           #endif
          }
          osal_mem_free( pRsp );
        }
      }
    break;
  }
}

/*********************************************************************
 * @fn      SerialApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys  - bit field for key events.
 *
 * @return  none
 */
void SerialApp_HandleKeys( uint8 shift, uint8 keys )
{
  zAddrType_t txAddr;
  //My code 
   char buf[128];
   //char str[10] = {"China"};
   struct zb_ann_reg msg;
   int rv;
  
  if ( !shift )// ! should be added
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
      // HalLcdWriteString( "KEY1 triggered", HAL_LCD_LINE_4 );
      // osal_start_timerEx( SerialAPP_TaskID,
      //                   SerialAPP_LIGHT_EVT,
      //                   SerialAPP_LIGHT_INTERVAL);
      // zigbee wireless test  ++SerialApp_TxSeq==0x0F
      // #define  len  64
      // uint8_t  Zb_Sigal1[len]="hellow";
      // uint8_t  Zb_Sigal2[len]="just a test";
      // uint8_t  Zb_Sigal3[len];
            //uint8_t  *zb_p ;
            //zb_p = osal_mem_alloc(sizeof(Zb_Sigal1) * 8);//it just a test
      if(testf1g.flag1==1)
      {
        // osal_memcpy(Zb_Sigal3, Zb_Sigal1, sizeof(Zb_Sigal1));
        testf1g.flag1=0;
      }
      else
      {
        // osal_memcpy(Zb_Sigal3, Zb_Sigal2, sizeof(Zb_Sigal2));
        testf1g.flag1=1;
      }
      #ifdef DEVICE_BUILD_ROUTER 
      if(testf1g.flag3 == 1)
      {
          msg.hdr.cmd = ZB_ID_ANN_REG;
          msg.hdr.addr = NLME_GetShortAddr();
          msg.cnt = 0x01;
          msg.item.type =0x02;
          msg.item.id = 0x03;
          strcpy(msg.item.name, "fridge");
          strcpy(msg.item.desc,"voltage");
          msg.item.form = "printf";
          //strcpy(msg.item.form,"printf");
          rv = zb_encode_ann_reg(&msg, buf, sizeof(buf));
          if ( AF_DataRequest(&SerialApp_TxAddr,(endPointDesc_t *)&SerialApp_epDesc,
                       SERIALAPP_CLUSTERID1,
                       rv, (byte *)&buf,
                       &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS) == afStatus_SUCCESS )
          {
            // Successfully requested to be sent.
          }
          else
          {
            // Error occurred in request to send.
          }
      }
      #endif
      // Since SW1 isn't used for anything else in this application...
#if defined( SWITCH1_BIND )
      // we can use SW1 to simulate SW2 for devices that only have one switch,
      keys |= HAL_KEY_SW_2;
#elif defined( SWITCH1_MATCH )
      // or use SW1 to simulate SW4 for devices that only have one switch
      keys |= HAL_KEY_SW_4;
#endif
    }
    if ( keys & HAL_KEY_SW_2 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
      
      // Initiate an End Device Bind Request for the mandatory endpoint
      txAddr.addrMode = Addr16Bit;
      txAddr.addr.shortAddr = 0x0000; // Coordinator
      ZDP_EndDeviceBindReq( &txAddr, NLME_GetShortAddr(), 
                            SerialApp_epDesc.endPoint,
                            SERIALAPP_PROFID,
                            SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
                            SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
       HalUARTWrite( SERIAL_APP_PORT,aExtendedAddress,8);//Array format test;
       osal_start_timerEx(SerialApp_TaskID,
                        SERIALAPP_TIMER_EVT,
                        SERIALAPP_TIMER_INTERVAL);
    }

    if ( keys & HAL_KEY_SW_4 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
      
      // Initiate a Match Description Request (Service Discovery)
      
      //txAddr.addrMode = AddrBroadcast;
      //txAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      txAddr.addrMode = AddrBroadcast;
      txAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &txAddr, NWK_BROADCAST_SHORTADDR,
                        SERIALAPP_PROFID,
                        SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
                        SERIALAPP_MAX_CLUSTERS, (cId_t *)SerialApp_ClusterList,
                        FALSE );
    }
  }
}
/*********************************************************************
 * @fn      get_val_by_id
 *
 * @brief   

* @param   
 *
 * @return  
 */
static int get_val_by_id(uint8_t id,uint16_t *val)
{
	int get_val;
	switch (id) {
		case 0x01:	// temperature
			get_val = *(val+id);
			break;
		case 0x02:	// switch
			get_val = *(val+id);
			break;
		default:
			get_val = *(val+id);
			break;
	}
	return get_val;
}

/*********************************************************************
 * @fn      SerialApp_ProcessMSGCmd
 *
 * @brief   Data message processor callback. This function processes
 *          any incoming data - probably from other devices. Based
 *          on the cluster ID, perform the intended action.
 *
 * @param   pkt - pointer to the incoming message packet
 *
 * @return  TRUE if the 'pkt' parameter is being used and will be freed later,
 *          FALSE otherwise.
 */
void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt )
{
  //uint8 stat;
  //uint8 seqnb;
  //uint8 delay;

  switch ( pkt->clusterId )
  {
  // A message with a serial data block to be transmitted on the serial port.
  case SERIALAPP_CLUSTERID1:
  // Store the address for sending and retrying.
  osal_memcpy(&SerialApp_RxAddr, &(pkt->srcAddr), sizeof( afAddrType_t ));
  //testf1g.flag3 = 0;
  if(testf1g.flag2==1) {
    testf1g.flag2=0;
    HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
  }
  else
  {
    testf1g.flag2=1;
    HalLedSet( HAL_LED_2, HAL_LED_MODE_OFF);
  }
  //print received wireless data
  /* if ( HalUARTWrite( SERIAL_APP_PORT, pkt->cmd.Data, (pkt->cmd.DataLength) ) ) {
  }
  else
  {
    
  } */
  #ifdef DEVICE_BUILD_ROUTER
  {
      int i,j;
      int rv;
      uint8_t id;
      uint8_t header_cmd;
      const uint8_t parameter_num =4 ;//三个功能号
      char buf[32];
      struct zb_get_req get_req;
      struct zb_get_rsp get_rsp;
      struct zb_set_req set_req;
      struct zb_set_rsp set_rsp;
      struct zb_item_pair items[10];
      #define zb_set_rsp_lenth   5//Response format [FF 00 01 83] 00
      header_cmd=pkt->cmd.Data[11];//cmd
      switch (header_cmd) {
      case ZB_ID_GET_REQ:
          rv = zb_decode_get_req(&get_req, (char*)pkt->cmd.Data, pkt->cmd.DataLength );//copy af message to ge_req
          // new item pair array
          //items = osal_malloc(sizeof(item_pair) * req.ids.count);
          // travel
          if(get_req.ids.count == 1 && get_req.ids.id[0] == 0){
              for(i = 1, j = 0; i < parameter_num ; i++,j++){
                id = i;
                items[j].id = i;
                items[j].val = get_val_by_id(id,func_pamter);

              }
              get_req.ids.count = 3;//function[1] function[2] function[3]  

          }
          else
          {
              for (i = 0; i < get_req.ids.count; ++i) {
              id = get_req.ids.id[i];
              items[i].id = id;
              items[i].val = get_val_by_id(id,func_pamter);
              }
          }
          get_rsp.pairs.count = get_req.ids.count;
          get_rsp.pairs.pairs = items;
          get_rsp.hdr.addr = NLME_GetShortAddr();
          rv = zb_encode_get_rsp( &get_rsp, buf, sizeof(buf));
          if (rv <= 0) {
          /*Failure handling procedures code*/ 
          break;//return;
          }
          else{
                 AF_DataRequest(&SerialApp_TxAddr,(endPointDesc_t *)&SerialApp_epDesc,
                               SERIALAPP_CLUSTERID1,
                               rv, (unsigned char *)buf,
                               &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS);
              {
                  osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
              }
          }
          break;//return;
          case ZB_ID_SET_REQ:
            rv = zb_decode_set_req(&set_req, (char*)pkt->cmd.Data , pkt->cmd.DataLength);
            for(i = 0;i < set_req.pairs.count ; i++ )
            {
              id = set_req.pairs.pairs[i].id;
              func_pamter[id] = set_req.pairs.pairs[i].val;
            }
           set_rsp.hdr.cmd = ZB_ID_SET_RSP;
           if (rv <= 0) {
           set_rsp.status = 0x01;//error report
          }
           else{
           set_rsp.status = 0x00;//OK
           }
           set_rsp.hdr.addr =NLME_GetShortAddr();
           rv = zb_encode_set_rsp(&set_rsp, buf, sizeof(set_rsp) );
           AF_DataRequest(&SerialApp_TxAddr,(endPointDesc_t *)&SerialApp_epDesc,
                     SERIALAPP_CLUSTERID1,
                     rv, (unsigned char *)buf,
                     &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS);
           osal_start_timerEx( SerialApp_TaskID,
                        SERIALAPP_LIGHT_EVT,
                        SERIALAPP_LIGHT_INTERVAL);
           
           break;
            default:
      break;
      }
  }
#endif
 break;
  }
}
/*********************************************************************
 * @fn      SerialApp_Send
 *
 * @brief   Send data OTA.
 *
 * @param   none
 *
 * @return  none
 */
static void SerialApp_Send(void)
{
#if SERIAL_APP_LOOPBACK
  if (SerialApp_TxLen < SERIAL_APP_TX_MAX)
  {
    SerialApp_TxLen += HalUARTRead(SERIAL_APP_PORT, SerialApp_TxBuf+SerialApp_TxLen+1,
                                                    SERIAL_APP_TX_MAX-SerialApp_TxLen);
  }

  if (SerialApp_TxLen)
  {
    (void)SerialApp_TxAddr;
    if (HalUARTWrite(SERIAL_APP_PORT, SerialApp_TxBuf+1, SerialApp_TxLen))
    {
      SerialApp_TxLen = 0;
    }
    else
    {
      osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
    }
  }
#else
  if (!SerialApp_TxLen && 
      (SerialApp_TxLen = HalUARTRead(SERIAL_APP_PORT, SerialApp_TxBuf, SERIAL_APP_TX_MAX)))
  {
    // Pre-pend sequence number to the Tx message.
    //SerialApp_TxBuf[0] = ++SerialApp_TxSeq;
  }

  if (SerialApp_TxLen)
  {
    SerialApp_TxAddr.addrMode = Addr16Bit;
    SerialApp_TxAddr.addr.shortAddr =  BUILD_UINT16(SerialApp_TxBuf[13],SerialApp_TxBuf[12]);
    if (afStatus_SUCCESS != AF_DataRequest(&SerialApp_TxAddr,
                                           (endPointDesc_t *)&SerialApp_epDesc,
                                            SERIALAPP_CLUSTERID1,
                                            SerialApp_TxLen, SerialApp_TxBuf,
                                            &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
    {
      osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
    }
    else
    {
        SerialApp_TxLen = 0;
    }
  }
#endif
}

/*********************************************************************
 * @fn      SerialApp_Resp
 *
 * @brief   Send data OTA.
 *
 * @param   none
 *
 * @return  none
 */
static void SerialApp_Resp(void)
{
  if (afStatus_SUCCESS != AF_DataRequest(&SerialApp_RxAddr,
                                         (endPointDesc_t *)&SerialApp_epDesc,
                                          SERIALAPP_CLUSTERID2,
                                          SERIAL_APP_RSP_CNT, SerialApp_RspBuf,
                                         &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
  {
    osal_set_event(SerialApp_TaskID, SERIALAPP_RESP_EVT);
  }
}

/*********************************************************************
 * @fn      SerialApp_CallBack
 *
 * @brief   Send data OTA.
 *
 * @param   port - UART port.
 * @param   event - the UART port event flag.
 *
 * @return  none
 */
static void SerialApp_CallBack(uint8 port, uint8 event)
{
  (void)port;

  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT)) &&
#if SERIAL_APP_LOOPBACK
      (SerialApp_TxLen < SERIAL_APP_TX_MAX))
#else
      !SerialApp_TxLen)
#endif
  {
    SerialApp_Send();
  }
}

/*********************************************************************
*********************************************************************/
