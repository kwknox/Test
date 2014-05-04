#include <stdlib.h>
#include <inttypes.h>
#include "io.h"
#include "hal_defs.h"
#include "ALPSlib.h"
#include "network.h"
#include "Parameter.h"
#include "Network_packet.h"
#include "Crypto.h"
#include "MemoryMap.h"
#include "Wdt.h"
#include "RF_task.h"
#define IO_MARKER_ON	1
uint8_t NewData = FALSE;


uint8_t *DesIamge;
uint8_t *TempIamge;
uint8_t LedSate=0;
static uint8_t IsJoin=1;



#define NEW_LOCK_SLOT_TIME	(log_TA1R + SLOT_TIME - SLOT_WAKEUP_ADVANCE)

//extern void ReceiveOn(void);
//extern void ReceiveOff(void);
uint8_t	ActiveChannel;

void ALPS_Start(uint8_t PlaneType)
{
    
   //ALPS_Start_Flag=PlaneType;
}

void ALPS_SetTotalTimeSlot(uint16_t count)
{
	//NewTotalSlot=count;
}

uint8_t GetActiveChannel (void)
{
	return ActiveChannel;
}

void ResetActiveChannel (void)
{
	ActiveChannel = NOT_CONNECTED;
}

uint16_t RF_Scan (void)
{
/*
	uint8_t Channel, Trial, TempChannel;
	int8_t TempRSSI[16], Maximum; 
	uint16_t List;
	
	if (DEFAULTCHANNEL > 0x0f)
	{
		for (Channel = 0; Channel < 0x10; Channel++)
		{
			TempRSSI[Channel] = -128;
				
			if (Channel != 8)
			{
				//WriteSingleReg (CHANNR, (Channel & 0x08 ? (Channel | 0xf0) : Channel));
	
				for (Trial = 0; Trial < 2; Trial++)
				{
					#if defined(IO_MARKER_ON)    			// indicate radio is on for doing scanning
					//P2OUT ^= BIT6;
					#endif	
					LED_Red_OFF();
					ReceiveOn ();
				
					//_BIS_SR(LPM3_bits + GIE);
					GotoSleep();
					// record the rssi only if a packet is received
					if ((radioMode != RADIO_MODE_TIMEOUT) && (rf_packet_received == TRUE))
					{
						TempRSSI[Channel] = GetRSSI ();
						Trial = 2;
					}
	
					ReceiveOff ();
		
				}			
			}
		}
		
		List = 0x8888;
		for (Trial = 0; Trial < 4; Trial++)
		{
			TempChannel = 8;
			Maximum = -128;
			for (Channel = 0; Channel < 0x10; Channel++)
			{
				if (TempRSSI[Channel] > Maximum)
				{
					Maximum = TempRSSI[Channel];
					TempChannel = Channel;
				}
			}
			TempRSSI[TempChannel] = - 128;
			
			List &= ~(0x0f << (Trial * 4));
			List |= (TempChannel << (Trial * 4));
		}
		return List;
	}
	else
	{
		return (0x8880 | DEFAULTCHANNEL);	// for fixed channel during testing only
	}
	*/
		return (0x8880 | DEFAULTCHANNEL);	// for fixed channel during testing only
}


uint8_t CheckReceiveImageFinish(uint16_t index)
{	
     uint8_t tmp=FALSE;
	 switch(New_ImageInfo->PanelType)
	 {
	 	//128*96/8=1536 bytes
		case ImageType_1_44:
			if(index==31)tmp=TRUE;	//1536/50=30.72
			break;
		//200*96/8=2400 bytes
		case ImageType_2_00:
			if(index==49)tmp=TRUE;	//2400/50=49
			break;
		//264*96/8=5808 bytes
		case ImageType_2_70:
			if(index==117)tmp=TRUE; //5808/50=116.16
			break;
	 }
	return tmp;
}


