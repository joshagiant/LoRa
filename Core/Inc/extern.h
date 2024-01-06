#include <stdint.h>
#include "define.h"
#include "main.h"
#include "cmsis_os.h"
#include "lora.h"

extern uint16_t debugBuddy;
extern LoRa myLoRa;
extern osMutexId lora_mutexHandle, lora_mutex;
extern SemaphoreHandle_t xloraMutex;
extern osTimerId debounceTimerHandle;
extern osTimerId blinkTimerHandle;

extern uint8_t loraRX_flag;
extern uint8_t stateChange_flag;
extern uint8_t red_flag;
extern uint8_t blue_flag;
extern uint8_t debouncing_Flag;

extern uint32_t red_counter;
extern uint32_t blue_counter;

extern uint8_t packet_size;
extern uint8_t receivedPacket[10]; 
extern uint8_t loraRXbuf[10]; 
extern char opcodeString [10];
extern char rxReadable[20];
extern char txReadable[20];

extern uint8_t gameState;
extern enum opcodes {START, goSTART, RED, goRED, BLUE, goBLUE, BLUEWINS, REDWINS, WAITING, PENALTY};// Prototypes
extern const char * const gameStateNames;

void resetGame();
void sendOpcode(uint8_t);
void getKOTHPacket();
void btnPressed();
void doGameState();
char* readableOpcode(uint8_t opcode);