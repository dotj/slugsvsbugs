/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Gabriel H Elkaim
 *
 * File: BotHSM.h
 *
 * Header file for the top-level HSM.
 ******************************************************************************/

#ifndef BOTHSM_H
#define BOTHSM_H


/*******************************************************************************
 * PUBLIC #INCLUDES                                                            *
 ******************************************************************************/

// defines ES_Event, INIT_EVENT, ENTRY_EVENT, and EXIT_EVENT
#include "ES_Configure.h"
#include "bot.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

// Functions to use in the HSM. Uses Bot functions in bot.c file.
// See bot.h for function documentation.

#define FullStop()      Bot_LeftMotorSpeed(0);Bot_RightMotorSpeed(0)
#define MoveStraight(x) Bot_LeftMotorSpeed(x); Bot_RightMotorSpeed(x)
#define BackStraight(x) Bot_LeftMotorSpeed(x*-1); Bot_RightMotorSpeed(x*-1)
#define SpinLeft(x)     Bot_LeftMotorSpeed(x*-1); Bot_RightMotorSpeed(x)
#define SpinRight(x)    Bot_LeftMotorSpeed(x); Bot_RightMotorSpeed(x*-1)

#define SolenoidOn()    Bot_ToggleSolenoid(1);
#define SolenoidOff()   Bot_ToggleSolenoid(0);
#define PitchingOn(x)   Bot_PitchingSpeed(x);
#define Turret(x)       Bot_Turret_Position(x);

// compass variables
#define UNDETERMINED 0
#define WEST 1
#define EAST 2

// BEACON MASK
#define BEACON_LEFT   0x4    // 100
#define BEACON_RIGHT  0x1    // 001
#define BEACON_CENTER 0x2    // 010

// tape sensor masks
#define TAPEMASK_LEFT   0xE0    // 1110_0000
#define TAPEMASK_RIGHT  0x1C    // 0001_1100
#define TAPEMASK_CENTER 0x02    // 0000_0010
#define TAPEMASK_ONTAPE 0x30    // 0011_0000
#define TAPEMASK_BACK   0x01    // 0000_0001

/*******************************************************************************
 * PUBLIC TYPEDEFS & VARIABLES                                                 *
 ******************************************************************************/

// typedefs for the states, which are used for the internal definition and also for
// the return of the query function. Be careful that all state names must be unique
// if they are exposed in the header file.

typedef enum {
    InitPState, FindBeacon, InitAngle, Moving, DetectTape, DetectBump, Stopped
} BotState_t;

extern uint8_t compass;
extern uint8_t bumpParam;
extern uint8_t beaconParam;
/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
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
uint8_t InitBotHSM(uint8_t Priority);

/**
 * @Function PostBotHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Returns TRUE if successful, FALSE otherwise
 * @author Gabriel H Elkaim, 2013.09.26 15:33 */
uint8_t PostBotHSM(ES_Event ThisEvent);

/**
 * @Function QueryBotSM(void)
 * @param none
 * @return Current state of the state machine
 * @brief This function is a wrapper to return the current state of the state
 *        machine. Return will match the ENUM above.
 * @author Gabriel H Elkaim, 2013.09.26 15:33 */
BotState_t QueryBotSM(void);

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
ES_Event RunBotHSM(ES_Event ThisEvent);

#endif /* ROACHFSM_H */

