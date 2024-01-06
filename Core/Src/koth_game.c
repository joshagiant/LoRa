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
    REDLED_OFF;
    BLUELED_OFF;
    red_flag = 0;
    blue_flag = 0;
    red_counter = 0;
    blue_counter = 0;
    gameState = WAITING;
    stateChange_flag = 1;

}

void doGameState()
{
    
    switch (gameState)
    {
        case WAITING:
            if(stateChange_flag) // do once
            { 
                stateChange_flag = 0;
                ssd1306_Fill(0); 
                ssd1306_SetCursor(0,0);
                if (THIS_PLAYER == RED) ssd1306_WriteString("RED",Font_11x18,1);
                else ssd1306_WriteString("BLUE",Font_11x18,1);
                ssd1306_SetCursor(25,21);
                ssd1306_WriteString("Push to play!",Font_7x10,1);
                ssd1306_UpdateScreen();
            }
        break;

        case goSTART:
        case goRED:
        case goBLUE:

            // Limbo state, don't increment stuff until other player syncs
            stateChange_flag = 0;
            red_flag = 0;
            blue_flag = 0;
            REDLED_OFF;
            BLUELED_OFF;

            while( (gameState == goSTART) || (gameState == goRED) || (gameState == goBLUE) )
            {
                sendOpcode(gameState);
                osDelay(50);
            }

        break;


        case RED:
            if(stateChange_flag)
            {
                stateChange_flag = 0;
                red_flag = 1;
                blue_flag = 0;
                REDLED_ON;
                BLUELED_OFF;
                ssd1306_Fill(0);
                ssd1306_SetCursor(0,0);
                ssd1306_WriteString("RED is king",Font_7x10,1);
                ssd1306_SetCursor(0,10);
                ssd1306_WriteString("of the hill!",Font_7x10,1);
                ssd1306_UpdateScreen();
            }


        break;

        case BLUE:
            if(stateChange_flag)
            {
                stateChange_flag = 0;
                red_flag = 0;
                blue_flag = 1;
                REDLED_OFF;
                BLUELED_ON;
                ssd1306_Fill(0);
                ssd1306_SetCursor(0,0);
                ssd1306_WriteString("BLUE is king",Font_7x10,1);
                ssd1306_SetCursor(0,10);
                ssd1306_WriteString("of the hill!",Font_7x10,1);
                ssd1306_UpdateScreen();
            }
        break;

        case REDWINS:
            if(stateChange_flag)
            {
                stateChange_flag = 0;
                REDLED_ON;
                BLUELED_OFF;
                ssd1306_Fill(0);
                ssd1306_SetCursor(0,0);
                ssd1306_WriteString("Red wins!",Font_7x10,1);
                char score [16];
                uint16_t difference = red_counter - blue_counter;
                sprintf(score, "Won by %ums", difference);
                lcd_put_cur(1,0);
                ssd1306_WriteString(score,Font_7x10,1);
                ssd1306_UpdateScreen();
            }
        break;

        case BLUEWINS:
            if(stateChange_flag)
            {
                stateChange_flag = 0;
                REDLED_OFF;
                BLUELED_ON;
                ssd1306_Fill(0);
                ssd1306_SetCursor(0,0);
                ssd1306_WriteString("Blue wins!",Font_7x10,1);
                char score [16];
                uint16_t difference = blue_counter - red_counter;
                sprintf(score, "Won by %ums", difference);
                lcd_put_cur(1,0);
                ssd1306_WriteString(score,Font_7x10,1);
                ssd1306_UpdateScreen();
            }
        break;

/*
            case penalty:
            if(stateChange_flag)
            {
            stateChange_flag = 0;
            REDLED_OFF;
            ssd1306_Fill(0); 
            ssd1306_SetCursor(0,0);
            ssd1306_WriteString("Penalty!",Font_7x10,1);
            ssd1306_UpdateScreen();
            }

        break;
*/
      
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
        long opcodeRX;
        char * trashCatcher[10]; 
        
        opcodeRX = strtol((char*)receivedPacket, trashCatcher,10);

        // Process opcode and respond if necessary
        // make a readable copy
        memcpy(rxReadable,&gameStateNames[opcodeRX], sizeof(&gameStateNames[opcodeRX]));

        // process
        if(gameState != opcodeRX)
        {
            switch (opcodeRX)
            {
                case START:
                case goSTART:
                    gameState = START;
                    stateChange_flag = 1; 
                break;

                case BLUE:
                case goBLUE: 
                    sendOpcode(BLUE);
                    gameState = BLUE;
                    stateChange_flag = 1; 
                break; 

                case RED:
                case goRED: 
                    sendOpcode(RED);
                    gameState = RED;
                    stateChange_flag = 1; 
                break; 

                case REDWINS:
                case BLUEWINS:
                    gameState = opcodeRX;
                    stateChange_flag = 1; 
                
                break;
                
                }
            }
    }
        
}

void btnPressed()
{
    switch (gameState)
    {
        case WAITING:
            stateChange_flag = 1; 
            gameState = goSTART;    
            
        break;
        
        case RED:
        case BLUE:

            stateChange_flag = 1; 
            
            if(THIS_PLAYER == gameState) // oops! button masher penalty!
            {
                if(THIS_PLAYER == RED) gameState = goBLUE;
                else gameState = goRED;

                // TBD: disable button for a bit, penalty
            }
            
            else // Took the hill!
            {
                if(THIS_PLAYER == RED) gameState = goRED;
                else gameState = goBLUE;
            }

        break;

        
        case REDWINS:
        case BLUEWINS:
            stateChange_flag = 1; 
            gameState = WAITING;
        break;

    }


}

void sendOpcode(uint8_t opcodeTX)
{
    memset(opcodeString, 0, sizeof(opcodeString));

    // Create encoded string
    sprintf(opcodeString, "%uKOTH", opcodeTX);

    // Wait for LoRa availability
    //xSemaphoreTake(xloraMutex, portMAX_DELAY);

    memcpy(txReadable, &gameStateNames[opcodeTX], sizeof(&gameStateNames[opcodeTX]));

    // Send it via LoRa
    LoRa_transmit(&myLoRa, (uint8_t*)opcodeString, 10, 100);

    LoRa_startReceiving(&myLoRa);

    // Release LoRa availability
    //xSemaphoreGive(xloraMutex);
    // osMutexRelease(lora_mutexHandle);

  
}
