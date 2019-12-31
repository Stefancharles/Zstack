#include <ioCC2530.h>
#include <stdio.h>
#include"Onboard.h"
#define uint unsigned int
#define uchar unsigned char

//温湿度口P0_4
#define wenshi P0_4

/*******函数声明*********/
void Delay_us(void); //1 us延时
void Delay_10us(void); //10 us延时
void Delay_ms(uint Time);//n ms延时
void COM(void); // 温湿写入
void DHT11(void) ;  //温湿传感启动

//温湿度定义
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
        延时函数
*****************************/
void Delay_us() //1 us延时
{
    MicroWait(1);
   
}

void Delay_10us() //10 us延时
{
    MicroWait(10); 
}

void Delay_ms(uint Time)//n ms延时
{
  unsigned char i;
  while(Time--)
  {
    for(i=0;i<100;i++)
      Delay_10us();
  }
}

/***********************
   温湿度传感数据的写入
  一次写8位
***********************/

void COM(void) // 温湿写入
{     
  uchar i;         
  for(i=0;i<8;i++)    
  {
    ucharFLAG=2;
    //如果高电平 或者uchatFLAG到达0 退出循环
    while((!wenshi)&&ucharFLAG++);
    Delay_10us();
    Delay_10us();
    Delay_10us();
    uchartemp=0;
    if(wenshi)// 如果P0_4是高电平，
      uchartemp=1;//超过30us依然为高电平，即数据为1;否则数据为0
    ucharFLAG=2;
    //如果是高电平并且ucharFLAG++ 一直为真 直到255 当做延时 
    //ucharFLAG变成0 停止执行 但是wenshi没有变成0 说明没有响应
    while((wenshi)&&ucharFLAG++);   //超时则跳出for循环
    if(ucharFLAG==1)
      break;    
    ucharcomdata<<=1;
    ucharcomdata|=uchartemp;  //判断数据是0还是1
  }    
}


void DHT11(void)   //温湿传感启动
{
  P0DIR |= 0x10;
  wenshi=0;
  Delay_ms(19);  //>18MS 主机拉低18ms
  wenshi=1; 
   
  //主机延时40us
  Delay_10us();
  Delay_10us();
  Delay_10us();
  Delay_10us();  
  
  wenshi=1; 
  P0DIR &= ~0x10;//重新配置IO口方向 0001 0000
  if(!wenshi) 
  {
    ucharFLAG=2;
    //判断从机是否80us的低电平，响应信号是否结束
    while((!wenshi)&&ucharFLAG++);
    ucharFLAG=2;
    while((wenshi)&&ucharFLAG++); 
    COM();//读湿度的高八位
    ucharRH_data_H_temp=ucharcomdata;
    COM();//读湿度的低八位
    ucharRH_data_L_temp=ucharcomdata;
    
    COM();//读温度的高八位
    ucharT_data_H_temp=ucharcomdata;
    COM();//读温度的低八位
    ucharT_data_L_temp=ucharcomdata;
    
    COM();
    ucharcheckdata_temp=ucharcomdata;//校验和
    
    //数据校验
    wenshi=1; 
    uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);    
    if(uchartemp==ucharcheckdata_temp)//校验结果正确则收集数据
    {
      ucharRH_data_H=ucharRH_data_H_temp;//收集湿度
      ucharRH_data_L=ucharRH_data_L_temp;
      
      ucharT_data_H=ucharT_data_H_temp;//收集温度
      ucharT_data_L=ucharT_data_L_temp;
      
      ucharcheckdata=ucharcheckdata_temp;
    } 
    wendu_shi=ucharT_data_H/10; 
    wendu_ge=ucharT_data_H%10;
    
    shidu_shi=ucharRH_data_H/10; 
    shidu_ge=ucharRH_data_H%10;        
  } 
  else //没用成功读取，返回0
  {
    wendu_shi=0; 
    wendu_ge=0;    
    shidu_shi=0; 
    shidu_ge=0;  
  } 
}
