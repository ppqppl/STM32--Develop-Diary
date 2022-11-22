/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stm32f1xx_hal.h"
#include "includes.h"
#include "stdbool.h"
#include "oled.h"
#include "bmp.h"
#include "gui.h"
#include "AHT20.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* 任务优先级 */
#define START_TASK_PRIO		3
#define SHOW_ROLL_TASK_PRIO		4
#define SHOW_AHT20_TASK_PRIO	  5

/* 任务堆栈大小	*/
#define START_STK_SIZE 		64
#define SHOW_ROLL_STK_SIZE 		64
#define SHOW_AHT20_STK_SIZE 			64//任务堆大小过大会报错，可以试着改小一点

/* 任务栈 */	
CPU_STK START_TASK_STK[START_STK_SIZE];
CPU_STK SHOW_ROLL_TASK_STK[SHOW_ROLL_STK_SIZE];
CPU_STK SHOW_AHT20_TASK_STK[SHOW_AHT20_STK_SIZE];
/* 任务控制块 */
OS_TCB StartTaskTCB;
OS_TCB ShowrollTaskTCB;
OS_TCB Showaht20TaskTCB;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* 任务函数定义 */
void start_task(void *p_arg);
static  void  AppTaskCreate(void);
static  void  AppObjCreate(void);
static  void  show_roll(void *p_arg);
static  void  show_aht20(void *p_arg);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//用于LED显示的温度和湿度
uint32_t CT_data[2]={0,0};	// 设置读取温度的参数
volatile int  c1,t1;
uint8_t temp[10];  
uint8_t hum[10];

