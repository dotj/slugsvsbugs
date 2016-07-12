 /******************************************************************************
 * Final Project, CMPE 118/218, Fall 2013
 * Authors: The Debuggers, Max Dunne
 *
 * File: bot.c
 *
 * C Source file for bot. Includes bot functions and pin defines.
 * Also includes test module without HSM to test functions.
 ******************************************************************************/

#include <bot.h>
#include <xc.h>
#include <pwm.h>
#include <serial.h>
#include <AD.h>
#include <BOARD.h>
#include <IO_Ports.h>

#include "timers.h"


/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/


// analog pins
//#define LIGHT_SENSOR    AD_PORTV4  //  delete later
//#define BATTERY_VOLTAGE AD_PORTV3

#define DELAY(x)    for (wait = 0; wait <= x; wait++) {asm("nop");}

#define A_BIT       4000
#define A_LOT       800000

// Beacon Detector input (non analog)
#define BEACON_DETECTOR_TRIS PORTZ03_TRIS // PORTZ03_BIT
#define BEACON_DETECTOR      PORTZ03_BIT // PORTZ03_BIT

// Tape Sensors input, can be used as AD pin if needed
#define TAPE_SENSOR_L1  AD_PORTW5
#define TAPE_SENSOR_L2  AD_PORTW4
#define TAPE_SENSOR_L3  AD_PORTW6
#define TAPE_SENSOR_R1  AD_PORTW7
#define TAPE_SENSOR_R2  AD_PORTV3
#define TAPE_SENSOR_R3  AD_PORTW8   //W3
#define TAPE_SENSOR_CT  AD_PORTV4
#define TAPE_SENSOR_BK  AD_PORTW3   //W8

// Bump Sensors input (non analog)
#define BUMP_SENSOR_01_TRIS PORTV05_TRIS        // FRONT LEFT
#define BUMP_SENSOR_01      PORTV05_BIT
#define BUMP_SENSOR_02_TRIS PORTV06_TRIS        // FRONT RIGHT
#define BUMP_SENSOR_02      PORTV06_BIT

// Motor outputs
#define LEFT_DIR_TRIS   PORTY09_TRIS
#define RIGHT_DIR_TRIS  PORTY11_TRIS
#define LEFT_DIR        PORTY09_LAT
#define RIGHT_DIR       PORTY11_LAT

#define LEFT_PWM        PWM_PORTY10
#define RIGHT_PWM       PWM_PORTY12

// Pitching motors and solenoid
#define SOLENOID_PIN_TRIS   PORTZ04_TRIS
#define SOLENOID_PIN        PORTZ04_LAT

#define PITCHING_PWM    PWM_PORTZ06

#define TAPE_THRESHOLD 800

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function Bot_Init(void)
 * @param None.
 * @return None.
 * @brief Performs all the initialization necessary for the bot.
 * Includes PWM module, A/D converter, data directions on some pins, and
 * the initial motor directions.
 * @note
 * @author Max Dunne, 2012.01.06 */
void Bot_Init(void) {

    // Initialize motor control
    PWM_Init();
    PWM_SetFrequency(500);
    PWM_AddPins(LEFT_PWM | RIGHT_PWM);
    PWM_AddPins(PITCHING_PWM);

    LEFT_DIR_TRIS = 0;      // TRIS 0: set as output
    RIGHT_DIR_TRIS = 0;
    LEFT_DIR = 0;
    RIGHT_DIR = 0;

    SOLENOID_PIN_TRIS = 0;
    SOLENOID_PIN = 0;

    // Initialize battery AD pin
    AD_Init();

    AD_AddPins(TAPE_SENSOR_R3 | TAPE_SENSOR_R2 | TAPE_SENSOR_R1 |
               TAPE_SENSOR_L3 | TAPE_SENSOR_L2 | TAPE_SENSOR_L1 |
               TAPE_SENSOR_CT | TAPE_SENSOR_BK);

    // Initialize sensor inputs
    BEACON_DETECTOR_TRIS = 1;   // 1: set as input


    IO_PortsSetPortInputs(PORTV, PIN5|PIN12|PIN7|PIN8);
    IO_PortsSetPortInputs(PORTZ, PIN7|PIN8|PIN9|PIN11);

}

/**
 * @Function Bot_BatteryVoltage(void)
 * @param None.
 * @return a 10-bit value corresponding to the current voltage of the roach
 * @brief  returns a 10:1 scaled value of the roach battery level
 * @author Max Dunne, 2013.07.12 */
/*
unsigned int Bot_BatteryVoltage(void) {
    return AD_ReadADPin(BATTERY_VOLTAGE);
}
*/
/**
 * @Function Bot_CheckBeacon
 * @param None
 * @return 1 if beacon is detected, 0 otherwise
 * @author
 */
