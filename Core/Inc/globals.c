#include <stdint.h>
#include "define.h"

uint16_t debugBuddy;
uint8_t loraRX_flag;
uint8_t stateChange_flag;
uint8_t red_flag;
uint8_t blue_flag;
uint8_t debouncing_Flag;

uint32_t red_counter;
uint32_t blue_counter;
uint16_t debouce_timer;

uint8_t packet_size;
uint8_t loraRXopcode;
uint8_t loraRXbuf[10]; 
uint8_t receivedPacket[10]; 
char opcodeString [10];
char rxReadable[20];
char txReadable[20];

uint8_t gameState;
enum opcodes {START, goSTART, RED, goRED, BLUE, goBLUE, BLUEWINS, REDWINS, WAITING, PENALTY};

const char * const gameStateNames[] = {
	[START] = "START",
	[goSTART] = "goSTART",
	[RED] = "RED",
	[goRED] = "goRED",
    [BLUE] = "BLUE",
	[goBLUE] = "goBLUE",
	[BLUEWINS] = "BLUEWINS",
	[REDWINS] = "REDWINS",
    [WAITING] = "WAITING",
	[PENALTY] = "PENALTY"
};