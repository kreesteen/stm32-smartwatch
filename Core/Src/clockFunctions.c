///*
// * clockFunctions.c
// *
// *  Created on: Aug 2, 2025
// *      Author: Allee
// */
//
#include "screen.h"
#include "lcd.h"
#include "main.h"
//#include "haptics.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "main.h"
#include "clockFunctions.h"
#include <stdio.h>

extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim6;

/**
 * @brief Plain 24h time container
 */
typedef struct {
    uint8_t hours;   // 0-23
    uint8_t minutes; // 0-59
    uint8_t seconds; // 0-59
} ClockTime;

/**
 * @brief UI state for blinking/editing which time field
 * 		  In edit modes, the selected field blinks at 1 Hz on the LCD
 */
typedef enum {
    CLOCK_VIEW = 0, // Normal view, no blinking
    CLOCK_EDIT_H,	// Editing hours, hours blinking
    CLOCK_EDIT_M,	// Editing minutes, minutes blinking
    CLOCK_EDIT_S	// Editing seconds, seconds blinking
} ClockUIState;

/**
 * @brief Create display render of a clock hh:mm:ss
 *		  This function renders hh:mm:ss on the second row (0xC0)
 *		  When an edit state is active, the selected field blinks every 500ms
 * @param clockType      The type of clock to display e.g stopwatch, countdown, etc...
 * @param clockStateType One of the ClockUIStates e..g CLOCK_VIEW, etc..
 * @retval None
 */
void createDisplay(ClockTime clockType, ClockUIState clockStateType) {
	char buf[17];
    uint32_t now = HAL_GetTick();
    uint8_t blinkOn = ((now / 500) % 2) == 0;

    switch (clockStateType) {
        case CLOCK_EDIT_H:
        	if (blinkOn) {
        		snprintf(buf, sizeof buf, "%02u:%02u:%02u", clockType.hours, clockType.minutes, clockType.seconds);
        	} else {
        		snprintf(buf, sizeof buf, "  :%02u:%02u", clockType.minutes, clockType.seconds);
        	}
        	break;
        case CLOCK_EDIT_M:
        	if (blinkOn) {
        		snprintf(buf, sizeof buf, "%02u:%02u:%02u", clockType.hours, clockType.minutes, clockType.seconds);
        	} else {
        		snprintf(buf, sizeof buf, "%02u:  :%02u", clockType.hours, clockType.seconds);
        	}
            break;
        case CLOCK_EDIT_S:
        	if (blinkOn) {
        		snprintf(buf, sizeof buf, "%02u:%02u:%02u", clockType.hours, clockType.minutes, clockType.seconds);
        	} else {
        		snprintf(buf, sizeof buf, "%02u:%02u:  ", clockType.hours, clockType.minutes);
        	}
        	break;
        default:
            snprintf(buf, sizeof buf, "%02u:%02u:%02u", clockType.hours, clockType.minutes, clockType.seconds);
            break;
    }

    LCD_SendCmd(0xC0);
    LCD_SendStr(buf);
}

// ------------------------------------------ Stopwatch Functions --------------------------------------------------

// --- Stopwatch Global Variables ---
static uint8_t swRunning = 0;
ClockTime sw = {0, 0, 0};

/**
 * @brief Initialize stopwatch state and start TIM7 1 Hz interrupt
 * 		  This function sets stopwatch time to 00:00:00 and clears run state
 *        and starts TIM7 in interrupt mode; the module also supports RTC-based ticking
 * @post swRunning = 0; sw = 00:00:00
 * @retval None
 */
void stopwatchInit(void) {
	HAL_TIM_Base_Start_IT(&htim6);
	swRunning = 0;
	sw.seconds = 0;
	sw.minutes = 0;
	sw.hours = 0;
}

/**
 * @brief Increment stopwatch by one second with rollover (hh:mm:ss, 24h)
 * @retval None
 */
void stopwatchUpdate(void) {
    sw.seconds++;
    if (sw.seconds >= 60) {
        sw.seconds = 0;
        sw.minutes++;
        if (sw.minutes >= 60) {
            sw.minutes = 0;
            sw.hours = (sw.hours + 1) % 24;
        }
    }
}

/**
 * @brief Per-second stopwatch tick using RTC edge detection
 * 		  This function reads RTC time and, if seconds changed from the last sample
 * 		  if changed, updates the stopwatch time
 * @retval None
 */
