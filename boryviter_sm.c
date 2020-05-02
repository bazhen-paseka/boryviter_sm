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
	uint16_t eeprom_packet_u16 = PACKET_START;

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/

	void BV_read_from_EEPROM (void);

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
	sprintf(DataChar,"\r\n\r\n\tBoryViter 2020-May-01 sleep!!! v%d.%d.%d \r\n\tUART1 for debug on speed 115200/8-N-1\r\n",
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
		DateSt.WeekDay		=	6;
		DateSt.Date			=	2;
		TimeSt.Hours		=	01;
		TimeSt.Minutes		=	23;
		TimeSt.Seconds		=	45;
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

	uint32_t adc_u32 = ADC1_GetValue( &hadc, ADC_CHANNEL_5 );
	sprintf(DataChar,"\tADC_IN5:%d\r\n", (int)adc_u32 );
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	AT24cXX_scan_I2C_bus(&hi2c1, &huart1);
	uint16_t packet = PACKET_START;
	uint8_t packet_char = 0;
	do {
		uint8_t str[32] = {0};
		AT24cXX_read_from_EEPROM(str, EEPROM_PACKET_SIZE, packet);
		packet_char= str[0];
		packet++;
	} while ((packet < PACKET_END) && (packet_char==MAGIK_CHAR));	// if true - do it again
	eeprom_packet_u16 = packet-1;
	sprintf(DataChar, "packet in EEPROM: %d \r\n\r\n", (int)(eeprom_packet_u16-PACKET_START));
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);


	sprintf(DataChar,"\tStart.");
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	ds3231_alarm_flag = 0;
	HAL_Delay(1000);
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
		sprintf(DataChar,"\r\nEXTI4 ");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		ds3231_alarm_flag = 0;

		RTC_TimeTypeDef TimeSt;
		RTC_DateTypeDef DateSt;
		HAL_RTC_GetTime( &hrtc, &TimeSt, RTC_FORMAT_BIN);
		HAL_RTC_GetDate( &hrtc, &DateSt, RTC_FORMAT_BIN);
		sprintf(DataChar,"%04d/%02d/%02d %02d:%02d:%02d ",
				(int)(2000+DateSt.Year),
				(int)DateSt.Month,
				(int)DateSt.Date,
				(int)TimeSt.Hours,
				(int)TimeSt.Minutes,
				(int)TimeSt.Seconds);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		uint32_t adc_u32 = ADC1_GetValue( &hadc, ADC_CHANNEL_5 );
		sprintf(DataChar,"ADC_IN5:%04d; ", (int)adc_u32 );
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		BV_read_from_EEPROM();	// read from EEPROM

		HAL_StatusTypeDef op_res_td = HAL_OK;
		sprintf(DataChar,"%d) ", (int)(eeprom_packet_u16 - PACKET_START) );
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		uint8_t str[EEPROM_PACKET_SIZE] = {0};
		sprintf((char *) str, "g%02d%02d %02d%02d %04d",
				(int) DateSt.Month,
				(int) DateSt.Date,
				(int) TimeSt.Hours,
				(int) TimeSt.Minutes,
				(int) adc_u32);

		str[0] = MAGIK_CHAR;
		uint8_t size_of_str_u8 = EEPROM_PACKET_SIZE;
		HAL_UART_Transmit(&huart1, (uint8_t *)str, size_of_str_u8, 100);

		op_res_td = AT24cXX_write_to_EEPROM(str, size_of_str_u8, eeprom_packet_u16);

		sprintf(DataChar," (eeprom_res:%d)\r\n", (int)op_res_td );
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		eeprom_packet_u16++;
		if (eeprom_packet_u16 > PACKET_END) {
			eeprom_packet_u16 = PACKET_START;
		}


		//	HAL_Delay(3000);
		sprintf(DataChar,"Zasnuli.. ");
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

void BV_read_from_EEPROM (void) {
	char DataChar[100];
	uint32_t errors_count_u32 = 0;
	sprintf(DataChar,"\r\n\tPresent %d packet. Read from EEPROM: \r\n", (int)(eeprom_packet_u16-PACKET_START));
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	uint16_t packet_qnt=0;
	//	if (button_pressed_flag == 1) packet_qnt = eeprom_packet_u16-1;
	//	if (button_pressed_flag == 2) packet_qnt = PACKET_END;
	packet_qnt = eeprom_packet_u16-1;

	for (int pkt_int = PACKET_START; pkt_int <= packet_qnt; pkt_int++) {
//		if (pkt_int %100 == 0 ) {
//			HAL_IWDG_Refresh(&hiwdg);
//		}
		sprintf(DataChar,"%04d) ", (int)(pkt_int-PACKET_START) );
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		uint8_t str[EEPROM_PACKET_SIZE] = {0};
		HAL_StatusTypeDef op_res_td = HAL_ERROR;
		op_res_td = AT24cXX_read_from_EEPROM(str, EEPROM_PACKET_SIZE, pkt_int);

		HAL_UART_Transmit(&huart1, (uint8_t *)str, EEPROM_PACKET_SIZE, 100);
		if (op_res_td == HAL_OK) {
			sprintf(DataChar,"\r\n");
		} else {
			sprintf(DataChar," (readStatus:%d)\r\n", (int)op_res_td );
			errors_count_u32++;
		}
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	}
	sprintf(DataChar,"\tReading from EEPROM is over. Errors = %d;\r\n\r\n", (int)errors_count_u32);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
}
//************************************************************************
