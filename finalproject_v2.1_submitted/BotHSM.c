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
#include "BotHSM.h"
#include "BotFrameworkEvents.h"
#include "bot.h"

#include <stdio.h>      // for debugging
#include <string.h>

// include sub-state machines header files
#include "BotSubTapeHSM.h"
#include "BotSubBumpHSM.h"

int TS = 0;		// tape sensor
int BS = 0;		// bump sensor
int Tdir = 0;
/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

#define SOLENOID_DURATION   75     // time the solenoid stays contracted to let ping pong balls through
#define FINDBEACON_DURATION 5000    //5000    // time the bot spins
#define INITMOVE_DURATION   2000    //2000    // time the bot moves forward to avoid initial cross
#define INITANGLE_DURATION  900        // ~ quarter turn

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
static const char *StateNames[] = {"InitPState",
    "FindBeacon",
    "InitMove",
    "Moving",
    "DetectTape",
    "DetectBump",
    "Stopped"};

static BotState_t CurrentState = InitPState;
static uint8_t MyPriority;

uint8_t bumpParam = 0;
uint8_t compass = UNDETERMINED;

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
uint8_t InitBotHSM(uint8_t Priority) {
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = InitPState;
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
uint8_t PostBotHSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function QueryBotSM(void)
 * @param none
 * @return Current state of the state machine
 * @brief This function is a wrapper to return the current state of the state
 *        machine. Return will match the ENUM above.
 * @author Gabriel H Elkaim, 2013.09.26 15:33 */
BotState_t QueryBotSM(void) {
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
ES_Event RunBotHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    BotState_t nextState; // <- need to change enum type here

    ES_Tattle(); // trace call stack

    switch (CurrentState) { // first set of switch cases are states...
        case InitPState:
            if (ThisEvent.EventType == ES_INIT) {
                printf("\nInit State\n");
                InitBotSubBumpHSM();
                InitBotSubTapeHSM();
                PitchingOn(8);
                nextState = FindBeacon;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        /*--- FindBeacon ---*/
        // No sub HSM, spins until either the beacon is detected or the beacon
        // timer expires. Transistions to InitMove in both cases.
        case FindBeacon:
            switch (ThisEvent.EventType) { // ... while the second set of switch cases within the states are events, aka transitions :)
                case ES_ENTRY: // start spinning, set timer
                    printf("\n* FindBeacon\n");
                    ES_Timer_InitTimer(INITIALSTATES_TIMEOUT, FINDBEACON_DURATION);
                    SpinRight(10);
                    break;
                case ES_EXIT:
                    FullStop();
                    break;
                case BEACONCHANGE: // detected beacon, go forward
                    if (ThisEvent.EventParam == BEACON_DETECTED) {
                        printf("BEACON DETECTED\n");
                        ES_Timer_InitTimer(SOLENOID_TIMEOUT, SOLENOID_DURATION);
                        SolenoidOn();
                        nextState = InitAngle;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;

                case ES_TIMEOUT: // timeout (so our bot won't spin forever)
                    if (ThisEvent.EventParam == SOLENOID_TIMEOUT) { // specific timer
                        printf("* DetectTape: SOLENOID TIMEOUT\n");
                        SolenoidOff(); // turn off solenoid, no change in state
                    }
                    if (ThisEvent.EventParam == INITIALSTATES_TIMEOUT) {
                        printf("Beacon TIMEOUT\n");
                        nextState = InitAngle;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: break;
            }
            break; // end of FindBeacon state

         /*--- InitAngle ---*/
        // Turns 45ish degrees left to try to reach tape
        case InitAngle:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\n* InitAngle\n");
                    ES_Timer_InitTimer(INITIALSTATES_TIMEOUT, INITANGLE_DURATION); // change time to move just past the cross
                    FullStop();
                    SpinLeft(10);
                    break;
                case ES_EXIT:
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == INITIALSTATES_TIMEOUT) {
                        //nextState = InitMove;
                        nextState = Moving;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: break;
            }
            break;

        /*--- InitMoveForward ---*/
        // No sub HSM. Moves forward for a certain amount of time to avoid
        // hitting the first cross. Exits to Moving state at timeout.
//        case InitMove:
//            switch (ThisEvent.EventType) {
//                case ES_ENTRY:
//                    printf("\n* InitMove\n");
//                    ES_Timer_InitTimer(INITIALSTATES_TIMEOUT, INITMOVE_DURATION); // change time to move just past the cross
//                    MoveStraight(7);
//                    break;
//                case ES_EXIT:
//                    break;
//                case BEACONCHANGE: // beaconchange w/param of 1 = beacon detected
//                    if (ThisEvent.EventParam == 1) { // set timer for solenoid, turn solenoid on (allow ball to fall through)
//                        printf("* Moving: BEACON DETECTED\n");
//                        ES_Timer_InitTimer(SOLENOID_TIMEOUT, SOLENOID_DURATION);
//                        SolenoidOn();
//                    }
//                case ES_TIMEOUT:
//                    if (ThisEvent.EventParam == SOLENOID_TIMEOUT) { // specific timer
//                        printf("* Moving: SOLENOID TIMEOUT\n");
//                        SolenoidOff(); // turn off solenoid, no change in state
//                    } else if (ThisEvent.EventParam == INITIALSTATES_TIMEOUT) {
//                        nextState = Moving;
//                        makeTransition = TRUE;
//                        ThisEvent.EventType = ES_NO_EVENT;
//                    }
//                    break;
//                default: break;
//            }
//            break;

        /*--- Moving ---*/
        // No sub HSM. ~just keep moving~
        case Moving:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\n* Moving\n");
                    ES_Timer_InitTimer(INITIALSTATES_TIMEOUT, 1200);
                    MoveStraight(10);
                    break;
                case ES_EXIT:
                    break;
                case TAPECHANGE:
                    printf("* Moving: TAPECHANGE, 0x%X", ThisEvent.EventParam);
                    printf("\n");
                    nextState = DetectTape;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BUMPCHANGE: // change in bumpers
                    if(ThisEvent.EventParam != 0) {
                        bumpParam = ThisEvent.EventParam;
                        printf("* Moving: BUMPCHANGE ");
                        printbin(ThisEvent.EventParam);
                        nextState = DetectBump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case BEACONCHANGE: // beaconchange w/param of 1 = beacon detected
                    if (ThisEvent.EventParam == 1) { // set timer for solenoid, turn solenoid on (allow ball to fall through)
                        printf("* Moving: BEACON DETECTED\n");
                        ES_Timer_InitTimer(SOLENOID_TIMEOUT, SOLENOID_DURATION);
                        SolenoidOn();
                    }
                    break;
                case ES_TIMEOUT: // timeout with param of timer name only reacts to expiration of that
                    if (ThisEvent.EventParam == SOLENOID_TIMEOUT) { // specific timer
                        printf("* Moving: SOLENOID TIMEOUT\n");
                        SolenoidOff(); // turn off solenoid, no change in state
                    } else if(ThisEvent.EventParam == INITIALSTATES_TIMEOUT){
                        if(TS == 1 && BS == 1){//next state BumpTape, happens when TS & BS happen within a certain time.
//                            nextState = BumpTape;
//                            makeTransition = TRUE;
//                            ThisEvent.EventType = ES_NO_EVENT;
                        }
//                            printf("* Pseudo BumpTape state, spin left or right & set timer\n");
//                            if(Tdir == 1)//execute inside BumpTape
//                                SpinLeft(10);//
//                            if(Tdir == 2)//
//                                SpinRight(10);//
//                            ES_Timer_InitTimer(SpinTime, 1250);//
//                        }else{//
//                            TS = 0;//
//                            BS = 0;//
//                        }//
//                    } else if(ThisEvent.EventParam == SpinTime){//Inside BumpTape
//                        FullStop();                            //return to Moving
//                        MoveStraight(10);
//                        printf("* SpinTime timeout, 'return' to moving state\n");
                    }
                    break;
                default:
                    break;
            }
            break;

		  /*--- BumpTape ---*/ 
        case BumpTape:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\n* BumpTape, spin after a timer\n");   // spin left or right and set timer
                    ES_Timer_InitTimer(SpinTime, 1250);
                    if (Tdir == 1) {
                        SpinLeft(10);
                    } else if (Tdir == 2) {
                        SpinRight(10);
                    } else {
                        TS = 0;
                        BS = 0;
                    }
                    break;
                case ES_EXIT:
                    break;
                case TAPECHANGE:
//                    printf("* BumpTape: TAPECHANGE, 0x%X", ThisEvent.EventParam);
//                    printf("\n");
//                    nextState = DetectTape;
//                    makeTransition = TRUE;
//                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BUMPCHANGE: // change in bumpers
//                    if(ThisEvent.EventParam != 0) {
//                        bumpParam = ThisEvent.EventParam;
//                        printf("* BumpTape: BUMPCHANGE ");
//                        printbin(ThisEvent.EventParam);
//                        nextState = DetectBump;
//                        makeTransition = TRUE;
//                        ThisEvent.EventType = ES_NO_EVENT;
//                    }
                    break;
                case BEACONCHANGE: // beaconchange w/param of 1 = beacon detected
//                    if (ThisEvent.EventParam == 1) { // set timer for solenoid, turn solenoid on (allow ball to fall through)
//                        printf("* BumpTape: BEACON DETECTED\n");
//                        ES_Timer_InitTimer(SOLENOID_TIMEOUT, SOLENOID_DURATION);
//                        SolenoidOn();
//                    }
                    break;
                case ES_TIMEOUT: // timeout with param of timer name only reacts to expiration of that
                    if(ThisEvent.EventParam == SpinTime){
                        printf("* BumpTape TIMEOUT, Moving\n");
                        FullStop();
                        MoveStraight(10);
                        nextState = Moving;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default:
                    break;
            }
            break;




        /*--- DetectTape state ---*/
        // Calls sub HSM. Still needs to be expanded for the cross but should
        // follow the line (theoretically). Posts DONE_FOLLOWINGTAPE event to
        // exit out of the sub HSM and return to the Moving state
        case DetectTape:
            ThisEvent = RunBotSubTapeHSM(ThisEvent); // Run sub-HSM
            if (ThisEvent.EventType != ES_NO_EVENT) { // event is still active in substate
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        break;
                    case ES_EXIT:
                        break;
                    case DONE_FOLLOWINGTAPE:
                        nextState = Moving;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                    case BEACONCHANGE: // beaconchange w/param of 1 = beacon detected
                        if (ThisEvent.EventParam == 1) { // set timer for solenoid, turn solenoid on (allow ball to fall through)
                            printf("* DetectTape: BEACON DETECTED\n");
                            ES_Timer_InitTimer(SOLENOID_TIMEOUT, SOLENOID_DURATION);
                            SolenoidOn();
                        }
                        break;
                    case BUMPCHANGE: // change in bumpers
                        if(ThisEvent.EventParam != 0) {
                            printf("* DetectTape: BUMPCHANGE ");
                            printbin(ThisEvent.EventParam);
                            bumpParam = ThisEvent.EventParam;
                            nextState = DetectBump;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                       }
                       break;
                    case ES_TIMEOUT: // timeout with param of timer name only reacts to expiration of that
                        if (ThisEvent.EventParam == SOLENOID_TIMEOUT) { // specific timer
                            printf("* DetectTape: SOLENOID TIMEOUT\n");
                            SolenoidOff(); // turn off solenoid, no change in state
                        }
                    default: break;
                }
            }
            break;

//        case DetectTapeBump:
//            ThistEvent = RunBotSubTapeBumpHsm(ThisEvent);
//            if(ThisEvent.EventType != ES_NO_EVENT){
//                switch(ThisEvent.EventType)
//                        case ES_ENTRY;
//                            InitBotSubTapeBumpHSM();
//                            break;
//                        case ES_EXIT:
//                            break;
//                        case DONE_TapeBump:
//                            nextState = Moving;
//                            makeTransition = TRUE;
//                            ThisEvent.EventType = ES_NO_EVENT;
//                            break;
//                        case BEACONCHANGE: // beaconchange w/param of 1 = beacon detected
//                            if (ThisEvent.EventParam == 1) { // set timer for solenoid, turn solenoid on (allow ball to fall through)
//                                printf("* DetectBump: BEACON DETECTED\n");
//                                ES_Timer_InitTimer(SOLENOID_TIMEOUT, SOLENOID_DURATION);
//                                SolenoidOn();
//                            }
//                        break;
//            }

        /*--- DetectBump ---*/
        // Calls sub HSM. Runs through a sequence: backs up, turns, continues
        // on with its life. Posts DONE_EVADING event to exit out of sub HSM
        // and goes back to Moving state
        case DetectBump:
            ThisEvent = RunBotSubBumpHSM(ThisEvent); // Run sub-HSM
            if (ThisEvent.EventType != ES_NO_EVENT) {
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        InitBotSubBumpHSM();    // init every entry b/c sub HSM is a sequence
                        break;
                    case ES_EXIT:
                        break;
                    case DONE_EVADING:
                        nextState = Moving;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                    case BEACONCHANGE: // beaconchange w/param of 1 = beacon detected
                        if (ThisEvent.EventParam == 1) { // set timer for solenoid, turn solenoid on (allow ball to fall through)
                            printf("* DetectBump: BEACON DETECTED\n");
                            ES_Timer_InitTimer(SOLENOID_TIMEOUT, SOLENOID_DURATION);
                            SolenoidOn();
                        }
                        break;
                   case BUMPCHANGE: // change in bumpers
                       if(ThisEvent.EventParam != 0) {
                            InitBotSubBumpHSM();
                            printf("* DetectBump: BUMPCHANGE ");
                            printbin(ThisEvent.EventParam);
                            bumpParam = ThisEvent.EventParam;
                            nextState = DetectBump;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                       }
                       break;
                    case TAPECHANGE:
                        printf("* DetectBump: TAPECHANGE ");
                        printbin(ThisEvent.EventParam);
                        nextState = DetectTape;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                    case ES_TIMEOUT: // timeout with param of timer name only reacts to expiration of that
                        if (ThisEvent.EventParam == SOLENOID_TIMEOUT) { // specific timer
                            printf("* DetectBump: SOLENOID TIMEOUT\n");
                            SolenoidOff(); // turn off solenoid, no change in state
                        }
                    default: break;
                }
            }
            break;

        default: break;
    }

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunBotHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunBotHSM(ENTRY_EVENT);
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

// printbin()
// used to print out binary reading of current sensor values for debugging
void printbin(uint16_t numnum) {
    static char b[10];
    b[0] = '\0';
    int z;
    for (z = 256; z > 0; z >>= 1)
    {
        strcat(b, ((numnum & z) == z) ? "1" : "0");
    }
    printf("%s\n",b);
}