void stopwatchTimer() {
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;
    static uint8_t lastSec = 255;

    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

    if (swRunning) {
    	if (t.Seconds != lastSec) {
            stopwatchUpdate();
        }
    }
}

/**
 * @brief Show stopwatch time on the second line (HH:MM:SS)
 */
void stopwatchDisplay(void) {
	char buf[17];
    sprintf(buf, "%02d:%02d:%02d", sw.hours, sw.minutes, sw.seconds);
    LCD_SendCmd(0xC0);
    LCD_SendStr(buf);
}

/**
 * @brief Handle single-button input for the stopwatch
 *		  On a short press (<500ms) it toggles start / pause
 *		  On a long press (>500ms) it resets stopwatch
 * @retval None
 */
void stopwatchInput(void) {
	uint32_t now = HAL_GetTick();

	static uint32_t pressStartTime = 0;
	static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
	GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

	if (lastButtonState == GPIO_PIN_RESET && currentButtonState == GPIO_PIN_SET) {
		pressStartTime = now;
	} else if (lastButtonState == GPIO_PIN_SET && currentButtonState == GPIO_PIN_RESET) {
		uint32_t pressDuration = now - pressStartTime;

		// on a short press
		if (pressDuration < 500) {
			swRunning ^= 1;

		//on a long press
		} else {
			swRunning = 0;
			stopwatchInit();
		}
	}

	lastButtonState = currentButtonState;
}

/**
 * @brief Intro screen for stopwatch mode
 * @retval None
 */
void showStopwatchScreen(void) {
    stopwatchInit();
    LCD_SendCmd(0x01);
    LCD_SendStr("Stopwatch Mode");
    HAL_Delay(1000);
    LCD_SendCmd(0x01);
    LCD_SendStr("   Stopwatch   ");
}

/**
 * @brief Processes the stopwatch start / pause and reset
 *		  This function starts and pauses the stopwatch on a SW4 short tap
 *		  and resets the stopwatch on a long press (> 500ms)
 * @retval None
 */
void processStopwatchInput(void) {
    stopwatchTimer();
    stopwatchDisplay();
    stopwatchInput();
}

// ------------------------------------------ Countdown Functions --------------------------------------------------

// --- Countdown Global Variables ---
static uint8_t cdRunning = 0;
static uint8_t cdState = 0; 		// 0 = pause 1 = start
static uint8_t cdSetting = 0; 		// 0 = view, 1 = edit
static ClockUIState cdClockState = CLOCK_VIEW;

ClockTime cd = {0, 1, 30};

/**
 * @brief Initialize countdown to default 00:01:30 and clear run state
 * @retval None
 */
void countdownInit(void) {
	cdRunning = 0;
	cdState = 0;
	cd.seconds = 30;
	cd.minutes = 1;
	cd.hours = 0;
}

/**
 * @brief Decrement countdown by one second with proper borrow between fields
 * 		  Rolls seconds→minutes→hours. Stops at 00:00:00 (no underflow below zero)
 * @retval None
 */
void countdownUpdate(void) {
	cd.seconds--;
	if (cd.seconds == 0) {
		cd.seconds = 59;
		if (cd.minutes == 0) {
			cd.minutes = 59;
			if (cd.hours > 0) cd.hours--;
		} else {
			cd.minutes--;
		}
	}
}

/**
 * @brief Per-second countdown tick using RTC second-edge detection
 *		  If seconds changed since the last sample and countdown is running,
 *		  increments countdown clock
 * @retval None
 */
void countdownTimer() {
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;
    static uint8_t lastSec = 255;

    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

    if (t.Seconds != lastSec) {
    	lastSec = t.Seconds;
        if (cdRunning) {
            countdownUpdate();
        }
    }
}

/**
 * @brief Detect countdown completion and trigger haptic notification.
 *		  When H:M:S reaches 00:00:00:
 *     	  	- Stops countdown, clears state flags.
 * 			- Calls @ref hapticResponse() with a preset pattern.
 * @retval None
 */
void countdownOver(void) {
	if (cd.seconds == 0 && cd.minutes == 0 && cd.hours == 0) {
		cdRunning = 0;
		cdState = 0;
//		hapticResponse(1, 4);
	}
}

/**
 * @brief Render countdown on LCD; in edit mode, blink selected field
 * @retval None
 */
