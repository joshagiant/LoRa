#include <stdint.h>
#include "define.h"
#include "lora_sx1276.h"
#include "main.h"
#include "cmsis_os.h"

extern uint16_t debugBuddy;
extern lora_sx1276 lora;
extern osMutexId lora_mutexHandle, lora_mutex;
extern SemaphoreHandle_t xloraMutex;
extern osTimerId debounceTimerHandle;
extern osTimerId blinkTimerHandle;

extern uint8_t loraRX_flag;
extern uint8_t stateChange_flag;
extern uint8_t p1King_flag;
extern uint8_t p2King_flag;
extern uint8_t debouncing_Flag;

extern uint32_t p1King_counter;
extern uint32_t p2King_counter;

extern char loraRXbuf[10]; 
extern char loraTXbuf[10]; 
extern char opcodeString [10];

extern uint8_t gameState;
extern enum gameStates {waiting,p1King,p2King,p1Winner,p2Winner,penalty};
extern enum opcodes {START, CONFIRM_START, CLAIM_KING, CONFIRM_KING};

// Prototypes
void resetGame();
void sendOpcode(uint8_t);
void getKOTHPacket();
void btnPressed();