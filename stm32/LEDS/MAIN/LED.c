#include "stm32f10x.h"

// APB2使能时钟寄存器
#define RCC_APB2ENR	*((unsigned volatile int*)0x40021018)
// GPIOA配置寄存器
#define GPIOA_CRL	*((unsigned volatile int*)0x40010800)
#define	GPIOA_ODR	*((unsigned volatile int*)0x4001080C)
// GPIOB配置寄存器
#define GPIOB_CRH	*((unsigned volatile int*)0x40010C04)
#define	GPIOB_ODR	*((unsigned volatile int*)0x40010C0C)
// GPIOC配置寄存器
#define GPIOC_CRH	*((unsigned volatile int*)0x40011004)
#define	GPIOC_ODR	*((unsigned volatile int*)0x4001100C)
	
void SystemInit(){}

// 设置延时函数，每个熄灭起后，延时1秒另一个灯亮起，delay_ms 表示为毫秒级延时
void delay(u32 time){
	u32 i = 0;
	while(time--){
		i = 2000;
		while(i--){
		}
	}
}

int main(void){
	
	RCC_APB2ENR |= (1<<2);	// GPIOA 时钟使能
	RCC_APB2ENR |= (1<<3);	// GPIOB 时钟使能
	RCC_APB2ENR |= (1<<4);	// GPIOC 时钟使能
	
	GPIOA_CRL &= 0xFF0FFFFF;	// 清空控制 PA5 的端口位
	// F 的位置表示的是对应引脚的位置上的电压，从右向左 CRL 分别为1、2、……、8，CRH 分别为9、10、……、16
	GPIOA_CRL |= 0x00300000;	// PB5 推挽输出（和电压设置有关）, 绿灯
	//GPIOA_ODR &= ~(1<<5);		
	GPIOA_ODR |= 1<<5;				// 设置灯初始状态为关闭
	
	GPIOB_CRH &= 0xFFFFFF0F;
	GPIOB_CRH |= 0x00000020;	// 红灯
	//GPIOA_ODR &= ~(1<<9);
	GPIOB_ODR |= 1<<9;
	
	
	GPIOC_CRH &= 0xF0FFFFFF;
	GPIOC_CRH |= 0x02000000;	// 黄灯
	//GPIOC_ODR &= ~(1<<14);
	GPIOC_ODR |= 1<<14;
	
	while(1){
		GPIOA_ODR &= ~(1<<5);	//PB5低电平,因为是置0，所以用按位与
		delay(1000);
		GPIOA_ODR |= 1<<5;		// 设置高电位，点亮
		//delay(1000);
		
		GPIOB_ODR &= ~(1<<9);
		delay(1000);
		GPIOB_ODR |= 1<<9;
		//delay(1000);
		
		GPIOC_ODR &= ~(1<<14);
		delay(1000);
		GPIOC_ODR |= 1<<14;
		//delay(1000);
	}
	
}