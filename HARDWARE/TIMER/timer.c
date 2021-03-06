#include "timer.h"
#include "led.h" 
#include "touch.h"
#include "background.h"
#include "lcd.h"
#include "usart.h"

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
long long count = 0;
u32 frequency = 50000;
float TIM_ExtCntFreq;

void choose(u16 color)
{
	switch(selected)
	{
		case 0:
			LCD_DrawRectangle(255,2,316,38,color);
			break;
		case 1: 
			LCD_DrawRectangle(255,42,316,78,color);
			break;
		case 2:	
			LCD_DrawRectangle(255,82,316,101,color);
			break;
		case 3:
			LCD_DrawRectangle(255,105,316,124,color);
			break;
		case 4:
			LCD_DrawRectangle(255,128,283,147,color);
			break;
		case 5:
			LCD_DrawRectangle(288,128,316,147,color);
			break;
		case 6:
			LCD_DrawRectangle(255,151,316,170,color);
			break;
		default: break;
	}
}

void TIM3_Init(void){
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	 
  GPIO_InitTypeDef GPIO_InitStructure;

  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOC clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  /* GPIOC Configuration: TIM3 CH1 (PC6), TIM3 CH2 (PC7), TIM3 CH3 (PC8) and TIM3 CH4 (PC9) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOC, &GPIO_InitStructure); 

  /* Connect TIM3 pins to AF2 */  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);
	
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 665;
  TIM_TimeBaseStructure.TIM_Prescaler = 167;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 1;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_Cmd(TIM3,ENABLE);
}


void TIM4_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  ///使能TIM2时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);//初始化TIM2
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //允许定时器2更新中断
	TIM_Cmd(TIM4,ENABLE); //使能定时器2
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn; //定时器2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//定时器2中断服务函数
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET) //溢出中断
	{
		tp_dev.scan(0);
		if(tp_dev.sta&TP_PRES_DOWN)
		{
			if(tp_dev.x[0]>=255&&tp_dev.x[0]<=316)
			{
				if(tp_dev.y[0]>=2&&tp_dev.y[0]<=38)
				{
					pause = 0;
					choose(BLUE);
					selected = 0;
					choose(WHITE);
				}
				else if(tp_dev.y[0]>=42&&tp_dev.y[0]<=78)
				{
					pause = 0;
					choose(BLUE);
					selected = 1;
					choose(WHITE);
				}
				else if(tp_dev.y[0]>=82&&tp_dev.y[0]<=101&&flag_mov ==0)
				{
					flag_mov = 1;
					pause = 1;
					choose(BLUE);
					selected = 2;
					choose(WHITE);
					
					if(flag_move ==0)
					{
						flag_move = 1;
					}
					else
					{
						flag_move = 0;
					}
					LCD_ShowString(258,84,200,16,16,arr_move[flag_move],BLACK,BLUE);
				}
				else if(tp_dev.y[0]>=105&&tp_dev.y[0]<=124&&flag_load ==0)
				{
					flag_load = 1;
					pause = 0;
					choose(BLUE);
					selected = 3;
					choose(WHITE);
					load_data();
				}
				else if(tp_dev.y[0]>=128&&tp_dev.y[0]<=147)
				{
					if(tp_dev.x[0]>=255&&tp_dev.x[0]<=283&&flag_acdc ==0)
					{
						flag_acdc = 1;
						pause = 0;
						choose(BLUE);
						selected = 4;
						choose(WHITE);
						
						if(C_dc_ac == 0)
						{
							C_dc_ac = 1;
						}
						else
						{
							C_dc_ac = 0;
						}
						c_acdc(C_dc_ac);
						LCD_ShowString(260,130,200,16,16,arr_C[C_dc_ac],BLACK,BLUE);
					}
					else if(tp_dev.x[0]>=288&&tp_dev.x[0]<=316&&flag_gain ==0)
					{
						flag_gain = 1;
						pause = 0;
						choose(BLUE);
						selected = 5;
						choose(WHITE);
						if(change_gain == 0)
						{
							change_gain = 1;
						}
						else
						{
							change_gain = 0;
						}
						c_gain(change_gain);
						LCD_ShowString(290,130,200,16,16,arr_JDQ[change_gain],BLACK,BLUE);
					}
				}
				else if(tp_dev.y[0]>=151&&tp_dev.y[0]<=170)
				{
					pause = 0;
					choose(BLUE);
					selected = 6;
					choose(WHITE);
				}
			}
			if(tp_dev.y[0]>=174&&tp_dev.y[0]<=195)
			{
				if(tp_dev.x[0]>=257&&tp_dev.x[0]<=281)
				{
					if(flag_inc==0)
					{
						n = 0;
						flag_inc = 1;
						switch(selected)
						{
							case 0:
								scan_flag++;
								if(scan_flag==22)
								{
									scan_flag = 1;
								}
								LCD_ShowString(258,20,200,16,16,arr_F[scan_flag-1],BLACK,BLUE);
								break;
							case 1:
								magnitude_flag++;
								if(magnitude_flag==8)
								{
									magnitude_flag = 1;
								}
								LCD_ShowString(258,60,200,16,16,arr_V[magnitude_flag-1],BLACK,BLUE);
								break;
							case 2:
								if(flag_move ==0)
								{
									update = 1;
									hor = hor + 5;
								}
								else
								{
									update = 1;
									ver = ver + 5;
								}
								break;
							case 6:
								gain++;
								if(gain == 9)
								{
									gain = 1;
								}
								c_gain(change_gain);
								LCD_ShowString(265,153,200,16,16,arr_gain[gain-1],BLACK,BLUE);
								break;
							default:
								break;
						}
					}else
					{
						n++;
					}
					if(n ==5)
					{
						flag_inc = 0;
					}
				}
				else if(tp_dev.x[0]>=290&&tp_dev.x[0]<=314)
				{
					if(flag_dec ==0)
					{
						n = 0;
						flag_dec = 1;
						switch(selected)
						{
							case 0:
								scan_flag--;
								if(scan_flag==0)
								{
									scan_flag = 21;
								}
								LCD_ShowString(258,20,200,16,16,arr_F[scan_flag-1],BLACK,BLUE);
								break;
							case 1:
								magnitude_flag--;
								if(magnitude_flag==0)
								{
									magnitude_flag = 7;
								}
								LCD_ShowString(258,60,200,16,16,arr_V[magnitude_flag-1],BLACK,BLUE);
								break;
							case 2:
								if(flag_move ==0)
								{
									update = 1;
									hor = hor - 5;
								}
								else
								{
									update = 1;
									ver = ver - 5;
								}
								break;
							case 6:
								gain--;
								if(gain == 0)
								{
									gain = 8;
								}
								c_gain(change_gain);
								LCD_ShowString(265,153,200,16,16,arr_gain[gain-1],BLACK,BLUE);
								break;
							default:
								break;
						}
					}
					else
					{
						n++;
					}
					if(n ==5)
					{
						flag_dec = 0;
					}
				}
			}
		}
		else
		{
			n = 0;
			flag_inc = 0;
			flag_dec = 0;
			flag_load = 0;
			flag_mov = 0;
			flag_acdc = 0;
			flag_gain = 0;
		}
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);  //清除中断标志位
}


