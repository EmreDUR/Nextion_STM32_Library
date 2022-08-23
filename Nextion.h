/*
 * Nextion.h
 *
 *  Created on: Aug 12, 2022
 *      Author: Emre DUR
 */

#ifndef INC_NEXTION_H_
#define INC_NEXTION_H_

//Include HAL Library from main header file :/
#include "main.h"

//Include libraries
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/*
 * DEFINES
 */
#define NEXTION_TIMEOUT 100
#define NEXTION_MAX_BUFF_LEN 128
#define NEXTION_MAX_COMP_COUNT 64

/*
 * STRUCT DECLARATIONS
 */
typedef struct Nextion Nextion;
typedef struct NexComp NexComp;

/*
 * NEXTION STRUCT
 */
struct Nextion
{
	//Handle for the UART used with the Nextion display
	UART_HandleTypeDef *nextionUARTHandle;

	//Variables for parsing the received data
	uint8_t _RxDataArr[NEXTION_MAX_BUFF_LEN], _RxData, _arrCount, _pkgCount;

	//Variables for component list
	NexComp** _NexCompArr[NEXTION_MAX_COMP_COUNT];
	uint8_t _NexCompCount;

};

/*
 * NexComp Struct
 */
struct NexComp
{
	uint8_t _page, _id;
	Nextion _nexStruct;

};

/*
 * Configure UART RX and TX DMA streams and UART Global Interrupt to use this library.
 * This library also requires the function below included in the main code wihtout ANY changes.
 *
 *
----------------------------
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Nextion_Update(huart, &nextion);
}
-----------------------------
 *
 *
 * Library functions
 *
 *
 */

uint8_t NextionAddComp(Nextion* nex, NexComp* _nexcomp, uint8_t __page, uint8_t __id);
uint8_t Nextion_Update(UART_HandleTypeDef *huart, Nextion *nex);
uint8_t Nextion_Init(Nextion *nex, UART_HandleTypeDef *nextionUARTHandle);
uint8_t Nextion_Restart_IT(Nextion *nex);
uint8_t Nextion_Stop_IT(Nextion *nex);
uint8_t Nextion_Get_Text(Nextion *nex, char *buf);

/*
 *
 * Low Level Functions
 *
 */
uint8_t Nextion_Send_Command(Nextion *nex, char *_command);
uint8_t Nextion_End_Command(Nextion *nex);


#endif /* INC_NEXTION_H_ */