void countdownDisplay(void) {
    char buf[17];

    if (cdSetting) {
    	createDisplay(cd, cdClockState);
    } else {
        snprintf(buf, sizeof buf, "%02u:%02u:%02u", cd.hours, cd.minutes, cd.seconds);
		LCD_SendCmd(0xC0);
		LCD_SendStr(buf);
    }
}

/**
 * @brief Handle single-button UI for countdown
 * 		  Short press (<500 ms):
 *     		- If not started: toggle run/pause.
 *     		- If editing: increment selected field (H/M/S) with wrap.
 *     		- Else: toggle internal start/pause state.
 * 		  Long press (≥500 ms):
 *     		- Stop and reset run state.
 *     		- Toggle edit mode on first long press, then cycle fields
 *       	  (H → M → S → back to VIEW) on subsequent long presses.
 * @retval None
 */
void countdownInput(void) {
	uint32_t now = HAL_GetTick();

	static uint32_t pressStartTime = 0;
	static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
	GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

	if (lastButtonState == GPIO_PIN_RESET && currentButtonState == GPIO_PIN_SET) {
		pressStartTime = now;
	} else if (lastButtonState == GPIO_PIN_SET && currentButtonState == GPIO_PIN_RESET) {
		uint32_t pressDuration = now - pressStartTime;

		// on a short press
		if (pressDuration < 500) {
			if (!cdState) {
				cdRunning ^= 1;
			}

			if (cdSetting == 1) {
				if (cdClockState == CLOCK_EDIT_H) {
					cd.hours = (cd.hours + 1) % 24;
				} else if (cdClockState == CLOCK_EDIT_M) {
					cd.minutes = (cd.minutes + 1) % 60;
				} else if (cdClockState == CLOCK_EDIT_S) {
					cd.seconds = (cd.seconds + 1) % 60;
				}
			} else {
				cdState ^= 1;
			}

		//on a long press
		} else {
			cdRunning = 0;
			cdState = 0;

			if (cdClockState == 0) {
				cdSetting ^= 1;
			}
			if (cdClockState < 3) {
				cdClockState++;
			} else {
				cdClockState = 0;
				cdSetting = 0;
			}

		}
	}

	lastButtonState = currentButtonState;
}

/**
 * @brief Intro screen for countdown mode, then title.
 * @retval None
 */
void showCountdownScreen(void) {
	countdownInit();
    LCD_SendCmd(0x01);
    LCD_SendStr("Countdown Mode");
    HAL_Delay(1000);
    LCD_SendCmd(0x01);
    LCD_SendStr("   Countdown   ");
}

/**
 * @brief Processes countdown
 * 		  Processes when countdown reaches the end, the countdown timer, SW4 input
 * 		  and display
 * @retval None
 */
void processCountdownInput(void) {
    countdownOver();
	countdownTimer();
    countdownInput();
    countdownDisplay();
}

// ------------------------------------------ Clock Functions --------------------------------------------------

// --- Clock Global Variables ---
static uint8_t clkRunning = 0;
static uint8_t clkState = 0; 		// 0 = pause 1 = start
static uint8_t clkSetting = 0; 		// 0 = view, 1 = edit

static ClockUIState clkClockState = CLOCK_VIEW;

ClockTime clk = {0, 0, 0};

/**
 * @brief Read RTC time into the global @ref clk instance.
 * @retval None
 */
void clockUpdate(void) {
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;
    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);
    clk.hours   = t.Hours;
    clk.minutes = t.Minutes;
    clk.seconds = t.Seconds;
}

/**
 * @brief Initialize clock UI state and load current RTC time
 * 		 Clears edit mode and sets initial UI state to CLOCK_VIEW
 * @retval None
 */
void clockInit(void) {
	clockUpdate();
    clkSetting = 0;
    clkClockState = 0;
}

/**
 * @brief Render the clock: in view mode, live RTC; in edit mode, blink field
 * @retval None
 */
void clockDisplay(void) {
	char buf[17];
    if (clkSetting) {
    	createDisplay(clk, clkClockState);
    } else {
    	clockUpdate();
        snprintf(buf, sizeof buf, "%02u:%02u:%02u", clk.hours, clk.minutes, clk.seconds);
		LCD_SendCmd(0xC0);
		LCD_SendStr(buf);
    }
}

