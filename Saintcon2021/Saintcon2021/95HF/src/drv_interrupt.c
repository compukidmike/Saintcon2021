/**
  ******************************************************************************
  * @file    drv_interrupt.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file configured the interruption raised by 95HF device.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
	******************************************************************************
  */ 

#include "drv_interrupt.h"

/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup _95HF_Driver
 * 	@{
 *  @brief  <b>This folder contains the driver layer of 95HF family (CR95HF, RX95HF, ST95HF)</b>
 */

/** @addtogroup drv_interrupt
 * 	@{
 *  @brief  This file contains the ressource configuration needed by the 95HF <br />  
 *  			  GPIO configuration is inside drv_interrupt header file <br /> 
 *  			  95HF driver use 1 timeout (need 1 TIMER resource from MCU)
 */
 
/**
 *	@brief  this uTimeOut variable is used as a timeout duting the communication with the RF tranceiver
 */
extern __IO uint8_t						uTimeOut;
extern volatile bool										uAppliTimeOut;

extern volatile bool RF_DataExpected;
extern volatile bool uDataReady;
 
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t delay_appli = 0;
uint16_t delay_timeout = 0;
/* Private functions Prototype -----------------------------------------------*/

static void IRQOut_GPIO_Config		( void );

/** @addtogroup drv_interrupt_Private_Functions
 * 	@{
 */

/**
 *	@brief This function configures the GPIO for the EXTI interupt
 *  @param  None
 *  @retval None
 */
static void IRQOut_GPIO_Config( void )
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Configure IRQ pin as open drain output */
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_Pin  = IRQOUT_RFTRANS_95HF_PIN;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);
	RFTRANS_95HF_IRQIN_HIGH() ;
}


/**
  * @}
  */

/** @addtogroup drv_interrupt_Public_Functions
 * 	@{
 */

/**
 *	@brief  Timeout timer config
 *  @param  None
 *  @retval None
 */
void drvInt_TimeoutTimerConfig(void)
{
	//TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
/* -------------------------------------------------------------------------- 
	 * TimeOut TIMER configuration
	 * -------------------------------------------------------------------------- 
	 * 72 MHz / 7200 = 10KHz (100us)
	 * 100us * 300 + 100us ~= 30ms	
	 * -------------------------------------------------------------------------- */
	//TIM_TimeBaseStructure.TIM_Period 					= TIMER_TIMEOUT_PERIOD;     
	//TIM_TimeBaseStructure.TIM_Prescaler 			= TIMER_TIMEOUT_PRESCALER;       
	//TIM_TimeBaseStructure.TIM_ClockDivision 	= TIM_CKD_DIV1;      
	//TIM_TimeBaseStructure.TIM_CounterMode 		= TIM_CounterMode_Down;	  
	/* Update the timeout timer (TIM3) 	*/
	//TIM_TimeBaseInit(TIMER_TIMEOUT, &TIM_TimeBaseStructure);
	
	//TIM_UpdateRequestConfig(TIMER_TIMEOUT, TIM_UpdateSource_Global);
	
	//TIM_ClearITPendingBit(TIMER_TIMEOUT, TIM_IT_Update);
		
	/* Enable TIMER Update interrupt */
	//TIM_ITConfig(TIMER_TIMEOUT, TIM_IT_Update, ENABLE);
	
	/* Disable timer	*/
	//TIM_Cmd(TIMER_TIMEOUT, DISABLE);
}


/**
 *	@brief  This function starts the time out used to avoid the STM32 freeze
 *  @param  delay : delay in tenth of milliseconds (100us).
 *  @retval None
 */
void StartTimeOut( uint16_t delay )
{
	/* Set the TimeOut flag to false */
	uTimeOut 	 = false;
	delay_timeout = delay;
	/* Set the timer counter */
	//TIM_SetCounter(TIMER_TIMEOUT, delay);
  /* Enable the Time out timer */
	//TIM_Cmd(TIMER_TIMEOUT, ENABLE);
}

/**
 *	@brief  Stop the timer used for the time out
 *  @param  None
 *  @retval None
 */
void StopTimeOut( void )
{	
	/* Disable the Time out timer */
	//TIM_Cmd(TIMER_TIMEOUT, DISABLE);	
}

/**
 *	@brief  Timeout timer config
 *  @param  None
 *  @retval None
 */
void drvInt_AppliTimeoutTimerConfig(void)
{
	//TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
/* -------------------------------------------------------------------------- 
	 * TimeOut TIMER configuration
	 * -------------------------------------------------------------------------- 
	 * 72 MHz / 72 = 1MHz (1us)
	 * 1us * 1000 + 1us ~= 1ms	
	 * -------------------------------------------------------------------------- */
	//TIM_TimeBaseStructure.TIM_Period 					= APPLI_TIMER_TIMEOUT_PERIOD;     
	//TIM_TimeBaseStructure.TIM_Prescaler 			= APPLI_TIMER_TIMEOUT_PRESCALER;       
	//TIM_TimeBaseStructure.TIM_ClockDivision 	= TIM_CKD_DIV1;      
	//TIM_TimeBaseStructure.TIM_CounterMode 		= TIM_CounterMode_Down;	  
	//TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	/* Update the timeout timer (TIM3) 	*/
	//TIM_TimeBaseInit(APPLI_TIMER_TIMEOUT, &TIM_TimeBaseStructure);
	
	//TIM_UpdateRequestConfig(APPLI_TIMER_TIMEOUT, TIM_UpdateSource_Global);
	
	//TIM_ClearITPendingBit(APPLI_TIMER_TIMEOUT, TIM_IT_Update);
		
	/* Enable TIMER Update interrupt */
	//TIM_ITConfig(APPLI_TIMER_TIMEOUT, TIM_IT_Update, ENABLE);

	/* Disable timer	*/
	//TIM_Cmd(APPLI_TIMER_TIMEOUT, DISABLE);
}

