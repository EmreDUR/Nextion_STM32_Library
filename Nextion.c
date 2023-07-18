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
		//Add the received byte to the array and increment the counter afterwards
		nex->_RxDataArr[nex->_arrCount] = nex->_RxData;
		nex->_arrCount++;

		//Count 0xFF
		if(nex->_RxData == 0xFF)
			nex->_pkgCount++;
		else
			nex->_pkgCount = 0;

		//Assume a package is received after three 0xFF commands,
		//and start processing the data
		if(nex->_pkgCount == 3)
		{
			//Determine the length (count) of the data
			uint8_t count = 0, FFCount = 0;
			for(uint8_t i = 0; FFCount < 3; i++)
			{
				count++;
				if(nex->_RxDataArr[i] == 0xFF) FFCount++;
			}

			//In case of a touch event call the callback function accordingly,
			if(nex->_RxDataArr[0] == NEX_RET_EVENT_TOUCH_HEAD)
			{
				//Loop through the component struct array,
				for(uint8_t i = 0; i < nex->_NexCompCount; i++)
				{
					//Detect the affected component by its Page and ID
					if( (nex->_RxDataArr[2] == (nex->_NexCompArr[i]->_id)) && (nex->_RxDataArr[1] == (nex->_NexCompArr[i]->_page)) )
					{
						//Call the desired On Press or On Release callback function,
						if((nex->_RxDataArr[3] == NEX_EVENT_ON_PRESS) && (nex->_NexCompArr[i]->callbackOnPress != NULL))
							nex->_NexCompArr[i]->callbackOnPress();
						if((nex->_RxDataArr[3] == NEX_EVENT_ON_RELEASE) && (nex->_NexCompArr[i]->callbackOnRelease != NULL))
							nex->_NexCompArr[i]->callbackOnRelease();
					}
				}
			}

			//If the received package contains string data
			if(nex->_RxDataArr[0] == NEX_RET_STRING_HEAD)
			{
				nex->NextTextLen = 0;
				for(int i = 0; i < (count - 4); i++)
				{
					nex->NexTextBuff[i] = nex->_RxDataArr[i+1];
					nex->NextTextLen++;
				}
			}

			//If the received package contains integer data
			if(nex->_RxDataArr[0] == NEX_RET_NUMBER_HEAD)
			{
				nex->NextNumBuff = ((uint32_t)nex->_RxDataArr[4]<<24)|((uint32_t)nex->_RxDataArr[3]<<16)|(nex->_RxDataArr[2]<<8)|(nex->_RxDataArr[1]);
			}

			//Reset the buffer counters
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
	HAL_Delay(10);

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

uint8_t NextionGetVal(Nextion *nex, NexComp *comp, int *valBuf)
{
	//Allocate a static buffer for combining the transfer command string
	char transmitBuff[NEXTION_TEXT_BUFF_LEN] = {0};

	//Combine required commands in a single string
	sprintf(transmitBuff, "get %s.val", comp->objname);

	//Send the combined command to Nextion and wait for the received answer
	NextionSendCommand(nex, transmitBuff);

	//Wait for Nextion to send the command back;
	//This line blocks the code for a while and provides a crude ensurement
	HAL_Delay(50);

	//Get the received value from the buffer and pass it to the user variable
	*valBuf = nex->NextNumBuff;

	//Return OK
	return 0;
}

uint8_t NextionSetVal(Nextion *nex, NexComp *comp, int userval)
{
	//Allocate a static buffer for combining the transfer command string
	char transmitBuff[NEXTION_TEXT_BUFF_LEN] = {0};

	//Combine required commands in a single string
	sprintf(transmitBuff, "%s.val=%d", comp->objname, userval);

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
