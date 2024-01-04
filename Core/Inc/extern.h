#include <stdint.h>
#include "define.h"
#include "main.h"

extern uint8_t loraRX_flag;
extern uint8_t stateChange_flag;
extern uint8_t p1King_flag;
extern uint8_t p2King_flag;
extern uint8_t debouncing_Flag;

extern uint32_t p1King_counter;
extern uint32_t p2King_counter;

extern uint8_t loraRXbuf[255]; 
extern uint8_t loraTXbuf[255]; 

extern uint8_t gameState;
extern enum gameStates {waiting,p1King,p2King,p1Winner,p2Winner,penalty};
extern enum opcodes {START, CONFIRM_START, CLAIM_KING, CONFIRM_KING};