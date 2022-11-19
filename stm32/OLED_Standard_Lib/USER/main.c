
/****************************************************************************************************
//=========================================电源接线================================================//
// OLED模块               STM32单片机
//   VCC         接       DC 5V/3.3V      //OLED屏电源正
//   GND         接          GND          //OLED屏电源地
//=======================================液晶屏数据线接线==========================================//
//本模块默认数据总线类型为4线制SPI
// OLED模块               STM32单片机
//   D1          接          PB15        //OLED屏SPI写信号
//=======================================液晶屏控制线接线==========================================//
// OLED模块               STM32单片机
//   CS          接          PB11        //OLED屏片选控制信号
//   RES         接          PB12        //OLED屏复位控制信号
//   DC          接          PB10        //OLED屏数据/命令选择控制信号
//   D0          接          PB13        //OLED屏SPI时钟信号
//=========================================触摸屏接线=========================================//
//本模块不带触摸功能，所以不需要触摸屏接线
****************************************************************************************************/	

#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "gui.h"
#include "test.h"
#include "AHT20-21_DEMO_V1_3.h" 

//存放温度和湿度
uint32_t CT_data[2]={0,0};
//湿度和温度
volatile int  c1,t1;

//用于LED显示的温度和湿度
u8 temp[10];  
u8 hum[10];

//初始化PC13用于测试
void GPIOC13_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);

}
//初始化以及前期准备
void Init(void);

//读取温湿度
void getData(void);

//显示温湿度
void showData(void);

int main(void)
{	
	//初始化
	Init();
	while(1){

		//获取数据
		getData();
		//显示数据
		showData();

		//开启滚动
		OLED_WR_Byte(0x2F,OLED_CMD);
		
		//延时
		Delay_1ms(6200);
		//OLED_Clear(0); 
	}
	
}


//初始化以及前期准备
void Init(void){
	//初始化PC12
	GPIOC13_Init();		
	
	//延时函数初始化	  
	delay_init();	   
	
	//初始化OLED 
	OLED_Init();

	//清屏（全黑）	
	OLED_Clear(0);    

	//开机显示信息	

	GUI_ShowString(40,0,"ppqppl",16,1);
	
	GUI_ShowString(20,24,"631907060520",16,1);
	
	GUI_ShowCHinese(40,48,16,"欢迎你",1);
	
	Delay_1ms(1000);
	
	AHT20_Init();
	/***********************************************************************************/
	/**///①刚上电，产品芯片内部就绪需要时间，延时100~500ms,建议500ms
	/***********************************************************************************/
	
	Delay_1ms(1000);
	
	OLED_Clear(0); 
	OLED_WR_Byte(0x2E,OLED_CMD); //关闭滚动

	OLED_WR_Byte(0x27,OLED_CMD); //水平向左或者右滚动 26/27

	OLED_WR_Byte(0x00,OLED_CMD); //虚拟字节

	OLED_WR_Byte(0x00,OLED_CMD); //起始页 0

	OLED_WR_Byte(0x07,OLED_CMD); //滚动时间间隔

	OLED_WR_Byte(0x02,OLED_CMD); //终止页 2

	OLED_WR_Byte(0x00,OLED_CMD); //虚拟字节

	OLED_WR_Byte(0xFF,OLED_CMD); //虚拟字节
	
	// 诗句
	GUI_ShowCHinese(10,0,16,"春有百花秋有月",1);
	OLED_WR_Byte(0x2F,OLED_CMD); //开启滚动
}

//读取温湿度
void getData(){
	//AHT20_Read_CTdata(CT_data);       //不经过CRC校验，直接读取AHT20的温度和湿度数据    推荐每隔大于1S读一次
		AHT20_Read_CTdata_crc(CT_data);;  //crc校验后，读取AHT20的温度和湿度数据 
		c1 = CT_data[0]*1000/1024/1024;  //计算得到湿度值c1（放大了10倍）
		t1 = CT_data[1]*2000/1024/1024-500;//计算得到温度值t1（放大了10倍）

		//转为字符串易于显示
		temp[0]=t1/100+'0';
		temp[1]=(t1/10)%10+'0';
		temp[2]='.';
		temp[3]=t1%10+'0';
		temp[4]='\0';
		
		hum[0]=c1/100+'0';
		hum[1]=(c1/10)%10+'0';
		hum[2]='.';
		hum[3]=c1%10+'0';
		hum[4]=32;
		hum[5]='%';
		hum[6]='\0';
}


//显示温湿度
void showData(){
		//显示温度
		GUI_ShowCHinese(15,28,16,"温度",1);
		GUI_ShowString(47,28,":",16,1);
		GUI_ShowString(62,28,temp,16,1);
		GUI_ShowCHinese(96,28,16,"℃",1);

		//显示湿度
		GUI_ShowCHinese(15,48,16,"湿度",1);
		GUI_ShowString(47,48,":",16,1);
		GUI_ShowString(62,48,hum,16,1);
}

