#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include<string.h>
#include "Coordinator.h"
#include "DebugTrace.h"
#include "DHT11.h"
#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL 硬件抽象层(HAL) */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#define SEND_DATA_EVENT  0x02

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList   //  byte *pAppInClusterList;
};

endPointDesc_t GenericApp_epDesc;


byte GenericApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GenericApp_Init() is called.
devStates_t GenericApp_NwkState;


byte GenericApp_TransID;  // This is the unique message ID (counter)

afAddrType_t GenericApp_DstAddr;

//uint8 *my_strcat(uint8 *str1, uint8 *str2);
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void GenericApp_SendTheMessage( void );
//void To_string(uint8 *dest,char *src,uint8 length);
void GenericApp_Init( byte task_id )
{
  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;

  P0SEL &= ~0x10; //初始化P0_4的温度
  P0DIR |= 0x10;   //0:输入  1：输出
  P0_4 = 0;
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  //中断设置为对协调器单播
  GenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  GenericApp_DstAddr.endPoint =GENERICAPP_ENDPOINT;
  GenericApp_DstAddr.addr.shortAddr = 0x00;
 
  // Fill out the endpoint description.
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GenericApp_epDesc );
 
  //串口的设置
  halUARTCfg_t  uartConfig;
  //HalUARTCfg_t uartConfig;
  uartConfig.configured   =TRUE;
  uartConfig.baudRate     =HAL_UART_BR_115200;
  uartConfig.flowControl  =FALSE;
  uartConfig.callBackFunc =NULL;    
  HalUARTOpen(0,&uartConfig);
}

UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  
  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
       /* case ZDO_CB_MSG:
          GenericApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;
          
        case KEY_CHANGE:
          GenericApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;
        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;*/

          //如果接收到消息
        case AF_INCOMING_MSG_CMD:
          GenericApp_MessageMSGCB( MSGpkt );
          break;  

         //加入网络后
        case ZDO_STATE_CHANGE:
          GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if (GenericApp_NwkState == DEV_END_DEVICE) 
          { 
            HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);//加入网络后亮灯
            osal_set_event(GenericApp_TaskID,SEND_DATA_EVENT);
             
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );//释放内存

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }


 if ( events & SEND_DATA_EVENT )
  {
    //Humidity_Measure(); //湿度的获取
    //接受到采集的湿度并发送数据给协调器
    GenericApp_SendTheMessage();
    
    // Setup to send message again
    //定时触发事件
    osal_start_timerEx( GenericApp_TaskID,SEND_DATA_EVENT,3000 );
  return( events & SEND_DATA_EVENT );
  }
  return 0;
}
/*uint8 *my_strcat(uint8 *str1, uint8 *str2)
{    
  uint8 *pt = str1; 
  while(*str1!='\0') str1++;  
  while(*str2!='\0') *str1++ = *str2++; 
  *str1 = '\0'; 
  return pt;
}*/
void GenericApp_SendTheMessage( void )
{ 
    uint8 humidity[26] = "Enddevice2 humidity is:";;
    //uint8 strTemp[26]= "Enddevice3 humidity is:";
    //uint8 data[]={0};
    DHT11();   //温度采集 
    humidity[24]=shidu_shi+0x30;
    humidity[25]=shidu_ge+0x30;
    //humidity[2]='\0';
    //data = my_strcat(strTemp,humidity);
    //osal_memcpy(&strTemp[3],humidity,3);
    //T[0]=wendu_shi+48;
    //T[1]=wendu_ge+48;
    //T[2]=' ';
    //T[3]=shidu_shi+0x30;
    //T[4]=shidu_ge+0x30;
    HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
    /*******串口打印*********/
    Delay_ms(3000);
    //HalUARTWrite(0,"temperature is:",osal_strlen("temperature is:"));
    //HalUARTWrite(0,humidity,26);
    //HalUARTWrite(0,T+1,1);
    //HalUARTWrite(0,"\n",1);*/    
 
    /*HalUARTWrite(0,"humidity is:",osal_strlen("humidity is:"));
    HalUARTWrite(0,humidity,1);
    HalUARTWrite(0,humidity+1,1);
    HalUARTWrite(0,"\n",1); */
    HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
    //strcpy(Txdata,"HELLO WEBEE\n"); 
    //char TXdata[15];
     AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       26,
                       humidity,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, 
                       AF_DEFAULT_RADIUS ) ;

}
/*
  uint8 lineFeed[2] = {0x0A,0x0D};        //串口输出换行
  uint16 nwk;
  
  nwk = NLME_GetShortAddr();             //自身网络地址 ZDApp.c 856line
  To_string(rftx.myNWK,(uint8 *)&nwk,2); //uint8*4 uint16 = uint8*2
  To_string(rftx.myMAC,NLME_GetExtAddr(),8);
  
  
  
  HalUARTWrite(0,"parentMAC:",osal_strlen("parentMAC:"));
  HalUARTWrite(0,rftx.pMAC,16);
  HalUARTWrite(0,lineFeed,2);
 
}*/
 
  /*AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       11,
                       (uint8 *)&rftx,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, 
                       AF_DEFAULT_RADIUS ) ;*/


/*************************************************************************
*uint16 NLME_GetShortAddr(void)函数的返回值为该节点16位二进制数的网络地址。
*如果需要将这个网络地址通过串口在PC机上以字符形式呈现就需要借助于二进制
*数据转换为字符串的函数
***************************************************************************/
//将数值转化为字符串。返回对应的字符串。
/*void To_string(uint8 *dest,char *src,uint8 length)
{
    uint8 *xad;
    uint8 i=0;
    uint8 ch;
    xad = src + length-1;
    for(i=0;i<length;i++,xad--)
    {
        ch=(*xad >> 4) & 0x0F;//除于十六
        dest[i<<1] = ch + ((ch<10) ? '0' : '7');
        ch = *xad & 0x0F;
        dest[(i<<1) + 1] = ch + ((ch<10) ? '0' : '7');
    }
}
 
  
  char theMessageData[4] = "LED";

   AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       (byte)osal_strlen( theMessageData ) + 1,
                       (byte *)&theMessageData,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) ;
  HalLedBlink(HAL_LED_2,0,50,500);*/

//接收到协调器发送过来的消息，并作出操作
void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pkt )
{
  char buf[7];
  switch(pkt->clusterId)
  {
  case (GENERICAPP_CLUSTERID):
    osal_memcpy(buf,pkt -> cmd.Data,7);
     if(osal_memcmp(buf,"LED_ON",7))
     {
      HalLedSet(HAL_LED_2,HAL_LED_MODE_ON); 
     }
     if(osal_memcmp(buf,"LED_OFF",7))
     {
      HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
     }
  }
}