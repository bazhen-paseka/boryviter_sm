/**
* \file
* \version 1.0
* \author bazhen.levkovets
* \date 2020 
* \mail bazhen.info(at)gmail.com
*************************************************************************************
* \copyright	Bazhen Levkovets
* \copyright	Brovary, Kyiv region
* \copyright	Ukraine
*
*
*************************************************************************************
*
* \brief
*
*/

#ifndef BORYVITER_SM_H_INCLUDED
#define BORYVITER_SM_H_INCLUDED

/*
**************************************************************************
*								INCLUDE FILES
**************************************************************************
*/
	#include "main.h"
	#include "gpio.h"
	#include "usart.h"
	#include "i2c.h"
	#include "stdio.h"
	#include <string.h>

	#include "boryviter_local_config.h"
	#include "bh1750_sm.h"
	#include "ds3231_sm.h"
	#include "at24cXX_sm.h"

/*
**************************************************************************
*								    DEFINES
**************************************************************************
*/

/*
**************************************************************************
*								   DATA TYPES
**************************************************************************
*/

/*
**************************************************************************
*								GLOBAL VARIABLES
**************************************************************************
*/

/*
**************************************************************************
*									 MACRO'S
**************************************************************************
*/

/*
**************************************************************************
*                              FUNCTION PROTOTYPES
**************************************************************************
*/
	void BoryViter_Init(void);
	void BoryViter_Main(void);

	void BoryViter_Alarm_1_Set_StatusBit (uint8_t _status_u8);
	void BoryViter_Alarm_2_Set_StatusBit (uint8_t _status_u8);

	uint8_t BoryViter_Alarm_1_Get_StatusBit (void);
	uint8_t BoryViter_Alarm_2_Get_StatusBit (void);

#endif /* MAX30100_SM_H_INCLUDED */
