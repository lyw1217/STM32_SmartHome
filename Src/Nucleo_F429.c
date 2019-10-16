
/*******************************************************************************
  
  * File name :  Nucleo_F429.c  
  
			* Application model : Nucleo_F429 Board  
 
			* Ver2.0
			* January 23. 2017
			* by Sang-min Kim, Jae-Il Kim  &  Dong-hyuk Cha
 
 ****************************************************************************** */

// -- <4> Nucleo_F429 ������ ��뿡 �ʿ��� �Լ��� ������ ����

// -- <4-1> ���α׷��� ���࿡ �ʿ��� ��������� ��Ŭ���
#include "main.h"
#include "Nucleo_F429.h"

// -- <4-2> �ֺ���ġ �ʱ�ȭ�� ����ü
GPIO_InitTypeDef		    						GPIO_Init_Struct; 					    // GPIO�� �ʱ�ȭ�� ����ü ���� 
TIM_HandleTypeDef								TimHandle;											// Ÿ�̸��� �ʱ�ȭ�� ����ü ����
TIM_OC_InitTypeDef								TIM_OCInit;												// Ÿ�̸� Channel(OC)�� �ʱ�ȭ�� ����ü ����
UART_HandleTypeDef						UartHandle;											// UART�� �ʱ�ȭ�� ����ü ���� 		
ADC_HandleTypeDef							AdcHandler;											// ADC�� �ʱ�ȭ�� ����ü ����
ADC_ChannelConfTypeDef   	Adc_sConfig; 									// ADC Channel�� �ʱ�ȭ�� ����ü ����
DAC_HandleTypeDef							DacHandler;										// DAC�� �ʱ�ȭ�� ����ü ����
DAC_ChannelConfTypeDef			Dac_sConfig;										// DAC Channel�� �ʱ�ȭ�� ����ü ����

// -------------------------------------------------------------------------------- //
// -- <5> Nucleo_F429 ������  LED�� GPIO (PD0~7)�� �ʱ⼳���� �� 

void LED_Config(void)
{
		// -- Nucleo Board LED GPIO (PB0, PB7)����
		// -- <5-1> GPIOB�� Ŭ���� enable	
		__HAL_RCC_GPIOB_CLK_ENABLE();

		// -- <5-2> LED�� ���� ������ ����
		GPIO_Init_Struct.Pin = GPIO_PIN_0 | GPIO_PIN_7;		// GPIO���� ����� PIN ����
		GPIO_Init_Struct.Mode  = GPIO_MODE_OUTPUT_PP; 		// Output Push-Pull ���
		GPIO_Init_Struct.Pull     = GPIO_PULLUP; 						// Pull Up ���
		GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_HIGH; 		// ���ۼӵ��� HIGH�� 	
		
		// -- <5-3> GPIO_Init_Struct�� ������ �������� �ʱ�ȭ��
		HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);
		
		// -- Ȯ�� IO Board LED GPIO (PD0 ~ PD7)����
		// -- <5-4> GPIOD�� Ŭ���� enable	
		__HAL_RCC_GPIOD_CLK_ENABLE();

		// -- <5-5> LED1~8�� ���� ������ ����
		GPIO_Init_Struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | 
								 GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;	
																	// GPIO���� ����� PIN ����
		GPIO_Init_Struct.Mode  = GPIO_MODE_OUTPUT_PP; 		// Output Push-Pull ���
		GPIO_Init_Struct.Pull     = GPIO_PULLUP; 						// Pull Up ���
		GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_HIGH; 		// ���ۼӵ��� HIGH�� 	
		
		// -- <5-6> GPIOLed�� GPIO_Init_Struct�� ������ �������� �ʱ�ȭ��
		HAL_GPIO_Init(GPIOD, &GPIO_Init_Struct); 
}


// -------------------------------------------------------------------------------- //
// -- <6> Nucleo_F429 ������  Switch�� GPIO (PG0, PG1, PG2, PG3)�� �ʱ⼳���� �� 
//				* �� �Լ��� SW�� GPIO �Է����� �޴� �Լ��̴�.

