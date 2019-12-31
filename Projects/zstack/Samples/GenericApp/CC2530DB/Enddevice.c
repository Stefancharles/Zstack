#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include<string.h>
#include "Coordinator.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#define SEND_DATA_EVENT  0x02

#define SPI_MGR_ZAPP_RX_READY 0x00
typedef struct RFTXBUF
{ 
    uint8 myNWK[4];   //�����ַ
    uint8 pNWK[4];    //���ڵ������ַ
    uint8 myMAC[16];
    uint8 pMAC[16];
}RFTX;

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
  // 0,
  //(cId_t *)NULL
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



void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
//void GenericApp_SendTheMessage( void );
//void To_string(uint8 *dest,char *src,uint8 length);
void GenericApp_Init( byte task_id )
{
  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;

  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  /*GenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  GenericApp_DstAddr.endPoint =GENERICAPP_ENDPOINT;
  GenericApp_DstAddr.addr.shortAddr = 0x0000;*/
 
  // Fill out the endpoint description.
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GenericApp_epDesc );
  halUARTCfg_t uartConfig;
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
          break;
*/
        case AF_INCOMING_MSG_CMD:
          GenericApp_MessageMSGCB( MSGpkt );
          break;  

        case ZDO_STATE_CHANGE:
          GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if (GenericApp_NwkState == DEV_END_DEVICE) 
          { 
            HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);//�������������
            osal_set_event(GenericApp_TaskID,SEND_DATA_EVENT);
             
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );//�ͷ��ڴ�

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

 if ( events & SEND_DATA_EVENT )
  {
    // Send "the" message
    //GenericApp_SendTheMessage();//�������ݸ�����

    // Setup to send message again
    ///osal_start_timerEx( GenericApp_TaskID,SEND_DATA_EVENT,5000 );
  return( events & SEND_DATA_EVENT );
  }
  return 0;
}

/*����NLME.h���涨���ר��API
��ȡ�豸����MAC��ַ ����ָ��ýڵ�MAC��ַ��ָ��
extern byte *NLME_GetExtAddr( void );

��ȡ�豸���������ַ
extern uint16 NLME_GetShortAddr( void );

��ȡ���豸�����ַ    
extern uint16 NLME_GetCoordShortAddr( void );

�ú����Ĳ�����ָ���Ÿ��ڵ��MAC��ַ�Ļ�������ָ��
extern void NLME_GetCoordExtAddr( byte * buf);*/
/*void GenericApp_SendTheMessage( void )
{ 
  RFTX rftx;
  uint8 buf[8];
  uint8 lineFeed[2] = {0x0A,0x0D};        //�����������
  uint16 nwk;
  
  nwk = NLME_GetShortAddr();             //���������ַ ZDApp.c 856line
  To_string(rftx.myNWK,(uint8 *)&nwk,2); //uint8*4 uint16 = uint8*2
  To_string(rftx.myMAC,NLME_GetExtAddr(),8);
  
  nwk = NLME_GetCoordShortAddr();        //��ȡ���豸�����ַ  ZDApp.c 2225line
  To_string(rftx.pNWK,(uint8 *)&nwk,2);
  
  NLME_GetCoordExtAddr(buf);             //��ȡ���豸MAC��ַ
  To_string(rftx.pMAC,buf,8);
  
  
  HalUARTWrite(0,"myNWK:",osal_strlen("myNWK:"));
  HalUARTWrite(0,rftx.myNWK,4);
  HalUARTWrite(0,lineFeed,2);
  
  HalUARTWrite(0,"myMAC:",osal_strlen("myMAC:"));
  HalUARTWrite(0,rftx.myMAC,16);
  HalUARTWrite(0,lineFeed,2);
  
  HalUARTWrite(0,"parentNWK:",osal_strlen("parentNWK:"));
  HalUARTWrite(0,rftx.myNWK,4);
  HalUARTWrite(0,lineFeed,2);
  
  HalUARTWrite(0,"parentMAC:",osal_strlen("parentMAC:"));
  HalUARTWrite(0,rftx.pMAC,16);
  HalUARTWrite(0,lineFeed,2);
 
}*/
 /* if(GenericApp_NwkState == DEV_END_DEVICE)             //�ж��Ƿ�Ϊ�ն˽ڵ�
  {
      osal_memcpy(rftx.type,"END",3);//Ŀ�� Դ ��
  }
   if(GenericApp_NwkState == DEV_ROUTER)                 //�ж��Ƿ���·����
  {
      osal_memcpy(rftx.type,"ROU",3);
  }*/
  
  
  /*nwk = NLME_GetShortAddr();          //���������ַ ZDApp.c 856line
  To_string(rftx.myNWK,(uint8 *)&nwk,2);//����ֵת��Ϊ�ַ��������ض�Ӧ���ַ�����
  
  nwk = NLME_GetCoordShortAddr();     //��ȡ���豸�����ַ  ZDApp.c 2225line
  To_string(rftx.pNWK,(uint8 *)&nwk,2);
  
  nwk = NLME_GetExtAddr();          //����MAC��ַZDApp.c 255line
  To_string(rftx.myNWK,(uint8 *)&nwk,2);
  
  nwk = NLME_GetCoordExtAddr();             //��ȡ���豸MAC��ַ
  To_string(rftx.myNWK,(uint8 *)&nwk,2);
  //���͸�Э����*/

  /*AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       11,
                       (uint8 *)&rftx,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, 
                       AF_DEFAULT_RADIUS ) ;*/


/*************************************************************************
*uint16 NLME_GetShortAddr(void)�����ķ���ֵΪ�ýڵ�16λ���������������ַ��
*�����Ҫ����������ַͨ��������PC�������ַ���ʽ���־���Ҫ�����ڶ�����
*����ת��Ϊ�ַ����ĺ���
***************************************************************************/
//����ֵת��Ϊ�ַ��������ض�Ӧ���ַ�����
/*void To_string(uint8 *dest,char *src,uint8 length)
{
    uint8 *xad;
    uint8 i=0;
    uint8 ch;
    xad = src + length-1;
    for(i=0;i<length;i++,xad--)
    {
        ch=(*xad >> 4) & 0x0F;//����ʮ��
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
  }
}