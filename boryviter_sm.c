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

	HAL_StatusTypeDef res = BH1750_Init( &h1_bh1750 );
	sprintf(DataChar,"\r\nBH1750 init status: %d;\r\n", (int)res);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

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

	if (eeprom_button_flag == 1) {
		sprintf(DataChar,"Read from EEPROM: \r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		eeprom_button_flag = 0;
	}

	if (alarm_flag == 1) {
		alarm_1_status = ds3231_Get_Alarm1_Status (ADR_I2C_DS3231);
		alarm_2_status = ds3231_Get_Alarm2_Status (ADR_I2C_DS3231);
		alarm_flag = 0;
	}

	if (alarm_1_status == 1){
		HAL_IWDG_Refresh(&hiwdg);
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
		ds3231_PrintTime( &TimeSt, &huart1);

		uint16_t lux_u16 = BH1750_Main( &h1_bh1750 );
		sprintf(DataChar," Lux: %04d; \r\n", (int)lux_u16);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);
		alarm_1_status = 0;
	}

	if (alarm_2_status == 1){
		sprintf(DataChar,"alarm_2_status; \r\n");

		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
		ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
		ds3231_PrintDate( &DateSt, &huart1);
		ds3231_PrintWeek( &DateSt, &huart1);
		ds3231_PrintTime( &TimeSt, &huart1);

		uint16_t lux_u16 = BH1750_Main( &h1_bh1750 );
		sprintf(DataChar," Lux: %04d; \r\n", (int)lux_u16);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

		ds3231_Alarm2_ClearStatusBit(ADR_I2C_DS3231);
		alarm_2_status = 0;
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