void TIM_Cnt_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	TIM_DeInit(TIM2);
	TIM_DeInit(TIM7);
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM7,ENABLE);          //TIM2????    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);         //??PORTA??        
//IO        
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //GPIOA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//????
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;        //??100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //??????
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //??
	GPIO_Init(GPIOB,&GPIO_InitStructure); //???PA0
	 
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_TIM2); //PA0??????5
//TIM2??
	TIM_TimeBaseStructure.TIM_Prescaler=0;  //?????
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //??????
	TIM_TimeBaseStructure.TIM_Period=0xFFFFFFFF;   //??????
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	 
	TIM_TIxExternalClockConfig(TIM2,TIM_TIxExternalCLK1Source_TI2,TIM_ICPolarity_Rising,0);//?????
//TIM7        100ms        
	TIM_TimeBaseStructure.TIM_Prescaler=18000-1;  //?????
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //??????
	TIM_TimeBaseStructure.TIM_Period=2000-1;   //??????
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseStructure);
//??
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//?????3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;                //????3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                        //IRQ????
	NVIC_Init(&NVIC_InitStructure);        //??????????VIC????
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);//?????? ,??CC1IE????        

	TIM_Cmd(TIM7,ENABLE );         //?????5
  TIM_Cmd(TIM2,ENABLE );         //?????5        
}
u32 TIM7_LastCnt;
void TIM7_IRQHandler(void)
{
	TIM_ExtCntFreq=(TIM2->CNT-TIM7_LastCnt)*(1/0.2);// SAMPLE_PERIOD?????0.1s
	TIM_ExtCntFreq/=2.142955f;
	TIM7_LastCnt=TIM2->CNT;
	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
}

