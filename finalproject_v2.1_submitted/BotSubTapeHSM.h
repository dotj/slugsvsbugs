/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Gabriel H Elkaim
 *
 * File: BotSubTapeHSM.h
 *
 * C Header file for the Tape Sensor SubHSM
 ******************************************************************************/

#ifndef BOT_TAPE_SUBHSM
#define BOT_TAPE_SUBHSM

extern int TS;
extern int Tdir;
/*******************************************************************************
 * PUBLIC #INCLUDES                                                            *
 ******************************************************************************/

#include "ES_Configure.h"   // defines ES_Event, INIT_EVENT, ENTRY_EVENT, and EXIT_EVENT

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PUBLIC TYPEDEFS                                                             *
 ******************************************************************************/

// In case of repeated state names for different state machines, put the ENUM
// declaration inside the .c file. If all names are unique, then it can be put
// here in the .h, and you can add a query function as well.

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
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
uint8_t InitBotSubTapeHSM(void);


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
ES_Event RunBotSubTapeHSM(ES_Event ThisEvent);

#endif /* BOT_TAPE_SUBHSM */