void getData(){	// 读取温度并转为字符串格式便于 OLED 进行显示
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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	OS_ERR  err;
	OSInit(&err);
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */


	
	OLED_Init();	// OLED 初始化

	OSTaskCreate((OS_TCB     *)&StartTaskTCB,                /* Create the start task                                */
				 (CPU_CHAR   *)"start task",
				 (OS_TASK_PTR ) start_task,
				 (void       *) 0,
				 (OS_PRIO     ) START_TASK_PRIO,
				 (CPU_STK    *)&START_TASK_STK[0],
				 (CPU_STK_SIZE) START_STK_SIZE/10,
				 (CPU_STK_SIZE) START_STK_SIZE,
				 (OS_MSG_QTY  ) 0,
				 (OS_TICK     ) 0,
				 (void       *) 0,
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
	/* 启动多任务系统，控制权交给uC/OS-III */
	OSStart(&err);            /* Start multitasking (i.e. give control to uC/OS-III). */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	/* YangJie add 2021.05.20*/
  BSP_Init();                                                   /* Initialize BSP functions */
  //CPU_Init();
  //Mem_Init();                                                 /* Initialize Memory Management Module */

#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  		//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN			//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  		//当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区
	/* 创建SHOW_ROLL任务 */
	OSTaskCreate((OS_TCB 	* )&ShowrollTaskTCB,		
				 (CPU_CHAR	* )"show_roll", 		
                 (OS_TASK_PTR )show_roll, 			
                 (void		* )0,					
                 (OS_PRIO	  )SHOW_ROLL_TASK_PRIO,     
                 (CPU_STK   * )&SHOW_ROLL_TASK_STK[0],	
                 (CPU_STK_SIZE)SHOW_ROLL_STK_SIZE/10,	
                 (CPU_STK_SIZE)SHOW_ROLL_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	/* 创建MSG任务 */
	OSTaskCreate((OS_TCB 	* )&Showaht20TaskTCB,		
				 (CPU_CHAR	* )"show_aht20", 		
                 (OS_TASK_PTR )show_aht20, 			
                 (void		* )0,					
                 (OS_PRIO	  )SHOW_AHT20_TASK_PRIO,     	
                 (CPU_STK   * )&SHOW_AHT20_TASK_STK[0],	
                 (CPU_STK_SIZE)SHOW_AHT20_STK_SIZE/10,	
                 (CPU_STK_SIZE)SHOW_AHT20_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区
}
/**
  * 函数功能: 启动任务函数体。
  * 输入参数: p_arg 是在创建该任务时传递的形参
  * 返 回 值: 无
  * 说    明：无
  */

// 实现函数

static  void  show_roll (void *p_arg)
{
  OS_ERR      err;

  (void)p_arg;

  BSP_Init();                                                 /* Initialize BSP functions                             */
  CPU_Init();

  Mem_Init();                                                 /* Initialize Memory Management Module                  */

#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

  CPU_IntDisMeasMaxCurReset();

  AppTaskCreate();                                            /* Create Application Tasks                             */

  AppObjCreate();                                             /* Create Application Objects                           */

		// 屏幕显示
	OLED_Clear(0);	// 清空屏幕
	OSTimeDlyHMSM(0, 0, 1, 0,OS_OPT_TIME_HMSM_STRICT,&err);
	OLED_WR_Byte(0x2E,OLED_CMD); //关闭滚动
	OLED_WR_Byte(0x27,OLED_CMD); //水平向左或者右滚动 26/27
	OLED_WR_Byte(0x00,OLED_CMD); //虚拟字节
	OLED_WR_Byte(0x00,OLED_CMD); //起始页 0
	OLED_WR_Byte(0x07,OLED_CMD); //滚动时间间隔
	OLED_WR_Byte(0x02,OLED_CMD); //终止页 2
	OLED_WR_Byte(0x00,OLED_CMD); //虚拟字节
	OLED_WR_Byte(0xFF,OLED_CMD); //虚拟字节
	GUI_ShowString(40,0,(uint8_t *)"ppqppl",16,1);
	//GUI_ShowCHinese(10,0,16,"春有百花秋有月",1);
  while (DEF_TRUE)
  {
		OLED_WR_Byte(0x2F,OLED_CMD); //开启滚动
		OSTimeDlyHMSM(0, 0, 30, 0,OS_OPT_TIME_HMSM_STRICT,&err);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
static  void  show_aht20 (void *p_arg)
{
  OS_ERR      err;

  (void)p_arg;

  BSP_Init();                                                 /* Initialize BSP functions                             */
  CPU_Init();

  Mem_Init();                                                 /* Initialize Memory Management Module                  */

#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

  CPU_IntDisMeasMaxCurReset();

  AppTaskCreate();                                            /* Create Application Tasks                             */

  AppObjCreate();                                             /* Create Application Objects                           */
	
/***********************************************************************************/
/**///①刚上电，产品芯片内部就绪需要时间，延时100~500ms,建议500ms
/***********************************************************************************/
/*	OSTimeDlyHMSM(0, 0, 0, 500,OS_OPT_TIME_HMSM_STRICT,&err);
	AHT20_Init();	// AHT20 初始化
		OSTimeDlyHMSM(0, 0, 0, 500,OS_OPT_TIME_HMSM_STRICT,&err);
	
  while (DEF_TRUE)
  {
	
			// 获取温度
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);	// 使用 PC13 引脚上的板载小灯泡进行测试——小灯泡亮
		AHT20_Read_CTdata_crc(CT_data);       //经过CRC校验，读取AHT20的温度和湿度数据    推荐每隔大于1S读一次
		c1 = CT_data[0]*1000/1024/1024;  //计算得到湿度值c1（放大了10倍）
		t1 = CT_data[1]*2000/1024/1024-500;//计算得到温度值t1（放大了10倍）
		
		
		
		getData();	// 获取温湿度
		
		//显示温度
		GUI_ShowCHinese(15,28,16,"温度",1);
		GUI_ShowString(47,28,":",16,1);
		GUI_ShowString(62,28,temp,16,1);
		GUI_ShowCHinese(96,28,16,"℃",1);

		//显示湿度
		GUI_ShowCHinese(15,48,16,"湿度",1);
		GUI_ShowString(47,48,":",16,1);
		GUI_ShowString(62,48,hum,16,1);
		
		OSTimeDlyHMSM(0, 0, 2, 0,OS_OPT_TIME_HMSM_STRICT,&err);
  }*/
  /* USER CODE END 3 */
}

/**
  * 函数功能: 创建应用任务
  * 输入参数: p_arg 是在创建该任务时传递的形参
  * 返 回 值: 无
  * 说    明：无
  */
static  void  AppTaskCreate (void)
{
  
}


/**
  * 函数功能: uCOSIII内核对象创建
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static  void  AppObjCreate (void)
{

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
