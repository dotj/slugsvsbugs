/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Max Dunne
 *
 * File: BotMain.c
 *
 * Main source file for the final project. Change main to main1 when using the
 * test harness in bot.c
 ******************************************************************************/

#include <xc.h>
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
#include "serial.h"
#include "bot.h"
#include "BOARD.h"

#define DELAY(x)    for (wait = 0; wait <= x; wait++) {asm("nop");}

void main(void) {
    ES_Return_t ErrorType;
    unsigned int wait = 0;
    // Hardware Initializations
    BOARD_Init();
    Bot_Init();

    DELAY(1000) //wait a little bit before starting up


    printf("\nHi. Starting Bot State Machine. \r\n");

    // Initializing Events and Services Framework, Run HSM
    ErrorType = ES_Initialize();
    if (ErrorType == Success) {
        ErrorType = ES_Run();
    }

    // Reached error
    switch (ErrorType) {
        case FailedPointer:
            printf("Failed on NULL pointer\n");
            break;
        case FailedInit:
            printf("Failed Initialization\n");
            break;
        default:
            printf("Other Failure\n");
            break;
    }

    while (1);
};

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
