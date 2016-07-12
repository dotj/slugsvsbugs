/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Gabriel H Elkaim
 *
 * File: BotSubBumpHSM.c
 *
 * C Source file for the Bump Sensor SubHSM
 *
 * Notes:
 * - change timers, need more special cases?
 ******************************************************************************/


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_TattleTale.h"
#include "BOARD.h"
#include "BotHSM.h"
#include "BotSubBumpHSM.h"
#include "serial.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

//#define BUMPMASK_FRONT  0x02    // might be helpful if we expand our bump sensors
#define BUMPMASK_FL     0x01
#define BUMPMASK_FR     0x02

#define FULL_TURN       5000
#define HALF_TURN       (FULL_TURN/2)  // 2500
#define QUARTER_TURN    (FULL_TURN/4)  // 1250

#define REVERSE_DURATION    500
#define ARC1_DURATION       600    // quarter turn is 1250
#define ARC2_DURATION       600
#define ARC3_DURATION       1000
#define ARC4_DURATION       600
#define MOVE1_DURATION      1500
#define MOVE2_DURATION      2000
#define MOVE3_DURATION      1500

/*
 *         ^
 *         |   arc 4, back to moving state
 *          \
 *           \
 *            \  arc 3 move 3
 *             ^
 *             |
 *             |  arc 2, move 2
 *             /
 * reverse |  /
 *         | / arc 1, move 1
 *         v
 */

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

uint8_t bumpLeft(uint8_t bumpParam);
uint8_t bumpRight(uint8_t bumpParam);

static uint8_t arcCount = 0;

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/

/* You will need the state variable; you may need others as well.
 * The type of state variable should match that of enum. If all of the enums are
 * unique, then you can move it to the header file and add a query function. */

typedef enum {
    InitPSubState,
    Reverse,
    Arc1,
    Arc2,
    Arc3,
    Arc4,
    Move,
    HalfTurn,
    AnotherTurn,
    DoneBump
} BotBumpSubState_t;

static const char *StateNames[] = {"InitPSubState",
    "Reverse",
    "Arc1",
    "Arc2",
    "Arc3",
    "Arc4",
    "Move",
    "HalfTurn",
    "DoneBump"};

static BotBumpSubState_t CurrentState = InitPSubState;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitBotSubBumpHSM(void)
 * @param none
 * @return TRUE or FALSE
 * @brief This is the init function used to initialize (or reset) your sub-state
 *        machine, which will call the RunTemplateSubHSM with an INIT_EVENT to
 *        go through the initialization. It is up to the higher level state machine
 *        to call this at the appropriate time.
 * @note Remember to rename this to something appropriate.
 *       Returns TRUE if successful, FALSE otherwise
 * @author Gabriel H Elkaim, 2013.17.23 00:18 */
