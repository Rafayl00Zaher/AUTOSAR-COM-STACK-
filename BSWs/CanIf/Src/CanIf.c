/********************************************************************************************************************************
 **
 FileName:                   CanIf.c
 AUTOSAR Version:            4.2.2
                                                                                                                                                                          **
 ********************************************************************************************************************************/
/********************************************************************************************************************************
 **                                                          Includes                                                                                                **
 ********************************************************************************************************************************/
#include <CanIf.h>
#include "Can.h"

/********************************************************************************************************************************
 **                                                       Global Variables                                                                                       **
 ********************************************************************************************************************************/
extern CanIf_ConfigType CanIf ;

 static enum CanIfStateType{CANIF_UNINIT, CANIF_INIT} CanIfState = CANIF_UNINIT;
 uint16 ModuleId=0x12;
 CanIf_ControllerModeType CanIfControllerMode[NUMBER_OF_CONTROLLERS]={CANIF_CS_UNINIT,CANIF_CS_UNINIT};
 CanIf_PduModeType  CanIfPduMode[NUMBER_OF_CHANNELS]={CANIF_OFFLINE,CANIF_OFFLINE};
#if(CanIfPublicTxConfirmPollingSupport == true)

 CanIf_TxConfirmationInfoType  CanIf_TxConfirmationInfo[CanIfMaxTxPduCfg]={false};

 
#endif
#if(CanIfPublicTxBuffering == true)
static CanIf_TxBufferType CanIfTxBuffer[NUMBER_OF_BUFFERS] = {
    {
        .CanIfBufferCfgRef = &CanIf.CanIfInitCfg.CanIfBufferCfg[0U],
        .CanIfTxBufferFront = -1,
        .CanIfTxBufferRear = -1,
        .CanIfTxBufferSize = -1,
        .CanIfTxBufferPduAvailable = {false}
    }
};
#endif

 
 /********************************************************************************************************************************
**                                                        Local Functions                                                                                        **
*********************************************************************************************************************************/
#if(CanIfPublicTxConfirmPollingSupport == true)

static void CanIf_SetTxConfirmationInfoBuffer(PduIdType pduid){
    CanIf_TxConfirmationInfo[pduid]= true;
}
static CanIf_TxConfirmationInfoType CanIf_GetTxConfirmationInfoBuffer(PduIdType pduid){
    return CanIf_TxConfirmationInfo[pduid];
}

static void CanIf_ClearTxConfirmationInfoBuffer(void){
    uint32  pduidndex;
    for(pduidndex=0 ; pduidndex <= CanIfMaxTxPduCfg ; pduidndex ++){
        CanIf_TxConfirmationInfo[pduidndex]= false;
    }
}


#endif

