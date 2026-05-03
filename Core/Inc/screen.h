#ifndef INC_SCREEN_H_
#define INC_SCREEN_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// This struct defines what a "screen" is.
// It holds pointers to its own unique functions.
typedef struct {
    void (*init)(void);      // Pointer to the initialization function
    void (*process)(void);   // Pointer to the continuous processing function
} Screen_t;

extern int isHomeScreenActive;
extern uint8_t degreeSymbol[8];

// --- Placeholder function ---
void processNothing(void);


extern void initHomeScreen(void);
void showHomeScreen(void);
void processHomeScreen(void);
void resetHomeScreenActiveFlag(void);
void showScreen1(void);
void showScreen2(void);
void showScreen3(void);
void showLEDBrightnessScreen(void);
void processLEDBrightnessInput(void);
void showScreen5(void);
void showStepCounterScreen(void);
void processStepCounterInput(void);
void showSleepTrackerScreen(void);
void processSleepTrackerInput(void);
void showHeartRateScreen(void);
void processHeartRateInput(void);
void createDegreeSymbol(void);
void showWeatherScreen(void);
void processWeatherInput(void);
void showFallDemoScreen(void);
void processFallDemoInput(void);
void showGameReactionScreen(void);
void updateReactionGame(void);
void processGameReactionScreen(void);
void showSolarChargingScreen(void);
void processSolarChargingScreen(void);
void showExerciseScreen(void);
void processExerciseInput(void);
void processLight(void);
void runStepCounter(void);
void showCoinFlipScreen(void);
void processCoinFlip(void);
void showMagicBallScreen(void);
void processMagicBall(void);
void showScoreTrackerScreen(void);
void processScoreTracker(void);




#endif /* INC_SCREEN_H_ */
