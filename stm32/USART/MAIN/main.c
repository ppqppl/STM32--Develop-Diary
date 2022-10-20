#include "stm32f10x_usart.h"
 
#define Fck 100000000			//串口时钟频率
 
char RX_BUFF[50]={0};
u8 Usart1_flag = 0;
 
//A9 TX	A10 RX
//PA9 推挽输出 速度25MHZ
//PA10上拉输入
	
void Usart1_Init(u32 bound)
{
		
	RCC->APB2ENR |= 1<<4;   //使能串口1时钟
	RCC->AHB1ENR |= 1<<0;   //使能A口时钟
	
	/*引脚配置*/
	GPIOA->MODER &= ~(0xf <<9*2 );	//清0
	GPIOA->MODER |= (0xa <<9*2 ); //复用 1010
	
	GPIOA->OTYPER &=~( 1<<9);     //A9推挽
	GPIOA->OSPEEDR &= ~(3 << 9*2);
	GPIOA->OSPEEDR |=(1 << 9*2);  //A9 25MHZ
	GPIOA->PUPDR &= ~( 3<<2*10);
	GPIOA->PUPDR |=( 1<<2*10);     //A10上拉 01
	
	GPIOA->AFR[1] |= (0x77 << 4);   //0111 0111高位  复用为AF7 usart1
	
	
	USART1->BRR =  Fck/bound;		//波特率
	
	USART1->CR1 |= 3 << 2;				//使能发送接收功能
	USART1->CR1 |= (1 << 13) |(1 << 5); 	//使能 开启接收中断
 
	
//	NVIC_SetPriorityGrouping(7-3);//抢占占3位（0~7），响应1位（0~1）//优先级分组设置，放在主函数中
	NVIC_EnableIRQ((IRQn_Type)37);    //使能串口中断
	NVIC_SetPriority((IRQn_Type)37, NVIC_EncodePriority (7-2, 0, 1));//配置抢占0和响应优先级1
	
	
}
 
//发送一个字节数据
void Usart1_SendData(char ch)
{
	while( !(USART1->SR & (1<<6)));	//等待数据发送完成
	USART1->DR = ch;
}
 
//发送字符串
void Usart1_Send_Srting(char *str)
{
	char i=0;
	while( *(str + i)!= '\0')
	{
		Usart1_SendData(*(str + i));
		i++;
	}
}
 
 
//重映射fputc函数
//可以直接使用C库printf函数
//需要添加<stdio.h>头文件
//并勾选MDK中Use MicroLIB选项（魔术棒中打开）
int fputc(int ch,FILE *stream)
{
	Usart1_SendData(ch);
	return 0;
}
 
 
 
//接收一个字节数据
char Usart1_ReceiveData(void)
{
	char ch=0;
	if( USART1->SR & (1<<5))	//接收到数据
	{
		ch = USART1->DR;
		return ch;
	}
	return 0;
}
 
 
 
//中断接收函数
void USART1_IRQHandler(void)
{
	char ch = 0;
	static u8 count = 0;
	
	if( USART1->SR & (1<<5))	//接收到数据
	{
		ch = USART1->DR;
		if( (ch != '\r') && (ch != '\n') )  //不是回车符（未结束），发送时需要添加回车
		{
			RX_BUFF[count++] = ch;
		}
		else 
		{
			RX_BUFF[count] = '\0'; //添加结束符
			Usart1_flag = 1;	//标记接收结束
			count = 0;
		}
	}
}
 
 int main(void){
	 
 }