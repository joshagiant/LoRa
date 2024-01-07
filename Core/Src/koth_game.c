#include "main.h"
#include <string.h>
#include "define.h"
#include "extern.h"
#include <stdio.h>
#include <stdlib.h>
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
    // New opcode rx
    if(newRX_flag)
    {

        switch (loraRXopcode)
            {

                case goSTART: // other player started game
                    if(THIS_PLAYER == RED) 
                    {
                        gameState = goBLUE;
                    }
                    else 
                    {
                        gameState = goRED;
                    }
                    stateChange_flag = 1; 
                break;

                case goBLUE: 
                    gameState = BLUE;
                    stateChange_flag = 1; 
                break; 
                
                case goRED:
                    gameState = RED;
                    stateChange_flag = 1; 
                break; 

                case RED:
                case BLUE:
                    gameState = loraRXopcode;
                    stateChange_flag = 0; 
                break;


                case REDWINS:
                case BLUEWINS:
                    gameState = loraRXopcode;
                    stateChange_flag = 1; 
                
                break;
                
        }

        newRX_flag = 0;
    }
    
    switch (gameState)
    {
        case WAITING:
            if(stateChange_flag) // do once
            { 
                stateChange_flag = 0;
                ssd1306_Fill(0); 
                ssd1306_SetCursor(0,0);
                if (THIS_PLAYER == RED) 
                { 
                    ssd1306_WriteString("RED",Font_11x18,1);
                }
                else    
                {
                    ssd1306_WriteString("BLUE",Font_11x18,1);
                }
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

            setOpcode(gameState);
            newTX_flag = 1;

            osDelay(100);

        break;


        case RED:
            if(stateChange_flag)
            {
                setOpcode(RED);
                newTX_flag = 1;
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
                setOpcode(BLUE);
                newTX_flag = 1;
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
                setOpcode(REDWINS);
                newTX_flag = 1;
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
                setOpcode(BLUEWINS);
                newTX_flag = 1;
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
            loraRXopcode = opcodeRX;
            gameState = opcodeRX;
            stateChange_flag = 1;
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
                if(THIS_PLAYER == RED) 
                {
                    gameState = goBLUE;
                }
                else 
                {
                    gameState = goRED;
                }

                // TBD: disable button for a bit, penalty
            }
            
            else // Took the hill!
            {
                if(THIS_PLAYER == RED) 
                {
                    gameState = goRED;
                }
                else 
                {
                    gameState = goBLUE;
                }
                
            }

        break;

        
        case REDWINS:
        case BLUEWINS:
            stateChange_flag = 1; 
            gameState = WAITING;
            
        break;

    }


}

void setOpcode(uint8_t opcodeTX)
{
    memset(opcodeString, 0, sizeof(opcodeString));

    // Create encoded string
    sprintf(opcodeString, "%uKOTH", opcodeTX);

    // Wait for LoRa availability
    //xSemaphoreTake(xloraMutex, portMAX_DELAY);

    memcpy(txReadable, &gameStateNames[opcodeTX], sizeof(&gameStateNames[opcodeTX]));

    // Send it via LoRa
    // LoRa_transmit(&myLoRa, (uint8_t*)opcodeString, 10, 100);

    // LoRa_startReceiving(&myLoRa);

    // Release LoRa availability
    //xSemaphoreGive(xloraMutex);
    // osMutexRelease(lora_mutexHandle);

  
}

void btnPressedSimple()
{

    //newTX_flag = 1;

    /*
    sprintf(opcodeString, "%uKOTH", THIS_PLAYER);

    gameState = THIS_PLAYER;
    stateChange_flag = 1;

    // Send it via LoRa
    LoRa_transmit(&myLoRa, (uint8_t*)opcodeString, 10, 100);

    LoRa_startReceiving(&myLoRa);
    */

}

void doGameStateSimple()
{

    switch (gameState)
        {
            case WAITING:
                if(stateChange_flag) // do once
                { 
                    stateChange_flag = 0;
                    ssd1306_Fill(0); 
                    ssd1306_SetCursor(0,0);
                    if (THIS_PLAYER == RED) 
                    {
                        ssd1306_WriteString("RED",Font_11x18,1);
                    }
                    else 
                    {
                        ssd1306_WriteString("BLUE",Font_11x18,1);
                    }
                    ssd1306_SetCursor(25,21);
                    ssd1306_WriteString("Push to play!",Font_7x10,1);
                    ssd1306_UpdateScreen();
                }
            break;


            case RED:
                if(stateChange_flag)
                {
                    setOpcode(RED);
                    newTX_flag = 1;
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
                    setOpcode(BLUE);
                    newTX_flag = 1;
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
                    setOpcode(REDWINS);
                    newTX_flag = 1;
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
                    setOpcode(BLUEWINS);
                    newTX_flag = 1;
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
    }
    

}

