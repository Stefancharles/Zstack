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

typedef struct RFTXBUF
{
    uint8 type[3];    //�豸����
    uint8 myNWK[4];   //�����ַ
    uint8 pNWK[4];    //���ڵ������ַ
}RFTX;
//�ýṹ�����ڴ�Žڵ�Ե���Ϣ���豸���ͣ������ַ�����ڵ������ַ

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
  //0,
  //(cId_t *)NULL
};

endPointDesc_t GenericApp_epDesc;

 
byte GenericApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GenericApp_Init() is called.
devStates_t GenericApp_NwkState;


byte GenericApp_TransID;  // This is the unique message ID (counter)

afAddrType_t GenericApp_DstAddr;
//unsigned char uartbuf[128];

/*********************************************************************
 * LOCAL FUNCTIONS
 */
//void GenericApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
//void GenericApp_HandleKeys( byte shift, byte keys );
//void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
//void GenericApp_SendTheMessage( void );
//static void rxCB(uint8 port,uint8 event);
void GenericApp_Init( byte task_id )
{ 
  //halUARTCfg_t uartConfig;
  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;
/*  
  GenericApp_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  GenericApp_DstAddr.endPoint = 0;
  GenericApp_DstAddr.addr.shortAddr = 0;*/

  /*GenericApp_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  GenericApp_DstAddr.endPoint = 10;
  GenericApp_DstAddr.addr.shortAddr = 0xFFFF;*/
 
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GenericApp_epDesc );
 /* uartConfig.configured   =TRUE;
  uartConfig.baudRate     =HAL_UART_BR_115200;
  uartConfig.flowControl  =FALSE;
  uartConfig.callBackFunc =rxCB;
  HalUARTOpen(0,&uartConfig);*/
}


UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
  return 0;
}

//��Ϣ������
/*void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
 
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:
      osal_memcpy(&nodeinfo[nodenum++],pkt->cmd.Data,11);
      break;
  }
}
 
static void rxCB(uint8 port,uint8 event)
{
  unsigned char changeline[2]={0x0A,0x0D};
  uint8 buf[8];
  uint8 uartbuf[16];
  uint8 i=0;
  HalUARTRead(0,buf,8);
  if(osal_memcmp(buf,"topology",8))
  {
        for(i=0;i<3;i++)
        {
            HalUARTWrite(0,nodeinfo[i].type,3);      //����豸����
            
            HalUARTWrite(0," NWK: ",6);
            HalUARTWrite(0,nodeinfo[i].myNWK,4);     //��������ַ
            
            HalUARTWrite(0," pNWK: ",7);
            HalUARTWrite(0,nodeinfo[i].pNWK,4);      //������ڵ������ַ
            
            HalUARTWrite(0,changeline,2);
        }
  }
}
/*static void rxCB(uint8 port,uint8 event)
{
   char theMessageData[7] = "LED_ON";
  HalUARTRead(0,uartbuf,16);
  if(osal_memcmp(uartbuf,"www.wlwmaker.com",16))
  {
      //ʹЭ��������������
        HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
        HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
        
        uint16 NLME_GetShortAddr(void)        //���ظýڵ�������ַ
        byte*  NLME_GetExtAddr(void)          //����ָ��ýڵ�MAC��ַ��ָ��
        uint16 NLME_GetCoordShortAddr(void)   //�������ظ��ڵ�������ַ
        void   NLME_GetCoordExtAdd(byte* buf) //�ú����Ĳ�����ָ���Ÿ��ڵ��MAC��ַ�Ļ���ָ��
        
        
        //ͨ���㲥�������ݸ��ն�
        AF_DataRequest( &GenericApp_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       (byte)osal_strlen( theMessageData ) + 1,
                       (byte *)&theMessageData,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) ;
        /*HalUARTWrite(0,uartbuf,16);
        for(unsigned int i=0;i<6666;i++);*/
      




