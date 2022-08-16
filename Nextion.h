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

/*
 * DEFINES
 */
#define NEXTION_TIMEOUT 50


/*
 * NEXTION STRUCT
 */
typedef struct
{
	//Handle for the UART used with the Nextion display
	UART_HandleTypeDef *nextionUARTHandle;

	//Variables for parsing the received data
	uint8_t _RxDataArr[12], _RxData, _arrCount, _pkgCount;

} Nextion;

/*
 * Configure UART RX and TX DMA streams and UART Global Interrupt to use this library.
 * This library also requires the function below included in the main code wihtout ANY changes.
 *
 * ----------------------------
 *
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Nextion_Update(huart, &nextion);
}
 *
 * -----------------------------
 *
 * Library functions
 *
 */

uint8_t Nextion_Update(UART_HandleTypeDef *huart, Nextion *nex);
uint8_t Nextion_Init(Nextion *nex, UART_HandleTypeDef *nextionUARTHandle);

void Nextion_Callbacks(Nextion *nex);

/*
 * LOW LEVEL FUNCTIONS
 */
void Nextion_Send(Nextion *nex);


#endif /* INC_NEXTION_H_ */