void Sw_Config(void)
{
		// -- Nucleo Board Switch (PC13)����
		// -- <6-1> GPIOC�� Ŭ���� enable		
		__HAL_RCC_GPIOC_CLK_ENABLE();	

		// -- <6-2> SW1�� ���� ������ GPIO �Է����� ����
		GPIO_Init_Struct.Pin 		= GPIO_PIN_13;	// GPIO���� ����� PIN ����
		GPIO_Init_Struct.Mode 	= GPIO_MODE_INPUT; 				// Input Floating ���
		GPIO_Init_Struct.Pull 		= GPIO_PULLDOWN; 						// Pull Down ���
		GPIO_Init_Struct.Speed 	= GPIO_SPEED_FREQ_HIGH; 		// ���ۼӵ��� HIGH�� 

		// -- <6-3> GPIOC�� GPIO_Init_Struct�� ������ �������� �ʱ�ȭ��	
		HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);    

		// -- Ȯ�� IO Board Switch (PG0, PG1, PG2, PG3)�� �ʱ⼳��
		// -- <6-4> GPIOG�� Ŭ���� enable		
		__HAL_RCC_GPIOG_CLK_ENABLE();	

		// -- <6-5>  SW1 ~ SW4�� ���� ������ GPIO �Է����� ����
		GPIO_Init_Struct.Pin 		= GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;	// GPIO���� ����� PIN ����
		GPIO_Init_Struct.Mode 	= GPIO_MODE_INPUT; 				// Input Floating ���
		GPIO_Init_Struct.Pull 		= GPIO_PULLUP; 						// Pull Up ���
		GPIO_Init_Struct.Speed 	= GPIO_SPEED_FREQ_HIGH; 		// ���ۼӵ��� HIGH�� 

		// -- <6-6> GPIOG�� GPIO_Init_Struct�� ������ �������� �ʱ�ȭ��	
		HAL_GPIO_Init(GPIOG, &GPIO_Init_Struct);      		
}

// -------------------------------------------------------------------------------- //
// -- <7> Nucleo_F429 ������  Switch�� GPIO (PG0, PG1, PG2, PG3)�� �ʱ⼳���� �� 
//				* �� �Լ��� SW�� ������ EXTI�� �߻��ϵ��� �����Ѵ�.

void SwEXTI_Config(void)
{
		// -- Nucleo Board Switch (PC13)����
		// -- <7-1> GPIOC�� Ŭ���� enable		
		__HAL_RCC_GPIOC_CLK_ENABLE();	

		// -- <7-2> SW1�� ���� ������ GPIO �Է����� ����
		GPIO_Init_Struct.Pin 		= GPIO_PIN_13;	// GPIO���� ����� PIN ����
		GPIO_Init_Struct.Mode 	= GPIO_MODE_IT_FALLING; 				// Input Floating ���
		GPIO_Init_Struct.Pull 		= GPIO_NOPULL; 						// Pull Up / Pull Down ��Ȱ��ȭ
		GPIO_Init_Struct.Speed 	= GPIO_SPEED_FREQ_HIGH; 		// ���ۼӵ��� HIGH�� 

		// -- <7-3> GPIOC�� GPIO_Init_Struct�� ������ �������� �ʱ�ȭ��	
		HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

		// -- Ȯ�� IO Board Switch (PG0, PG1, PG2, PG3)�� �ʱ⼳��
		// -- <7-4> GPIOG�� Ŭ���� enable			
		__HAL_RCC_GPIOG_CLK_ENABLE();	

		// -- <7-5>  SW1 ~ SW4�� ���� ������ GPIO �Է����� ����
		GPIO_Init_Struct.Pin 		= GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;	// GPIO���� ����� PIN ����
		GPIO_Init_Struct.Mode 	= GPIO_MODE_IT_FALLING;				// Falling edge���� Interrupt �߻�
		GPIO_Init_Struct.Pull 		= GPIO_NOPULL; 						// Pull Up / Pull Down ��Ȱ��ȭ
		GPIO_Init_Struct.Speed 	= GPIO_SPEED_FREQ_HIGH; 		// ���ۼӵ��� HIGH��  	

		// -- <7-6>  Nucleo SW, SW1 ~ SW4�� GPIO_Init_Struct�� ������ �������� �ʱ�ȭ��	
		HAL_GPIO_Init(GPIOG, &GPIO_Init_Struct);    

		// -- <7-7> SW1 ~ SW4�� EXTI�� ���� ���ͷ�Ʈ�� �켱������ �����ϰ�, ���ͷ�Ʈ�� enable��
		HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);					//  EXTI �� priority�� ����	
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);								//  EXTI ���ͷ�Ʈ�� enable
	
	  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);					//  EXTI �� priority�� ����	
		HAL_NVIC_EnableIRQ(EXTI1_IRQn);								//  EXTI ���ͷ�Ʈ�� enable
	
	  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);					//  EXTI �� priority�� ����	
		HAL_NVIC_EnableIRQ(EXTI2_IRQn);								//  EXTI ���ͷ�Ʈ�� enable
		
		HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);					//  EXTI �� priority�� ����	
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);								//  EXTI ���ͷ�Ʈ�� enable
		
		HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);					//  EXTI �� priority�� ����	
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);								//  EXTI ���ͷ�Ʈ�� enable
}

