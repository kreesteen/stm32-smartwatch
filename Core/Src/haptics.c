/*
 * haptics.c
 *
 *  Created on: Aug 4, 2025
 *      Author: Allee
 */

#include "main.h"
#include "haptics.h"
#include "time.h"
#include "lcd.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

//  --- Global Setting Variables ---
uint8_t motorEnabled = 1;  // motor response ON by default
uint8_t buzzerEnabled = 1; // buzzer response ON by default

// --- Motor Direction ---
//typedef enum {
//    MOTOR_CLOCKWISE,		// Motor direction is clockwise
//    MOTOR_ANTICLOCKWISE		// Motor diretion is anticlockwise
//} MotorDirection;

// Step sequence for full step motor
// stepSequence[i][j]
// where i = step in the full stepping sequence and j is the respective motor coil
const uint8_t stepSequence[4][4] = {
    {1, 0, 0, 1},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1}
};

// --- Global Constants ---
#define STEPS_PER_ROTATION 1024
#define STEP_DELAY_MS 1
#define LONG_PRESS 500


// ------------------------------------------ Vibration Functions (Motor) --------------------------------------------------

/**
 * @brief  Initalises the motors
 * 		   This function initalises all pins of the motor to off
 * @retval None
 */
void motorInit(void) {
    HAL_GPIO_WritePin(COIL_A_GPIO_Port, COIL_A_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COIL_B_GPIO_Port, COIL_B_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COIL_C_GPIO_Port, COIL_C_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COIL_D_GPIO_Port, COIL_D_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  Initalises the Haptics Settings Screen
 * 		   This function powers the stepper motor coils for a specific step
 * 		   in the full stepping sequence
 * @param  stepIndex
 * 		   indicates which step in the full stepping sequence the motor is in
 * @retval None
 */
void motorStep(uint8_t stepIndex) {
    HAL_GPIO_WritePin(COIL_A_GPIO_Port, COIL_A_Pin, stepSequence[stepIndex][0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COIL_B_GPIO_Port, COIL_B_Pin, stepSequence[stepIndex][1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COIL_C_GPIO_Port, COIL_C_Pin, stepSequence[stepIndex][2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(COIL_D_GPIO_Port, COIL_D_Pin, stepSequence[stepIndex][3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief  Rotates motor depending on desired direction
 * 		   This function rotates the motor in either a clockwise or anticlockwise
 * 		   direction
 * @param  direction
 * 		   indications desired direction of motor
 * @retval None
 */
void motorRotate(MotorDirection direction) {
	if (!motorEnabled) {
		return;
	}

    static uint8_t step = 0;

    for (int i = 0; i < STEPS_PER_ROTATION; i++) {
        if (direction == MOTOR_CLOCKWISE) {
            step = (step + 1) % 4;
        } else {
            step = (step + 3) % 4;
        }

        motorStep(step);
        HAL_Delay(STEP_DELAY_MS);
    }

    motorInit();
}

// ------------------------------------------ Buzzer Functions (Sound)  --------------------------------------------------

/**
 * @brief Play a single haptic tone at one of four preset pitches.
 *
 * @param pitch  Discrete pitch selector:
 *               - 1: Low (~130 Hz)
 *               - 2: Med-Low (~260 Hz)
 *               - 3: Med-High (~390 Hz)
 *               - 4: High (~520 Hz)
 * @retval None
 */
void buzzerHapticPulse(uint8_t pitch) {
	if (!buzzerEnabled) {
		return;
	}

	if (pitch == 1) {
		buzzerLow();
	} else if (pitch == 2) {
		buzzerMedLow();
	} else if (pitch == 3) {
		buzzerMedHigh();
	} else if (pitch == 4) {
		buzzerHigh();
	}
}

/**
 * @brief  Plays a ~130 Hz tone for 500 ms, then silence.
 * @retval None
 */
void buzzerLow(void) {
	if (!buzzerEnabled) {
		return;
	}

	//~130 Hz tone
	TIM1->CCR3=376;
	TIM1->ARR=552;
	HAL_Delay(500);

	// silence
	TIM1->CCR3=0;
}

/**
 * @brief  Plays a ~260 Hz tone for 500 ms, then silence.
 * @retval None
 */
void buzzerMedLow(void) {
	if (!buzzerEnabled) {
		return;
	}

	//~260 Hz tone
	TIM1->CCR3=188;
	TIM1->ARR=276;
	HAL_Delay(500);

	// silence
	TIM1->CCR3=0;
}

/**
 * @brief  Plays a ~390 Hz tone for 500 ms, then silence.
 * @retval None
 */
void buzzerMedHigh(void) {
	if (!buzzerEnabled) {
		return;
	}

     //~390 Hz tone
    TIM1->CCR3 = 92;
    TIM1->ARR  = 184;
    HAL_Delay(500);

    // silence
    TIM1->CCR3 = 0;
}

/**
 * @brief  Plays a ~520 Hz tone for 500 ms, then silence.
 * @retval None
 */
void buzzerHigh(void) {
	if (!buzzerEnabled) {
		return;
	}

    //~520 Hz tone
	TIM1->CCR3=94;
	TIM1->ARR=138;
	HAL_Delay(500);

	// silence
	TIM1->CCR3=0;
}

// ------------------------------------------ Haptic Action --------------------------------------------------


void hapticResponse(MotorDirection direction, uint8_t pitch) {
	motorRotate(direction);
	buzzerHapticPulse(pitch);
}

// ------------------------------------------ Haptics Settings Screen --------------------------------------------------

// --- Global Variables for Haptics Settings Screen Feature ---
uint8_t hapticToggle[2] = {1, 1}; // By default, motor and buzzers are ON
int8_t selectedHapticSettings = 0; // Motor settings selected by default

/**
 * @brief  Initalises the Haptics Settings Screen
 * 		   This function presents the Haptics Settings start screen
 * @retval None
 */
void showHapticsScreen(void) {
    LCD_SendCmd(0x01);
    LCD_SendStr("Haptics Settings");
}

/**
 * @brief  Processes the Haptics Settings Screen
 * 		   This function processes the Haptics Settings
 * 		   On SW4 tap, the selected haptic setting toggles ON / OFF
 * 		   On SW4 long press, the selected setting alternates between VIBRATION
 * 		   (motor) and SOUND (buzzer)
 * @retval None
 */
void processHaptics(void) {
    LCD_SendCmd(0x01);

    char line1[17];
    char line2[17];
    uint32_t now = HAL_GetTick();

    static int8_t lastSelected = 0;
    static uint32_t pressStartTime = 0;
    static int lastDisplayed[2] = {-1, -1};

    static GPIO_PinState lastButtonState = GPIO_PIN_SET;
    GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

    if (lastButtonState == GPIO_PIN_SET && currentButtonState == GPIO_PIN_RESET) {
        pressStartTime = now;
    } else if (lastButtonState == GPIO_PIN_RESET && currentButtonState == GPIO_PIN_SET) {
        uint32_t pressDuration = now - pressStartTime;

            // On a short press, toggle haptics enabled for selected setting ON / OFF
        if (pressDuration < LONG_PRESS) {
            hapticToggle[selectedHapticSettings] ^= 1;
            motorEnabled = hapticToggle[0];
            buzzerEnabled = hapticToggle[1];

        	// On a long press, switch between haptic settings; motor and buzzer
        } else {
        	selectedHapticSettings ^= 1;
        }
    }

    lastButtonState = currentButtonState;

    // If selected haptics switch or is toggled, update LCD display
    if (hapticToggle[0] != lastDisplayed[0] ||
    	hapticToggle[1] != lastDisplayed[1] ||
		lastSelected != selectedHapticSettings) {
        if (selectedHapticSettings == 0) {
            sprintf(line1, ">Vibration %s", hapticToggle[0] ? "ON " : "OFF");
            sprintf(line2, " Sound     %s", hapticToggle[1] ? "ON " : "OFF");
        } else {
            sprintf(line1, " Vibration %s", hapticToggle[0] ? "ON " : "OFF");
            sprintf(line2, ">Sound     %s", hapticToggle[1] ? "ON " : "OFF");
        }

        LCD_SendCmd(0x80);
        LCD_SendStr(line1);
        LCD_SendCmd(0xC0);
        LCD_SendStr(line2);

        //Update toggle and selected settings
        lastDisplayed[0] = hapticToggle[0];
        lastDisplayed[1] = hapticToggle[1];
        lastSelected = selectedHapticSettings;
    }
}


