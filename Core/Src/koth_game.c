#include "main.h"
#include <string.h>
#include "define.h"
#include "extern.h"
#include "lora_sx1276.h"


void resetGame()
{

    p1King_counter = 0;
    p1King_counter = 0;
    gameState = waiting;

}

void sendOpcode(uint8_t opcode)
{
    // Create encoded string
    char opcodeString [10];
    sprintf(opcodeString, "%uKOTH", opcode);

    // Send it via LoRa
    uint8_t res = lora_send_packet(&lora, (uint8_t *)"test", 4);
    if (res != LORA_OK) {
    // Send failed
  }
}