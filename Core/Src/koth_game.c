#include "main.h"
#include <string.h>
#include "define.h"
#include "extern.h"
#include <stdio.h>
#include <stdlib.h>
#include "lora.h"
#include "ssd1306.h"
#include "semphr.h"
#include "FreeRTOS.h"



void resetGame()
{
    if( xSemaphoreTake(xloraMutex, portMAX_DELAY) == pdTRUE )
    {
        REDLED_OFF;
        BLUELED_OFF;
        red_flag = 0;
        blue_flag = 0;
        red_counter = 0;
        blue_counter = 0;
        gameState = WAITING;
        stateChange_flag = 1;

        xSemaphoreGive(xloraMutex);
    }

}

void doGameState()
{
    uint8_t newGameState, newStateChange, newTXready, newRXready, newblue_flag, newred_flag;

    // Save current values into temp vars
    newGameState = gameState;
    newStateChange = stateChange_flag;
    newTXready = TXready_flag;
    newRXready = RXready_flag;
    newblue_flag = blue_flag;
    newred_flag = red_flag;

    // New opcode rx
    if(RXready_flag)
    {

        switch (loraRXopcode)
            {

                case goSTART: // other player started game
                    if(THIS_PLAYER == RED) 
                    {
                        newGameState = BLUE;
                    }
                    else 
                    {
                        newGameState = RED;
                    }
                    newStateChange = 1; 
                break;

                case goBLUE: 
                    newGameState = BLUE;
                    newStateChange = 1; 
                break; 
                
                case goRED:
                    newGameState = RED;
                    newStateChange = 1; 
                break; 

                case RED:
                case BLUE:
                    newGameState = loraRXopcode;
                    newStateChange = 0; 
                break;


                case REDWINS:
                case BLUEWINS:
                    if(gameState != loraRXopcode)
                    {
                        newGameState = loraRXopcode;
                        newStateChange = 1;
                    }
                    
                
                break;
                
            }

            newRXready = 0;

    }
    
    switch (gameState)
    {
        case WAITING:
            if(stateChange_flag) // do once
            { 
                newStateChange = 0;
                
            }
        break;


        case goSTART:
        case goRED:
        case goBLUE:

            // Limbo state, don't increment stuff until other player syncs
            newStateChange = 0;
            newred_flag = 0;
            newblue_flag = 0;

            setOpcode(gameState);
            newTXready = 1;

            osDelay(100);

        break;


        case RED:
            if(stateChange_flag)
            {
                setOpcode(RED);
                newTXready = 1;
                newStateChange = 0;
            }
                newred_flag = 1;
                newblue_flag = 0;
                REDLED_ON;
                BLUELED_OFF;
    
        break;

        case BLUE:
            if(stateChange_flag)
            {
                setOpcode(BLUE);
                newTXready = 1;
                newStateChange = 0;
            }
                newred_flag = 0;
                newblue_flag = 1;
                REDLED_OFF;
                BLUELED_ON;
        break;

        case REDWINS:
            if(stateChange_flag)
            {
                setOpcode(REDWINS);
                newTXready = 1;
                newStateChange = 0;
                REDLED_ON;
                BLUELED_OFF;
            }
            // blinking flag
        break;

        case BLUEWINS:
            if(stateChange_flag)
            {
                setOpcode(BLUEWINS);
                newTXready = 1;
                newStateChange = 0;
                REDLED_OFF;
                BLUELED_ON;
            }
            // blinking flag
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

    // Protected action: update globals
    if(xSemaphoreTake(xloraMutex, portMAX_DELAY) == pdTRUE )
    { 
        gameState = newGameState;
        stateChange_flag = newStateChange;

        RXready_flag = newRXready;
        TXready_flag = newTXready;
        red_flag = newred_flag;
        blue_flag = newblue_flag;
        xSemaphoreGive(xloraMutex);
    }


}

void decodeKOTHPacket()
{

    // Parse the data
    if(strstr((char*)receivedPacket, "KOTH"))
    {
        long opcodeRX;
        char * trashCatcher[10]; 
        
        opcodeRX = strtol((char*)receivedPacket, trashCatcher,10);

        // process
        if(gameState != opcodeRX) 
        {
            // Protected action: update globals
            if(xSemaphoreTake(xloraMutex, portMAX_DELAY) == pdTRUE )
            { 
                loraRXopcode = opcodeRX;
                gameState = opcodeRX;
                stateChange_flag = 1;
                xSemaphoreGive(xloraMutex);
            }
        }

    }
        
}

void btnPressed()
{
    uint8_t newGameState, newStateChange;

    newGameState = gameState;
    newStateChange = stateChange_flag;

    switch (gameState)
    {
        case WAITING:
            newStateChange = 1; 
            newGameState = goSTART;    
            
        break;
        
        case RED:
        case BLUE:

            newStateChange = 1; 
            
            /*
            if(THIS_PLAYER == gameState) // oops! button masher penalty!
            {
                if(THIS_PLAYER == RED) 
                {
                    newGameState = goBLUE;
                }
                else 
                {
                    newGameState = goRED;
                }

                // TBD: disable button for a bit, penalty
            }
            */
            

                if(THIS_PLAYER == RED) 
                {
                    newGameState = goRED;
                }
                else 
                {
                    newGameState = goBLUE;
                }
                

        break;

        
        case REDWINS:
        case BLUEWINS:
            newStateChange = 1; 
            resetGame();
            
        break;

    }

    // Protected action: update globals
    if(xSemaphoreTake(xloraMutex, portMAX_DELAY) == pdTRUE )
    {
        gameState = newGameState;
        stateChange_flag = newStateChange;

        xSemaphoreGive(xloraMutex);
    }

}

void setOpcode(uint8_t opcodeTX)
{
    if(xSemaphoreTake(xloraMutex, portMAX_DELAY) == pdTRUE )
    {
        memset(opcodeString, 0, sizeof(opcodeString));

        // Create encoded string
        sprintf(opcodeString, "%uKOTH", opcodeTX);

        memcpy(txReadable, &gameStateNames[opcodeTX], sizeof(&gameStateNames[opcodeTX]));
        
        xSemaphoreGive(xloraMutex);
    }
  
}