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

	uint8_t alarm_1_status_bit = 0;
	uint8_t alarm_2_status_bit = 0;
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

	Set_Date_and_Time_to_DS3231(0x20, 0x04, 0x12, 0x21, 0x15, 0x36);
	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);

	ds3231_PrintDate( &DateSt, &huart1);
	ds3231_PrintWeek( &DateSt, &huart1);
	ds3231_PrintTime( &TimeSt, &huart1);

	HAL_StatusTypeDef res = BH1750_Init( &h1_bh1750 );
	sprintf(DataChar,"\r\nBH1750 init status: %d;\r\n\r\n", (int)res);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);


	ds3231_Alarm1_SetSeconds(ADR_I2C_DS3231, 0x00);
	HAL_Delay(100);
	ds3231_Alarm1_SetEverySeconds(ADR_I2C_DS3231);
	HAL_Delay(100);
	ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);

}
//************************************************************************

void BoryViter_Main(void) {
	if (BoryViter_Alarm_1_Get_StatusBit() == 1) {
		char DataChar[100];
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
		ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
		ds3231_PrintDate( &DateSt, &huart1);
		ds3231_PrintWeek( &DateSt, &huart1);
		ds3231_PrintTime( &TimeSt, &huart1);

		uint16_t lux_u16 = BH1750_Main( &h1_bh1750 );
		sprintf(DataChar," Lux: %04d; \r\n", (int)lux_u16);
		HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	//	HAL_Delay(1000);
	//	sprintf(DataChar,"RTC_bit: %d; \r\n", (int)alarm_1_status_bit);
	//	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);
		BoryViter_Alarm_1_Set_StatusBit(0);
		//ds3231_Alarm1_SetEverySeconds(ADR_I2C_DS3231);
		ds3231_Alarm1_ClearStatusBit(ADR_I2C_DS3231);
	}
}
//************************************************************************

void BoryViter_Alarm_1_Set_StatusBit (uint8_t _status_u8) {
	alarm_1_status_bit = _status_u8 ;
}
//-----------------------------------------------------------

void BoryViter_Alarm_2_Set_StatusBit (uint8_t _status_u8) {
	alarm_2_status_bit = _status_u8 ;
}
//-----------------------------------------------------------

uint8_t BoryViter_Alarm_1_Get_StatusBit (void) {
	return alarm_1_status_bit ;
}
//-----------------------------------------------------------

uint8_t BoryViter_Alarm_2_Get_StatusBit (void) {
	return alarm_2_status_bit ;
}
//************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

//************************************************************************