/**
 * @brief Handle single-button UI for the clock, including RTC writes
 * 		  Short press (<500 ms):
 *     		- If not started: toggle @ref clkRunning (no effect on RTC reading)
 *     		- If in edit mode: increment selected field (H/M/S) and write back to RTC
 * 		  Long press (≥500 ms):
 *     		- Stop any “running” state.
 *     		- Toggle edit mode on first long press, then cycle fields H→M→S→VIEW
 * @retval None
 */
void clockInput(void) {
	uint32_t now = HAL_GetTick();
	static uint32_t pressStartTime = 0;
	static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
	GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

	if (lastButtonState == GPIO_PIN_RESET && currentButtonState == GPIO_PIN_SET) {
		pressStartTime = now;
	} else if (lastButtonState == GPIO_PIN_SET && currentButtonState == GPIO_PIN_RESET) {
		uint32_t pressDuration = now - pressStartTime;

		// on a short press
		if (pressDuration < 500) {
			if (!clkState) {
				clkRunning ^= 1;
			}
			clkState ^= 1;

			if (clkSetting == 1) {
				if (clkClockState == CLOCK_EDIT_H) {
					clk.hours = (clk.hours + 1) % 24;
				} else if (clkClockState == CLOCK_EDIT_M) {
					clk.minutes = (clk.minutes + 1) % 60;
				} else if (clkClockState == CLOCK_EDIT_S) {
					clk.seconds = (clk.seconds + 1) % 60;
				}

			    RTC_TimeTypeDef newClk = {0};
			    newClk.Hours   = clk.hours;
			    newClk.Minutes = clk.minutes;
			    newClk.Seconds = clk.seconds;
			    HAL_RTC_SetTime(&hrtc, &newClk, RTC_FORMAT_BIN);
			}

		//on a long press
		} else {
			clkRunning = 0;
			clkState = 0;

			if (clkClockState == 0) {
				clkSetting ^= 1;
			}
			if (clkClockState < 3) {
				clkClockState++;
			} else {
				clkClockState = 0;
				clkSetting = 0;
			}
		}
	}

	lastButtonState = currentButtonState;
}

/**
 * @brief Intro screen for clock mode
 * @retval None
 */
void showClockScreen(void) {
	clockInit();
    LCD_SendCmd(0x01);
    LCD_SendStr("     Clock     ");
    HAL_Delay(1000);
}

/**
 * @brief Processes clock
 * 		  Processes the clock timer, SW4 input and display
 * @retval None
 */
void processClock(void) {
    clockInput();
    clockDisplay();
}

// ------------------------------------------ Alarm Functions --------------------------------------------------

// --- Alarm Global Variables ---
ClockTime al = {0, 1, 0};
static uint8_t alSetting = 0; 		// 0 = view, 1 = edit
static int alarmTrigger = 0;
static uint8_t snoozeSetting = 0; 	//0 = snooze 1 = stop
static uint8_t alarmOn = 0; 		//0 = off 1 = on
static uint8_t alarmSet = 0;
static ClockUIState alClockState = CLOCK_VIEW;

/**
 * @brief Initialize alarm UI state and (if unset) default alarm time
 *		  If @ref alarmSet == 0, initializes alarm to RTC time + 1 minute, seconds = 0
 * 		  Resets UI flags for edit vs. snooze/stop selection
 * @retval None
 */
void alarmInit(void) {
	snoozeSetting = 0;
	alarmOn = 1;
	alSetting = 0;

	if (!alarmSet) {
		RTC_TimeTypeDef t; RTC_DateTypeDef d;
		HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

		al.hours   = t.Hours;
		al.minutes = t.Minutes + 1;
		al.seconds = 0;
	}
}

/**
 * @brief Render alarm screen
 * 		  If alarmTrigger is active: show "Alarm" on line 1 and
 *   	  a selector between "SNOOZE" and "STOP" on line 2 (arrow on current choice).
 * 		  Otherwise: show "Alarm ON/OFF" on line 1 and the alarm time on line 2,
 *   	  with field blinking when editing
 * @retval None
 */