#if(CanIfPublicTxBuffering == true)
static Std_ReturnType CanIf_TxBufferGet(CanIfBufferCfg* CanIfTxPduBufferRefLocal, CanIf_TxBufferType *CanIfTxBufferLocal);
static Std_ReturnType CanIf_TxBufferNotEmpty(CanIf_TxBufferType *CanIfTxBufferLocal);
static Std_ReturnType CanIf_TxBufferNotFull(CanIf_TxBufferType *CanIfTxBufferLocal);
static Std_ReturnType CanIf_TxBufferDequeue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr);
static Std_ReturnType CanIf_TxBufferEnqueue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr);
static Std_ReturnType CanIf_TxBufferClear(uint8 ControllerId);
#endif
static Std_ReturnType CanIf_GetTxPdu(PduIdType CanIfTxSduId, CanIfTxPduCfg* TxPduPtr);
static Std_ReturnType CanIf_GetRxPdu(PduIdType CanIfRxSduId, CanIfRxPduCfg* RxPduPtr);

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferGet
Parameters (in):                      CanIfTxPduBufferRefLocal-->Tx PDU reference to a CanIf buffer configuration.
                                           CanIfBufferRefLocal -->Reference to the Tx Buffer
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                           This service searches for the CanIf buffer referenced by the available
                                        CanIf buffers and the Tx Pdu
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferGet(CanIfBufferCfg *CanIfTxPduBufferRefLocal, CanIf_TxBufferType *CanIfBufferRefLocal)
{
    uint8 BufferIndex;

    /* Check if the reference to the buffer configurations is a valid reference */
    if(CanIfTxPduBufferRefLocal == NULL)
    {
        return E_NOT_OK;
    }
    else
    {
        /* Loop over the available buffers and return the one with the same buffer configuration reference as the Tx PDU */
        for(BufferIndex = 0; BufferIndex < NUMBER_OF_BUFFERS ; BufferIndex ++)
        {
            if(CanIfTxBuffer[BufferIndex].CanIfBufferCfgRef == CanIfTxPduBufferRefLocal)
            {
                CanIfBufferRefLocal = &CanIfTxBuffer[BufferIndex];
                return E_OK;
            }
            else
            {

            }
        }
    }

    return E_NOT_OK;
}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferNotEmpty
Parameters (in):                        CanIfTxBufferRefLocal -->Reference to the Tx Buffer
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                           This service checks if the buffer is empty
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferNotEmpty(CanIf_TxBufferType *CanIfTxBufferRefLocal)
{
    /* Check if the reference to the buffer is a valid reference */
    if(CanIfTxBufferRefLocal == NULL)
    {
        return E_NOT_OK;
    }
    else
    {
        if(CanIfTxBufferRefLocal->CanIfTxBufferSize < 0)
        {
            return E_NOT_OK;
        }
        else
        {
            return E_OK;
        }
    }

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferNotFull
Parameters (in):                        CanIfTxBufferRefLocal -->Reference to the Tx Buffer
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                           This service checks if the buffer is full
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferNotFull(CanIf_TxBufferType *CanIfTxBufferRefLocal)
{
    /* Check if the reference to the buffer is a valid reference */
    if(CanIfTxBufferRefLocal == NULL)
    {
        return E_NOT_OK;
    }
    else
    {
        if(CanIfTxBufferRefLocal->CanIfTxBufferSize < CanIfTxBufferRefLocal->CanIfBufferCfgRef->CanIfBufferSize)
        {
            return E_OK;
        }
        else
        {
            return E_NOT_OK;
        }
    }

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferDequeue
 Parameters (in):                               TxPdu -->Reference to the Tx PDU
                                                PduInfoPtr --> Reference to the PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                           This service returns the next PDU in the FIFO buffer
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferDequeue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr)
{
    CanIf_TxBufferType* CanIfTxBufferRefLocal = NULL;

    /* Check if the reference to the TxPdu is a valid reference */
    if(TxPdu == NULL)
    {
        return E_NOT_OK;
    }
    else
    {
        /* Get the buffer of the Tx PDU */
        if(CanIf_TxBufferGet(TxPdu->CanIfTxPduBufferRef, CanIfTxBufferRefLocal) == E_NOT_OK)
        {
            return E_NOT_OK;
        }
        else
        {
            /* Check if the buffer is empty */
            if(CanIf_TxBufferNotEmpty(CanIfTxBufferRefLocal) == E_NOT_OK)
            {
                return E_NOT_OK;
            }
            else
            {
                /* Set the PDU pointer to the next PDU in the buffer */
                PduInfoPtr = &CanIfTxBufferRefLocal->CanIfTxBufferPduData[CanIfTxBufferRefLocal->CanIfTxBufferFront];
                /* Set the PDU state as not available in the buffer */
                CanIfTxBufferRefLocal->CanIfTxBufferPduAvailable[TxPdu->CanIfTxPduId] = false;
                /* Decrease the current avaiable PDUs */
                CanIfTxBufferRefLocal->CanIfTxBufferSize --;
                CanIfTxBufferRefLocal->CanIfTxBufferFront ++;
                return E_OK;
            }
        }
    }

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferEnqueue
 Parameters (in):                               TxPdu -->Reference to the Tx PDU
                                                PduInfoPtr --> Reference to the PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                               This service adds a PDU to the FIFO buffer
 *******************************************************************************************************************************/
#if(CanIfPublicTxBuffering == true)
Std_ReturnType CanIf_TxBufferEnqueue(CanIfTxPduCfg* TxPdu, const Can_PduType* PduInfoPtr)
{
    CanIf_TxBufferType* CanIfTxBufferRefLocal = NULL;

    /* Check if the reference to the PDUs is a valid reference */
    if(TxPdu == NULL || PduInfoPtr == NULL)
    {
        return E_NOT_OK;
    }
    else
    {
        /* Get the buffer of the Tx PDU */
        if(CanIf_TxBufferGet(TxPdu->CanIfTxPduBufferRef, CanIfTxBufferRefLocal) == E_NOT_OK)
        {
            return E_NOT_OK;
        }
        else
        {
            /* Check if the buffer is full */
            if(CanIf_TxBufferNotFull(CanIfTxBufferRefLocal) == E_NOT_OK)
            {
                return E_NOT_OK;
            }
            else
            {
                /* Check if the PDU is already stored in the buffer */
                if(CanIfTxBufferRefLocal->CanIfTxBufferPduAvailable[TxPdu->CanIfTxPduId] == true)
                {
                    return E_NOT_OK;
                }
                else
                {
                    /* The buffer is not initialized yet. We will initialize the buffer by making both front and rear 0 and size 1
                    and give a value to the first PDU */
                    if(CanIfTxBufferRefLocal->CanIfTxBufferSize < 0)
                    {
                        CanIfTxBufferRefLocal->CanIfTxBufferPduData[0] = *PduInfoPtr;
                        CanIfTxBufferRefLocal->CanIfTxBufferRear = 0;
                        CanIfTxBufferRefLocal->CanIfTxBufferFront = 0;
                        CanIfTxBufferRefLocal->CanIfTxBufferSize = 1;
                    }
                    /* This is the case when the rear is the last PDU in the buffer. We will add a new PDU at the index 0 */
                    else if(CanIfTxBufferRefLocal->CanIfTxBufferRear == CanIfTxBufferRefLocal->CanIfBufferCfgRef->CanIfBufferSize - 1)
                    {
                        CanIfTxBufferRefLocal->CanIfTxBufferPduData[0] = *PduInfoPtr;
                        CanIfTxBufferRefLocal->CanIfTxBufferRear = 0;
                        CanIfTxBufferRefLocal->CanIfTxBufferSize ++;
                    }
                    /* This is the normal case*/
                    else
                    {
                        CanIfTxBufferRefLocal->CanIfTxBufferPduData[CanIfTxBufferRefLocal->CanIfTxBufferRear + 1] = *PduInfoPtr;
                        CanIfTxBufferRefLocal->CanIfTxBufferRear ++;
                        CanIfTxBufferRefLocal->CanIfTxBufferSize ++;
                    }

                    /* Set the PDU as currently available in the buffer */
                    CanIfTxBufferRefLocal->CanIfTxBufferPduAvailable[TxPdu->CanIfTxPduId] = true;
                    return E_OK;
                }
            }
        }
    }

}
#endif

/*********************************************************************************************************************************
 Service name:                                       CanIf_TxBufferClear
 Parameters (in):                             ControllerId -->Controller Id
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                               This service clear the FIFO buffer
 *******************************************************************************************************************************/
Std_ReturnType CanIf_TxBufferClear(uint8 ControllerId)
{
    uint8 BufferIndex;
    uint8 PduIndex;
    if(ControllerId < NUMBER_OF_CONTROLLERS)
    {
        for(BufferIndex = (uint8)0; BufferIndex < NUMBER_OF_BUFFERS; BufferIndex++)
        {
            if(CanIfTxBuffer[BufferIndex].CanIfBufferCfgRef->CanIfBufferHthRef->CanIfHthCanCtrlIdRef->CanIfCtrlId == ControllerId)
            {
                CanIfTxBuffer[BufferIndex].CanIfTxBufferFront = -1;
                CanIfTxBuffer[BufferIndex].CanIfTxBufferRear = -1;
                CanIfTxBuffer[BufferIndex].CanIfTxBufferSize = -1;

                for(PduIndex = 0; PduIndex < CanIfMaxTxPduCfg ; PduIndex++)
                {
                    CanIfTxBuffer[BufferIndex].CanIfTxBufferPduAvailable[PduIndex] = false;
                }
            }
            else
            {

            }
        }
    }
    else
    {
        return E_NOT_OK;
    }

    return E_OK;

}

/*********************************************************************************************************************************
 Service name:                                       CanIf_GetTxPdu
 Parameters (in):                               CanIfTxSduId -->Id of the Tx PDU
                                                TxPduPtr --> Reference to the Tx PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                               This service returns the Tx PDU with the desired ID
 *******************************************************************************************************************************/
Std_ReturnType CanIf_GetTxPdu(PduIdType CanIfTxSduId, CanIfTxPduCfg* TxPduPtr)
{
    uint32 TxPduIndex;
    if(CanIfTxSduId < CanIfMaxTxPduCfg)
    {
        for(TxPduIndex = 0; TxPduIndex < CanIfMaxTxPduCfg ; TxPduIndex++)
        {
            if(CanIf.CanIfInitCfg.CanIfTxPduCfg[TxPduIndex].CanIfTxPduId == CanIfTxSduId)
            {
                TxPduPtr = &CanIf.CanIfInitCfg.CanIfTxPduCfg[TxPduIndex];
                return E_OK;
            }
            else
            {

            }
        }
    }
    else
    {
        return E_NOT_OK;
    }

    return E_NOT_OK;
}




/*********************************************************************************************************************************
 Service name:                                       CanIf_GetRxPdu
 Parameters (in):                               CanIfRxSduId -->Id of the Rx PDU
                                                RxPduPtr --> Reference to the Rx PDU
 Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                            Std_ReturnType
 Description:                               This service returns the Rx PDU with the desired ID
 *******************************************************************************************************************************/
Std_ReturnType CanIf_GetRxPdu(PduIdType CanIfRxSduId, CanIfRxPduCfg* RxPduPtr)
{
    uint32 RxPduIndex;
    if(CanIfRxSduId < CanIfMaxRxPduCfg)
    {
        for(RxPduIndex = 0; RxPduIndex < CanIfMaxRxPduCfg ; RxPduIndex++)
        {
            if(CanIf.CanIfInitCfg.CanIfRxPduCfg[RxPduIndex].CanIfRxPduId == CanIfRxSduId)
            {
                RxPduPtr = &CanIf.CanIfInitCfg.CanIfRxPduCfg[RxPduIndex];
                return E_OK;
            }
            else
            {

            }
        }
    }
    else
    {
        return E_NOT_OK;
    }

    return E_NOT_OK;
}







/********************************************************************************************************************************
**                                                      Functions  defination                                                                        **
*********************************************************************************************************************************/


/*********************************************************************************************************************************
 Service name:                                              CanIf_Init
 Service ID[hex]:                                               0x01
 Sync/Async:                                               Synchronous
 Reentrancy:                                               Non Reentrant
 Parameters (in):                                           ConfigPtr
 Parameters (inout):                                          None
 Parameters (out):                                             None
 Return value:                                                   None
 Description:
 This service Initializes internal and external interfaces of the CAN Inter-
face for the further processing.
 *******************************************************************************************************************************/
void CanIf_Init(const CanIf_ConfigType* ConfigPtr){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}

/*********************************************************************************************************************************
 Service name:                                       CanIf_SetControllerMode
 Service ID[hex]:                                               0x03
 Sync/Async:                                               Asynchronous
 Reentrancy:                                               Reentrant (Not for the same controller)
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
																	ControllerMode -->Requested mode transition
 Parameters (inout):                                          None
 Parameters (out):                                             None
 Return value:                                                   Std_ReturnType
 Description:
					This service calls the corresponding CAN Driver service for changing of
					the CAN controller mode.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_SetControllerMode(uint8 ControllerId,CanIf_ControllerModeType ControllerMode){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}


/*********************************************************************************************************************************
 Service name:                                       CanIf_GetControllerMode
 Service ID[hex]:                                               0x04
 Sync/Async:                                               Synchronous
 Reentrancy:                                               Non Reentrant
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
 Parameters (inout):                                          None
 Parameters (out):                                     ControllerModePtr -->Pointer to a memory location, where the current
																								mode of the CAN controller will be stored.
 Return value:                                                   Std_ReturnType
 Description:
					This service reports about the current status of the requested CAN controller.
 *******************************************************************************************************************************/
Std_ReturnType CanIf_GetControllerMode(uint8 ControllerId,CanIf_ControllerModeType* ControllerModePtr){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}

/*********************************************************************************************************************************
 Service name:                                       CanIf_Transmit
 Service ID[hex]:                                               0x05
 Sync/Async:                                               Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                           CanIfTxSduId    -->L-SDU handle to be transmitted.This handle specifies the corresponding CAN L-
																								SDU ID and implicitly the CAN Driver instance as 
																								well as the corresponding CAN controller device.
																	CanIfTxInfoPtr   -->Pointer to a structure with CAN L-SDU related data: 
																								DLC and pointer to CAN L-SDU buffer including the
																								MetaData of dynamic L-PDUs.
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              Std_ReturnType
 Description:
					This service initiates a request for transmission of the CAN L-PDU specified by 	
					 the CanTxSduId and CAN related data in the L-SDU structure.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_Transmit(PduIdType CanIfTxSduId,const PduInfoType* CanIfTxInfoPtr){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}

/*********************************************************************************************************************************
 Service name:                                       CanIf_ReadRxPduData
 Service ID[hex]:                                               0x06
 Sync/Async:                                               Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                           CanIfRxSduId    -->Receive L-SDU handle specifying the corresponding 
																								CAN L-SDU ID and implicitly the CAN Driver instance 
																								as well as the corresponding CAN controller device.
Parameters (inout):                                          None
 Parameters (out):                                       CanIfRxInfoPtr   -->Pointer to a structure with CAN L-SDU related data:
																								DLC and pointer to CAN L-SDU buffer including the
																								MetaData of dynamic L-PDUs.
 Return value:                                              Std_ReturnType
 Description:
					This service provides the CAN DLC and the received data of the requested
					CanIfRxSduId to the calling upper layer.
 *******************************************************************************************************************************/
Std_ReturnType CanIf_ReadRxPduData(PduIdType CanIfRxSduId,PduInfoType* CanIfRxInfoPtr){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}


/*********************************************************************************************************************************
 Service name:                                       CanIf_ReadTxNotifStatus
 Service ID[hex]:                                               0x07
 Sync/Async:                                               Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                           CanIfTxSduId    -->L-SDU handle to be transmitted.
																								This handle specifies the corresponding CAN L-
																								SDU ID and implicitly the CAN Driver instance as
																								well as the corresponding CAN controller device.
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              CanIf_NotifStatusType
 Description:
					This service returns the confirmation status (confirmation occurred or not)
					of a specific static or dynamic CAN Tx L-PDU, requested by the
					CanIfTxSduId.
 *******************************************************************************************************************************/

CanIf_NotifStatusType CanIf_ReadTxNotifStatus(PduIdType CanIfTxSduId){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	






























}


/*********************************************************************************************************************************
 Service name:                                       CanIf_ReadRxNotifStatus
 Service ID[hex]:                                               0x08
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                            CanIfRxSduId    -->Receive L-SDU handle specifying the corresponding 
																								CAN L-SDU ID and implicitly the CAN Driver instance 
																								as well as the corresponding CAN controller device.
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              CanIf_NotifStatusType
 Description:
					This service returns the indication status (indication occurred or not) of a
					specific CAN Rx L-PDU, requested by the CanIfRxSduId.
 *******************************************************************************************************************************/

CanIf_NotifStatusType CanIf_ReadRxNotifStatus(PduIdType CanIfRxSduId){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}

/*********************************************************************************************************************************
 Service name:                                         CanIf_SetPduMode
 Service ID[hex]:                                               0x09
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Non Reentrant
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
																    PduModeRequest-->Requested PDU mode change
Parameters (inout):                                          None
 Parameters (out):                                            None
 Return value:                                              Std_ReturnType
 Description:
					This service sets the requested mode at the L-PDUs of a predefined logical PDU channel.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_SetPduMode(uint8 ControllerId, CanIf_PduModeType PduModeRequest){
	
#if(CanIfPublicDevErrorDetect == true)

    uint8 InstanceId=0x00;
    uint8 ApiId=0x09;
    /*
       [SWS_CANIF_00344] d Caveats of CanIf_SetPduMode(): CanIf must be initialized after Power ON.
    */
    if(CanIfState == CANIF_UNINIT){
        Det_ReportError(ModuleId,InstanceId, ApiId,CANIF_E_UNINIT);
    }
    /*
     * [SWS_CANIF_00341] d If CanIf_SetPduMode() is called with invalid ControllerId
       , CanIf shall report development error code CANIF_E_PARAM_CONTROLLERID
        to the Det_ReportError service of the DET module. c(SRS_BSW_00323)
     */
    if(ControllerId > NUMBER_OF_CONTROLLERS){
        Det_ReportError(ModuleId,InstanceId, ApiId,CANIF_E_PARAM_CONTROLLERID);
    }
#endif
    /*
     * [SWS_CANIF_00874] d The service CanIf_SetPduMode() shall not accept any request and shall return E_NOT_OK,
     *  if the CCMSM referenced by ControllerId is notin state CANIF_CS_STARTED.
     */
    CanIf_GetControllerMode(ControllerId, &CanIfControllerMode[ControllerId]);
    if(CanIfControllerMode[ControllerId] != CANIF_CS_STARTED)
    {
        return E_NOT_OK;
    }

    else
    {
        /*
         * [SWS_CANIF_00860] d If CanIf_SetPduMode() is called with invalid PduModeRequest,
           CanIf shall report development error code CANIF_E_PARAM_PDU_MODE to the Det_ReportError service of the DET module.
         */
        if(PduModeRequest != CANIF_OFFLINE || PduModeRequest != CANIF_TX_OFFLINE || PduModeRequest != CANIF_TX_OFFLINE_ACTIVE || PduModeRequest != CANIF_ONLINE )
        {
#if(CanIfPublicDevErrorDetect == true)
            uint8 InstanceId=0x00;
            uint8 ApiId=0x09;
            Det_ReportError(ModuleId,InstanceId, ApiId,CANIF_E_PARAM_PDU_MODE);
#endif
            return E_NOT_OK;

        }
        else
        {
            CanIfPduMode[ControllerId]=PduModeRequest;
            return  E_OK ;
        }
    }



	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}

/*********************************************************************************************************************************
 Service name:                                         CanIf_GetPduMode
 Service ID[hex]:                                               0x0a
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant (Not for the same channel)
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
Parameters (inout):                                          None
 Parameters (out):                                            PduModePtr  -->Pointer to a memory location, where the current
																								mode of the logical PDU channel will be stored.
 Return value:                                              Std_ReturnType
 Description:
					This service reports the current mode of a requested PDU channel.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_GetPduMode(uint8 ControllerId,CanIf_PduModeType* PduModePtr){









































}


/*********************************************************************************************************************************
 Service name:                                         CanIf_SetBaudrate
 Service ID[hex]:                                               0x27
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant for different ControllerIds. Non reentrant for the same ControllerId.
 Parameters (in):                                           ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
																	BaudRateConfigID-->references a baud rate configuration by ID (see Can-
																									ControllerBaudRateConfigID)							
Parameters (inout):                                          None
 Parameters (out):                                            PduModePtr  -->Pointer to a memory location, where the current
																								mode of the logical PDU channel will be stored.
 Return value:                                              Std_ReturnType
 Description:
					This service shall set the baud rate configuration of the CAN controller.
					Depending on necessary baud rate modifications the controller might
					have to reset.
 *******************************************************************************************************************************/

Std_ReturnType CanIf_SetBaudrate(uint8 ControllerId,uint16 BaudRateConfigID) {
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}



/*********************************************************************************************************************************
 Service name:                                         CanIf_TxConfirmation
 Service ID[hex]:                                               0x13
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                           CanTxPduId      -->L-PDU handle of CAN L-PDU successfully transmitted.
																				This ID specifies the corresponding CAN L-PDU ID
																				and implicitly the CAN Driver instance as well as the
																				corresponding CAN controller device.
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service confirms a previously successfully processed transmission of a CAN TxPDU.
 *******************************************************************************************************************************/

void CanIf_TxConfirmation(PduIdType CanTxPduId){
    CanIfTxPduCfg* txpduptr_0x13=0;
    Can_ReturnType transmitcheck_0x13;
    CanIf_GetTxPdu(CanTxPduId, txpduptr_0x13);


#if(CanIfPublicDevErrorDetect == true)
    uint8 InstanceId=0x00;
    uint8 ApiId=0x13;
    /*
     * [SWS_CANIF_00410] d If parameter CanTxPduId of CanIf_TxConfirmation()
       has an invalid value, CanIf shall report development error code CANIF_E_PARAM_LPDU
       to the Det_ReportError service of the DET module, when CanIf_TxConfirmation()
       is called.
     */
    if(CanTxPduId != txpduptr_0x13->CanIfTxPduId ) {
        Det_ReportError(ModuleId,InstanceId, ApiId,CANIF_E_INVALID_TXPDUID);
    }
#endif
    /*
     * The CanIf must be initialized after Power ON.
     */
    if(CanIfState == CANIF_UNINIT){
#if(CanIfPublicDevErrorDetect == true)
        Det_ReportError(ModuleId,InstanceId, ApiId,CANIF_E_UNINIT);
#endif
    }
    else{
#if(CanIfPublicTxConfirmPollingSupport == true)
        /*
         * [SWS_CANIF_00740] d If CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT (see ECUC_CanIf_00
            is enabled, CanIf shall buffer the information about a received TxConfirmation per
            CAN Controller, if the CCMSM of that controller is in state CANIF_CS_STARTED.
         */
        CanIf_GetControllerMode(txpduptr_0x13->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthCanCtrlIdRef->CanIfCtrlId, &CanIfControllerMode[txpduptr_0x13->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthCanCtrlIdRef->CanIfCtrlId]);
           if(CanIfControllerMode[txpduptr_0x13->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthCanCtrlIdRef->CanIfCtrlId] == CANIF_CS_STARTED)
           {
               CanIf_TxConfirmationInfo[txpduptr_0x13->CanIfTxPduId]= true;
           }


#endif
#if(CanIfPublicTxBuffering == true)
           /*
            * The transmit buffers of CanIf checked, whether a pending L-PDU is stored or not.
            */
        if(CanIf_TxBufferNotEmpty(CanIfTxBuffer)== E_OK){
            /*
             * In case of pending L-PDUs in the transmit buffers the highest
               priority order the latest L-PDU is requested for transmission by Can_Write().
             */
            CanIf_TxBufferGet(CanIf.CanIfInitCfg.CanIfTxPduCfg->CanIfTxPduBufferRef, CanIfTxBuffer);
            transmitcheck_0x13 = Can_Write(txpduptr_0x13->CanIfTxPduBufferRef->CanIfBufferHthRef->CanIfHthIdSymRef->CanObjectId, CanIfTxBuffer->CanIfTxBufferPduData);
            if(transmitcheck_0x13 == CAN_OK){
              /*
               * The L-PDU pending for transmission is removed from the
                 transmission buffers by CanIf.
               */
                CanIf_TxBufferDequeue(txpduptr_0x13, CanIfTxBuffer->CanIfTxBufferPduData);
            }
            else if(transmitcheck_0x13 == CAN_BUSY){
                /*do nothing*/
            }
            else{
                /*misra*/
            }
        }
        else if(CanIf_TxBufferNotEmpty(CanIfTxBuffer)== E_NOT_OK){
            /*do nothing*/
        }
        else{
            /*misra*/
        }

#endif
#if(CanIfPublicReadTxPduNotifyStatusApi == true)

        /*
         * [SWS_CANIF_00391] d If configuration parameters CANIF_PUBLIC_READTXPDU_NOTIFY_STATUS
        (ECUC_CanIf_00609) and CANIF_TXPDU_READ_NOTIFYSTATUS (ECUC_CanIf_00589)
        for the Transmitted L-PDU are set to TRUE, and if CanIf_TxConfirmation() is
        called, CanIf shall set the notification status for the Transmitted L-PDU.
         */


#endif
        /*
         * [SWS_CANIF_00414] d Configuration of CanIf_TxConfirmation(): Each Tx L-
        PDU (see ECUC_CanIf_00248) has to be configured with a corresponding transmit
        confirmation service of an upper layer module (see [SWS_CANIF_00011]) which is
        called in CanIf_TxConfirmation().
         */

        if(CanIf.CanIfInitCfg.CanIfTxPduCfg[CanTxPduId].CanIfTxPduUserTxConfirmationUL == PDUR ){

            /* PDUR_TxConfirmation(CanTxPduId);*/



        }
        else if (CanIf.CanIfInitCfg.CanIfTxPduCfg[CanTxPduId].CanIfTxPduUserTxConfirmationUL == CAN_TP){

            /*CAN_TP_TxConfirmation(CanTxPduId);*/



        }
        else {
            /* misra */
        }

    }














	
	
	
	
	
	

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}



/*********************************************************************************************************************************
 Service name:                                         CanIf_RxIndication
 Service ID[hex]:                                               0x14
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                           Mailbox      -->Identifies the HRH and its corresponding CAN Controller.
																	PduInfoPtr  -->Pointer to the received L-PDU.
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service indicates a successful reception of a received CAN Rx L-PDU
					to the CanIf after passing all filters and validation checks.
 *******************************************************************************************************************************/

void CanIf_RxIndication(const Can_HwType* Mailbox,const PduInfoType* PduInfoPtr){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}


/*********************************************************************************************************************************
 Service name:                                         CanIf_ControllerBusOff
 Service ID[hex]:                                               0x14
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                            ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service indicates a Controller BusOff event referring to the corresponding 
					CAN Controller with the abstract CanIf ControllerId.
 *******************************************************************************************************************************/

void CanIf_ControllerBusOff(uint8 ControllerId){
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}


/*********************************************************************************************************************************
 Service name:                                         CanIf_ControllerModeIndication
 Service ID[hex]:                                               0x17
 Sync/Async:                                                 Synchronous
 Reentrancy:                                                 Reentrant
 Parameters (in):                                            ControllerId      -->Abstracted CanIf ControllerId which is assigned to a
                                                                                                CAN controller, which is requested for mode transition.
                                                                    ControllerMode  -->Mode to which the CAN controller transitioned
Parameters (inout):                                          None
Parameters (out):                                             None
Return value:                                                   None
Description:
					This service indicates a controller state transition referring to the corresponding
					CAN controller with the abstract CanIf ControllerId.
 *******************************************************************************************************************************/

void CanIf_ControllerModeIndication(uint8 ControllerId,CanIf_ControllerModeType ControllerMode)
{
	
   
    uint8 ApiId=0x17;                                         /*ID of API service in which error is detected (defined in SWS of calling module)*/
    uint8 InstanceId=0;                                       /*The identifier of the index based instance of a module, starting from 0,
                                                                If the module is a single instance module it shall pass 0 as the InstanceId.*/

    /*
     * [SWS_CANIF_00661] d If the switch CANIF_PUBLIC_DEV_ERROR_DETECT is enabled,
       all CanIf API services shall:
       - not execute their normal operation
       - report to the DET (using CANIF_E_UNINIT)
       unless the CanIf has been initialized with a preceding call of CanIf_Init().
     */

        #if (CanIfDevelopmentError==1)            /*Development Error in case of UN_INT of CanIf or invalid parameters*/


        /*
         * [SWS_CANIF_00702] d If CanIf was not initialized before calling CanIf_ControllerModeIndication(),
           CanIf shall not execute state transition notification, when CanIf_ControllerModeIndication()
           is called. c()
         */
           if(CanIfState == CANIF_UNINIT)
            {
              // Det_ReportError(ModuleId,InstanceId,ApiId,CANIF_E_UNINIT);

            }

         /*
          * [SWS_CANIF_00700] d If parameter ControllerId of CanIf_ControllerModeIndication()
            has an invalid value, CanIf shall report development error code CANIF_E_PARAM_CONTROLLER
            to the Det_ReportError service of the DET module, when CanIf_ControllerModeIndication()
            is called. c
          */
           if(ControllerId >=NUMBER_OF_CONTROLLERS)
             {
               //Det_ReportError(ModuleId,InstanceId,ApiId,CANIF_E_PARAM_CONTROLLERID);
             }


        #else

                 switch (ControllerMode)
                    {
                        /*
                         * Indication to the new state setted by CanDrv
                         */

                     case CANIF_CS_SLEEP:
                         CanIfControllerMode[ControllerId] =CANIF_CS_SLEEP;
                            break;

                     case CANIF_CS_STARTED:
                         CanIfControllerMode[ControllerId] =CANIF_CS_STARTED;
                            break;

                     case CANIF_CS_STOPPED:
                         CanIfControllerMode[ControllerId] =CANIF_CS_STOPPED;
                            break;

                     default:
                         //Det_ReportError(ModuleId,InstanceId,ApiId,CANIF_E_PARAM_CTRLMODE);
                            break;
                    }


    #endif

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}




