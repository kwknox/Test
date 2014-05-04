#ifndef RF_TASK_H_
#define RF_TASK_H_
// enum for receiving state
enum
{
	RX_RESET,
	RX_IDLE_FOR_SYNC,
	RX_SYNC,
	RX_IDLE_FOR_ADDRESS,
	RX_ADDRESS,
	RX_IDLE_FOR_DATA,
	RX_DATA,
	RX_SCAN,
	RX_JOINING,
	RX_IDLE_FOR_SCAN
};

extern uint8_t NewData;
uint16_t RF_Task (void);
uint8_t GetActiveChannel (void);
void ResetActiveChannel (void);



#endif 