void alarmDisplay(void) {
    char buf[17];

	if (alarmTrigger) {
	    LCD_SendCmd(0x80);
	    LCD_SendStr("     Alarm      ");
	    if (snoozeSetting) {
			snprintf(buf, sizeof buf, "  SNOOZE >STOP");
		} else {
			snprintf(buf, sizeof buf, " >SNOOZE  STOP");
		}
	    LCD_SendCmd(0xC0);
	    LCD_SendStr(buf);
	} else {
	    LCD_SendCmd(0x80);
	    if (alarmOn) {
			LCD_SendStr("Alarm      ON ");
	    } else {
			LCD_SendStr("Alarm      OFF");
	    }
	    createDisplay(al, alClockState);
    }
}

/**
 * @brief Handle alarm button input, including snooze/stop interactions
 *		 When alarm is ringing (@ref alarmTrigger):
 *   		- Short press: if "SNOOZE" selected, add 5 minutes and re-arm; if "STOP", disable alarm
 *   		- Long press: toggle selector between SNOOZE and STOP
 *
 * 		 When alarm is not ringing:
 *   		- Short press:
 *       		- If in edit mode: increment selected field (H/M/S) with wrap
 *       		- Else: toggle @ref alarmOn ON/OFF
 *   		- Long press:
 *       		- Toggle edit mode on first long press, then cycle fields H→M→S→VIEW
 *       		- On exiting the cycle to VIEW, sets @ref alarmSet = 1
 *  @retval None
 */
void alarmInput(void) {
	uint32_t now = HAL_GetTick();

	static uint32_t pressStartTime = 0;
	static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
	GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

	if (lastButtonState == GPIO_PIN_RESET && currentButtonState == GPIO_PIN_SET) {
		pressStartTime = now;
	} else if (lastButtonState == GPIO_PIN_SET && currentButtonState == GPIO_PIN_RESET) {
		uint32_t pressDuration = now - pressStartTime;

		if (alarmTrigger) {
			// on a short press
			if (pressDuration < 500) {

				// snooze alarm for 5 minutes
				if (snoozeSetting) {
					al.minutes = al.minutes + 5;
					alarmInit();
				// stop alarm
				} else {
					alarmInit();
					alarmTrigger = 0;
					alarmOn = 0;
					alarmSet = 0;
				}

			//on a long press
			} else {
				snoozeSetting ^= 1;
			}

			lastButtonState = currentButtonState;
			return;
		}

		// on a short press
		if (pressDuration < 500) {
			if (alSetting == 1) {
				if (alClockState == CLOCK_EDIT_H) {
					al.hours = (al.hours + 1) % 24;
				} else if (alClockState == CLOCK_EDIT_M) {
					al.minutes = (al.minutes + 1) % 60;
				} else if (alClockState == CLOCK_EDIT_S) {
					al.seconds = (al.seconds + 1) % 60;
				}
			} else {
				alarmOn ^= 1;
			}

		//on a long press
		} else {
			if (alClockState == 0) {
				alSetting ^= 1;
			}

			if (alClockState < 3) {
				alClockState++;
			} else {
				alClockState = 0;
				alSetting = 0;
				alarmSet = 1;
			}

		}
	}

	lastButtonState = currentButtonState;
}

/**
 * @brief Check current time vs. alarm; trigger if equal (once per second)
 * 		  Compares @ref clk (updated elsewhere) to @ref al. If equal:
 *   		- Sets @ref alarmTrigger = 1 and @ref snoozeSetting = 0
 *   		- Plays a haptic pattern three times
 * @retval None
 */
void checkAlarm(void) {
    static uint8_t lastSecond = 255;
    uint8_t currentSecond = clk.seconds;

    if (alarmOn && !alarmTrigger && currentSecond != lastSecond) {
        lastSecond = currentSecond;

        if (clk.hours == al.hours &&
            clk.minutes == al.minutes &&
            clk.seconds == al.seconds) {
            alarmTrigger = 1;
            snoozeSetting = 0;

//            hapticResponse(1, 4);
//            hapticResponse(1, 4);
//            hapticResponse(1, 4);
        }
    }
}

/**
 * @brief Intro screen for alarm mod, then clears
 * @retval None
 */
void showAlarmScreen(void) {
	alarmInit();
    LCD_SendCmd(0x01);
    LCD_SendStr("Alarm Mode");
    HAL_Delay(1000);
    LCD_SendCmd(0x01);
}

/**
 * @brief Processes alarm
 * 		  Processes when clock reaches the end, the alarm timer, SW4 input
 * 		  and display
 * @retval None
 */
void processAlarm(void) {
	clockUpdate();
	checkAlarm();
    alarmInput();
    alarmDisplay();
}
