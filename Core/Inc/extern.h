#include <stdint.h>
#include "define.h"
#include "main.h"
#include "lora.h"
#include "cmsis_os.h"

extern uint16_t debugBuddy;
extern uint16_t txCount, rxCount;
extern LoRa myLoRa;

extern uint8_t THIS_PLAYER;
extern uint8_t OTHER_PLAYER;

extern osTimerId debounceTimerHandle;

extern uint8_t loraRX_flag;
extern uint8_t stateChange_flag;
extern uint8_t red_flag;
extern uint8_t blue_flag;
extern uint8_t debouncing_Flag;
extern uint8_t newTX_flag;
extern uint8_t newRX_flag;
extern uint16_t debounce_timer;
extern uint8_t redLED_timer;
extern uint8_t blueLED_timer;
extern uint8_t redLED_flag;
extern uint8_t blueLED_flag;

extern uint32_t red_counter;
extern uint32_t blue_counter;

extern uint8_t packet_size;
extern uint8_t loraRXopcode;
extern uint8_t receivedPacket[10]; 
extern uint8_t loraRXbuf[10]; 
extern char opcodeString [10];
extern char rxReadable[20];
extern char txReadable[20];

extern uint8_t gameState;
extern enum opcodes {START, goSTART, RED, goRED, BLUE, goBLUE, BLUEWINS, REDWINS, WAITING, PENALTY};// Prototypes
extern const char * const gameStateNames;

void resetGame();
void setOpcode(uint8_t);
void decodeKOTHPacket();
void btnPressed();
void doGameState();
char* readableOpcode(uint8_t opcode);
void btnPressedSimple();
void doGameStateSimple();