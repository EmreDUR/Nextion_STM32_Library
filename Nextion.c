/*
 * Nextion.c
 *
 *  Created on: Aug 12, 2022
 *      Author: Emre DUR
 */

#include "Nextion.h"

uint8_t NextionAddComp(Nextion* nex, NexComp* _nexcomp, char* objectname, uint8_t __page, uint8_t __id, void (*callbackFuncOnPress)(), void (*callbackFuncOnRelease)())
{
	//Make space before passing the object name to the nexcomp struct
	_nexcomp->objname = (char *) malloc((strlen(objectname)*sizeof(char)) + 1);
	//Pass the object name to the struct
	strcpy(_nexcomp->objname, objectname);

	//Pass the corresponding data from component to component struct
	_nexcomp->_id = __id;
	_nexcomp->_page = __page;

	//Add the component struct to the list on the Nextion Struct
	nex->_NexCompArr[nex->_NexCompCount] = _nexcomp;
	nex->_NexCompCount++;

	//Bind the correct callback functions together
	_nexcomp->callbackOnPress = callbackFuncOnPress;
	_nexcomp->callbackOnRelease = callbackFuncOnRelease;

	//Return OK
	return 0;
}

uint8_t NextionInit(Nextion *nex, UART_HandleTypeDef *nextionUARTHandle)
{
	//Pass the used UART handle to the struct
	nex->nextionUARTHandle = nextionUARTHandle;

	//Start the parsing counters from zero
	nex->_arrCount = 0;
	nex->_pkgCount = 0;

	//Start UART transaction using DMA
	HAL_UART_Receive_IT(nex->nextionUARTHandle, (uint8_t *)&nex->_RxData, 1);

	//Start the component count variable from zero
	nex->_NexCompCount  = 0;

	//Return OK
	return 0;
}

uint8_t NextionUpdate(UART_HandleTypeDef *huart, Nextion *nex)
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

		//If the data is recieved correctly,
		if(nex->_pkgCount == 3)
		{
			//Send the data to the sender back:
			uint8_t count = 0, FFCount = 0;
			for(uint8_t i = 0; FFCount < 3; i++)
			{
				count++;
				if(nex->_RxDataArr[i] == 0xFF) FFCount++;
			}

			//Create and place the data in a dynamically allocated buffer for easy proccessing,
			uint8_t *transferBuf = (uint8_t*) malloc(count * sizeof(uint8_t));

			for(uint8_t i = 0; i < count; i++)
			{
				transferBuf[i] = nex->_RxDataArr[i];
			}

			//In case of a touch event call the callback function accordingly,
			if(transferBuf[0] == NEX_RET_EVENT_TOUCH_HEAD)
			{
				//Loop through the component struct array,
				for(uint8_t i = 0; i < nex->_NexCompCount; i++)
				{
					//Detect the affected component by its ID
					if(transferBuf[2] == (nex->_NexCompArr[i]->_id))
					{
						//Call the desired On Press or On Release callback function,
						if((transferBuf[3] == NEX_EVENT_ON_PRESS) && (nex->_NexCompArr[i]->callbackOnPress != NULL))
							nex->_NexCompArr[i]->callbackOnPress();
						if((transferBuf[3] == NEX_EVENT_ON_RELEASE) && (nex->_NexCompArr[i]->callbackOnRelease != NULL))
							nex->_NexCompArr[i]->callbackOnRelease();
					}
				}
			}

			if(transferBuf[0] == NEX_RET_STRING_HEAD)
			{
				nex->NextTextLen = 0;
				for(int i = 0; i < (count - 4); i++)
				{
					nex->NexTextBuff[i] = transferBuf[i+1];
					nex->NextTextLen++;
				}
			}

			//Send the received command back for debugging purposes,
			//HAL_UART_Transmit(nex->nextionUARTHandle, transferBuf, count, 50);
			//HAL_UART_Transmit(nex->nextionUARTHandle, nex->NexTextBuff, nex->NextTextLen, 50);

			//Clear the dynamically allocated buffer after working with it,
			free(transferBuf);

			//Reset the buffer counters,
			nex->_pkgCount = 0;
			nex->_arrCount = 0;
		}

		HAL_UART_Receive_IT(nex->nextionUARTHandle, (uint8_t *)&nex->_RxData, 1);
	}

	//Return OK
	return 0;
}

uint8_t NextionGetText(Nextion *nex, NexComp *comp, char *buf)
{
	//Allocate a static buffer for combining the transfer command string
	char transmitBuff[NEXTION_TEXT_BUFF_LEN] = {0};

	//Combine required commands in a single string
	sprintf(transmitBuff, "get %s.txt", comp->objname);

	//Send the combined command to Nextion and wait for the received answer
	NextionSendCommand(nex, transmitBuff);

	//Copy the received string to the desired buffer (and add NULL character to the end),
	for(uint8_t i = 0; i < nex->NextTextLen; i++)
	{
		buf[i] = nex->NexTextBuff[i];
	}
	buf[nex->NextTextLen] = '\0';

	//Return OK
	return 0;
}

uint8_t NextionSetText(Nextion *nex, NexComp *comp, char *usertext)
{
	//Allocate a static buffer for combining the transfer command string
	char transmitBuff[NEXTION_TEXT_BUFF_LEN] = {0};

	//Combine required commands in a single string
	sprintf(transmitBuff, "%s.txt=\"%s\"", comp->objname, usertext);

	//Send the combined command to Nextion and wait for the received answer
	NextionSendCommand(nex, transmitBuff);

	//Return OK
	return 0;
}

char ENDTERMS[]={255,255,255};
uint8_t NextionSendCommand(Nextion *nex, char *_command)
{
	HAL_UART_Transmit(nex->nextionUARTHandle, (uint8_t *)_command, strlen((const char*)_command), NEXTION_TIMEOUT);
	NextionEndCommand(nex);

	//Return OK
	return 0;
}

uint8_t NextionEndCommand(Nextion *nex)
{
	uint8_t EndCommand[3] = {255, 255, 255};
	HAL_UART_Transmit(nex->nextionUARTHandle, EndCommand, 3, NEXTION_TIMEOUT);
	NextionRestartIT(nex);

	//Return OK
	return 0;
}

//Following two functions are not needed anymore and will be removed in the future,
uint8_t NextionRestartIT(Nextion *nex)
{
	HAL_UART_Receive_IT(nex->nextionUARTHandle, (uint8_t *)&nex->_RxData, 1);

	//Return OK
	return 0;
}

uint8_t NextionStopIT(Nextion *nex)
{
	//Stop UART interrupts. Required for Nextion communication functions,
	HAL_UART_AbortReceive_IT(nex->nextionUARTHandle);

	//Return OK
	return 0;
}