//----------------------------------------------------------------------------------
//  void RF_Task(void)
//
//  DESCRIPTION:
//    This is the main entry of the "link" application. It sets up the board and
//    lets the user select operating mode (RX or TX) by pressing either button
//    S1 or S2. In TX mode, one packet (with arbitrary length) is sent over the air
//    every time the button is pressed. In RX mode, the LCD will be updated every time
//    a new packet is successfully received. If an error occurred during reception,
//    one of the LEDs will flash.
//----------------------------------------------------------------------------------
uint16_t RF_Task (void)
{
	static uint8_t	outage_count = 0;			// number of time of fail to synchronize to address slot since last successful synchonization	
	static uint16_t sync_count = 0;				// number of times we have been continiously in sync
	static uint16_t	SlotToWait = 0;				// number of time slot to put into sleep (Receiver off) state
	static uint8_t	LockToSlot = 0;				// which slot is the system get synchronized
	static uint8_t	MyDataSlot = 0;				// slot number that contains the data to me as indicated by the position of the apprence of my address in the list
	static uint8_t	DataSpan = 0;				// number of consecutive slots that contains data to me
	static uint16_t DataOffset = 0;				// to make sure data is put in correct memory when data spans over multiple time slot
	static uint8_t	ReceiverState = RX_RESET;	// state to manage the state machine
	static uint8_t	TotalSlot = 0;				// total number of time slot in this frame as indicated in the configuration packet (default to TOTAL_SLOT)
	static uint16_t	ActiveAddress = 0x0000;		// what is the address used for receiveing data
	static uint16_t ChannelList;				// list of channel suggested after channel scanning
	//static uint8_t	ConnectAttempts = 0;		
	//static uint8_t	ConnectPending = 0;
	static uint16_t	TempRandom[4];
	//static uint8_t IsJoin=0;
	uint16_t i, j, k, addoffset;
	uint8_t currentType;
	
	//if(GetAlwaysOn()==1) return 0;
	if (TotalSlot == 0)
		TotalSlot = TOTAL_SLOT;

	switch (ReceiverState)
	{
		case RX_RESET :
		
			//LED_Green_OFF();
			LED_Red_OFF();
			/*if (ActiveChannel <= (CHANNEL_F + CHANNEL_JOINING))
			{
				SlotToWait = 0;
				ReceiverState = RX_SYNC;
				ActiveAddress = 0x0000;
			}
			else
			{*/
				ReceiverState = RX_SCAN;
				//ConnectAttempts = 0;	
			//}
		break;

		case RX_IDLE_FOR_SYNC :				
			LED_Red_OFF();
			//LED_Green_OFF();	

			ReceiveOff ();
			IsJoin=1;
			//TA1CCR1 += rand ();
			//P2OUT ^= BIT7;

			for (SlotToWait = (IDLE_FOR_SYNC_SLOT + (outage_count << 3)); SlotToWait > 0; SlotToWait--)		// idle time is increasing with number of outage but limited to 255 time slot
			{
				watchdog_clear();
				GotoSleep();
			}
			
			ReceiverState = RX_SYNC;
			if (outage_count < 30)
			{
				outage_count++;
			}
			//else
			//{
			//	ReceiverState =RX_IDLE_FOR_SYNC;// RX_SCAN;
			//}
		break;
		
		case RX_SYNC :			
			//LED_Red_ON();
			LED_Green_OFF();
			sync_count=0;

			for (SlotToWait = SYNC_SLOT; SlotToWait > 0;)
			{
				ReceiveOn ();
				watchdog_clear();
				GotoSleep();

				if (radioMode != RADIO_MODE_TIMEOUT)
				{
					if ((rf_packet_received == TRUE)
						&& ((comm_buffer.packet.header.packet_type == ADDRESS_PACKET)
							|| (comm_buffer.packet.header.packet_type == ADDRESS_PACKET_LAST)  
							|| (comm_buffer.packet.header.packet_type == DATA_PACKET) 
							|| (comm_buffer.packet.header.packet_type == DATA_PACKET_LAST) 
							|| (comm_buffer.packet.header.packet_type == IDLE_PACKET) 
							|| (comm_buffer.packet.header.packet_type == HOST_RESPOND_PACKET)
							|| (comm_buffer.packet.header.packet_type == CONFIG_PACKET)
							|| (comm_buffer.packet.header.packet_type == POLL_PACKET)
							)
						)
					{
						if (comm_buffer.packet.header.sequence_num == 0)
						{
							TA1CCR1 = NEW_LOCK_SLOT_TIME;			// Synchronise the time slot to the received package							
							ReceiverState = RX_IDLE_FOR_ADDRESS;
							LockToSlot = comm_buffer.packet.header.slot_number;
							if(IsJoin)
							{
								Send_Respond_Packet(1,JOIN_CHANNEL);
								IsJoin=0;
							}
								return 0;
						}
					}
				}
				else
				{
					SlotToWait--;
				}
			}
			
			ActiveAddress = 0x0000;

			ReceiverState = RX_IDLE_FOR_SYNC;
		break;
		
		case RX_IDLE_FOR_ADDRESS :				    
			LED_Red_OFF();
			LED_Green_OFF();
			outage_count = 0;
          
			ReceiveOff ();
			for (SlotToWait = LockToSlot; SlotToWait > 0; SlotToWait--)
			{								   
				watchdog_clear();
				GotoSleep();
				LedSate=SlotToWait;
				//LED_Green_Trigger();
			}
			//LED_Green_ON();
			ActiveAddress = 0x0000;			
			ReceiverState = RX_ADDRESS;												
			//LED_Green_OFF();
		break;				
		case RX_ADDRESS :			
			//LED_Green_OFF();
			// flash LED for the first 10 address packet when we are in sync...
			if (sync_count < 10) LED_Green_ON();
			
			ReceiverState = RX_SYNC;
			
			LockToSlot = TotalSlot;
			
			DataSpan = 0;

			do 
			{
				ReceiveOn ();

				GotoSleep();
				if ((rf_packet_received == TRUE))
				{
					if (comm_buffer.packet.header.slot_number == 0)
					{
						if (comm_buffer.packet.header.sequence_num == 0)
						{
							TA1CCR1 = NEW_LOCK_SLOT_TIME;			// Synchronise the time slot to the received package

							if (comm_buffer.packet.header.packet_type == CONFIG_PACKET)
							{
	    	    				TotalSlot = comm_buffer.packet.body.config_packet.NumberOfTimeSlot;
								LockToSlot = TotalSlot + 1;

								if (ActiveChannel <= CHANNEL_F)
								{
		    						RTCCTL01 &= ~RTCRDYIE;
		    						RTCCTL01 |= RTCHOLD;
		       						RTCDOW = comm_buffer.packet.body.config_packet.DayOfWeek;
		    						RTCSEC = comm_buffer.packet.body.config_packet.Second;
		    						RTCMIN = comm_buffer.packet.body.config_packet.Minute;
		    						RTCHOUR = comm_buffer.packet.body.config_packet.Hour;
		    						RTCDAY = comm_buffer.packet.body.config_packet.Day;
		    						RTCMON = comm_buffer.packet.body.config_packet.Month;
		    						RTCYEAR = comm_buffer.packet.body.config_packet.Year;
		    						RTCCTL01 &= ~RTCHOLD;
		    						RTCCTL01 |= RTCRDYIE;
								}

								ReceiverState = RX_IDLE_FOR_ADDRESS;
							}
						}
						else
						{
							if (((comm_buffer.packet.header.packet_type == ADDRESS_PACKET) 
								|| (comm_buffer.packet.header.packet_type == ADDRESS_PACKET_LAST))
								&& (ReceiverState != RX_SYNC)
								)
							{
								sync_count++;

								for (i = 0; i < 28; i++)
								{
									if ((comm_buffer.packet.body.address_packet.list.address_word[i] == MY_ADDRESS))																				
									{
										if (DataSpan == 0)
										{
											MyDataSlot = (comm_buffer.packet.header.sequence_num - 1) * 28 + i + 1;
											
											ActiveAddress = comm_buffer.packet.body.address_packet.list.address_word[i];

											if (ActiveAddress == MY_ADDRESS)
											{
												//AcknowledgeRequired = TRUE;
												ReceiverState = RX_IDLE_FOR_DATA;
											}
											else
											{
												//AcknowledgeRequired = FALSE;												
												ReceiverState = RX_IDLE_FOR_DATA;
											}
										}
										else
										{
											if (comm_buffer.packet.body.address_packet.list.address_word[i] != ActiveAddress)
											{
												return 0;
											}
										}
										DataSpan++;
									}
									else
									{
										if (DataSpan > 0)
										{
											return 0;	
										}
									} 
								}
							}
						}
					}
					else
					{
						if 	((comm_buffer.packet.header.packet_type == DATA_PACKET) 
							|| (comm_buffer.packet.header.packet_type == DATA_PACKET_LAST) 
							|| (comm_buffer.packet.header.packet_type == IDLE_PACKET) 
							)
						{
							if (comm_buffer.packet.header.sequence_num == 0)
							{
								TA1CCR1 = NEW_LOCK_SLOT_TIME;			// Synchronise the time slot to the received package
								LockToSlot = comm_buffer.packet.header.slot_number;

								ReceiverState = RX_IDLE_FOR_ADDRESS;
								return 0;
							}
						}
						
					}
				}
			} while ((radioMode != RADIO_MODE_TIMEOUT) 
					&& (comm_buffer.packet.header.packet_type != ADDRESS_PACKET_LAST));
			/*
			if (radioMode == RADIO_MODE_TIMEOUT)
			{
				LockToSlot -= 1;
				if (ActiveChannel > CHANNEL_F)
				{
					ConnectPending++;
					if (ConnectPending > 2)
					{
						ReceiverState = RX_JOINING;
					}
				}
			} 
			else if (comm_buffer.packet.header.packet_type == ADDRESS_PACKET_LAST)
			{
				if (ActiveChannel > CHANNEL_F)
				{
					ConnectPending++;
					if (ConnectPending > 2)
					{
						ReceiverState = RX_JOINING;
					}
				}
			}*/
		break;
		
		case RX_IDLE_FOR_DATA :				
			LED_Red_OFF();
			LED_Green_OFF();
			ReceiveOff ();
			
			for (SlotToWait = TotalSlot - (LockToSlot - 1); SlotToWait < MyDataSlot; SlotToWait++)
			{				
							    
				//_BIS_SR(LPM3_bits + GIE);
				GotoSleep();
			}
			
			ReceiverState = RX_DATA;
						
			//LED_Green_OFF();			
		break;
		
		case RX_DATA :			
			LED_Red_ON();
			i = 0;
			DataOffset = 0;
			
			k=sizeof(ImageInfo_t);
			do
			{
			    //LedSate=2;
				ReceiveOn ();				
				GotoSleep();
				if ((rf_packet_received == TRUE))
				{
				
					if (comm_buffer.packet.body.data_packet.address == ActiveAddress)
					{
						if ((comm_buffer.packet.header.packet_type == DATA_PACKET) 
							|| (comm_buffer.packet.header.packet_type == DATA_PACKET_LAST) 
							|| (comm_buffer.packet.header.packet_type == DATA_PACKET_EXTEND)
							)
						{
							
							if (i == comm_buffer.packet.header.sequence_num)	// if expected sequence number is the same as the received sequence number
							{
						     
								if(i==0)
								{		
								    addoffset=0;
									GetUseImageInfo((ImageInfo_t *)&comm_buffer.packet.body.data_packet.data[0],0);
									TempIamge=(uint8_t *)RAM_Image1_Info;
									DesIamge=(uint8_t *)New_ImageInfo;									
									Image_Write_Ex(TempIamge,k,(uint8_t *)&comm_buffer.packet.body.data_packet.data[sizeof(ImageInfo_t)], 50-sizeof(ImageInfo_t));									
									currentType=RAM_Image1_Info->PanelType; 
									k+=(50-sizeof(ImageInfo_t));
								}
								else
								{
									Image_Write_Ex(TempIamge,k,(uint8_t *)&comm_buffer.packet.body.data_packet.data[0],	50);   
 									k+=50;
								}							   							
						        
								if(currentType==ImageType_2_70)
								{
								  if(i==31 || i==63 || i==95)
								  {									
									k=0;
									//LED_Green_Trigger();
									for(j=0;j<25;j++)
									{
										Image_Write_Ex((uint8_t *)(DesIamge),addoffset,TempIamge+k, 64);										
										addoffset+=64;
										k+=64;
									}
									k=0;
								 }
								  if(i==116)
								  {
									  k=0;
									 // LED_Green_Trigger();
									  for(j=0;j<17;j++)
									  {
										  Image_Write_Ex((uint8_t *)(DesIamge),addoffset,TempIamge+k, 64);									  
										  addoffset+=64;
										  k+=64;
									  }
									  k=0;
								  }								 
								}
								i++;		    // next expected sequence number
								DataOffset++;
						  								
							}
							else
							{
								NewData = FALSE;
								
								DataOffset += (comm_buffer.packet.header.sequence_num + 1 - i);	// next write location
								i = comm_buffer.packet.header.sequence_num + 1;			//  next expected sequence number
							}
						}							
						else if ((comm_buffer.packet.header.packet_type == POLL_PACKET))
						{
							NewData = FALSE;
							if (comm_buffer.packet.body.poll_packet.Address == MY_ADDRESS)
							{
							 Send_Respond_Packet( comm_buffer.packet.header.sequence_num++,POLLING);	
							 ReceiveOff ();
							GotoSleep();
							// break;
							} 
						}
						else if (comm_buffer.packet.header.packet_type == SIGNATURE_PACKET)
						{
							NewData = FALSE;

						// if expected sequence number is the same as received one
							if (i == comm_buffer.packet.header.sequence_num)
							{
	 								 							
									// decrypt the signature
								if (comm_buffer.packet.body.signature_packet.address == MY_ADDRESS)
								{
								    memcpy((uint16_t *)&TempRandom,(uint16_t *)(&comm_buffer.packet.body.signature_packet.signature),8);
									decipher ((uint32_t *)TempRandom, Key);
								}
								i=*((uint16_t *)New_ImageInfo+ TempRandom[0]);//(uint16_t *)(&New_ImageInfo+comm_buffer.packet.body.signature_packet.signature[0]);
								j=*((uint16_t *)New_ImageInfo+ TempRandom[2]);
								if(i==TempRandom[1] && 
								   j==TempRandom[3])
								{										
			  						Send_Respond_Packet( comm_buffer.packet.header.sequence_num++,IMAGE);					
									NewData = TRUE;
									ActiveChannel &= 0x0f;											
								    ReceiverState =RX_IDLE_FOR_SYNC;		
									
								}
								else
								{
									DataOffset = 0;									
								}	 																
							}
						}		
						ReceiverState = RX_SYNC;
					}	
					else if ((comm_buffer.packet.header.packet_type == IDLE_PACKET))
				    	{
						  do{
							NewData = FALSE;	
							LED_Red_OFF();							
						    ReceiveOff ();							
							GotoSleep();
						  }while (radioMode != RADIO_MODE_TIMEOUT);
						  ReceiverState =RX_SYNC;
						}
					
				}
				else ReceiverState = RX_SYNC;
			}
			while (((radioMode != RADIO_MODE_TIMEOUT) || (comm_buffer.packet.header.packet_type == DATA_PACKET_EXTEND)) && (NewData==FALSE)); 				  
	  /*
	         if (AcknowledgeRequired == TRUE)
			   {	   
				   if (comm_buffer.packet.header.packet_type == SIGNATURE_PACKET)
				   {
					  Send_Respond_Packet( comm_buffer.packet.header.sequence_num++,IMAGE);
				   }
				   else if ((comm_buffer.packet.header.packet_type == POLL_PACKET))
				   {
				  
					   Send_Respond_Packet( comm_buffer.packet.header.sequence_num++,POLLING);
				   }
			   }
         */
         
			ReceiveOff ();
	  
	 /*
			#if defined(IO_MARKER_ON)
			if(i>50)
			{
		      //P2OUT &= ~BIT6;		      
			   LED_Red_OFF();
			}
			#endif
	*/
		//if (ReceiverState != RX_JOINING)
		   {
			 //ReceiverState = RX_SYNC;
		   }
			
		break;
		
		case RX_SCAN :					
			LED_Red_ON();			
			ChannelList = RF_Scan ();				// get the list of scanned channels
			if (ChannelList != 0x8888)
			{
				//ConnectAttempts = 0;
				ReceiverState = RX_JOINING;
			}
			else
			{
				SlotToWait = rand () & 0xff;
				ReceiverState = RX_IDLE_FOR_SCAN;
			}
		break;
		
		case RX_IDLE_FOR_SCAN :
			//LED_Green_OFF();
			LED_Red_OFF();

			ReceiveOff ();
			
			for (SlotToWait &= 0x1ff; SlotToWait > 0; SlotToWait--)
			{
				watchdog_clear();
				GotoSleep();
			}
			
			ReceiverState = RX_SCAN;

		break;
		
		case RX_JOINING :
			
			//LED_Green_OFF();			

			//ConnectPending = 0;
			
			//ActiveChannel = (ChannelList >> (ConnectAttempts * 4)) & 0x000f;				// set to one of the channels in the list
			
			//if (ActiveChannel != 8)
			{
				//WriteSingleReg (CHANNR,DEFAULTCHANNEL);//(ActiveChannel & 0x08 ? (ActiveChannel | 0xf0) : ActiveChannel));
				
				for (SlotToWait = SYNC_SLOT; SlotToWait > 0;)
				{
					// wait a random number of cycles
					/*for (j = (rand() & 0x1f); j > 0; j--)
					{
						//_BIS_SR (LPM3_bits + GIE);
						GotoSleep();
					}*/

				    watchdog_clear();
					LED_Red_ON();
				    if(GetAlwaysOn()==1) return 0;
					ReceiveOn ();

					GotoSleep();

					if (radioMode != RADIO_MODE_TIMEOUT)
					{
						if (rf_packet_received == TRUE)
						{
							if (comm_buffer.packet.header.sequence_num == 0)
							{
								TA1CCR1 = NEW_LOCK_SLOT_TIME;			// Synchronise the time slot to the received package
		
								if (comm_buffer.packet.header.packet_type == IDLE_PACKET)
								{
								/*
									encipher (comm_buffer.packet.body.idle_packet.idle_packet_body.random_num, Key);
									comm_buffer.packet.header.network_id0 = NETWORK_ID0;
									comm_buffer.packet.header.network_id1 = NETWORK_ID1;
									comm_buffer.packet.header.packet_length = HOST_PACKET_LENGTH;
									comm_buffer.packet.header.packet_type = HOST_REQUEST_PACKET;
									comm_buffer.packet.header.sequence_num = 1; 

									
									for (j = 4; j > 0; j--)
									{
										comm_buffer.packet.body.host_packet.data[j - 1] = *(((uint16_t *)(&comm_buffer.packet.body.idle_packet.idle_packet_body.random_num)) + j - 1);
										TempRandom[j - 1] = *(((uint16_t *)(&comm_buffer.packet.body.idle_packet.idle_packet_body.random_num)) + j - 1); 
									} 
									
									comm_buffer.packet.body.host_packet.address = MY_ADDRESS;
									comm_buffer.packet.body.host_packet.type = HOST_JOIN;
									comm_buffer.packet.body.host_packet.data_length = 4;
									comm_buffer.packet.body.host_packet.data[0]=GetLQI();
									comm_buffer.packet.body.host_packet.data[1]=GetRSSI();
									prepare_send_packet(&comm_buffer);
									*/
									//Send_Respond_Packet(1,JOIN_CHANNEL);
									ActiveChannel |=  CHANNEL_JOINING;
	
									ReceiverState = RX_SYNC;

									outage_count=0;
									break;
								}
							}
						}
					}
					else
					{
						SlotToWait--;
					}
					
					ReceiveOff ();
					LED_Red_OFF();

					for (SlotToWait = (IDLE_FOR_SYNC_SLOT + (outage_count << 3)); SlotToWait > 0; SlotToWait--)		// idle time is increasing with number of outage but limited to 255 time slot
					{
						watchdog_clear();
						GotoSleep();
					}

					if (outage_count < 30)
					{
						outage_count++;
					}
				}
				

				ReceiveOff ();
				LED_Red_OFF();
			/*
				if ((++ConnectAttempts > 3) && (ReceiverState == RX_JOINING))
				{
					ActiveChannel = NOT_CONNECTED;
					SlotToWait = rand () & 0xff;
					ReceiverState = RX_IDLE_FOR_SCAN;
				}
				*/
			}
			/*
			else	// all suggested channels checked
			{
				ActiveChannel = NOT_CONNECTED;
				SlotToWait = rand () & 0xff;
				ReceiverState = RX_IDLE_FOR_SCAN;
			}
			*/
		break;
		
		default :
			ReceiverState = RX_RESET;		
		break;	
	}
	
	return j;	// j contains data category when NewData is TRUE otherwise it should be ignored
}		



