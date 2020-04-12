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
	sprintf(DataChar,"\r\n\tBoryViter 2020-April-10 v%d.%d.%d \r\n\tUART1 for debug on speed 115200/8-N-1\r\n\r\n",
			soft_version_arr_int[0], soft_version_arr_int[1], soft_version_arr_int[2]);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	HAL_StatusTypeDef res = BH1750_Init( &h1_bh1750 );
	sprintf(DataChar,"\r\n\tBH1750 init status: %d;\r\n", (int)res);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	I2Cdev_init(&hi2c1);
	I2C_ScanBusFlow(&hi2c1, &huart1);

	//	Set_Date_and_Time_to_DS3231(0x20, 0x04, 0x12, 0x20, 0x15, 0x36);
	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
	ds3231_PrintTime( &TimeSt, &huart1);
	ds3231_PrintDate( &DateSt, &huart1);

}
//************************************************************************

void BoryViter_Main(void) {
	char DataChar[100];
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

	ds3231_GetTime(ADR_I2C_DS3231, &TimeSt);
	ds3231_GetDate(ADR_I2C_DS3231, &DateSt);
	ds3231_PrintDate( &DateSt, &huart1);
	ds3231_WeekDay  ( &DateSt, &huart1);
	ds3231_PrintTime( &TimeSt, &huart1);

	uint16_t lux_u16 = BH1750_Main( &h1_bh1750 );
	sprintf(DataChar," Lux: %04d; \r\n", (int)lux_u16);
	HAL_UART_Transmit(&huart1, (uint8_t *)DataChar, strlen(DataChar), 100);

	HAL_Delay(1000);
}
//-------------------------------------------------------------------------------------------------


//************************************************************************

/*
**************************************************************************
*                           LOCAL FUNCTIONS
**************************************************************************
*/

//************************************************************************
