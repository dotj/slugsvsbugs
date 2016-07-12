/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Gabriel H Elkaim
 *
 * File: BotHSM.h
 *
 * C Source file for the top-level HSM.
 *
 * Notes:
 * - hi
 ******************************************************************************/

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_TattleTale.h"
#include "BOARD.h"
#include "BotFSM.h"
#include "BotFrameworkEvents.h"
#include "bot.h"


#include <stdio.h>      // for debugging
#include <string.h>


/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

unsigned short int position = 1500;
BOOL scan = FALSE;
BOOL SHOOT = FALSE;

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

void printbin(uint16_t numnum); // to print binary numbers

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */
static const char *StateNames[] ={"InitState",
"ScanLBeacon",
"ScanRBeacon",
"Still"};


static BotFSMState_t CurrentState = InitState;
static uint8_t MyPriority;

uint8_t beaconParam = 0;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitBotHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunRoachFSM function.
 *        Returns TRUE if successful, FALSE otherwise
 * @author Gabriel H Elkaim, 2013.09.26 15:33 */
uint8_t InitBotFSM(uint8_t Priority) {
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = InitState;
    // post the initial transition event
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostBotHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Returns TRUE if successful, FALSE otherwise
 * @author Gabriel H Elkaim, 2013.09.26 15:33 */
uint8_t PostBotFSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function QueryBotSM(void)
 * @param none
 * @return Current state of the state machine
 * @brief This function is a wrapper to return the current state of the state
 *        machine. Return will match the ENUM above.
 * @author Gabriel H Elkaim, 2013.09.26 15:33 */
BotFSMState_t QueryBotFSM(void) {
    return (CurrentState);
}

/**
 * @Function RunBotHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This is the function that implements the actual state machine; in the
 *        roach case, it is a simple ping pong state machine that will form the
 *        basis of your more complicated exploration. This function will be called
 *        recursively to implement the correct order for a state transition to be:
 *        exit current state -> enter next state using the ES_EXIT and ES_ENTRY
 *        events.
 * @author Gabriel H Elkaim, 2013.09.26 15:33 */
ES_Event RunBotFSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    BotFSMState_t nextState; // <- need to change enum type here

    ES_Tattle(); // trace call stack


     switch (CurrentState) { // first set of switch cases are states...
        case InitState:
            printf("FSM Init\n");
            if (ThisEvent.EventType == ES_INIT) {
                nextState = ScanLBeacon;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case ScanLBeacon:
            switch (ThisEvent.EventType) { 
                case ES_ENTRY:
                    printf("Scan Left Entry\n");
                    ES_Timer_InitTimer(SERVO_TIMEOUT, 5);
                    break;
                case ES_EXIT:
                    break;
                case BEACONRIGHT:
                    printf("Scan Left Beacon RIght\n");
                    nextState = ScanRBeacon;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
//                case BEACONLEFT:
//                    printf("Scan Left BeaconLeft\n");
//                    break;
                case BEACONCHANGE:
                    printf("Scan Left BeaconCenter\n");
                    nextState = Still;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case ES_TIMEOUT:
                     if (ThisEvent.EventParam == SERVO_TIMEOUT) {
                         position-=2;
                         Turret(position);
                         if(position <= 700){
                            nextState = ScanRBeacon;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                         }
                     ES_Timer_InitTimer(SERVO_TIMEOUT, 5);
                    }
                     break;
                default: break;
            }
            break;

        case ScanRBeacon:
            switch (ThisEvent.EventType) { // while the second set of switch cases within the states are events, aka transitions :)
                case ES_ENTRY:
                    printf("Scan Right Entry\n");
                    ES_Timer_InitTimer(SERVO_TIMEOUT_2, 5);// SET TIMER HERE
                    break;
                case ES_EXIT:
                    break;
//                case BEACONRIGHT:
//                    printf("Scan RIGHT Beacon RIght\n");
//                    break;
                case BEACONLEFT:
                    printf("Scan RIGHT BeaconLeft\n");
                    nextState = ScanLBeacon;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BEACONCHANGE:
                    printf("Scan RIGHT BeaconCenter\n");
                    nextState = Still;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case ES_TIMEOUT:
                     if (ThisEvent.EventParam == SERVO_TIMEOUT_2) {
                         position+=2;
                         Turret(position);
                         if(position >= 2300){
                             nextState = ScanLBeacon;
                             makeTransition = TRUE;
                             ThisEvent.EventType = ES_NO_EVENT;
                         }
                     ES_Timer_InitTimer(SERVO_TIMEOUT_2, 5);
                    }
                     break;
                default: break;
            }
            break;

         case Still:

            switch (ThisEvent.EventType) { 
                case ES_ENTRY:
                    printf("Still Entry\n");
                    break;
                case ES_EXIT:
                    break;
                case BEACONRIGHT:
                    printf("Scan STILL Beacon RIght\n");
                    nextState = ScanRBeacon;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BEACONLEFT:
                    printf("Scan STILL BeaconLeft\n");
                    nextState = ScanLBeacon;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT; 
                    break;
//                case BEACONCHANGE:
//                    printf("Scan RIGHT BeaconCenter\n");
//                    break;
                default: break;
            }
        default: break;
    }

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunBotFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunBotFSM(ENTRY_EVENT);
    }
     
    ES_Tail(); // trace call stack end
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
//
//void printbin(uint16_t numnum) {
//    static char b[10];
//    b[0] = '\0';
//    int z;
//    for (z = 256; z > 0; z >>= 1)
//    {
//        strcat(b, ((numnum & z) == z) ? "1" : "0");
//    }
//    printf("%s\n",b);
//}