// -------------------------------------------------------------------------------- //
// -- <8> Timer�� �ʱ⼳���� ���� �Լ�
//			  * TIM2�� Up couting ���� ���۽��� 1�ʿ� 1���� �߻��ϵ��� ���ļ��� 1 Hz�� �����Ѵ�.

void TIM2_Config(int prescaler_value, int period_value)
{

		// -- <8-1> TIM2�� clock�� enable
		__HAL_RCC_TIM2_CLK_ENABLE();
		
		// -- <8-2> TIM�� ���� ���� ���� (���ͷ�Ʈ�� 1�ʿ� 1���� �߻��ϵ��� ���ļ��� 1 Hz�� ����)
		TimHandle.Instance = TIM2;											// TIM2 ���
		TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;	//	Up Counter ��� ����
		TimHandle.Init.Prescaler = prescaler_value;								// Prescaler =8999�� ����
		TimHandle.Init.Period = period_value;						// ������Ʈ �̺�Ʈ �߻��� Auto-Reload �������� = 9999�� ����
		TimHandle.Init.ClockDivision = 0;								// Clock Division ����
		//TimHandle.Init.RepetitionCounter = 0;	// Repetition Counter ����  ���� (TIM1�� �ش�)

		// -- <8-3> TIM2�� TimHandle�� ������ ������ �ʱ�ȭ�ϰ� TIM�� ������
		HAL_TIM_Base_Init(&TimHandle);	
		HAL_TIM_Base_Start_IT(&TimHandle);	
		
		// -- <8-4> Ÿ�̸� ���ͷ�Ʈ �켱���� ���� �� ENABLE
		HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);		// ������ �������� ������ ����Ʈ�� ������
		HAL_NVIC_EnableIRQ(TIM2_IRQn);	
}

// -------------------------------------------------------------------------------- //
// -- <9> Timer�� �ʱ⼳���� ���� �Լ�
//			  * TIM3�� Down couting ���� ���۽��� 1�ʿ� 1���� �߻��ϵ��� ���ļ��� 1 Hz�� �����Ѵ�.

void TIM3_Config(int prescaler_value, int period_value)
{

		// -- <9-1> TIM2�� clock�� enable
		__HAL_RCC_TIM3_CLK_ENABLE();
		
		// -- <9-2> TIM�� ���� ���� ���� (���ͷ�Ʈ�� 1�ʿ� 1���� �߻��ϵ��� ���ļ��� 1 Hz�� ����)
		TimHandle.Instance = TIM3;								// TIM3 ���
		TimHandle.Init.CounterMode = TIM_COUNTERMODE_DOWN;	//	Down Counter ��� ����
		TimHandle.Init.Prescaler = prescaler_value;					// Prescaler =6399�� ����
		TimHandle.Init.Period = period_value;							// ������Ʈ �̺�Ʈ �߻��� Auto-Reload �������� = 9999�� ����
		TimHandle.Init.ClockDivision = 0;					// Clock Division ����
		//TimHandle.Init.RepetitionCounter = 0;		// Repetition Counter ����  ���� (TIM1�� �ش�)

		// -- <9-3> TIM3�� TimHandle�� ������ ������ �ʱ�ȭ�ϰ� TIM�� ����
		HAL_TIM_Base_Init(&TimHandle);	
		HAL_TIM_Base_Start_IT(&TimHandle);	
		
		// -- <9-4> Ÿ�̸� ���ͷ�Ʈ �켱���� ���� �� ENABLE
		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);		// ������ �������� ������ ����Ʈ�� ������
		HAL_NVIC_EnableIRQ(TIM3_IRQn);		
}