uint8_t InitBotSubBumpHSM(void) {
    ES_Event returnEvent;
    //printf("InitRoachMovingSubHSM\n");
    CurrentState = InitPSubState;
    returnEvent = RunBotSubBumpHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function RunBotSubBumpHSM(ES_Event ThisEvent)
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
ES_Event RunBotSubBumpHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    BotBumpSubState_t nextState;
    ES_Tattle(); // trace call stack
    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State

            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                printf("BotSubBumpHSM, 0x%X\n", bumpParam);
                nextState = Reverse;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT; // ES_NO_EVENT passes event back up to top level HSM
            }
            break;

            /* Reverse State */
            // Reverses until timer is up, saves param
        case Reverse:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("Backing Away\n");
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, REVERSE_DURATION);
                    FullStop();
                    BackStraight(10);
                    break;
                case ES_EXIT:
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        printf("Backing Away: TIMEOUT\n");
                        nextState = Arc1;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: break;
            }
            break;

            /* Arc1 State */
            // Turn left or right depending on prev event param
        case Arc1:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    BS = 1;
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, QUARTER_TURN); // quarter turn
                    if (bumpRight(bumpParam)) {
                        SpinLeft(10);
                        printf("Spin Left\n");
                    } else {
                        SpinRight(10);
                        printf("Spin Right\n");
                    }
                    break;
                case ES_EXIT:
                    break;
                case TAPECHANGE:
                    printf("* Bump sub HSM: TAPECHANGE ");
                    printbin(ThisEvent.EventParam);
                    nextState = DetectTape; //son
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        arcCount = 2; // next arc state
                        ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, 1000); // timer to move straight
                        nextState = Move;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        FullStop();
                    }
                    break;
                default: break;
            }
            break;

            // second arc
        case Arc2:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("arc 2, 0x%X\n", bumpParam);
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, QUARTER_TURN);
                    if (bumpRight(bumpParam)) {
                        SpinRight(10);
                    } else {
                        SpinLeft(10);
                    }
                    break;
                case ES_EXIT:
                    break;
                case TAPECHANGE:
                    printf("* Bump sub HSM: TAPECHANGE ");
                    printbin(ThisEvent.EventParam);
                    nextState = DetectTape; //son
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        arcCount = 3; // next arc state
                        ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, MOVE2_DURATION); // timer to move straight
                        nextState = DoneBump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        FullStop();
                    }
                    break;
                default: break;
            }
            break;

        case Arc3:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("arc 3, 0x%X\n", bumpParam);
                    if (compass == WEST) {
                        SpinLeft(10);
                    } else if (compass == EAST) {
                        SpinRight(10);
                    } else {
                        if (bumpRight(bumpParam)) {
                            SpinRight(10);
                        } else {
                            SpinLeft(10);
                        }
                    }
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, ARC3_DURATION); //(1250*2/3)
                    break;
                case ES_EXIT:
                    break;
                case TAPECHANGE:
                    printf("* Bump sub HSM: TAPECHANGE ");
                    printbin(ThisEvent.EventParam);
                    nextState = DetectTape; //son
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        arcCount = 4; // next arc state
                        ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, MOVE3_DURATION); // timer to move straight
                        nextState = DoneBump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        FullStop();
                    }
                    break;
                default: break;
            }
            break;

            /* Arc1 State */
            // Turn left or right depending on prev event param
        case Arc4:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("arc 4, 0x%X\n", bumpParam);
                    FullStop();
                    if (compass == WEST) {
                        SpinRight(10);
                    } else if (compass == EAST) {
                        SpinLeft(10);
                    } else {
                        if (bumpRight(bumpParam)) {
                            SpinLeft(10);
                        } else {
                            SpinRight(10);
                        }
                    }
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, ARC4_DURATION);
                    break;
                case ES_EXIT:
                    break;
                case TAPECHANGE:
                    printf("* Bump sub HSM: TAPECHANGE ");
                    printbin(ThisEvent.EventParam);
                    nextState = DetectTape; //son
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        nextState = DoneBump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        FullStop();
                    }
                    break;
                default: break;
            }
            break;

        case Move:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\nmove, %d\n", arcCount);
                    MoveStraight(10);
                    break;
                case ES_EXIT:
                    break;
                case TAPECHANGE:
                    printf("* Bump sub HSM: TAPECHANGE ");
                    printbin(ThisEvent.EventParam);
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, HALF_TURN);
                    //SpinLeft(10);
                    arcCount = 3;
                    nextState = HalfTurn; //turn 180
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        switch (arcCount) {
                            case 1: nextState = Arc1;
                                break;
                            case 2: nextState = Arc2;
                                break;
                            case 3: nextState = Move;
                                break;
                            case 4: nextState = Arc4;
                                break;
                            default: break;
                        }
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        FullStop();
                    }
                    break;
                default: break;
            }
            break;

        case HalfTurn:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("ARE WE TURNING 180?\n");
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, HALF_TURN);
                    SpinRight(7);
                    break;
                case ES_EXIT:
                    FullStop();
                    break;
                case ES_TIMEOUT:
                    printf("DO WE MAKE IT HERE?");
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        nextState = AnotherTurn;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: break;
            }
            break;

        case AnotherTurn:
            switch(ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(BUMPSTATES_TIMEOUT, QUARTER_TURN);
                    SpinRight(7);
                    break;
                case ES_EXIT:
                    FullStop();
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BUMPSTATES_TIMEOUT) {
                        nextState = DoneBump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;

            /*--- DoneBump ---*/
            // Done in sub HSM, post new event to exit out of state.
        case DoneBump:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                {
                    printf("\n*   DoneBump\n");
                    ES_Event newEvent;
                    newEvent.EventType = DONE_EVADING;
                    newEvent.EventParam = 0x0000;
                    PostBotHSM(newEvent);
                    break;
                }
                case ES_EXIT:
                    InitBotSubBumpHSM(); //restart
                    break;
                default: break;
            }
            break;
        default: break;
    }

    if (makeTransition == TRUE) {
        RunBotSubBumpHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunBotSubBumpHSM(ENTRY_EVENT);
    }

    ES_Tail();
    return ThisEvent;
}


/***************************************************************************
 private functions
 ***************************************************************************/

// returns TRUE if bumped from the left

uint8_t bumpLeft(uint8_t bumpval) {
    return ((bumpval & BUMPMASK_FL) == BUMPMASK_FL);
}

// return TRUE if bumped from the right

uint8_t bumpRight(uint8_t bumpval) {
    return ((bumpval & BUMPMASK_FR) == BUMPMASK_FR);
}