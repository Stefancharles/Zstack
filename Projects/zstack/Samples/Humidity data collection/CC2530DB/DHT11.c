#include <ioCC2530.h>
#include <stdio.h>
#include"Onboard.h"
#define uint unsigned int
#define uchar unsigned char

//��ʪ�ȿ�P0_4
#define wenshi P0_4

/*******��������*********/
void Delay_us(void); //1 us��ʱ
void Delay_10us(void); //10 us��ʱ
void Delay_ms(uint Time);//n ms��ʱ
void COM(void); // ��ʪд��
void DHT11(void) ;  //��ʪ��������

//��ʪ�ȶ���
uchar ucharFLAG,uchartemp;
uchar shidu_shi,shidu_ge,
wendu_shi,wendu_ge;

uchar ucharT_data_H,  ucharT_data_L,
      ucharRH_data_H,  ucharRH_data_L,
      ucharcheckdata;

uchar ucharT_data_H_temp,   ucharT_data_L_temp,
      ucharRH_data_H_temp,   ucharRH_data_L_temp,
      ucharcheckdata_temp;

uchar ucharcomdata;

uchar temp[2]={0,0}; 
//uchar temp1[5]="temp=";
uchar humidity[2]={0,0};
//uchar humidity1[9]="humidity=";

/****************************
        ��ʱ����
*****************************/
void Delay_us() //1 us��ʱ
{
    MicroWait(1);
   
}

void Delay_10us() //10 us��ʱ
{
    MicroWait(10); 
}

void Delay_ms(uint Time)//n ms��ʱ
{
  unsigned char i;
  while(Time--)
  {
    for(i=0;i<100;i++)
      Delay_10us();
  }
}

/***********************
   ��ʪ�ȴ������ݵ�д��
  һ��д8λ
***********************/

void COM(void) // ��ʪд��
{     
  uchar i;         
  for(i=0;i<8;i++)    
  {
    ucharFLAG=2;
    //����ߵ�ƽ ����uchatFLAG����0 �˳�ѭ��
    while((!wenshi)&&ucharFLAG++);
    Delay_10us();
    Delay_10us();
    Delay_10us();
    uchartemp=0;
    if(wenshi)// ���P0_4�Ǹߵ�ƽ��
      uchartemp=1;//����30us��ȻΪ�ߵ�ƽ��������Ϊ1;��������Ϊ0
    ucharFLAG=2;
    //����Ǹߵ�ƽ����ucharFLAG++ һֱΪ�� ֱ��255 ������ʱ 
    //ucharFLAG���0 ִֹͣ�� ����wenshiû�б��0 ˵��û����Ӧ
    while((wenshi)&&ucharFLAG++);   //��ʱ������forѭ��
    if(ucharFLAG==1)
      break;    
    ucharcomdata<<=1;
    ucharcomdata|=uchartemp;  //�ж�������0����1
  }    
}


void DHT11(void)   //��ʪ��������
{
  P0DIR |= 0x10;
  wenshi=0;
  Delay_ms(19);  //>18MS ��������18ms
  wenshi=1; 
   
  //������ʱ40us
  Delay_10us();
  Delay_10us();
  Delay_10us();
  Delay_10us();  
  
  wenshi=1; 
  P0DIR &= ~0x10;//��������IO�ڷ��� 0001 0000
  if(!wenshi) 
  {
    ucharFLAG=2;
    //�жϴӻ��Ƿ�80us�ĵ͵�ƽ����Ӧ�ź��Ƿ����
    while((!wenshi)&&ucharFLAG++);
    ucharFLAG=2;
    while((wenshi)&&ucharFLAG++); 
    COM();//��ʪ�ȵĸ߰�λ
    ucharRH_data_H_temp=ucharcomdata;
    COM();//��ʪ�ȵĵͰ�λ
    ucharRH_data_L_temp=ucharcomdata;
    
    COM();//���¶ȵĸ߰�λ
    ucharT_data_H_temp=ucharcomdata;
    COM();//���¶ȵĵͰ�λ
    ucharT_data_L_temp=ucharcomdata;
    
    COM();
    ucharcheckdata_temp=ucharcomdata;//У���
    
    //����У��
    wenshi=1; 
    uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);    
    if(uchartemp==ucharcheckdata_temp)//У������ȷ���ռ�����
    {
      ucharRH_data_H=ucharRH_data_H_temp;//�ռ�ʪ��
      ucharRH_data_L=ucharRH_data_L_temp;
      
      ucharT_data_H=ucharT_data_H_temp;//�ռ��¶�
      ucharT_data_L=ucharT_data_L_temp;
      
      ucharcheckdata=ucharcheckdata_temp;
    } 
    wendu_shi=ucharT_data_H/10; 
    wendu_ge=ucharT_data_H%10;
    
    shidu_shi=ucharRH_data_H/10; 
    shidu_ge=ucharRH_data_H%10;        
  } 
  else //û�óɹ���ȡ������0
  {
    wendu_shi=0; 
    wendu_ge=0;    
    shidu_shi=0; 
    shidu_ge=0;  
  } 
}
