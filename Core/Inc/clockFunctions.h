/*
 * clockFunctions.h
 *
 *  Created on: Aug 2, 2025
 *      Author: Allee
 */

#ifndef INC_CLOCKFUNCTIONS_H_
#define INC_CLOCKFUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void showStopwatchScreen(void);
void processStopwatchInput(void);

void showCountdownScreen(void);
void processCountdownInput(void);

void showClockScreen(void);
void processClock(void);

void showAlarmScreen(void);
void processAlarm(void);

#endif /* INC_CLOCKFUNCTIONS_H_ */
