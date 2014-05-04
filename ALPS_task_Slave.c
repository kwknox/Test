//--------------------------------------------------------------------------
//
//  Software for CC430 based star network for e-label demo.
//
//  THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
//  REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY, 
//  INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
//  FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//  COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE. 
//  TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET 
//  POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY 
//  INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR 
//  YOUR USE OF THE PROGRAM.
//
//  IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
//  CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY 
//  THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED 
//  OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT 
//  OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM. 
//  EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF 
//  REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS 
//  OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF 
//  USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S 
//  AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF 
//  YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS 
//  (U.S.$500).
//
//  Unless otherwise stated, the Program written and copyrighted 
//  by Texas Instruments is distributed as "freeware".  You may, 
//  only under TI's copyright in the Program, use and modify the 
//  Program without any charge or restriction.  You may 
//  distribute to third parties, provided that you transfer a 
//  copy of this license to the third party and the third party 
//  agrees to these terms by its first use of the Program. You 
//  must reproduce the copyright notice and any other legend of 
//  ownership on each copy or partial copy, of the Program.
//
//  You acknowledge and agree that the Program contains 
//  copyrighted material, trade secrets and other TI proprietary 
//  information and is protected by copyright laws, 
//  international copyright treaties, and trade secret laws, as 
//  well as other intellectual property laws.  To protect TI's 
//  rights in the Program, you agree not to decompile, reverse 
//  engineer, disassemble or otherwise translate any object code 
//  versions of the Program to a human-readable form.  You agree 
//  that in no event will you alter, remove or destroy any 
//  copyright notice included in the Program.  TI reserves all 
//  rights not specifically granted under this license. Except 
//  as specifically provided herein, nothing in this agreement 
//  shall be construed as conferring by implication, estoppel, 
//  or otherwise, upon you, any license or other right under any 
//  TI patents, copyrights or trade secrets.
//
//  You may not use the Program in non-TI devices.
//
//--------------------------------------------------------------------------
#include <stdlib.h>
#include <inttypes.h>
#include "io.h"
#include "hal_defs.h"
#include "ALPSlib.h"
#include "network.h"
#include "Parameter.h"
#include "Network_packet.h"
#include "RF_task.h"
#include "Wdt.h"
//#include "MemoryMap.h"



//static uint8_t  SlotNumber=0;
//static uint8_t  TotalSlot=0;//,NewTotalSlot=10;
//static uint8_t  InitSettinfCount=10;
extern uint8_t	LedSate;

void ALPS_Init(void)
{	
   
	LoadParameters();
	RF_Init();
	watchdog_init();
	watchdog_start(20);

}

void  ALPS_Timer(void)
{	
	SetradioMode(RADIO_MODE_TIMEOUT);	
	//LED_Red_Trigger();
	SaveParaPoll();

	//if(LedSate==2) LED_Green_OFF();
	//else LED_Green_Trigger();
	
	/*
   if(InitSettinfCount!=0)
  	{
  	    InitSettinfCount--;
  		LED_Green_Trigger();
		LED_Red_Trigger();
  	}
  	else if(InitSettinfCount==1)
   	{
	   InitSettinfCount=0;

   	}
   */
}

void ALPS_Task_Poll(void)
{    	
	if(GetAlwaysOn()==1) return;

  //  if(InitSettinfCount==0)
	{
		RF_Task();
	 	if(NewData)
		{
		    LedSate=2;
			DisplayImage(New_ImageInfo->PanelType,(uint8_t *)(Previous_ImageInfo),(uint8_t *)(New_ImageInfo));	
			NewData=FALSE;
			UnAlwaysOn();
		}
    }
}



