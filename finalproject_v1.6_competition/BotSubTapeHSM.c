/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Gabriel H Elkaim
 *
 * File: BotSubTapeHSM.c
 *
 * C Source file for the Tape Sensor SubHSM
 * 
 * Notes:
 * - Does not yet account for the cross.
 ******************************************************************************/


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_TattleTale.h"
#include "BOARD.h"
#include "BotHSM.h"
#include "BotSubTapeHSM.h"
#include "serial.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

//#define TAPEMASK_LEFT   0xE0    // 1110_0000
//#define TAPEMASK_RIGHT  0x1C    // 0001_1100
//#define TAPEMASK_CENTER 0x02    // 0000_0010
//#define TAPEMASK_ONTAPE 0x30    // 0011_0000
//#define TAPEMASK_BACK   0x01    // 0000_0001

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

uint8_t detectLeft(uint8_t tapeparam); // TRUE if detected from left
uint8_t detectRight(uint8_t tapeparam); // TRUE if detected from right
uint8_t detectCenter(uint8_t tapeparam); // TRUE if center is detected
uint8_t onTape(uint8_t tapeparam); // TRUE if CL, CR on tape
uint8_t offTape(uint8_t tapeparam); // TRUE if completely off tape

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/

/* You will need the state variable; you may need others as well.
 * The type of state variable should match that of enum. If all of the enums are
 * unique, then you can move it to the header file and add a query function. */

typedef enum {
    InitPSubState,
    NagivateOffTape,
    DoneTape
} BotTapeSubState_t;

static BotTapeSubState_t CurrentState = InitPSubState;

static const char *StateNames[] ={"InitPSubState",
"DetectingCenter",
"AlignToTape",
"FollowTape",
"SlightTurn",
"DoneTape"};

static uint8_t initTapeParam = 0; // keep track of which side hits tape first
static uint8_t tapeParam = 0; // another one

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitBotSubTapeHSM(void)
 * @param none
 * @return TRUE or FALSE
 * @brief This is the init function used to initialize (or reset) your sub-state
 *        machine, which will call the RunTemplateSubHSM with an INIT_EVENT to
 *        go through the initialization. It is up to the higher level state machine
 *        to call this at the appropriate time.
 * @note Remember to rename this to something appropriate.
 *       Returns TRUE if successful, FALSE otherwise
 * @author Gabriel H Elkaim, 2013.17.23 00:18 */
uint8_t InitBotSubTapeHSM(void) {
    ES_Event returnEvent;
    //printf("InitRoachMovingSubHSM\n");
    CurrentState = InitPSubState;
    returnEvent = RunBotSubTapeHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function RunBotSubTapeHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
ES_Event RunBotSubTapeHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    BotTapeSubState_t nextState;

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT) {
                printf("\n*    Init BotSubTapeHSM\n");
                nextState = NagivateOffTape;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        /*--- Detecting Center State ---*/
        // Continue moving until another tape sensor is detected.
        case NagivateOffTape:       // so grammar wow slow the fuck down bro
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    // keep going until center is found
                    FullStop();
                    MoveStraight(7);
                    initTapeParam = ThisEvent.EventParam; 
                    break; 
                case ES_EXIT:
                    break;
                case TAPECHANGE:
                    printf("Navigate off tape, 0x%X", ThisEvent.EventParam);
                    if (detectLeft(ThisEvent.EventParam) && !detectRight(ThisEvent.EventParam)) {
                        printf("\n*    Spin Right off tape\n");
                        SpinRight(7);
                    } else if (detectRight(ThisEvent.EventParam)) {
                        SpinLeft(7);
                        printf("\n*    Spin left off tape\n");
                    }
                    if (!detectLeft(ThisEvent.EventParam) && !detectRight(ThisEvent.EventParam)){
                    //if (offTape(ThisEvent.EventParam)) {
                        FullStop(); //stop to prevent electricity DEATH
                        nextState = DoneTape;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;

        /*--- DoneTape ---*/
        // Done in sub HSM, post new event to exit out of state.
        case DoneTape:
            switch (ThisEvent.EventType) {
                case ES_ENTRY: // not exiting state machine
                {
                    printf("DoneTape\n");
                    ES_Event newEvent;
                    newEvent.EventType = DONE_FOLLOWINGTAPE;
                    newEvent.EventParam = 0x0000;
                    PostBotHSM(newEvent);
                }
                    break;
                case ES_EXIT:
                    InitBotSubTapeHSM(); //restart 
                    break;

                default: break;
            }
            break;
        default: break;
    }

    if (makeTransition == TRUE) {
        RunBotSubTapeHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunBotSubTapeHSM(ENTRY_EVENT);
    }

    ES_Tail();
    return ThisEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

// Returns true if any of the 3 left tape sensors are on the tape. (xxx0_0000)

uint8_t detectLeft(uint8_t tapeval) {
    if ((tapeval & TAPEMASK_LEFT) != 0) {
        return TRUE;
    }
    return FALSE;
}

// Returns true if any of the 3 right tape sensors are on the tape. (000x_xx00);

uint8_t detectRight(uint8_t tapeval) {
    if ((tapeval & TAPEMASK_RIGHT) != 0) {
        return TRUE;
    }
    return FALSE;
}

// Returns true if the center sensor detects the tape. (0000_00x0);

uint8_t detectCenter(uint8_t tapeval) {
    if ((tapeval & TAPEMASK_CENTER) != 0) {
        return TRUE;
    }
    return FALSE;
}

// Returns true the front center two sensors are on the tape. (00xx_0000);

uint8_t onTape(uint8_t tapeval) {
    if ((tapeval & TAPEMASK_ONTAPE) == TAPEMASK_ONTAPE) {
        return TRUE;
    }
    return FALSE;
}

// Returns true if no tape is detected.

uint8_t offTape(uint8_t tapeval) {
    if (tapeval == 0) {
        return TRUE;
    }
    return FALSE;
}