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
	#include "stdio.h"
	#include <string.h>

	#include "boryviter_local_config.h"

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

#endif /* MAX30100_SM_H_INCLUDED */
