/*******************************************************************************************************************************
FileName:                    							 CanSm.h                                      
AUTOSAR Version:          								  4.2.2
******************************************************************************************************************************/
#ifndef CANSM_H
#define CANSM_H

/******************************************************************************************************************************
 **                                                                        Includes                                                                             **
 ******************************************************************************************************************************/
#include "Platform_Types.h"
#include "ComStack_Types.h"
/*******************************************************************************************************************************
**                                               						  Defines                                                                                **
********************************************************************************************************************************/



/*******************************************************************************************************************************
**                                               						   Variables                                                                             **
********************************************************************************************************************************/



/*******************************************************************************************************************************
**                                               				    Function prototypes                                                                       **
********************************************************************************************************************************/
void CanSM_ControllerBusOff(uint8 ControllerId);
/*void CanSM_ControllerModeIndication(uint8 ControllerId,CanIf_ControllerModeType ControllerMode);*/

#endif /*CANSM_H*/