// -------------------------------------------------------------------------------- //
// -- <10> Timer�� Output Compare ���� �ʱ⼳���� ���� �Լ�

void TIM_OC_Config(int pulse_value)
{

		// -- <10-1> TIM�� OC �������� ����
		TIM_OCInit.OCMode = TIM_OCMODE_TIMING;					// Output Compare ���۸�� ����
		TIM_OCInit.Pulse = pulse_value;											// Capture/Compare Register(CCR)�� ������
		TIM_OCInit.OCPolarity = TIM_OCPOLARITY_HIGH;			// OC ����� High�� ����
		TIM_OCInit.OCFastMode = TIM_OCFAST_DISABLE; 		// TIM Output Compare Fast�� Disable

		// -- <10-2> TIM OC�� TimHandle�� ������ ������ �ʱ�ȭ��  	
		HAL_TIM_OC_Init(&TimHandle);
		// -- <10-3> TIM OC�� Channel�� TIM_OCInit�� ������ ������ �ʱ�ȭ�� 
		HAL_TIM_OC_ConfigChannel(&TimHandle, &TIM_OCInit, TIM_CHANNEL_1);
		HAL_TIM_OC_ConfigChannel(&TimHandle, &TIM_OCInit, TIM_CHANNEL_2);
		// -- <10-4> TIM OC�� ������
		HAL_TIM_OC_Start_IT(&TimHandle,TIM_CHANNEL_1);
		HAL_TIM_OC_Start_IT(&TimHandle,TIM_CHANNEL_2);
}

// -------------------------------------------------------------------------------- //
// -- <11> Timer�� PWM ���� �ʱ⼳���� ���� �Լ�

void TIM_PWM_IT_Config(int pwm_value)
{

		// -- <11-1> TIM�� OC �������Ǽ���
		TIM_OCInit.OCMode = TIM_OCMODE_PWM1;					
		TIM_OCInit.Pulse = pwm_value;											
		TIM_OCInit.OCPolarity = TIM_OCPOLARITY_HIGH;						

		// -- <11-2> TIM PWM�� TimHandle�� ������ ������ �ʱ�ȭ �� 	
		HAL_TIM_PWM_Init(&TimHandle);

		// -- <11-3> TIM PWM�� Channel�� TIM_OCInit�� ������ ������ �ʱ�ȭ ��
 		HAL_TIM_PWM_ConfigChannel(&TimHandle, &TIM_OCInit, TIM_CHANNEL_1);

		// -- <11-4> TIM PWM�� ������
		HAL_TIM_PWM_Start_IT(&TimHandle,TIM_CHANNEL_1);
}

// -------------------------------------------------------------------------------- //
// -- <12> Timer�� PWM ���� �ʱ⼳���� ���� �Լ�

void TIM_PWM_Poll_Config(int pwm_value)
{

		// -- <12-1> TIM�� OC �������Ǽ���
		TIM_OCInit.OCMode = TIM_OCMODE_PWM1;					
		TIM_OCInit.Pulse = pwm_value;											
		TIM_OCInit.OCPolarity = TIM_OCPOLARITY_HIGH;						

		// -- <12-2> TIM PWM�� TimHandle�� ������ ������ �ʱ�ȭ �� 	
		HAL_TIM_PWM_Init(&TimHandle);

		// -- <12-3> TIM PWM�� Channel�� TIM_OCInit�� ������ ������ �ʱ�ȭ ��
 		HAL_TIM_PWM_ConfigChannel(&TimHandle, &TIM_OCInit, TIM_CHANNEL_1);

		// -- <12-4> TIM PWM�� ������
		HAL_TIM_PWM_Start(&TimHandle,TIM_CHANNEL_1);
}

// -------------------------------------------------------------------------------- //
#ifdef __GNUC__
 #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
 #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
 HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 1);
 return ch;
}

