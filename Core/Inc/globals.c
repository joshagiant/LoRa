#include <stdint.h>
#include "define.h"

uint16_t debugBuddy;
uint8_t loraRX_flag;
uint8_t stateChange_flag;
uint8_t p1King_flag;
uint8_t p2King_flag;
uint8_t debouncing_Flag;

uint32_t p1King_counter;
uint32_t p2King_counter;
uint16_t debouce_timer;

uint8_t packet_size;
uint8_t loraRXbuf[10]; 
char opcodeString [10];

uint8_t gameState;
enum gameStates {waiting,p1King,p2King,p1Winner,p2Winner,penalty,unconfirmed};
enum opcodes {START, CONFIRM_START, CLAIM_KING, CONFIRM_KING};