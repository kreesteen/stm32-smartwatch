/*
 * haptics.h
 *
 *  Created on: Aug 4, 2025
 *      Author: Allee
 */

#ifndef INC_HAPTICS_H_
#define INC_HAPTICS_H_

#include "main.h"

// --- Motor Direction ---
typedef enum {
    MOTOR_CLOCKWISE,		// Motor direction is clockwise
    MOTOR_ANTICLOCKWISE		// Motor diretion is anticlockwise
} MotorDirection;

void motorInit(void);
void motorRotate(MotorDirection dir);

void buzzerSetFrequency(uint32_t freq_hz);
void buzzerHapticPulse(uint8_t pitch);

void buzzerLow(void);
void buzzerMedLow(void);
void buzzerMedHigh(void);
void buzzerHigh(void);

void processHaptics(void);
void showHapticsScreen(void);

void hapticResponse(MotorDirection direction, uint8_t pitch);

#endif /* INC_HAPTICS_H_ */