// -------------------------------------------------------------------------------- //
// -- <13> UART�� �ʱ⼳���� ���� �Լ� 
void UART3_Config(void)
{
	// -- <13-1> UART�� Ŭ���� Ȱ��ȭ 
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART3_CLK_ENABLE();

	// -- <13-2> GPIO D��Ʈ 8�� ���� UART Tx�� ����
	GPIO_Init_Struct.Pin	= GPIO_PIN_8;
	GPIO_Init_Struct.Mode	= GPIO_MODE_AF_PP;
	GPIO_Init_Struct.Pull	= GPIO_PULLUP;
	GPIO_Init_Struct.Speed	= GPIO_SPEED_FREQ_HIGH;
	GPIO_Init_Struct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_Init_Struct);

	// -- <13-3> GPIO D��Ʈ 3�� ���� UART Rx�� ����
	GPIO_Init_Struct.Pin	= GPIO_PIN_9;
	GPIO_Init_Struct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_Init_Struct);	

	// -- <13-4> UART�� ���� ���� ���� 
	UartHandle.Instance	= USART3;  
	UartHandle.Init.BaudRate	= 115200;
	UartHandle.Init.WordLength	= UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits	= UART_STOPBITS_1;
	UartHandle.Init.Parity	= UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl	= UART_HWCONTROL_NONE;
	UartHandle.Init.Mode	= UART_MODE_TX_RX;
	
	// -- <13-5> UART ���������� UartHandle�� ������ ������ �ʱ�ȭ �� 
	HAL_UART_Init(&UartHandle);
	
	// -- <13-6> UART ���ͷ�Ʈ �켱���� ���� �� Ȱ��ȭ
	HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);	
	HAL_NVIC_EnableIRQ(USART3_IRQn); 
}

// -------------------------------------------------------------------------------- //
// -- <14> ADC�� Polling�� �ʱ⼳���� ���� �Լ� 
void ADC1_Polling_Config(void)
{	  
    // -- <14-1> ADC�� clock�� Ȱ��ȭ 
    __HAL_RCC_GPIOC_CLK_ENABLE();	
    __HAL_RCC_ADC1_CLK_ENABLE();
	
    GPIO_Init_Struct.Mode  = GPIO_MODE_ANALOG;
    GPIO_Init_Struct.Pin 	= GPIO_PIN_0;
		GPIO_Init_Struct.Pull = GPIO_NOPULL;	
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);	

    // -- <14-2> ADC�� ���� ���� ����
		AdcHandler.Instance = ADC1;
		AdcHandler.Init.ScanConvMode = DISABLE;
		AdcHandler.Init.ContinuousConvMode = ENABLE;
		AdcHandler.Init.ExternalTrigConv = ADC_SOFTWARE_START;
		AdcHandler.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		AdcHandler.Init.NbrOfConversion = 1;
	
    // -- <14-3> ADC�� AdcHandler�� ������ ������ �ʱ�ȭ �� 	
    HAL_ADC_Init(&AdcHandler);

    // -- <14-4> ADC Channel�������� ����	
    Adc_sConfig.Channel	= ADC_CHANNEL_10;
    Adc_sConfig.Rank         	= 1;
    Adc_sConfig.SamplingTime 	= ADC_SAMPLETIME_3CYCLES;

		// -- <14-5> ������ sConfig�� AdcHandler�� �������� �ʱ�ȭ ��
		HAL_ADC_ConfigChannel(&AdcHandler, &Adc_sConfig);

		// -- <14-6> ADC�� ���۽�Ŵ
		HAL_ADC_Start(&AdcHandler);
						
		// -- <14-7> ��ȯ�� �Ϸ�� ������ ���
		HAL_ADC_PollForConversion(&AdcHandler, 10);	

}

