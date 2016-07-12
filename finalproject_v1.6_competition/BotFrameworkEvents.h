/******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers
 *
 * File: BotFrameworkEvents.h
 *
 * Header File for the Framework Events.
 ******************************************************************************/

#ifndef BOTFRAMEWORKEVENTS_H
#define	BOTFRAMEWORKEVENTS_H

//typedef enum {
//    DARK, LIGHT
//} lightstate_t;

/**
 * @Function CheckBeacon(void)
 * @param None.
 * @return eventDetected: TRUE or FALSE.
 * @brief Checks the beacon to see if an event has occured.
 * @note Also sets BEACONCHANGE event type, param: 0 or 1
 * @author */
uint8_t CheckBeacon(void);

uint8_t CheckRBeacon(void);

uint8_t CheckLBeacon(void);


//// run servo
//
//uint8_t CheckServo(void);



/**
 * @Function CheckTape(void)
 * @param None.
 * @return eventDetected: TRUE or FALSE.
 * @brief Checks the tape sensors to see if an event has occured.
 * @note Also sets TAPECHANGE event type, param: 6-bit val
 * @author */
uint8_t CheckTape(void);

/**
 * @Function CheckBumps(void)
 * @param None.
 * @return eventDetected: TRUE or FALSE.
 * @brief Checks the bump sensors to see if an event has occured.
 * @note Also sets BUMPCHANGE event type, param: 10-bit val
 * @author */
uint8_t CheckBump(void);

#endif	/* BOTFRAMEWORKEVENTS_H */