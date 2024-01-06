#include "main.h"
#include <string.h>
#include "define.h"
#include "extern.h"
#include <stdio.h>
#include <stdlib.h>
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
    //xSemaphoreTake(xloraMutex, portMAX_DELAY);

    // Get the packet
    uint8_t result;
    lora_receive_packet_dma_start(&lora,loraRXbuf,10,&result);

    //xSemaphoreGive(xloraMutex);
    
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
            P2LED_ON;
            P1LED_OFF;
            stateChange_flag = 1; 
            p1King_flag = 0;
            p2King_flag = 1;
            break;

            case CLAIM_KING: 
            sendOpcode(CONFIRM_KING);
            gameState = p2King;
            P2LED_ON;
            P1LED_OFF;
            stateChange_flag = 1; 
            p1King_flag = 1;
            p2King_flag = 0;
            break; 

            // Confirmations received
            case CONFIRM_START:
            case CONFIRM_KING:
            stateChange_flag = 1; 
            gameState = p1King;
            P1LED_ON;
            P2LED_OFF;
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
            P2LED_ON;
            P1LED_OFF;
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
            P1LED_OFF;
            P2LED_OFF;
        break;

        case penalty:
            // nothing
        break;
    }


}

void sendOpcode(uint8_t opcode)
{
    memset(opcodeString, 0, sizeof(opcodeString));
    // Create encoded string
    sprintf(opcodeString, "%uKOTH", opcode);

    // Wait for LoRa availability
    //xSemaphoreTake(xloraMutex, portMAX_DELAY);

    // Send it via LoRa
    uint8_t res = lora_send_packet_dma_start(&lora, (uint8_t *)opcodeString, 10);
    if (res != LORA_OK) {
    // Send failed
    debugBuddy = res;

    }

    // Release LoRa availability
    //xSemaphoreGive(xloraMutex);
    // osMutexRelease(lora_mutexHandle);

  
}