// -------------------------------------------------------------------------------- //
// -- <15> ADC�� Interrupt�� �ʱ⼳���� ���� �Լ� 
void ADC1_Interrupt_Config(void)
{	  
    // -- <15-1> ADC�� clock�� Ȱ��ȭ 
    __HAL_RCC_GPIOC_CLK_ENABLE();	
    __HAL_RCC_ADC1_CLK_ENABLE();
	
    GPIO_Init_Struct.Mode  = GPIO_MODE_ANALOG;
    GPIO_Init_Struct.Pin 	= GPIO_PIN_0;
		GPIO_Init_Struct.Pull = GPIO_NOPULL;	
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);	

    // -- <15-2> ADC�� ���� ���� ����
		AdcHandler.Instance = ADC1;
		AdcHandler.Init.ScanConvMode = DISABLE;
		AdcHandler.Init.ContinuousConvMode = ENABLE;
		AdcHandler.Init.ExternalTrigConv = ADC_SOFTWARE_START;
		AdcHandler.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		AdcHandler.Init.NbrOfConversion = 1;
	
    // -- <15-3> ADC�� AdcHandler�� ������ ������ �ʱ�ȭ �� 	
    HAL_ADC_Init(&AdcHandler);

    // -- <15-4> ADC Channel�������� ����	
    Adc_sConfig.Channel	= ADC_CHANNEL_10;
    Adc_sConfig.Rank         	= 1;
    Adc_sConfig.SamplingTime 	= ADC_SAMPLETIME_3CYCLES;

		// -- <15-5> ������ sConfig�� AdcHandler�� �������� �ʱ�ȭ ��
		HAL_ADC_ConfigChannel(&AdcHandler, &Adc_sConfig);

		// -- <15-6> ADC�� ���۽�Ŵ
		HAL_ADC_Start_IT(&AdcHandler);
	
    // -- <15-7> ADC ���ͷ�Ʈ �켱���� ���� �� Ȱ��ȭ 
    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(ADC_IRQn);

}

// -------------------------------------------------------------------------------- //
// -- <16> DAC�� �ʱ⼳���� ���� �Լ� 
void DAC_Config(void)
{
    // -- <16-1> DAC�� clock �� LED �� clock�� Ȱ��ȭ 
    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
	
    // -- <16-2> �߰� LED ���� ������ ����
    GPIO_Init_Struct.Pin 	= GPIO_PIN_4;
    GPIO_Init_Struct.Mode 	= GPIO_MODE_ANALOG;
    GPIO_Init_Struct.Pull 	= GPIO_NOPULL;

    // -- <16-3> GPIOA�� GPIO_Init_Struct�� ������ �������� �ʱ�ȭ��
    HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);
	
    // -- <16-4> DAC�� ���� ���� ���� 
    DacHandler.Instance 		= DAC;
    Dac_sConfig.DAC_Trigger 		= DAC_TRIGGER_NONE;
    Dac_sConfig.DAC_OutputBuffer 	= DAC_OUTPUTBUFFER_DISABLE;

    // -- <16-5> DacHandler�� ������ ������ �ʱ�ȭ �� 	
    HAL_DAC_Init(&DacHandler);

    // -- <16-6> DAC�� Channel 1�� sConfig�� ������ ������ �ʱ�ȭ ��
    HAL_DAC_ConfigChannel(&DacHandler, &Dac_sConfig, DAC_CHANNEL_1);

    // -- <16-7> DAC Channel 1���� ��ü���� ������ �������� ���� ����
    HAL_DAC_SetValue(&DacHandler, DAC_CHANNEL_1, DAC_ALIGN_8B_R, 0xff);

    // -- <16-8> DAC�� Ȱ��ȭ�ϰ� ��ȯ�� ����
    HAL_DAC_Start(&DacHandler, DAC_CHANNEL_1);

}

// -------------------------------------------------------------------------------- //
// -- <17> �ǽ�������  LED�� ON��Ű�� ���� �ð��Ŀ� �ٽ� OFF ��Ű�� �Լ�

void LED_OnOff(int led, int interval) 
{
		// -- <17-1> GPIOLed ��Ʈ�� �ִ� led�� ON ��
		HAL_GPIO_WritePin(GPIONucleo, led, GPIO_PIN_SET );
		HAL_GPIO_WritePin(GPIOExt, led, GPIO_PIN_SET );		
		// -- <17-2> interval (msec) �ð���ŭ �ð� ������ ��
		HAL_Delay(interval);
		// -- <17-3> GPIOLed ��Ʈ�� �ִ� led�� OFF ��
		HAL_GPIO_WritePin(GPIONucleo, led, GPIO_PIN_RESET );	
		HAL_GPIO_WritePin(GPIOExt, led, GPIO_PIN_RESET );	
}


// -------------------------------------------------------------------------------- //
//   --  <18>  System Clock �� �������ִ� �Լ�
/*				- Ŭ���� ������ ���� �����ȴ�.

  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  */
// -------------------------------------------------------------------------------- //

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
  }
  
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
  }
	
}

// -------------------------------------------------------------------------------- //
//   --  <19>  Clock ������ ������ �߻��ϸ� ó�����ִ� �Լ�

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
	while (1)   {
	}	
}
#endif

// -------------------------------------------------------------------------------- //

