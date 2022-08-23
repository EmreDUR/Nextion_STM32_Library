/*
 * Nextion.c
 *
 *  Created on: Aug 12, 2022
 *      Author: Emre DUR
 */

#include "Nextion.h"

uint8_t NextionAddComp(Nextion* nex, NexComp* _nexcomp, uint8_t __page, uint8_t __id, void (*callbackFunc)())
{
	//Pass the corresponding data from component to component struct
	_nexcomp->_id = __id;
	_nexcomp->_page = __page;

	//Add the component struct to the list on the Nextion Struct
	nex->_NexCompArr[nex->_NexCompCount] = _nexcomp;
	nex->_NexCompCount++;

	_nexcomp->callback = callbackFunc;

	//Return OK
	return 0;
}

uint8_t Nextion_Init(Nextion *nex, UART_HandleTypeDef *nextionUARTHandle)
{
	//Pass the used UART handle to the struct
	nex->nextionUARTHandle = nextionUARTHandle;

	//Start the parsing counters from zero
	nex->_arrCount = 0;
	nex->_pkgCount = 0;

	//Start UART transaction using DMA
	HAL_UART_Receive_DMA(nex->nextionUARTHandle, (uint8_t *)&nex->_RxData, 1);

	//Start the component count variable from zero
	nex->_NexCompCount  = 0;

	//Return OK
	return 0;
}

uint8_t Nextion_Update(UART_HandleTypeDef *huart, Nextion *nex)
{
	if(huart->Instance == (nex->nextionUARTHandle->Instance))
	{
		//Add the recieved byte to the array and increment the counter afterwards
		nex->_RxDataArr[nex->_arrCount] = nex->_RxData;
		nex->_arrCount++;

		//Count 0xFF
		if(nex->_RxData == 0xFF)
			nex->_pkgCount++;
		else
			nex->_pkgCount = 0;

		//If the data is recieved correctly;
		if(nex->_pkgCount == 3)
		{
			//Send the data to the sender back:
			uint8_t count = 0, FFCount = 0;
			for(uint8_t i = 0; FFCount < 3; i++)
			{
				count++;
				if(nex->_RxDataArr[i] == 0xFF) FFCount++;
			}

			uint8_t *transferBuf = (uint8_t*) malloc(count * sizeof(uint8_t));

			for(uint8_t i = 0; i < count; i++)
			{
				transferBuf[i] = nex->_RxDataArr[i];
			}

			for(uint8_t i = 0; i < nex->_NexCompCount; i++)
			{
				if( nex->_NexCompArr[i]->_id == transferBuf[2])
				{
					//HAL_GPIO_TogglePin(TOGGLE_GPIO_Port, TOGGLE_Pin);
					if(nex->_NexCompArr[i]->callback != NULL)
						nex->_NexCompArr[i]->callback();
				}

			}

			HAL_UART_Transmit(nex->nextionUARTHandle, transferBuf, count, 50);
			//HAL_UART_Transmit_DMA(nex->nextionUARTHandle, transferBuf, count);
			free(transferBuf);

			//Reset the buffer counters
			nex->_pkgCount = 0;
			nex->_arrCount = 0;
		}

		HAL_UART_Receive_DMA(nex->nextionUARTHandle, (uint8_t *)&nex->_RxData, 1);
	}

	//Return OK
	return 0;
}

uint8_t Nextion_Restart_IT(Nextion *nex)
{
	HAL_UART_Receive_IT(nex->nextionUARTHandle, (uint8_t *)&nex->_RxData, 1);

	//Return OK
	return 0;
}
uint8_t Nextion_Stop_IT(Nextion *nex)
{
	HAL_UART_AbortReceive_IT(nex->nextionUARTHandle);

	//Return OK
	return 0;
}

uint8_t Nextion_Get_Text(Nextion *nex, char *buf)
{
	char cmd[10]={0};
	//sprintf (cmd, "get t0.txt");
	Nextion_Send_Command(nex, cmd);

	//Gelen verinin işlenip geri döndürülmesi

	//Return OK
	return 0;
}

char ENDTERMS[]={255,255,255};
uint8_t Nextion_Send_Command(Nextion *nex, char *_command)
{
	HAL_UART_Transmit(nex->nextionUARTHandle, (uint8_t *)_command, strlen((const char*)_command), NEXTION_TIMEOUT);
	Nextion_End_Command(nex);

	//Return OK
	return 0;
}

uint8_t Nextion_End_Command(Nextion *nex)
{
	uint8_t EndCommand[3] = {255, 255, 255};
	HAL_UART_Transmit(nex->nextionUARTHandle, EndCommand, 3, NEXTION_TIMEOUT);
	Nextion_Restart_IT(nex);

	//Return OK
	return 0;
}
