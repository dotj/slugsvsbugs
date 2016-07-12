/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers
 *
 * File: BotFrameworkEvents.c
 *
 * Source file for the Framework Events. Includes functions to pass events and
 * parameters to the framework to drive the HSM.
 *
 * Notes:
 * - CheckLightLevel() function kept as example of passing an event with an AD
 * pin value.
 ******************************************************************************/

#include <BOARD.h>
#include "BotFrameworkEvents.h"
#include "ES_ServiceHeaders.h"
#include "ES_Events.h"
#include "bot.h"

// Beacon parameters
#define BEACON_DETECTED     1
#define BEACON_NOTDETECTED  0

//#define DARK_THRESHOLD 234
//#define LIGHT_THRESHOLD 88

/**
 * @Function CheckBeacon(void)
 * @param None.
 * @return eventDetected: TRUE or FALSE.
 * @brief Checks the beacon to see if an event has occured.
 * @note Also sets BEACONCHANGE event type, param: 0 or 1
 * @author */
uint8_t CheckBeacon(void) {
    static uint8_t lastBeaconState = 0;
    uint8_t currentBeaconState = 0;
    uint8_t eventDetected = FALSE;

    currentBeaconState = Bot_CheckBeacon();

    if(currentBeaconState != lastBeaconState) {
        ES_Event ThisEvent;
        ThisEvent.EventType = BEACONCHANGE;
        ThisEvent.EventParam = currentBeaconState;
        PostBotHSM(ThisEvent);
        eventDetected = TRUE;
    }

    lastBeaconState = currentBeaconState;
    return eventDetected;
}

/**
 * @Function CheckTape(void)
 * @param None.
 * @return eventDetected: TRUE or FALSE.
 * @brief Checks the tape sensors to see if an event has occured.
 * @note Also sets TAPECHANGE event type, param: 6-bit val
 * @author */
uint8_t CheckTape(void) {
    static uint8_t lastTapeState = 0;
    uint8_t currentTapeState = 0;
    uint8_t eventDetected = FALSE;
    uint8_t i = 0;

    currentTapeState = Bot_ReadTape();

    if( currentTapeState!=lastTapeState ) {
//        for(i=0; i<1000; i++);
//        currentTapeState = Bot_ReadTape();
        if( currentTapeState!=lastTapeState ) {
            ES_Event ThisEvent;
            ThisEvent.EventType = TAPECHANGE;
            ThisEvent.EventParam = currentTapeState;
            PostBotHSM(ThisEvent);
            eventDetected = TRUE;
        }
    }

    lastTapeState = currentTapeState;
    return eventDetected;
}


/**
 * @Function CheckBumps(void)
 * @param None.
 * @return eventDetected: TRUE or FALSE.
 * @brief Checks the bump sensors to see if an event has occured.
 * @note Also sets BUMPCHANGE event type, param: 10-bit val
 * @author */
uint8_t CheckBump(void) {
    static uint16_t lastBumpState = 0;
    static uint8_t count = 0;
    static uint8_t bumpcount = 0;
    uint16_t currentBumpState = 0;
    uint8_t eventDetected = FALSE;
    uint16_t i;

    currentBumpState = Bot_CheckBump();

    if( (currentBumpState!=lastBumpState) && (currentBumpState!=0) ) {
        for(i=0; i< 1000; i++);
        currentBumpState = Bot_CheckBump();
        if( (currentBumpState!=lastBumpState) && (currentBumpState!=0) ) {
            ES_Event ThisEvent;
            ThisEvent.EventType = BUMPCHANGE;
            ThisEvent.EventParam = currentBumpState;
            PostBotHSM(ThisEvent);
            eventDetected = TRUE;
        }
    }

    lastBumpState = currentBumpState;
    return eventDetected;
}


/**
 * @Function CheckLightLevel(void)
 * @param None.
 * @return None.
 * @brief example of AD input with hysteresis
 * @note
 * @author ? */
/*uint8_t CheckLightLevel(void) {
    static lightstate_t lastLightState = LIGHT;     // keeps val between calls
    lightstate_t currentLightState = lastLightState;
    unsigned int currentLightValue;
    uint8_t returnVal = FALSE;
    // check the light level and assign LIGHT or DARK
    currentLightValue = Bot_LightLevel();
    if (currentLightValue > DARK_THRESHOLD) {
        currentLightState = DARK;   // change light state, #defined in header
    }
    if (currentLightValue < LIGHT_THRESHOLD) {
        currentLightState = LIGHT;
    }
    if (currentLightState != lastLightState) { //event detected
        ES_Event ThisEvent;
        ThisEvent.EventType = LIGHTLEVEL;
        ThisEvent.EventParam = (uint16_t) currentLightState;
        PostBotHSM(ThisEvent);
        returnVal = TRUE;
    }
    lastLightState = currentLightState;
    return returnVal;
}*/