/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file.
  ******************************************************************************
  * @attention
  *
  * This software is provided "AS IS", without warranty of any kind.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN1_Init(void);

/* USER CODE BEGIN Prototypes */

/**
  * @brief Invia un frame CAN standard (o extended) sul bus.
  * @param can_id    ID del frame (bit 0..10 per standard, 0..28 per extended).
  * @param pData     Puntatore ai dati da inviare.
  * @param dlc       Lunghezza del frame (0..8).
  * @return HAL status (HAL_OK se tutto ok).
  */
HAL_StatusTypeDef CAN_SendFrame(uint32_t can_id, uint8_t *pData, uint8_t dlc);

/**
  * @brief Callback richiamato quando un messaggio è in arrivo su FIFO0
  *        (viene automaticamente chiamato dall'HAL se abilitato).
  * @param hcan Puntatore all'handle del CAN (es. &hcan1).
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);

/**
  * @brief Funzione di elaborazione di un messaggio CAN ricevuto, da implementare a piacere
  *        o chiamare dentro la callback.
  * @param pHeader Puntatore all'header del frame ricevuto.
  * @param pData   Byte di payload ricevuti.
  */
void processCANMessage(CAN_RxHeaderTypeDef *pHeader, uint8_t *pData);

/**
  * @brief Invia un frame di comando CAN (20 byte) per configurare il dispositivo.
  * @param speed     Valore di velocità (codificato come nell'originale).
  * @param mode      Modalità operativa (es. 0x00 per NORMAL).
  * @param frameType Tipo di frame (0 per standard, 1 per extended).
  * @return 0 se ok, negativo in caso di errore.
  */
int CAN_CommandSettings(uint8_t speed, uint8_t mode, uint8_t frameType);

/**
  * @brief Invia un frame dati CAN costruito secondo la logica originale.
  * @param frameType 0 per frame standard, 1 per extended.
  * @param id_lsb    Byte meno significativo dell'ID.
  * @param id_msb    Byte più significativo dell'ID.
  * @param dataPayload Puntatore al payload da inviare.
  * @param dataLength Lunghezza del payload (0-8).
  * @return 0 se ok, negativo in caso di errore.
  */
int CAN_SendDataFrame(uint8_t frameType, uint8_t id_lsb, uint8_t id_msb, uint8_t *dataPayload, uint8_t dataLength);

/**
  * @brief Invia i frame iniziali di configurazione (tre frame da 8 byte ciascuno).
  * @return 0 se ok, negativo in caso di errore.
  */
int CAN_SendInitialFrames(void);

/**
  * @brief Decrementa gradualmente la velocità del motore fino ad arrestarlo.
  * @return 0 se ok, negativo in caso di errore.
  */

/**
  * @brief  Invia un frame CAN standard con ID e payload specificati.
  * @param  stdId: 11-bit Standard ID (0x000 - 0x7FF).
  * @param  data: array di bytes da inviare (max 8).
  * @param  data_len: lunghezza del payload (0..8).
  * @retval None
  */
HAL_StatusTypeDef CAN_SendFrame_Std(uint32_t stdId, uint8_t *data, uint8_t data_len);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

