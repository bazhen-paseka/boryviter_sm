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

	bh1750_struct h1_bh1750 =
	{
		.i2c = &hi2c1,
		.device_i2c_address = BH1750_I2C_ADDR
	};

	RTC_TimeTypeDef TimeSt;
	RTC_DateTypeDef DateSt;

	uint8_t alarm_flag		= 0 ;
	uint8_t alarm_1_status	= 0 ;
	uint8_t alarm_2_status	= 0 ;

	uint8_t eeprom_button_flag = 0;
	uint16_t eeprom_packet_u16 = PACKET_START;

	uint8_t you_can_read_from_memory_u8 = 0;

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

	//Set_Date_and_Time_to_DS3231(0x20, 0x04, 0x13, 0x17, 0x18, 0x36);
	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);

	ds3231_PrintDate( &DateSt, &huart1);
	ds3231_PrintWeek( &DateSt, &huart1);
	ds3231_PrintTime( &TimeSt, &huart1);

	HAL_StatusTypeDef operation_result_td = BH1750_Init( &h1_bh1750 );
	sprintf(DataChar,"\r\nBH1750 init status: %d;\r\n", (int)operation_result_td);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	sprintf(DataChar,"\r\nPACKET_END: %d;\r\n", (int)PACKET_END);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	//for (int p=PACKET_START; p<=PACKET_END; p++) {
	uint16_t packet = PACKET_START;
	uint8_t packet_char = 0;

	do {
		uint8_t str[32] = {0};
		Read_from_EEPROM(str, EEPROM_PACKET_SIZE, packet);
		packet_char= str[0];
		packet++;
	} while ((packet < PACKET_END) && (packet_char==MAGIK_CHAR));	// if true - do it again

	sprintf(DataChar, "packet in EEPROM: %d \r\n\r\n", (int)(packet-1));
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	eeprom_packet_u16 = packet-1;

	ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00);		HAL_Delay(10);
	ds3231_Alarm1_SetEverySeconds (ADR_I2C_DS3231);		HAL_Delay(10);
	ds3231_Alarm1_ClearStatusBit  (ADR_I2C_DS3231);		HAL_Delay(10);

	ds3231_Alarm2_SetEveryMinutes (ADR_I2C_DS3231);		HAL_Delay(10);
	ds3231_Alarm2_ClearStatusBit  (ADR_I2C_DS3231);		HAL_Delay(10);

	HAL_IWDG_Refresh(&hiwdg);
}
//************************************************************************

void BoryViter_Main(void) {
	char DataChar[100];

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);

	if (	(eeprom_button_flag == 1)
		&&	(you_can_read_from_memory_u8 == 1) ) {
		eeprom_button_flag = 0;
		you_can_read_from_memory_u8 = 0;

		sprintf(DataChar,"\r\n\tRead from EEPROM: \r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		for (int p=PACKET_START; p<eeprom_packet_u16; p++) {
			uint8_t str[32] = {0};
			HAL_StatusTypeDef operation_result_td = HAL_ERROR;
			operation_result_td = Read_from_EEPROM(str, EEPROM_PACKET_SIZE, p);

			HAL_UART_Transmit(&huart1, (uint8_t *)str, EEPROM_PACKET_SIZE, 100);
			if (operation_result_td == HAL_OK) {
				sprintf(DataChar,"\r\n");
			} else {
				sprintf(DataChar," (readStatus:%d)\r\n", (int)operation_result_td );
			}
			HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		}
		HAL_IWDG_Refresh(&hiwdg);
		sprintf(DataChar,"\tReading from EEPROM is over.\r\n\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
	}

	if (alarm_flag == 1) {
		alarm_1_status = ds3231_Get_Alarm1_Status (ADR_I2C_DS3231);
		alarm_2_status = ds3231_Get_Alarm2_Status (ADR_I2C_DS3231);
		alarm_flag = 0;
	}

	if (alarm_1_status == 1){
		HAL_IWDG_Refresh(&hiwdg);
		//HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
		you_can_read_from_memory_u8 = 1;
		eeprom_button_flag = 0;

		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
		ds3231_PrintTime( &TimeSt, &huart1);

		uint16_t lux_u16 = BH1750_Main( &h1_bh1750 );
		sprintf(DataChar," Lux:%04d\r\n", (int)lux_u16);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);
		alarm_1_status = 0;
	}

	if (alarm_2_status == 1) {
		alarm_2_status = 0;
		ds3231_Alarm2_ClearStatusBit(ADR_I2C_DS3231);

		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
		ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
		uint16_t lux_u16 = BH1750_Main( &h1_bh1750 );

		uint8_t str[32] = {0};
		sprintf((char *) str, "g%02d%02d %02d%02d %04d",
				(int) DateSt.Month,
				(int) DateSt.Date,
				(int) TimeSt.Hours,
				(int) TimeSt.Minutes,
				(int) lux_u16			);

		str[0] = MAGIK_CHAR;
		//uint8_t size_of_str_u8 = sizeof(str) / sizeof(str[0]);
		uint8_t size_of_str_u8 = EEPROM_PACKET_SIZE;
		HAL_UART_Transmit(&huart1, (uint8_t *)str, size_of_str_u8, 100);

		HAL_StatusTypeDef operation_result_td = HAL_ERROR;
		operation_result_td = Write_to_EEPROM(str, size_of_str_u8, eeprom_packet_u16);

		sprintf(DataChar," (writeStatus:%d)\r\n", (int)operation_result_td );
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		eeprom_packet_u16++;
	}
}
//************************************************************************

void BoryViter_Set_Alarm_Flag (void) {
	alarm_flag = 1 ;
}
//************************************************************************

void BoryViter_Set_EEPROM_Button (void) {
	eeprom_button_flag = 1 ;
}

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

//************************************************************************
