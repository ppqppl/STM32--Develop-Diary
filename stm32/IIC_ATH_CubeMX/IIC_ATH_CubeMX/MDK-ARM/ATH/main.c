#include"stdio.h"
#include"AHT20.h"
#include"stm32f10x.h"
#include"delay.h"
#include"usart3.h"









int32_t main(void)
{
	
	uint32_t CT_data[2];
	volatile int  c1,t1;
	
	
	
	Init_I2C_Sensor_Port();//初始化SDA，SCL的IO口
	

	
	Delay_1ms(500);
	
	if((AHT20_Read_Status()&0x18)!=0x18)
	{
	AHT20_Start_Init(); //重新初始化寄存器，一般不需要此初始化，只有当读回的状态字节不正确时才初始化AHT20
	Delay_1ms(10);
	}

	
	while(1)
	{
	 AHT20_Read_CTdata(CT_data);  //读取温度和湿度 ， 可间隔1.5S读一次
	 c1 = CT_data[0]*1000/1024/1024;  //计算得到湿度值（放大了10倍,如果c1=523，表示现在湿度为52.3%）
	 t1 = CT_data[1] *200*10/1024/1024-500;//计算得到温度值（放大了10倍，如果t1=245，表示现在温度为24.5℃）
	 	 
	 
	 delay_ms(1500); //延时1.5S
	 
	 //为读取的数据更稳定，还可以使用平均值滤波或者窗口滤波，或者前面读取的值与后面的值相差不能太大。
	 }
	

 }	
























