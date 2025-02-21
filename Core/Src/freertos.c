/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"    // Per dichiarare CAN_SendFrame
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CONSOLE_RX_BUFFER_SIZE 128
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

  if (defaultTaskHandle == NULL){
  	printf("Errore nella creazione del task.\r\n");
  	return;
  }
  else {
  	printf("Default task creato correttamente.\r\n");
  }

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */

void Send_Configuration_Frames();
int convertHexStringToBytes(const char *hexStr, uint8_t *outBuf, int maxBytes);
void print_Buffer_Content(uint32_t COB_identifier, uint8_t bufferData);

/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

	  char lineID[CONSOLE_RX_BUFFER_SIZE];
	  char lineData[CONSOLE_RX_BUFFER_SIZE];
	  uint32_t cob_id = 0;
	  uint8_t dataBuf[8];
	  int dataLen = 0;

	  Send_Configuration_Frames();

	  for(;;)
	  {
	    // Reset buffer
	    memset(lineID, 0, sizeof(lineID));
	    memset(lineData, 0, sizeof(lineData));
	    dataLen = 0;
	    cob_id = 0;

	    printf("\r\n====================================================\r\n");
	    fflush(stdout);
	    printf(">>> Insert COB-ID (hex): ");
	    fflush(stdout);

	    if (scanf("%s", lineID) != 1) {
	        printf("Errore nella lettura del COB-ID\r\n");
	        continue; // o gestisci l'errore opportunamente
	    }

	    HAL_Delay(10);

	    // Convert ID from hex string
	    cob_id = (uint32_t)strtol(lineID, NULL, 16);

	    printf("\r>>> Insert data to send (hex): ");
	    fflush(stdout);

	    scanf("%s", lineData);

	    HAL_Delay(10);

	    // Convert data hex string -> bytes
	    dataLen = convertHexStringToBytes(lineData, dataBuf, 8);
	    // Returns number of bytes of string

	    print_Buffer_Content(cob_id, dataBuf);

	    // Send CAN frame
	    CAN_SendFrame_Std(cob_id, dataBuf, dataLen);

	    HAL_Delay(10);

	    printf("\r\nFrame inviato! COB-ID=0x%lX, %d bytes.\r", cob_id, dataLen);

	    fflush(stdout);

	    HAL_Delay(10);

	  }

  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void Send_Configuration_Frames(){

	// Frame CAN da inviare una sola volta all'avvio
	// Servono a configurare lo stato della macchina a stati CANopen
	uint8_t frame1[8] = {0x2F, 0x40, 0x60, 0x00, 0x06, 0x00, 0x00, 0x00};
	uint8_t frame2[8] = {0x2F, 0x40, 0x60, 0x00, 0x07, 0x00, 0x00, 0x00};
	uint8_t frame3[8] = {0x2F, 0x40, 0x60, 0x00, 0x0F, 0x00, 0x00, 0x00};

	// Invio dei tre frame iniziali (ID 0x601, 8 byte)
	CAN_SendFrame_Std(0x601, frame1, 8);
	HAL_Delay(10); // piccola pausa per evitare sovrapposizioni
	CAN_SendFrame_Std(0x601, frame2, 8);
	HAL_Delay(10);
	CAN_SendFrame_Std(0x601, frame3, 8);
	HAL_Delay(10);

	return;

}

int convertHexStringToBytes(const char *hexStr, uint8_t *outBuf, int maxBytes) {
    int i = 0, dataLen = 0, highNibble = -1;
    while (hexStr[i] != '\0' && dataLen < maxBytes) {
        int val = -1;
        if (hexStr[i] >= '0' && hexStr[i] <= '9')
            val = hexStr[i] - '0';
        else if (hexStr[i] >= 'A' && hexStr[i] <= 'F')
            val = hexStr[i] - 'A' + 10;
        else if (hexStr[i] >= 'a' && hexStr[i] <= 'f')
            val = hexStr[i] - 'a' + 10;

        if(val >= 0) {
            if(highNibble < 0)
                highNibble = val;
            else {
                outBuf[dataLen++] = (uint8_t)((highNibble << 4) | val);
                highNibble = -1;
            }
        }
        i++;
    }
    return dataLen;
}

void print_Buffer_Content(uint32_t COB_identifier, uint8_t bufferData){

	printf("\r\nCOB-ID CONTENUTO: %lx", COB_identifier);
	printf("\r\nPAYLOAD 0 CONTENUTO: %x", bufferData[0]);
	printf("\r\nPAYLOAD 1 CONTENUTO: %x", bufferData[1]);
	printf("\r\nPAYLOAD 2 CONTENUTO: %x", bufferData[2]);
	printf("\r\nPAYLOAD 3 CONTENUTO: %x", bufferData[3]);
	printf("\r\nPAYLOAD 4 CONTENUTO: %x", bufferData[4]);
	printf("\r\nPAYLOAD 5 CONTENUTO: %x", bufferData[5]);
	printf("\r\nPAYLOAD 6 CONTENUTO: %x", bufferData[6]);
	printf("\r\nPAYLOAD 7 CONTENUTO: %x\r\n", bufferData[7]);

	return;

}

/* USER CODE END Application */

