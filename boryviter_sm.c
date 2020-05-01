/**
* \file
* \version 1.0
* \author bazhen.levkovets
** \date 2020
*
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary, Kyiv region
* \copyright	Ukraine
*
*************************************************************************************
*
* \brief
*
*/

/*
**************************************************************************
*							INCLUDE FILES
**************************************************************************
*/
	#include "boryviter_sm.h"
/*
**************************************************************************
*							LOCAL DEFINES
**************************************************************************
*/

/*
**************************************************************************
*							LOCAL CONSTANTS
**************************************************************************
*/
/*
**************************************************************************
*						    LOCAL DATA TYPES
**************************************************************************
*/
/*
**************************************************************************
*							  LOCAL TABLES
**************************************************************************
*/
/*
**************************************************************************
*								 MACRO'S
**************************************************************************
*/

/*
**************************************************************************
*						    GLOBAL VARIABLES
**************************************************************************
*/
	uint8_t ds3231_alarm_flag		= 0 ;

	#define RX_BUFFER_SIZE 			1
	uint8_t rx_circular_buffer	= 'c';
	uint8_t previous_char 		= 's';

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/

/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void BoryViter_Init(void) {
	HAL_Delay(100);
	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	char DataChar[100];
	sprintf(DataChar,"\r\n\tBoryViter 2020-May-01 sleep!!! v%d.%d.%d \r\n\tUART1 for debug on speed 115200/8-N-1\r\n",
			soft_version_arr_int[0], soft_version_arr_int[1], soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	RTC_TimeTypeDef TimeSt;
	RTC_DateTypeDef DateSt;

	HAL_RTCStateTypeDef RTCState = HAL_RTC_GetState( &hrtc);
	sprintf(DataChar,"\tRTC state:%d\r\n", (int)RTCState);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	HAL_RTC_GetTime( &hrtc, &TimeSt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate( &hrtc, &DateSt, RTC_FORMAT_BIN);
	if ((TimeSt.Hours == 0) && (TimeSt.Minutes == 0) && (TimeSt.Seconds == 0)) {
		DateSt.Year			=	20;
		DateSt.Month		=	5;
		DateSt.WeekDay		=	5;
		DateSt.Date			=	1;
		TimeSt.Hours		=	12;
		TimeSt.Minutes		=	34;
		TimeSt.Seconds		=	56;
		HAL_RTC_SetTime( &hrtc, &TimeSt, RTC_FORMAT_BIN );
		HAL_RTC_SetDate( &hrtc, &DateSt, RTC_FORMAT_BIN );
	}
	HAL_Delay(100);
	HAL_RTC_GetTime( &hrtc, &TimeSt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate( &hrtc, &DateSt, RTC_FORMAT_BIN);
	sprintf(DataChar,"\t%04d/%02d/%02d (%d) \t %02d:%02d:%02d\r\n",
			(int)(DateSt.Year+2000),
			(int)DateSt.Month,
			(int)DateSt.Date,
			(int)DateSt.WeekDay,
			(int)TimeSt.Hours,
			(int)TimeSt.Minutes,
			(int)TimeSt.Seconds);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);



	sprintf(DataChar,"\tStart.");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	ds3231_alarm_flag = 0;
	HAL_Delay(2000);
	sprintf(DataChar,"\r\nZasnuli...    ");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	HAL_Delay(100);

	PWR->CR &= ~(PWR_CR_PDDS);	/* флаг PDDS определяет выбор между Stop и Standby, его надо сбросить */
	PWR->CR |= PWR_CR_CWUF;/* флаг Wakeup должн быть очищен, иначе есть шанс проснуться немедленно */
	PWR->CR |= PWR_CR_LPSDSR;/* стабилизатор питания в low-power режим, у нас в Stop потребления-то почти не будет */
	PWR->CR |= PWR_CR_ULP;		/* источник опорного напряжения Vref выключить автоматически */
	SCB->SCR |=  (SCB_SCR_SLEEPDEEP_Msk);/* с точки зрения ядра Cortex-M, что Stop, что Standby - это режим Deep Sleep *//* поэтому надо в ядре включить Deep Sleep */
	//	unsigned state = irq_disable();		/* выключили прерывания; пробуждению по ним это не помешает */
	__DSB();	/* завершили незавершённые операция сохранения данных */
	__WFI(); /* заснули */
}
//************************************************************************

void BoryViter_Main(void) {

	if (ds3231_alarm_flag == 1) {
		char DataChar[100];
		sprintf(DataChar,"\r\nIRQ");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		ds3231_alarm_flag = 0;

		RTC_TimeTypeDef TimeSt;
		RTC_DateTypeDef DateSt;
		HAL_RTC_GetTime( &hrtc, &TimeSt, RTC_FORMAT_BIN);
		HAL_RTC_GetDate( &hrtc, &DateSt, RTC_FORMAT_BIN);
		sprintf(DataChar,"\t%04d/%02d/%02d (%d) \t %02d:%02d:%02d\t",
				(int)(2000+DateSt.Year),
				(int)DateSt.Month,
				(int)DateSt.Date,
				(int)DateSt.WeekDay,
				(int)TimeSt.Hours,
				(int)TimeSt.Minutes,
				(int)TimeSt.Seconds);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		HAL_Delay(5000);
		sprintf(DataChar,"Zasnuli... ");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		HAL_Delay(500);

		PWR->CR &= ~(PWR_CR_PDDS);	/* флаг PDDS определяет выбор между Stop и Standby, его надо сбросить */
		PWR->CR |= PWR_CR_CWUF;/* флаг Wakeup должн быть очищен, иначе есть шанс проснуться немедленно */
		PWR->CR |= PWR_CR_LPSDSR;/* стабилизатор питания в low-power режим, у нас в Stop потребления-то почти не будет */
		PWR->CR |= PWR_CR_ULP;		/* источник опорного напряжения Vref выключить автоматически */
		SCB->SCR |=  (SCB_SCR_SLEEPDEEP_Msk);/* с точки зрения ядра Cortex-M, что Stop, что Standby - это режим Deep Sleep *//* поэтому надо в ядре включить Deep Sleep */
		//	unsigned state = irq_disable();		/* выключили прерывания; пробуждению по ним это не помешает */
		__DSB();	/* завершили незавершённые операция сохранения данных */
		__WFI(); /* заснули */

		sprintf(DataChar," Prosnulis");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		/* переинициализация рабочих частот */
	//	init_clk();

		/* после просыпания восстановили прерывания */
	//	irq_restore(state);

		// __WFI();
		//PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); // WFI
	}
}
//************************************************************************

void BoryViter_Set_Alarm_Flag (void) {
	ds3231_alarm_flag = 1 ;
}
//************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/
