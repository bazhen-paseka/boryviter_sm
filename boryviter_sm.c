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
	uint8_t ds3231_alarm_1_status	= 0 ;
	uint8_t ds3231_alarm_2_status	= 0 ;

	uint8_t button_pressed_flag = 0;
	uint8_t you_can_read_from_memory_flag = 0;

	uint16_t eeprom_packet_u16 = PACKET_START;

	uint8_t stop_print = 0;

/*
**************************************************************************
*                        LOCAL FUNCTION PROTOTYPES
**************************************************************************
*/
	void BV_do_it_every_seconds (void);
	void BV_write_to_EEPROM     (void);
	void BV_read_from_EEPROM    (void);
/*
**************************************************************************
*                           GLOBAL FUNCTIONS
**************************************************************************
*/

void BoryViter_Init(void) {
	int soft_version_arr_int[3];
	soft_version_arr_int[0] = ((SOFT_VERSION) / 100) %10 ;
	soft_version_arr_int[1] = ((SOFT_VERSION) /  10) %10 ;
	soft_version_arr_int[2] = ((SOFT_VERSION)      ) %10 ;

	char DataChar[100];
	sprintf(DataChar,"\r\n\tBoryViter 2020-April-10 v%d.%d.%d \r\n\tUART1 for debug on speed 115200/8-N-1\r\n",
			soft_version_arr_int[0], soft_version_arr_int[1], soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	I2Cdev_init(&hi2c1);
	I2C_ScanBusFlow(&hi2c1, &huart1);

	RTC_TimeTypeDef TimeSt;
	RTC_DateTypeDef DateSt;

	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);

	//Set_Date_and_Time_to_DS3231(2020, 4, 17, 15, 55, 30);

	if ((DateSt.Month == 1) && ( DateSt.Date == 1) && ( TimeSt.Minutes == 0 ) && ( TimeSt.Hours ==0)) {
		sprintf(DataChar, "Time and Day is wrong. We set new!!!\r\n\r\n" );
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		Set_Date_and_Time_to_DS3231(2020, 4, 18, 15, 55, 55);
		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
		ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
	}

	ds3231_PrintDate( &DateSt, &huart1);
	ds3231_PrintWeek( &DateSt, &huart1);
	ds3231_PrintTime( &TimeSt, &huart1);

	sprintf(DataChar,"\r\nPACKET_END: %d;\r\n", (int)PACKET_END);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

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

	ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00);
	ds3231_Alarm1_SetEverySeconds (ADR_I2C_DS3231);
	ds3231_Alarm1_ClearStatusBit  (ADR_I2C_DS3231);
	ds3231_Alarm2_SetEveryMinutes (ADR_I2C_DS3231);
	ds3231_Alarm2_ClearStatusBit  (ADR_I2C_DS3231);

	HAL_IWDG_Refresh(&hiwdg);
}
//************************************************************************

void BoryViter_Main(void) {

	if (ds3231_alarm_flag == 1) {
		ds3231_alarm_1_status = ds3231_Get_Alarm1_Status (ADR_I2C_DS3231);
		ds3231_alarm_2_status = ds3231_Get_Alarm2_Status (ADR_I2C_DS3231);
		ds3231_alarm_flag = 0;
	}

	if (ds3231_alarm_1_status == 1){
		you_can_read_from_memory_flag = 1;
		button_pressed_flag = 0;
		BV_do_it_every_seconds();
		ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);
		ds3231_alarm_1_status = 0;
	}

	if (ds3231_alarm_2_status == 1) {
		BV_write_to_EEPROM();
		ds3231_alarm_2_status = 0;
		ds3231_Alarm2_ClearStatusBit(ADR_I2C_DS3231);
	}

	if (	(button_pressed_flag > 0 )
		&&	(you_can_read_from_memory_flag == 1) ) {
		BV_read_from_EEPROM ();
		button_pressed_flag = 0;
		you_can_read_from_memory_flag = 0;
	}

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
}
//************************************************************************

void BoryViter_Set_Alarm_Flag (void) {
	ds3231_alarm_flag = 1 ;
}
//************************************************************************

void BoryViter_Set_EEPROM_Button (void) {
	button_pressed_flag = 1 ;
}

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

void BV_do_it_every_seconds (void) {
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
	HAL_IWDG_Refresh(&hiwdg);

	char DataChar[100];
	RTC_TimeTypeDef TimeSt;

	uint8_t uart_rx_char = 0x30;
	HAL_UART_Receive(&huart1, &uart_rx_char,1, 300);

	if (stop_print == 0) {
		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
		ds3231_PrintTime( &TimeSt, &huart1);

		uint32_t adc_u32 = ADC1_GetValue( &hadc, ADC_CHANNEL_5 );

		sprintf(DataChar," ADC:%04d; rx:%c\r\n", (int)adc_u32, (char)uart_rx_char);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	}

	switch (uart_rx_char) {
		case 'r': button_pressed_flag = 1 ; break;	//	'r'
		case 'a': button_pressed_flag = 2 ; break;	//	'a'
		case 's': stop_print = 1; 			break;
		case 'c': stop_print = 0; 			break;
		default : break;
	}
}
//************************************************************************

void BV_write_to_EEPROM (void) {
	RTC_TimeTypeDef TimeSt;
	RTC_DateTypeDef DateSt;
	char DataChar[100];
	HAL_StatusTypeDef op_res_td = HAL_OK;

	sprintf(DataChar,"%d) ", (int)(eeprom_packet_u16 - PACKET_START) );
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);

	uint32_t adc_u32 = ADC1_GetValue(&hadc, ADC_CHANNEL_5);

	uint8_t str[EEPROM_PACKET_SIZE] = {0};
	sprintf((char *) str, "g%02d%02d %02d%02d xxxx %04d",
			(int) DateSt.Month,
			(int) DateSt.Date,
			(int) TimeSt.Hours,
			(int) TimeSt.Minutes,
			(int) adc_u32);

	str[0] = MAGIK_CHAR;
	uint8_t size_of_str_u8 = EEPROM_PACKET_SIZE;
	HAL_UART_Transmit(&huart1, (uint8_t *)str, size_of_str_u8, 100);

	op_res_td = op_res_td + AT24cXX_write_to_EEPROM(str, size_of_str_u8, eeprom_packet_u16);

	sprintf(DataChar," (eeprom_res:%d)\r\n", (int)op_res_td );
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	eeprom_packet_u16++;
	if (eeprom_packet_u16 > PACKET_END) {
		eeprom_packet_u16 = PACKET_START;
	}
}
//************************************************************************

void BV_read_from_EEPROM (void) {
	char DataChar[100];
	uint32_t errors_count_u32 = 0;
	sprintf(DataChar,"\r\n\tPresent %d packet. Read from EEPROM: \r\n", (int)(eeprom_packet_u16-PACKET_START));
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	uint16_t packet_qnt=0;
	if (button_pressed_flag == 1) packet_qnt = eeprom_packet_u16-1;
	if (button_pressed_flag == 2) packet_qnt = PACKET_END;

	for (int pkt_int = PACKET_START; pkt_int <= packet_qnt; pkt_int++) {
		if (pkt_int %100 == 0 ) {
			HAL_IWDG_Refresh(&hiwdg);
		}
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
