#ifndef __MDR32F9Qx_CAN_HELPER_H
#define __MDR32F9Qx_CAN_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "MDR32F9Qx_lib.h"

#ifdef __cplusplus
} // extern "C" block end
#endif

#define CAN_STATUS_ERR_PASSIVE      (((uint32_t)0x1) <<  9) /*!< Flag indicating that CAN controller is in the ERROR PASSIVE state */
#define CAN_STATUS_ERR_BUSOFF       (((uint32_t)0x1) << 10) /*!< Flag indicating that CAN controller is in the BUS OFF state */

typedef struct
{
  uint32_t ID;                             
  uint8_t OVER_EN;                        
  uint8_t IDE;                             
  uint8_t DLC;                             
	uint8_t RTR;                             
}CAN_MyRxMsgHeaderTypeDef;

typedef struct
{
  CAN_MyRxMsgHeaderTypeDef Rx_Header;        /*!< Contains message header. */
  CAN_DataTypeDef Data;                    /*!< Contains received data. */
}CAN_MyRxMsgTypeDef;

FunctionalState CAN_GetCmdStatus(MDR_CAN_TypeDef* CANx);
void CAN_Buffer_TX_Init(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber);
void CAN_Buffer_RX_Init(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, FunctionalState OverWrite, uint32_t Mask, uint32_t Filter);
uint32_t CAN_GetTEC(MDR_CAN_TypeDef* CANx);
uint32_t CAN_GetREC(MDR_CAN_TypeDef* CANx);
void CAN_MyGetRawReceivedData(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_MyRxMsgTypeDef* RxMessage);
void CAN_ClearFlag(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, uint32_t Flag);

#endif /* __MDR32F9Qx_CAN_HELPER_H */
