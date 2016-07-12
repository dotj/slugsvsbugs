             /******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Max Dunne
 *
 * File: bot.h
 *
 * C Header file for bot functions.
 ******************************************************************************/

#ifndef BOT_H
#define BOT_H
/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/
#define BUMPER_TRIPPED 1
#define BUMPER_NOT_TRIPPED 0

#define BEACON_DETECTED 1
#define BEACON_NOT_DETECTED 0


/**
 * @Function Bot_Init(void)
 * @param None.
 * @return None.
 * @brief Performs all the initialization necessary for the bot.
 * Includes PWM module, A/D converter, data directions on some pins, and
 * the initial motor directions.
 * @note 
 * @author Max Dunne, 2012.01.06 */
void Bot_Init(void);

/**
 * @Function Bot_BatteryVoltage(void)
 * @param None.
 * @return a 10-bit value corresponding to the current voltage of the roach
 * @brief  returns a 10:1 scaled value of the roach battery level
 * @author Max Dunne, 2013.07.12 */
unsigned int Bot_BatteryVoltage(void);


/**
 * @Function Bot_CheckBeacon
 * @param None
 * @return 1 if beacon is detected, 0 otherwise
 * @author */
char Bot_CheckBeacon(void);

/**
 * @Function Bot_CheckLBeacon
 * @param None
 * @return 1 if beacon Left is detected, 0 otherwise
 * @author */
char Bot_CheckLBeacon(void);

/**
 * @Function Bot_CheckRBeacon
 * @param None
 * @return 1 if beacon Right is detected, 0 otherwise
 * @author */
char Bot_CheckRBeacon(void);

/**
 * @Function Bot_ReadTape(void)
 * @param None.
 * @return 6-bit value representing all six tape sensors in following order:
 * @return Far L, Center L, Center R, Far R, Center, Back
 * @author */
char Bot_ReadTape(void);

/**
 * @Function Bot_CheckBumps(void)
 * @param None.
 * @return 8-bit value representing all 8 bump sensors in the following order:
 *    We might not need 8 bits.
 * @author */
char Bot_CheckBumps(void);

/**
 * @Function Bot_LeftMtrSpeed(char newSpeed)
 * @param newSpeed - A value between -10 and 10 which is the new speed
 * @param of the motor. 0 stops the motor. A negative value is reverse.
 * @return SUCCESS or ERROR
 * @brief Set the speed and direction of the left motor.
 * @author Max Dunne, 2012.01.06 */
char Bot_LeftMotorSpeed(char newSpeed);

/**
 * @Function Bot_RightMtrSpeed(char newSpeed)
 * @param newSpeed - A value between -10 and 10 which is the new speed
 * @param of the motor. 0 stops the motor. A negative value is reverse.
 * @return SUCCESS or ERROR
 * @brief Set the speed and direction of the right motor.
 * @author Max Dunne, 2012.01.06 */
char Bot_RightMotorSpeed(char newSpeed);

char Bot_Turret_Position(unsigned short int newposition);
/**
 * @Function Bot_OnSolenoid
 * @brief Turn solenoid pin or off
 * @return SUCCESS or ERROR
 */
char Bot_ToggleSolenoid(char x);

#endif
