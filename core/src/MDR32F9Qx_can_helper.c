/* Includes ------------------------------------------------------------------*/
#include "MDR32F9Qx_config.h"
#include "MDR32F9Qx_can.h"
#include "MDR32F9Qx_can_helper.h"

#define ASSERT_INFO_FILE_ID FILEID__MDR32F9X_CAN_C

/** @defgroup CAN_Private_FunctionPrototypes CAN Private Function Prototypes
  * @{
  */
static uint32_t CAN_ReadBufferSFR(__IO uint32_t* SFR);

FunctionalState CAN_GetCmdStatus(MDR_CAN_TypeDef* CANx)
{
	/* Check the parameters */
	assert_param(IS_CAN_ALL_PERIPH(CANx));
	if ( !(CANx->CONTROL & CAN_CONTROL_CAN_EN) ) {
		return DISABLE;
	} else {
		return ENABLE;
	}
}

void CAN_Buffer_TX_Init(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber)
{
	/* Check the parameters */
	assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_CAN_BUFFER(BufferNumber));
	
	CANx->BUF_CON[BufferNumber] |= CAN_STATUS_EN;
}

void CAN_Buffer_RX_Init(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, FunctionalState OverWrite, uint32_t Mask, uint32_t Filter)
{
	/* Check the parameters */
	assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_CAN_BUFFER(BufferNumber));
	
	// фильтр для приема пакетов
	CANx->CAN_BUF_FILTER[ BufferNumber ].MASK = Mask;
	CANx->CAN_BUF_FILTER[ BufferNumber ].FILTER = Filter;

	CAN_Receive(CANx, BufferNumber, OverWrite); // при ENABLE - разрешена перезапись буфера, DISABLE - перезапись запрещена	
	CANx->BUF_CON[BufferNumber] = (CAN_STATUS_RX_TXn | CAN_STATUS_EN); //буфер на прием
}

uint32_t CAN_GetTEC(MDR_CAN_TypeDef* CANx)
{
	/* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));

	return ( CANx->STATUS >> 24 );
}

uint32_t CAN_GetREC(MDR_CAN_TypeDef* CANx)
{
	/* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));

	return ( ( CANx->STATUS << 8 ) >> 24 );
}

/**
  * @brief  Reads received message (containing both header and data) from buffer.
  * @param  CANx: Select the CAN peripheral.
  *         This parameter can be one of the following values:
  *         CAN1, CAN2.
  * @param  BufferNumber: the number of the buffer that is used for reception.
  * @param  RxMessage: pointer to a CAN_MyRxMsgTypeDef.
  * @retval None
  */
void CAN_MyGetRawReceivedData(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, CAN_MyRxMsgTypeDef* RxMessage)
{
  uint32_t tmpreg;

  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_CAN_BUFFER(BufferNumber));

  /* Get the DLC */
  tmpreg = CAN_ReadBufferSFR(&(CANx->CAN_BUF[BufferNumber].DLC));

  RxMessage->Rx_Header.DLC = (uint8_t)(tmpreg & CAN_DLC_DATA_LENGTH);
	/* Get the RTR */
	RxMessage->Rx_Header.RTR = 0;
	if (tmpreg & CAN_DLC_RTR)
	{
		RxMessage->Rx_Header.RTR = 1;
	}
  /* Get the IDE */
  RxMessage->Rx_Header.IDE = CAN_ID_STD;
  if ((tmpreg & CAN_DLC_IDE) != 0)
  {
    RxMessage->Rx_Header.IDE = CAN_ID_EXT;
  }
  /* Get the OVER_EN */
  RxMessage->Rx_Header.OVER_EN = DISABLE;
  tmpreg = CAN_ReadBufferSFR(&(CANx->BUF_CON[BufferNumber]));
  if ((tmpreg & CAN_BUF_CON_OVER_EN) != 0)
  {
    RxMessage->Rx_Header.OVER_EN = ENABLE;
  }
  /* Get the Id */
  RxMessage->Rx_Header.ID = CAN_ReadBufferSFR(&(CANx->CAN_BUF[BufferNumber].ID));

  /* Get the data field */
  RxMessage->Data[0] = CAN_ReadBufferSFR(&(CANx->CAN_BUF[BufferNumber].DATAL));
  RxMessage->Data[1] = CAN_ReadBufferSFR(&(CANx->CAN_BUF[BufferNumber].DATAH));
}

void CAN_ClearFlag(MDR_CAN_TypeDef* CANx, uint32_t BufferNumber, uint32_t Flag)
{
	CANx->BUF_CON[BufferNumber] &= ~Flag;
}

static __INLINE uint32_t CAN_ReadBufferSFR(__IO uint32_t* SFR)
{
#ifdef WORKAROUND_MDR32F9QX_ERROR_0002
  /*
   * MDR32F9Qx Series Errata Notice, Error 0002:
   * The registers CAN_BUFxx_ID, CAN_BUFxx_DLC, CAN_BUFxx_DATAL,
   * CAN_BUFxx_DATAH, CAN_BUFxx_MASK or CAN_BUFxx_FILTER value
   * may be incorrectly read as 0 if CAN controller accessed any
   * of that registers at the moment of reading.
   * In such case, another read should be performed in a time less
   * than minimum CAN package.
   */
  uint32_t tmpreg;

  __disable_irq();
  tmpreg = *SFR;
  if (tmpreg == 0)
  {
    tmpreg = *SFR;
  }
  __enable_irq();
  return tmpreg;
#else
  return *SFR;
#endif  /* WORKAROUND_MDR32F9QX_ERROR_0002 */
}