/**
 *	@brief  This function starts the time out used to avoid the STM32 freeze
 *  @param  delay : delay in tenth of milliseconds (100us).
 *  @retval None
 */
void StartAppliTimeOut( uint16_t delay )
{
	/* Set the TimeOut flag to false */
	uAppliTimeOut 	 = false;
	delay_appli = delay;
	//TIM_SetCounter(APPLI_TIMER_TIMEOUT, delay);
	/* TIM2 enable counter */
  //TIM_Cmd(APPLI_TIMER_TIMEOUT, ENABLE);
}

/**
 *	@brief  Stop the timer used for the time out
 *  @param  None
 *  @retval None
 */
void StopAppliTimeOut( void )
{	
	/* Disable the Time out timer */
	//TIM_Cmd(APPLI_TIMER_TIMEOUT, DISABLE);	
}


/**
 *	@brief  this function sends a negative pulse on SPI_NSS pin
 */
void SendSPINSSPulse(void)
{
		RFTRANS_95HF_NSS_HIGH() ;
		delayHighPriority_ms(1);
		RFTRANS_95HF_NSS_LOW() ;
		delayHighPriority_ms(1);
		RFTRANS_95HF_NSS_HIGH() ;
}

/**
 *  @brief  This function configures the Interrupt Output to send interuption to xx95HF
 *  @param  None
 *  @return None
 */
void IRQOut_Config( void )
{
	// GPIOA pin2 (as Open drain output)
	IRQOut_GPIO_Config( ); 
	
	// GPIOA pin3 (as floatting)
	//EXTI_GPIO_Config();
}

/**
 *	@brief  This function configures the Extern Interrupt for the IRQ coming from the RF transceiver
 */
void drvInt_Enable_Reply_IRQ( void )
{
	//EXTI_InitTypeDef EXTI_InitStructure;
	
	RF_DataExpected = false;
	uDataReady = false;
	
	/* Configure RF transceiver IRQ EXTI line */
	//EXTI_InitStructure.EXTI_Line 		= EXTI_RFTRANS_95HF_LINE;
	//EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	//EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;    
	//EXTI_InitStructure.EXTI_LineCmd 	= ENABLE;
	//EXTI_Init(&EXTI_InitStructure); 

}

/**
 *	@brief  This function configures the Extern Interrupt for the IRQ coming from the RF transceiver
 */
void drvInt_Enable_RFEvent_IRQ( void )
{
	//EXTI_InitTypeDef EXTI_InitStructure;
	
	RF_DataExpected = true;
	uDataReady = false;
	
	/* Configure RF transceiver IRQ EXTI line */
	//EXTI_InitStructure.EXTI_Line 		= EXTI_RFTRANS_95HF_LINE;
	//EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	//EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;    
	//EXTI_InitStructure.EXTI_LineCmd 	= ENABLE;
	//EXTI_Init(&EXTI_InitStructure); 

}

/**
 *	@brief  This function configures the Extern Interrupt for the IRQ coming from the RF transceiver
 */
void drvInt_Disable_95HF_IRQ( void )
{
	//EXTI_InitTypeDef EXTI_InitStructure;
	
	RF_DataExpected = false;
	uDataReady = false;
	
	/* Configure RF transceiver IRQ EXTI line */
	//EXTI_InitStructure.EXTI_Line 		= EXTI_RFTRANS_95HF_LINE;
	//EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	//EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;    
	//EXTI_InitStructure.EXTI_LineCmd 	= DISABLE;
	//EXTI_Init(&EXTI_InitStructure); 

}

/* Next function are wrapped in the IRQ handler */

/**
 * @brief  This function handles the timer interrupt.
 * @param  None
 * @retval None
 */
void TIMER_TIMEOUT_IRQ_HANDLER(void)
{
	if (delay_timeout > 0)
		delay_timeout--;
	else
	{
		uTimeOut = true;	
		/* Disable the Time out timer */
		//TIM_Cmd(TIMER_TIMEOUT, DISABLE);
	}
	/* Clear TIMER update interrupt flag */
	//TIM_ClearITPendingBit(TIMER_TIMEOUT, TIM_IT_Update);
}

/**
 * @brief  This function handles the timer interrupt.
 * @param  None
 * @retval None
 */
void APPLI_TIMER_TIMEOUT_IRQ_HANDLER(void)
{
	if (delay_appli > 0)
		delay_appli--;
	else
	{
		uAppliTimeOut = true;	
		/* Disable the Time out timer */
		//TIM_Cmd(APPLI_TIMER_TIMEOUT, DISABLE);
	}
	/* Clear TIMER update interrupt flag */
	//TIM_ClearITPendingBit(APPLI_TIMER_TIMEOUT, TIM_IT_Update);
}

#ifdef CR95HF
/**
 *	@brief  This function configures the SPI NSS pin to send a interuption pulse
 *  @param  None
 *  @retval None
 */
void SPINSS_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* SPINSS as output push pull  */
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   			= RFTRANS_95HF_SPI_NSS_PIN;
	GPIO_InitStructure.GPIO_Mode  			= GPIO_Mode_Out_PP; 
	GPIO_Init(RFTRANS_95HF_SPI_NSS_GPIO_PORT, &GPIO_InitStructure);
	
	/* SPI_NSS  = High Level  */
	GPIO_SetBits(RFTRANS_95HF_SPI_NSS_GPIO_PORT, RFTRANS_95HF_SPI_NSS_PIN);  

}
#endif /* CR95HF */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
 * @}
 */ 


/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