char Bot_CheckBeacon(void) {
    return !BEACON_DETECTOR;
}

/**
 * @Function Bot_ReadTape(void)
 * @param None.
 * @return 6-bit value representing all six tape sensors in following order:
 * @return Far L, Center L, Center R, Far R, Center, Back
 * @author */
char Bot_ReadTape(void) {
    char L3 = 0;    // bit 7
    char L2 = 0;    // bit 6
    char L1 = 0;    // bit 5
    char R1 = 0;    // bit 4
    char R2 = 0;    // bit 3
    char R3 = 0;    // bit 2
    char CT = 0;    // bit 1
    char BK = 0;    // bit 0

    if(AD_ReadADPin(TAPE_SENSOR_L3) < TAPE_THRESHOLD) L3 = 1;
    if(AD_ReadADPin(TAPE_SENSOR_L2) < TAPE_THRESHOLD) L2 = 1;
    if(AD_ReadADPin(TAPE_SENSOR_L1) < TAPE_THRESHOLD) L1 = 1;
    if(AD_ReadADPin(TAPE_SENSOR_R3) < TAPE_THRESHOLD) R3 = 1;
    if(AD_ReadADPin(TAPE_SENSOR_R2) < TAPE_THRESHOLD) R2 = 1;
    if(AD_ReadADPin(TAPE_SENSOR_R1) < TAPE_THRESHOLD) R1 = 1;
    //if(AD_ReadADPin(TAPE_SENSOR_CT) < TAPE_THRESHOLD) CT = 1;
    //if(AD_ReadADPin(TAPE_SENSOR_BK) < TAPE_THRESHOLD) BK = 1; // not using back

    return ( BK+(CT<<1)+(R3<<2)+(R2<<3)+(R1<<4)+(L1<<5)+(L2<<6)+(L3<<7) );
}

/**
 * @Function Bot_CheckBumps(void)
 * @param None.
 * @return 10-bit value representing all 10 bump sensors in the following order:
 *    ???
 * @author */
char Bot_CheckBump(void) {
//    return ((BUMP_SENSOR_01 << 0) +
//            (BUMP_SENSOR_02 << 1) +
//            (BUMP_SENSOR_03 << 2) +
//            (BUMP_SENSOR_04 << 3) +
//            (BUMP_SENSOR_05 << 4) +
//            (BUMP_SENSOR_06 << 5) +
//            (BUMP_SENSOR_07 << 6) +
//            (BUMP_SENSOR_08 << 7));

    return ((BUMP_SENSOR_01 << 0) +
            (BUMP_SENSOR_02 << 1));
}

/**
 * @Function Bot_LeftMtrSpeed(char newSpeed)
 * @param newSpeed - A value between -10 and 10 which is the new speed
 * @param of the motor. 0 stops the motor. A negative value is reverse.
 * @return SUCCESS or ERROR
 * @brief Set the speed and direction of the left motor.
 * @author Max Dunne, 2012.01.06 */
char Bot_LeftMotorSpeed(char newSpeed) {
    if ((newSpeed < -10) || (newSpeed > 10)) {
        return (ERROR);
    }
    if (newSpeed < 0) {
        LEFT_DIR = 0;
        //LEFT_DIR = 1;
        newSpeed = (newSpeed * (-1)); // set speed to a positive value
    } else {
        LEFT_DIR = 1;
        //LEFT_DIR = 0;
    }
    if (PWM_SetDutyCycle(LEFT_PWM, newSpeed * 97) == ERROR) {
        printf("ERROR: setting channel 1 speed!\n");
        return (ERROR);
    }
    return (SUCCESS);
}

/**
 * @Function Bot_RightMtrSpeed(char newSpeed)
 * @param newSpeed - A value between -10 and 10 which is the new speed
 * @param of the motor. 0 stops the motor. A negative value is reverse.
 * @return SUCCESS or ERROR
 * @brief Set the speed and direction of the right motor.
 * @author Max Dunne, 2012.01.06 */
char Bot_RightMotorSpeed(char newSpeed) {
    if ((newSpeed < -10) || (newSpeed > 10)) {
        return (ERROR);
    }
    if (newSpeed < 0) {
        RIGHT_DIR = 0;
        newSpeed = (newSpeed * (-1)); // set speed to a positive value
    } else {
        RIGHT_DIR = 1;
    }
    if (PWM_SetDutyCycle(RIGHT_PWM, newSpeed * 100) == ERROR) {
        printf("ERROR: setting channel 1 speed!\n");
        return (ERROR);
    }
    return (SUCCESS);
}

/**
 * @Function Bot_PitchingSpeed(char newSpeed)
 * @param newSpeed - A value between 0 and 10 which is the new speed
 * @param of the motor. 0 stops the motor.
 * @return SUCCESS or ERROR
 * @brief Set the speed of the pitching motors
 * @author Max Dunne, 2012.01.06 */
