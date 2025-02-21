/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "can.h"

/* USER CODE BEGIN 0 */
#include <string.h>  // per memcpy
#include <stdio.h>   // per printf/Debug, se necessario
#include "usart.h"

/**
  * @brief  Invia un frame CAN standard con ID e payload specificati.
  * @param  stdId: 11-bit Standard ID (0x000 - 0x7FF).
  * @param  data: puntatore all'array di bytes da inviare.
  * @param  data_len: lunghezza del payload (0-8).
  * @retval HAL status (HAL_OK se tutto va bene, altro in caso di errore)
  */

HAL_StatusTypeDef CAN_SendFrame_Std(uint32_t cob_id, uint8_t *data, uint8_t data_len)
{
  CAN_TxHeaderTypeDef txHeader;
  uint8_t payload[8] = {0};  				// Buffer di 8 byte per il payload
  uint8_t cob_id_transmitted[2] = {0};   	// Buffer di 2 byte per il COB-ID da trasmettere
  uint32_t txMailbox;

  // Controllo sul puntatore ai dati
  if (data == NULL) {
    printf("Error: data pointer is NULL\r\n");
    return HAL_ERROR;
  }

  // Se data_len supera 8, troncala a 8 byte
  if (data_len > 8) {
    printf("Warning: data_len %d > 8, truncating to 8 bytes\r\n", data_len);
    data_len = 8;
  }

//  cob_id_transmitted[0] = (uint8_t)(cob_id & 0xFF);           // COB-ID low byte
//  cob_id_transmitted[1] = (uint8_t)((cob_id >> 8) & 0xFF);    // COB-ID high byte
//
//  printf("\r\nIl COB-ID in LE: [0x%02X 0x%02X]\r\n",
//         cob_id_transmitted[0],
//         cob_id_transmitted[1]);

  // Inizializza l'area payload a zero e copia i dati
  memset(&payload[0], 0, 8);
  memcpy(&payload[0], data, data_len);

  // Imposta il COB-ID come CAN identifier (questo verrà visualizzato in esadecimale)
  txHeader.StdId = cob_id;
  txHeader.ExtId = 0;
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.IDE = CAN_ID_STD;
  txHeader.DLC = data_len;  // DLC è il numero di byte del payload
  txHeader.TransmitGlobalTime = DISABLE;

  // Invio del messaggio tramite l'interfaccia CAN 1
  if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, payload, &txMailbox) != HAL_OK)
  {
    // Stampa di debug in caso di errore, eventualmente puoi implementare un retry o loggare in un buffer di errori
    printf("\r\nError: HAL_CAN_AddTxMessage failed (TxMailbox = %lu)\r\n", txMailbox);
    return HAL_ERROR;
  }

//  printf("\r\nTxMailbox = %lu\r\n", txMailbox);

  // Opzionale: stampa di debug per confermare l'invio
  printf("\r\nCAN TX: ID=0x%03lX DLC=%lu Data=", txHeader.StdId, (unsigned long)txHeader.DLC);
  for (uint8_t i = 0; i < txHeader.DLC; i++) {
      printf("%02X ", payload[i]);
  }

  printf("\r\n");

  return HAL_OK;

}

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 9;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  CAN_FilterTypeDef canFilterConfig;

  canFilterConfig.FilterBank = 0;                    // Primo filtro disponibile
  canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canFilterConfig.FilterIdHigh = 0x0000;            // Nessun filtro su ID
  canFilterConfig.FilterIdLow = 0x0000;
  canFilterConfig.FilterMaskIdHigh = 0x0000;        // "Maschera aperta"
  canFilterConfig.FilterMaskIdLow = 0x0000;
  canFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  canFilterConfig.FilterActivation = ENABLE;
  canFilterConfig.SlaveStartFilterBank = 14;         // Dipende dalla serie, su F7 spesso si usa 14 come separatore

  if (HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig) != HAL_OK)
  {
     // Gestisci errore
     Error_Handler();
  }

  HAL_CAN_ActivateNotification(&hcan1,
      CAN_IT_RX_FIFO0_MSG_PENDING  |  // notifica quando arriva un msg in FIFO0
      CAN_IT_RX_FIFO0_FULL         |
      CAN_IT_TX_MAILBOX_EMPTY      |
      CAN_IT_ERROR_WARNING         |
      CAN_IT_ERROR_PASSIVE         |
      CAN_IT_BUSOFF                |
      CAN_IT_LAST_ERROR_CODE       |
      CAN_IT_ERROR
  );

  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
     // Gestisci errore
     Error_Handler();
  }

  /* USER CODE END CAN1_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  Callback di ricezione messaggi FIFO0.
  * @param  hcan: puntatore all'handle CAN (CAN_HandleTypeDef).
  * @retval None
  */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];

  // Prova a leggere il messaggio dalla FIFO0
  if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK)
  {
    // Stampa di debug: mostra l'ID, il DLC e i dati ricevuti
    printf("\r\nCAN RX: ID=0x%03lX DLC=%ld Data=", rxHeader.StdId, rxHeader.DLC);
    for (uint8_t i = 0; i < rxHeader.DLC; i++)
    {
      printf("%02X ", rxData[i]);
    }
    printf("\r\n");

    // Qui potresti chiamare una funzione per processare il messaggio
    // Esempio: Process_CAN_Message(&rxHeader, rxData);
  }
  else
  {
    // Se la lettura fallisce, stampa un messaggio di errore per il debug
    printf("Error: Failed to retrieve CAN message from FIFO0\r\n");
  }
}

/* USER CODE END 1 */
