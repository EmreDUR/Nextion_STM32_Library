/*
 * Nextion.h
 *
 *  Created on: Aug 12, 2022
 *      Author: Emre DUR
 *
 *
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
 * Defines
 */
//Reminder: To use more components or to receive more characters increase buffer or list counts!
#define NEXTION_TIMEOUT 250
#define NEXTION_MAX_BUFF_LEN 96
#define NEXTION_TEXT_BUFF_LEN 64
#define NEXTION_MAX_COMP_COUNT 32

#define NEX_RET_CMD_FINISHED                 (0x01)
#define NEX_RET_EVENT_LAUNCHED               (0x88)
#define NEX_RET_EVENT_UPGRADED               (0x89)
#define NEX_RET_EVENT_TOUCH_HEAD             (0x65)
#define NEX_RET_EVENT_POSITION_HEAD          (0x67)
#define NEX_RET_EVENT_SLEEP_POSITION_HEAD    (0x68)
#define NEX_RET_CURRENT_PAGE_ID_HEAD         (0x66)
#define NEX_RET_STRING_HEAD                  (0x70)
#define NEX_RET_NUMBER_HEAD                  (0x71)
#define NEX_RET_INVALID_CMD                  (0x00)
#define NEX_RET_INVALID_COMPONENT_ID         (0x02)
#define NEX_RET_INVALID_PAGE_ID              (0x03)
#define NEX_RET_INVALID_PICTURE_ID           (0x04)
#define NEX_RET_INVALID_FONT_ID              (0x05)
#define NEX_RET_INVALID_BAUD                 (0x11)
#define NEX_RET_INVALID_VARIABLE             (0x1A)
#define NEX_RET_INVALID_OPERATION            (0x1B)
#define NEX_EVENT_ON_PRESS                   (0x01)
#define NEX_EVENT_ON_RELEASE                 (0x00)

/*
 * NexComp Struct
 */
typedef struct
{
	//Variables for storing page and ID for every component
	uint8_t _page, _id;

	//Function pointers for storing the callback functions
	void (*callbackOnPress)();
	void (*callbackOnRelease)();

	//Variable for storing object name
	char *objname;

} NexComp;


/*
 * Nextion Struct
 */
typedef struct
{
	//Handle for the UART used with the Nextion display
	UART_HandleTypeDef *nextionUARTHandle;

	//Variables for parsing the received data
	uint8_t _RxDataArr[NEXTION_MAX_BUFF_LEN], _RxData, _arrCount, _pkgCount;

	//Variables for component list
	NexComp* _NexCompArr[NEXTION_MAX_COMP_COUNT];
	uint8_t _NexCompCount;

	//Variables for receiving strings and numbers,
	uint8_t NexTextBuff[NEXTION_TEXT_BUFF_LEN], NextTextLen;
	int32_t NextNumBuff;

} Nextion;


/*
 * Library User functions
 */

uint8_t NextionAddComp(Nextion* nex, NexComp* _nexcomp, char* objectname, uint8_t __page, uint8_t __id, void (*callbackFuncOnPress)(), void (*callbackFuncOnRelease)());
uint8_t NextionUpdate(UART_HandleTypeDef *huart, Nextion *nex);
uint8_t NextionInit(Nextion *nex, UART_HandleTypeDef *nextionUARTHandle);
uint8_t NextionGetText(Nextion *nex, NexComp *comp, char *buf);
uint8_t NextionSetText(Nextion *nex, NexComp *comp, char *usertext);
uint8_t NextionGetVal(Nextion *nex, NexComp *comp, int *valBuf);
uint8_t NextionSetVal(Nextion *nex, NexComp *comp, int userval);


/*
 * Library Low Level Functions
 */
uint8_t NextionSendCommand(Nextion *nex, char *_command);
uint8_t NextionEndCommand(Nextion *nex);
uint8_t NextionRestartIT(Nextion *nex);
uint8_t NextionStopIT(Nextion *nex);

#endif /* INC_NEXTION_H_ */
