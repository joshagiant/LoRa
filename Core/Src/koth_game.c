#include "main.h"
#include <string.h>
#include "define.h"
#include "extern.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "lora.h"
#include "ssd1306.h"


void resetGame()
{
    p1King_flag = 0;
    p2King_flag = 0;
    p1King_counter = 0;
    p1King_counter = 0;
    gameState = waiting;

}

void checkGameState()
{
    
    switch (gameState)
    {
      case waiting:
        if(stateChange_flag)
        {
          stateChange_flag = 0;
          ssd1306_Fill(0); 
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("Push to play",Font_7x10,1);
          ssd1306_UpdateScreen();
        }
      break;

      case p1King:
        if(stateChange_flag)
        {
          stateChange_flag = 0;
          P1LED_OFF; // The hill is yours
          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("You are king",Font_7x10,1);
          ssd1306_SetCursor(0,10);
          ssd1306_WriteString("of the hill!",Font_7x10,1);
          ssd1306_UpdateScreen();
        }
      break;

      case p2King:
        if(stateChange_flag)
        {
          stateChange_flag = 0;
          P1LED_ON; // Better push the button!
          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("Take the hill!!",Font_7x10,1);
          ssd1306_UpdateScreen();
        }
      break;

      case p1Winner:
        if(stateChange_flag)
        {
          stateChange_flag = 0;
          P1LED_OFF;
          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("P1 wins!",Font_7x10,1);
          char score [16];
          uint16_t difference = p1King_counter - p2King_counter;
          sprintf(score, "Won by %ums", difference);
          lcd_put_cur(1,0);
          ssd1306_WriteString(score,Font_7x10,1);
          ssd1306_UpdateScreen();

        }
      break;

      case p2Winner:
        if(stateChange_flag)
        {
          stateChange_flag = 0;
          P1LED_OFF;
          ssd1306_Fill(0);
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("P2 wins!",Font_7x10,1);
          char score [16];
          uint16_t difference = p2King_counter - p1King_counter;
          sprintf(score, "Won by %ums", difference);
          lcd_put_cur(1,0);
          ssd1306_WriteString(score,Font_7x10,1);
          ssd1306_UpdateScreen();
        }
      break;

        case penalty:
        if(stateChange_flag)
        {
          stateChange_flag = 0;
          P1LED_OFF;
          ssd1306_Fill(0); 
          ssd1306_SetCursor(0,0);
          ssd1306_WriteString("Penalty!",Font_7x10,1);
          ssd1306_UpdateScreen();
        }

      break;
      
    }
}

void getKOTHPacket()
{
    // Wait for LoRa availability
    //xSemaphoreTake(xloraMutex, portMAX_DELAY);

    // Get the packet
    packet_size = LoRa_receive(&myLoRa, loraRXbuf, 10);

    if(packet_size > 0)
    {
        if(loraRXbuf[0] != receivedPacket[0])
        {
            memcpy(receivedPacket,loraRXbuf,10);
        }
    }
    //xSemaphoreGive(xloraMutex);
    
    // Parse the data
    if(strstr((char*)receivedPacket, "KOTH"))
    {
        long opcode;
        char * trashCatcher[10]; 
        
        opcode = strtol((char*)receivedPacket, trashCatcher,10);

        // Process opcode and respond if necessary
        switch (opcode)
        {
            case START:
            // Reset and start
            resetGame();
            // Send confirmation
            sendOpcode(START);
            gameState = p1King;
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
            gameState = p1King;
            stateChange_flag = 1; 
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
            // send message to opponent, wait on confirmation

            while(gameState == waiting)
            { 
                ssd1306_SetCursor(0,0);
                ssd1306_WriteString("Syncing..",Font_16x24,1);
                ssd1306_UpdateScreen();
                sendOpcode(START);
                osDelay(100);
            }
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
    LoRa_transmit(&myLoRa, (uint8_t*)opcodeString, 10, 100);

    LoRa_startReceiving(&myLoRa);
    //if (res != LORA_OK) {
    // Send failed
    //debugBuddy = res;

   // }

    // Release LoRa availability
    //xSemaphoreGive(xloraMutex);
    // osMutexRelease(lora_mutexHandle);

  
}