char Bot_PitchingSpeed(char newSpeed) {
    if (PWM_SetDutyCycle(PITCHING_PWM, newSpeed * 100) == ERROR) {
        printf("ERROR: setting pitching speed!\n");
        return (ERROR);
    }
    return (SUCCESS);
}

/**
 * @Function Bot_OffSolenoid
 * @brief Turn solenoid pin on
 * @return SUCCESS or ERROR
 */
char Bot_ToggleSolenoid(char x) {
    if(x==1) {
        SOLENOID_PIN = 1;
    } else SOLENOID_PIN = 0;

    return SUCCESS;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/


/*--- TEST MODULE ------------------------------------------------------------*/

//#define BOT_TEST  // Uncomment to enable test module for bot functions
#ifdef BOT_TEST
#pragma config FPLLIDIV 	= DIV_2		//PLL Input Divider
#pragma config FPLLMUL 		= MUL_20	//PLL Multiplier
#pragma config FPLLODIV 	= DIV_1 	//System PLL Output Clock Divid
#pragma config FNOSC 		= PRIPLL	//Oscillator Selection Bits
#pragma config FSOSCEN 		= OFF		//Secondary Oscillator Enable
#pragma config IESO 		= OFF		//Internal/External Switch O
#pragma config POSCMOD 		= XT		//Primary Oscillator Configuration
#pragma config OSCIOFNC 	= OFF		//CLKO Output Signal Active on the OSCO Pin
#pragma config FPBDIV 		= DIV_2		//Peripheral Clock Divisor
#pragma config FCKSM 		= CSECMD	//Clock Switching and Monitor Selection
#pragma config WDTPS 		= PS1		//Watchdog Timer Postscaler
#pragma config FWDTEN		= OFF		//Watchdog Timer Enable
#pragma config ICESEL		= ICS_PGx2	//ICE/ICD Comm Channel Select
#pragma config PWP 		= OFF		//Program Flash Write Protect
#pragma config BWP 		= OFF		//Boot Flash Write Protect bit
#pragma config CP 		= OFF		//Code Protect

#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
//#include "ES_Timers.h"
#include "LED.h"

/* Function Prototypes for test module */
void printbinary(int blah);

/* main loop for test module */
void main(void) {
    ES_Return_t ErrorType;

    // Hardware Initializations
    BOARD_Init();
    Bot_Init();

    printf("Bot Testing Module.\n");

    unsigned int wait = 0;
    unsigned int bump = 0;
    unsigned int prevbump = 0;
    unsigned int x = 0;

//    Bot_RightMotorSpeed(5);
//    Bot_LeftMotorSpeed(5);
//    Bot_PitchingSpeed(5);

    while(1) {

//        Bot_ToggleSolenoid(1);
//        DELAY(A_LOT);
//        Bot_ToggleSolenoid(0);
//        DELAY(A_LOT);

//        printf("Battery: %4d, Beacon: %d, Tape: 0x%X, Bump: 0x%X\n",
//               Bot_BatteryVoltage(), Bot_CheckBeacon(), Bot_ReadTape(),
//               Bot_CheckBump());

        printf("%4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d\n",
                AD_ReadADPin(TAPE_SENSOR_R3), AD_ReadADPin(TAPE_SENSOR_R2),
                AD_ReadADPin(TAPE_SENSOR_R1), AD_ReadADPin(TAPE_SENSOR_L1),
                AD_ReadADPin(TAPE_SENSOR_L2), AD_ReadADPin(TAPE_SENSOR_L3),
                AD_ReadADPin(TAPE_SENSOR_CT), AD_ReadADPin(TAPE_SENSOR_BK));

//        printf("Beacon: %d, ", Bot_CheckBeacon());

//        if((x%2) == 1) {
//            Bot_RightMotorSpeed(-9);
//            Bot_LeftMotorSpeed(9);
//        } else {
//            Bot_RightMotorSpeed(9);
//            Bot_LeftMotorSpeed(-9);
//        }
//        x++;

//        bump = Bot_CheckBump();
//        if(bump != prevbump) {
//            prevbump = bump;
//            printf("Bump change: ");
//            printbinary(Bot_CheckBump());
//            printf("\n");
//        }

        printf("Bump: ");
        printbinary(Bot_CheckBump());

        printf(", Tape: ");
        printbinary(Bot_ReadTape());

        printf("\n");

        DELAY(A_LOT);


        //while(!IsTransmitEmpty()); // blocking code!
    }
}

void printbinary(int blah) {
    static char b[9];
    b[0] = '\0';
    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((blah & z) == z) ? "1" : "0");
    }
    printf("%s",b);
}


#endif