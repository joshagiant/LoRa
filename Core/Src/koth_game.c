#include "main.h"
#include <string.h>
#include "define.h"
#include "extern.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "lora_sx1276.h"


void resetGame()
{
    p1King_flag = 0;
    p2King_flag = 0;
    p1King_counter = 0;
    p1King_counter = 0;
    gameState = waiting;

}

void getKOTHPacket()
{
    // Wait for LoRa availability
    osSemaphoreWait(lora_mutexHandle, osWaitForever);

    // Get the packet
    uint8_t result;
    lora_receive_packet(&lora,loraRXbuf,10,&result);

    osSemaphoreRelease(lora_mutexHandle);
    
    // Parse the data
    if(strstr(loraRXbuf, "KOTH"))
    {
        long opcode;
        char * trashCatcher[10]; 
        
        opcode = strtol(loraRXbuf, trashCatcher,10);

        // Process opcode and respond if necessary
        switch (opcode)
        {
            case START:
            // Reset and start
            resetGame();
            // Send confirmation
            sendOpcode(CONFIRM_START);
            gameState = p2King;
            stateChange_flag = 1; 
            p1King_flag = 0;
            p2King_flag = 1;
            break;

            case CLAIM_KING: 
            sendOpcode(CONFIRM_KING);
            gameState = p2King;
            stateChange_flag = 1; 
            p1King_flag = 1;
            p2King_flag = 0;
            break; 

            // Confirmations received
            case CONFIRM_START:
            case CONFIRM_KING:
            stateChange_flag = 1; 
            gameState = p1King;
            p1King_flag = 1;
            p2King_flag = 0;
            break;
            
            }
        }
}

void btnPressed()
{
    switch (gameState)
    {
        case waiting:
            // send message to opponent, start on confirmation
            sendOpcode(START);
        break;
        
        case p1King:
            // oops! penalty
            stateChange_flag = 1; 
            gameState = penalty;
            sendOpcode(CONFIRM_KING);
        break;

        case p2King:
            // Take the hill
            sendOpcode(CLAIM_KING);
        break;

        case p1Winner:
        case p2Winner:
            stateChange_flag = 1; 
            gameState = waiting;
        break;

        case penalty:
            // nothing
        break;
    }


}

void sendOpcode(uint8_t opcode)
{
    // Create encoded string
    char opcodeString [10];
    sprintf(opcodeString, "%uKOTH", opcode);

    // Wait for LoRa availability
    osMutexWait(lora_mutexHandle, osWaitForever);

    // Send it via LoRa
    uint8_t res = lora_send_packet(&lora, (uint8_t *)opcodeString, 10);
    if (res != LORA_OK) {
    // Send failed
    }

    // Release LoRa availability
    osMutexRelease(lora_mutexHandle);

  
}