#include "screen.h"
#include "lcd.h"
#include "main.h"

#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAX_LOCATION_INDEX 3 		// Max number of locations
#define BRIGHTNESS_LEVELS 5 		// PWM duty cycle levels for the flashlight brightness
#define INACTIVITY_PERIOD_MS 5000   // Time in ms to wait before considering user 'asleep'
#define MOVEMENT_THRESHOLD   5      // Number of movements required to register as 'waking up'
#define WAKE_UP_WINDOW_MS    3000   // The time window in ms for the above movements to occur


// Weather data structure
typedef struct {
    const char* location;
    const char* temperature;
    const char* condition;
} WeatherData;

// Locations and weather info
WeatherData weatherLocations[MAX_LOCATION_INDEX + 1] = {
		{"Albury", "17", "Sunny"},
		{"Cairns", "26", "Cloudy"},
		{"Perth",  "17", "Rainy"},
		{"Sydney", "17", "Cloudy"},
};

// Custom Character for weather display (Degree for temperature)
uint8_t degreeSymbol[8] = {
    0b00110,
    0b01001,
    0b01001,
    0b00110,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};


static struct tm currentTime; // Tracking the current time

// Reaction Game States
typedef enum {
    REACTION_STATE_READY,
    REACTION_STATE_WAITING,
    REACTION_STATE_GO,
    REACTION_STATE_RESULT,
    REACTION_STATE_TOO_EARLY
} ReactionGameState_t;

// -------------------------------------------- Global Variables --------------------------------------------

// Step counter variables
static int stepCount = 0;
static int lastDisplayedCount = -1;

// Heart Rate Variables
static int heartRate = 78; // Initial Heart rate (Hard-coded)
static int lastDisplayedHeartRate = -1;

// Variable for current location
static uint8_t currentLocationIndex = 0;

int isHomeScreenActive = 0;

// FOR SLEEP TRACKER STATE
// These keep track of the state only for this screen
static uint32_t lastMovementTime;
static uint32_t sleepStartTime;
static bool isSleeping;

// New state variables for the improved logic
static int movementCount;
static uint32_t firstMovementTime;

// For Flashlight Function
uint8_t brightnessLevels[BRIGHTNESS_LEVELS] = {0, 10, 50, 80, 100};
uint8_t currentBrightnessIndex = 0;

// Static variables for the reaction game
static ReactionGameState_t reactionState = REACTION_STATE_READY;
static uint32_t gameStartTime = 0;
static uint32_t reactionTime = 0;
static uint32_t waitStartTime = 0;
static uint32_t randomDelay = 0;

// ---------------------------------------------------- Screen Functions ---------------------------------------------------
void showScreen0(void) {
    LCD_SendCmd(0x01);     			// Clear display
    LCD_SendStr("Screen 0");      	// Show 0
}

void showScreen1(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01);
    LCD_SendStr("Screen 1");
}

void showScreen2(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01);
    LCD_SendStr("Screen 2");
}

void showScreen3(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01);
    LCD_SendStr("Screen 3");
}

void showScreen4(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01);
    LCD_SendStr("Screen 4");
}

void showScreen5(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01);
    LCD_SendStr("Screen 5");
}

void showScreen6(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01);
    LCD_SendStr("Screen 6");
}

void showScreen7(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01);
    LCD_SendStr("Screen 7");
}

// ------------------------------------------------------------------------------------------------------------------------------

void initHomeScreen(void) {
	//Setting an initial time (00:00:00, 14 Aug 2025)
	static bool timeInitialized = false;
	if (!timeInitialized) {
	    currentTime.tm_hour = 00;
	    currentTime.tm_min = 00;
        currentTime.tm_sec = 0;
        currentTime.tm_mday = 14;
	    currentTime.tm_mon = 8;
        currentTime.tm_year = 2025;
        currentTime.tm_wday = 4; // Thursday
        timeInitialized = true;
    }
}

/**
  * @brief:  Displays the home screen on the LCD.
  *          The home screen shows the current day, date, and time.
  *          The date has been hard-coded as "THU 14/08/2025" and time
  *          is taken from the currentTime structure.
  * @retval: None
**/
void showHomeScreen(void) {
    LCD_SendCmd(0x01);
    isHomeScreenActive = true;

    // Display day and date
    char line1[17];
    snprintf(line1, sizeof(line1), "THU   14/08/2025");
    LCD_SendCmd(0x80);
    LCD_SendStr(line1);

    // Display time
    char line2[17];
    snprintf(line2, sizeof(line2), "    %02d:%02d:%02d    ",
             currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec);
    LCD_SendCmd(0xC0);
    LCD_SendStr(line2);
}

/**
  * @brief:  Processes the home screen by updating the time every second.
  *          The function increments the seconds, minutes, and hours in
  *          the currentTime structure and updates the displayed time on the LCD.
  * @note:   The time is updated every 1 second based on the
  *          HAL_GetTick() function.
  * @retval: None
**/
void processHomeScreen(void) {
    static uint32_t lastUpdate = 0;
    uint32_t now = HAL_GetTick();

    if (!isHomeScreenActive) return;  // Only happens when home screen is active

    if (now - lastUpdate >= 1000) {  // Keeps updating the time every second
        currentTime.tm_sec++;
        if (currentTime.tm_sec >= 60) {
            currentTime.tm_sec = 0;
            currentTime.tm_min++;
            if (currentTime.tm_min >= 60) {
                currentTime.tm_min = 0;
                currentTime.tm_hour++;
                if (currentTime.tm_hour >= 24) {
                    currentTime.tm_hour = 0;
                }
            }
        }

        // Updates only the time on the home screen
        char timeStr[9];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
                currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec);

        LCD_SendCmd(0xC0 + 4);
        LCD_SendStr(timeStr);

        lastUpdate = now;
    }
}

/**
  * @brief:  Resets the home screen.
  * @note:   This function is used to deactivate the home screen when needed.
  * @retval: None
**/
void resetHomeScreenActiveFlag(void) {
    isHomeScreenActive = false;
}

// ------------------------------------------------------------------------------------------------------------------------------

// This function is called ONCE to draw the initial screen
void showStepCounterScreen(void) {
	isHomeScreenActive = false;
    lastDisplayedCount = -1;

    LCD_SendCmd(0x01); // Clear display

    // Display the current, persistent step count
    char buf[17];
    snprintf(buf, 17, "Steps: %d", stepCount);
    LCD_SendStr(buf);
}

// This function is called REPEATEDLY in the main loop
void processStepCounterInput(void) {
    static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
    GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

    if (currentButtonState == GPIO_PIN_SET && lastButtonState == GPIO_PIN_RESET) {
        HAL_Delay(20); // optional debounce
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
            stepCount++;
        }
    }

    lastButtonState = currentButtonState;

    if (stepCount != lastDisplayedCount) {
        LCD_SendCmd(0x01);
        char buf[17];
        snprintf(buf, 17, "Steps: %d", stepCount);
        LCD_SendStr(buf);
        lastDisplayedCount = stepCount;
    }
}

// ------------------------------------------------------------------------------------------------------------------------------
/**
 * @brief  Initializes the Sleep Tracker screen.
 */
void showSleepTrackerScreen(void) {
	isHomeScreenActive = false;
    LCD_SendCmd(0x01); // Clear LCD
    LCD_GoTo(0, 0);
    LCD_SendStr("Tracking sleep..");

    // Reset all state variables
    lastMovementTime = HAL_GetTick();
    sleepStartTime = 0;
    isSleeping = false;
    movementCount = 0;
    firstMovementTime = 0;
}


/**
 * @brief  Processes input and logic for the Sleep Tracker screen.
 */
void processSleepTrackerInput(void) {
    uint32_t now = HAL_GetTick();
    bool movementDetected = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET);

    if (isSleeping) {
        // --- LOGIC FOR WHEN THE USER IS ASLEEP ---

        // A. MOVEMENT is detected
        if (movementDetected) {
            movementCount++;

            if (movementCount == 1) {
                // This is the first movement in a potential wake-up sequence.
                // Start the timer for the wake-up window.
                firstMovementTime = now;
            }

            // Check if enough movements have occurred to wake the user up
            if (movementCount >= MOVEMENT_THRESHOLD) {
                // WOKE UP!
                uint32_t sleepDurationSec = (now - sleepStartTime) / 1000;

                // Format the time into hours, minutes, and seconds
                int hours = sleepDurationSec / 3600;
                int minutes = (sleepDurationSec % 3600) / 60;
                int seconds = sleepDurationSec % 60;

                char buf[17];
                LCD_SendCmd(0x01);
                LCD_GoTo(0, 0);
                LCD_SendStr("You woke up!");
                HAL_Delay(1500); // Show message briefly

                // Display the formatted total sleep time
                LCD_SendCmd(0x01);
                LCD_GoTo(0, 0);
                LCD_SendStr("Slept for:");
                LCD_GoTo(1, 0);
                snprintf(buf, 17, " %dh %02dm %02ds", hours, minutes, seconds);
                LCD_SendStr(buf);

                // Wait for a button press before restarting the tracker
                while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET); // wait for release
                while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET); // wait for press

                // Re-initialize the screen to start over
                showSleepTrackerScreen();
                return; // Exit function to prevent further processing this cycle
            }
             // Debounce by waiting for button release
            while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET);
        }

        // B. NO MOVEMENT is detected
        else {
            // If the user twitched but didn't fully wake up within the window,
            // reset the counter. It was just a false alarm.
            if (movementCount > 0 && (now - firstMovementTime > WAKE_UP_WINDOW_MS)) {
                movementCount = 0; // Reset the counter
            }
        }

    } else {
        // --- LOGIC FOR WHEN THE USER IS AWAKE ---

        // A. If there's movement, just update the last known movement time.
        if (movementDetected) {
            lastMovementTime = now;
            // Debounce by waiting for button release
            while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET);
        }

        // B. If there has been no movement for the inactivity period, fall asleep.
        if (now - lastMovementTime > INACTIVITY_PERIOD_MS) {
            isSleeping = true;
            sleepStartTime = now;
            movementCount = 0; // Ensure movement count is zero when sleep starts

            LCD_SendCmd(0x01);
            LCD_GoTo(0, 0);
            LCD_SendStr("Sleeping... zZz");
        }
    }
}

// ------------------------------------------------------------------------------------------------------------------------------

// A dummy function for static screens that don't need continuous updates.
void processNothing(void) {
    // This function intentionally does nothing.
}

void processLight(void) {
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == 1) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_10);
	}
}

/**
  * @brief:  Displays the heart rate screen on the LCD.
  *          The screen shows the label "Heart Rate" and the current heart rate.
  *          The heart rate value is displayed in beats per minute i.e.BPM.
  * @retval: None
**/
void showHeartRateScreen(void) {
    LCD_SendCmd(0x01);
    LCD_SendCmd(0x80);
    LCD_SendStr("   Heart Rate   ");
    char buf[16];
    snprintf(buf, sizeof(buf), "    %d  BPM    ", heartRate);

    LCD_SendCmd(0xC0);
    LCD_SendStr(buf);

    lastDisplayedHeartRate = heartRate;
}

/**
  * @brief:  Processes the heart rate input and simulates heart rate increase.
  *         The heart rate is incremented by 1 every time the SW4 is pressed.
  * @retval: None
**/
void processHeartRateInput(void) {
    static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
    GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

    if (currentButtonState == GPIO_PIN_SET && lastButtonState == GPIO_PIN_RESET) {
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
            heartRate++;
        }
    }

    lastButtonState = currentButtonState;

    // If the heart rate value has changed, update the display
    if (heartRate != lastDisplayedHeartRate) {
        char buf[17];
        snprintf(buf, sizeof(buf), "    %d  BPM    ", heartRate);
        LCD_SendCmd(0xC0);
        LCD_SendStr(buf);  // Displays the updated heart rate
        lastDisplayedHeartRate = heartRate;
    }
}

/**
  * @brief:  Creates the degree symbol for temperature (Celsius) display on the LCD.
  * @retval: None
**/
void createDegreeSymbol(void) {
	LCD_SendCmd(0x40 + (0 * 8));
	for (int i = 0; i < 8; i++) {
	    LCD_SendData(degreeSymbol[i]);
	}
}

/**
  * @brief:  Displays the weather screen, including location, temperature, and weather condition.
  *          The temperature is displayed with the degree symbol and the condition is displayed
  *          under the temperature.
  * @retval: None
**/
void showWeatherScreen(void) {
	isHomeScreenActive = false;

	createDegreeSymbol();
    LCD_SendCmd(0x01);

    // Display location on the first line
    LCD_SendCmd(0x80);
    LCD_SendStr((char*)weatherLocations[currentLocationIndex].location);

    // Calculate position for temperature
    char tempString[8];
    snprintf(tempString, sizeof(tempString), "%s", weatherLocations[currentLocationIndex].temperature);
    int tempLength = strlen(tempString) + 2; // For degree symbol
    int tempPosition = 16 - tempLength;

    // Temperature position
    LCD_SendCmd(0x80 + tempPosition);
    LCD_SendStr(tempString);
    LCD_SendData(0);
    LCD_SendStr("C");

    // Weather condition under temperature
    int conditionLen = strlen(weatherLocations[currentLocationIndex].condition);
    int conditionPos = 16 - conditionLen;
    LCD_SendCmd(0xC0 + conditionPos);
    LCD_SendStr((char*)weatherLocations[currentLocationIndex].condition);
}

/**
  * @brief:  Processes the weather location input and switches between weather locations.
  *          The function changes through different locations and updates the weather screen.
  * @retval: None
**/
void processWeatherInput(void) {
    static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
    GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);  // Assuming SW4 is connected to PC1

    // Checks for button press
    if (currentButtonState == GPIO_PIN_SET && lastButtonState == GPIO_PIN_RESET) {
        HAL_Delay(200);  // Debounce delay
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
            // Changes through weather locations
            currentLocationIndex++;
            if (currentLocationIndex > MAX_LOCATION_INDEX) {
                currentLocationIndex = 0;  // Gets back to the first location
            }
            showWeatherScreen();  // Update
            while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
                HAL_Delay(10);
            }
        }
    }

    lastButtonState = currentButtonState;
}

// ------------------------------------------ Fall Detection  --------------------------------------------------

// States for our fall detection logic
typedef enum {
    FALL_STATE_IDLE,
    FALL_STATE_COUNTDOWN,
    FALL_STATE_ALERTED
} FallAlertState_t;

// Static variables to manage the fall detection state and timing
static FallAlertState_t fallState = FALL_STATE_IDLE;
static uint32_t alertStartTime = 0;
static const int countdownDuration = 15; // Countdown from 15 seconds

/**
 * @brief  Initializes the fall detection simulation screen.
 * @retval None
 */
void showFallDemoScreen(void) {
	isHomeScreenActive = false;
    fallState = FALL_STATE_IDLE; // Reset state when entering the screen
    LCD_SendCmd(0x01);           // Clear LCD
    LCD_SendStr("Fall Sim Ready");
    LCD_GoTo(1, 0);
    LCD_SendStr("Press to test"); // Instruct user how to trigger the fall
}

/**
 * @brief  Processes inputs and state changes for the fall detection simulation.
 * @retval None
 */
void processFallDemoInput(void) {
    switch (fallState) {
        case FALL_STATE_IDLE:
            // In the idle state, we wait for the user to press PC1 to simulate a fall.
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
                HAL_Delay(50); // Simple debounce
                if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
                    // --- A FALL IS DETECTED ---
                    fallState = FALL_STATE_COUNTDOWN;
                    alertStartTime = HAL_GetTick(); // Start the countdown timer

                    // Display alert message and turn on LED to simulate alarm/vibration
                    LCD_SendCmd(0x01);
                    LCD_SendStr("FALL DETECTED!");
                    LCD_GoTo(1, 0);
                    LCD_SendStr("Press to cancel");
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
                }
            }
            break;

        case FALL_STATE_COUNTDOWN: {
            // --- ALERT IS ACTIVE, COUNTING DOWN ---

            // Check if the user cancels the alert with PC1 (the same button)
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET); // Turn off alarm LED
                showFallDemoScreen(); // Reset the screen and state
                HAL_Delay(250); // Add delay to prevent immediate re-trigger
                return; // Exit to prevent further processing this cycle
            }

            // Calculate how many seconds are left
            uint32_t elapsed_ms = HAL_GetTick() - alertStartTime;
            int remainingSeconds = countdownDuration - (elapsed_ms / 1000);

            if (remainingSeconds >= 0) {
                // This check prevents flickering by only redrawing when the number changes.
                static int lastDisplayedSecond = -1;
                if (remainingSeconds != lastDisplayedSecond) {
                    lastDisplayedSecond = remainingSeconds;
                    char buf[17];
                    // Note: The primary instruction to cancel is now on the first screen.
                    // Here we just show the countdown.
                    snprintf(buf, 17, "Countdown: %ds  ", remainingSeconds);
                    LCD_GoTo(1, 0);
                    LCD_SendStr(buf);
                }
            } else {
                // --- COUNTDOWN FINISHED ---
                fallState = FALL_STATE_ALERTED;
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET); // Turn off alarm LED

                // Simulate sending the alert to emergency contacts
                LCD_SendCmd(0x01);
                LCD_SendStr("ALERTING HELP...");

                // Reuse alertStartTime to time how long this message is shown
                alertStartTime = HAL_GetTick();
            }
            break;
        }

        case FALL_STATE_ALERTED:
            // --- SHOW "ALERTED" MESSAGE FOR 4 SECONDS ---
            if (HAL_GetTick() - alertStartTime > 4000) {
                showFallDemoScreen(); // Reset to the initial state
            }
            break;
    }
}

/**
  * @brief:  Shows the current brightness level as a percentage.
  *          The function returns the brightness value based on the index from
  *          the 'brightnessLevels' array.
  * @retval: Current brightness percentage from the 'brightnessLevels' array.
**/
uint8_t getCurrentBrightnessPercent(void) {
    extern uint8_t brightnessLevels[];
    extern uint8_t currentBrightnessIndex;
    return brightnessLevels[currentBrightnessIndex];
}

/**
  * @brief:  Displays the LED brightness screen.
  * @retval: None
**/
void showLEDBrightnessScreen(void) {
    isHomeScreenActive = false;
    LCD_SendCmd(0x01);

    LCD_SendCmd(0x80);
    LCD_SendStr("   FLASHLIGHT   ");
}

/**
  * @brief:  Processes the input from SW4 to adjust the LED brightness level.
  *          The button goes through different brightness levels with debouncing.
  * @retval: None
**/
void processLEDBrightnessInput(void) {
    static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
    static uint32_t lastButtonTime = 0;
    uint32_t now = HAL_GetTick();
    GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1); // SW4

    // Debouncing
    if (now - lastButtonTime < 300) {
        return;
    }

    // Detecting button press
    if (currentButtonState == GPIO_PIN_SET && lastButtonState == GPIO_PIN_RESET) {
        lastButtonTime = now;

        // Cycles through the brightness levels
        currentBrightnessIndex++;
        if (currentBrightnessIndex >= BRIGHTNESS_LEVELS) {
            currentBrightnessIndex = 0;
        }

        // Updates LCD display
        char buf[20];
        if (brightnessLevels[currentBrightnessIndex] == 0) {
            snprintf(buf, sizeof(buf), "   TURNED OFF   ");
        } else {
            snprintf(buf, sizeof(buf), " Brightness:%d ", brightnessLevels[currentBrightnessIndex]);
        }
        LCD_SendCmd(0xC0);
        LCD_SendStr(buf);
    }

    lastButtonState = currentButtonState;
}

/**
 * @brief:  Initializes the reaction game screen.
 *          This function sets up the initial state of the reaction game, displays the title, and
 *          asks the user to press the button to start the game.
 * @retval: None
**/
void showGameReactionScreen(void) {
    isHomeScreenActive = false;
    reactionState = REACTION_STATE_READY;

    LCD_SendCmd(0x01);
    LCD_SendCmd(0x80);
    LCD_SendStr(" Reaction Game! ");
    LCD_SendCmd(0xC0);
    LCD_SendStr("Press SW4: Start");
}

/**
 * @brief:  Processes the reaction game logic and input.
 *          This function handles the different states of the game, such as waiting for the user
 *          to start, reacting to the "GO!", and displaying the result.
 * @retval: None
**/
void processGameReactionScreen(void) {
    static GPIO_PinState lastButtonState = GPIO_PIN_RESET;
    GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
    uint32_t now = HAL_GetTick();

    // Detect button press
    bool buttonPressed = false;
    if (currentButtonState == GPIO_PIN_SET && lastButtonState == GPIO_PIN_RESET) {
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
            buttonPressed = true;
            while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
                HAL_Delay(10);
            }
        }
    }
    lastButtonState = currentButtonState;

    switch (reactionState) {
        case REACTION_STATE_READY:
            // Waiting for user to press SW4 to start the game
            if (buttonPressed) {
                reactionState = REACTION_STATE_WAITING;
                waitStartTime = now;

                randomDelay = 1000 + (HAL_GetTick() % 4000);

                LCD_SendCmd(0x01);
                LCD_SendCmd(0x80);
                LCD_SendStr("  Get ready...  ");
                LCD_SendCmd(0xC0);
                LCD_SendStr("  Wait for GO!  ");
            }
            break;

        case REACTION_STATE_WAITING:
            // Checking if user pressed button too early
            if (buttonPressed) {
                reactionState = REACTION_STATE_TOO_EARLY;
                LCD_SendCmd(0x01);
                LCD_SendCmd(0x80);
                LCD_SendStr("   Too early!   ");
                LCD_SendCmd(0xC0);
                LCD_SendStr(" Press SW4 retry ");
            }
            // Checking if it is time to show "GO!"
            else if (now - waitStartTime >= randomDelay) {
                reactionState = REACTION_STATE_GO;
                gameStartTime = now;

                LCD_SendCmd(0x01);
                LCD_SendCmd(0x80);
                LCD_SendStr("      GO!!     ");
                LCD_SendCmd(0xC0);
                LCD_SendStr(" Press SW4 NOW! ");
            }
            break;

        case REACTION_STATE_GO:
            // Waiting for user to press SW4 and calculate reaction time
            if (buttonPressed) {
                reactionTime = now - gameStartTime;
                reactionState = REACTION_STATE_RESULT;

                char buf[17];
                LCD_SendCmd(0x01);
                LCD_SendCmd(0x80);
                LCD_SendStr("   Your time:   ");
                LCD_SendCmd(0xC0);
                snprintf(buf, sizeof(buf), "    %lu ms    ", reactionTime);
                LCD_SendStr(buf);

                // Displays result for 5 seconds, before restarting
                HAL_Delay(5000);

                LCD_SendCmd(0x01);
                LCD_SendCmd(0x80);
                if (reactionTime < 2000) {
                    LCD_SendStr("   Excellent!   ");
                } else if (reactionTime < 5000) {
                    LCD_SendStr("    Good job!   ");
                } else if (reactionTime < 10000) {
                    LCD_SendStr("   Not bad!  ");
                } else {
                    LCD_SendStr("  Play Again??   ");
                }
                LCD_SendCmd(0xC0);
                LCD_SendStr("Press SW4: Retry!");
            }
            break;

        case REACTION_STATE_RESULT:
        case REACTION_STATE_TOO_EARLY:
            // Waiting for user to press SW4 to restart
            if (buttonPressed) {
                showGameReactionScreen(); // Reset
            }
            break;
    }
}

// ------------------------------------------ Guided Chair Exercise --------------------------------------------------

// --- Data Structures ---

// Represents a single step in an exercise routine
typedef struct {
    const char* instruction; // Text to display for the exercise
    uint8_t duration_s;      // How long to perform the action in seconds
} ExerciseStep;


// --- State Management ---

typedef enum {
    EXERCISE_STATE_IDLE,     // Waiting for the user to start the routine
    EXERCISE_STATE_ACTIVE,   // An exercise is in progress
    EXERCISE_STATE_REST,     // A rest period is in progress
    EXERCISE_STATE_DONE      // The routine is complete
} ExerciseState_t;


// --- Global Variables for Exercise Feature ---

#define EXERCISE_ROUTINE_STEPS 3
#define REST_PERIOD_S 5 // 5 seconds of rest between exercises

static ExerciseStep routine[EXERCISE_ROUTINE_STEPS] = {
    {"Raise Arms", 10},
    {"Ankle Circles", 15},
    {"Seated March", 20}
};

static ExerciseState_t exerciseState = EXERCISE_STATE_IDLE;
static uint8_t currentStepIndex = 0;
static uint32_t stepStartTime = 0;


/**
 * @brief  Initializes the Guided Exercise screen.
 * @retval None
 */
void showExerciseScreen(void) {
    exerciseState = EXERCISE_STATE_IDLE;
    currentStepIndex = 0;

    LCD_SendCmd(0x01);
    LCD_SendStr("Morning Moves");
    LCD_GoTo(1, 0);
    LCD_SendStr("Press to begin");
}

/**
 * @brief  Processes logic and timers for the Guided Exercise screen.
 * @retval None
 */
void processExerciseInput(void) {
    uint32_t now = HAL_GetTick();
    char buf[17];

    switch (exerciseState) {
        case EXERCISE_STATE_IDLE:
            // Wait for a button press to start the routine
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET) {
                HAL_Delay(50); // Debounce
                currentStepIndex = 0;
                exerciseState = EXERCISE_STATE_ACTIVE;
                stepStartTime = now;

                // Display the first instruction
                LCD_SendCmd(0x01);
                LCD_SendStr((char*)routine[currentStepIndex].instruction);
                
                // Wait for button release
                while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET);
            }
            break;

        case EXERCISE_STATE_ACTIVE: {
            uint32_t elapsed_s = (now - stepStartTime) / 1000;
            int remaining_s = routine[currentStepIndex].duration_s - elapsed_s;

            // Update countdown timer on the screen
            snprintf(buf, 17, "Time: %ds   ", remaining_s);
            LCD_GoTo(1, 0);
            LCD_SendStr(buf);

            // Check if the exercise duration is over
            if (remaining_s <= 0) {
                // Move to the rest state
                exerciseState = EXERCISE_STATE_REST;
                stepStartTime = now;

                LCD_SendCmd(0x01);
                LCD_SendStr("Great! Now rest");
            }
            break;
        }

        case EXERCISE_STATE_REST: {
            uint32_t elapsed_s = (now - stepStartTime) / 1000;
            int remaining_s = REST_PERIOD_S - elapsed_s;

            // Update rest countdown on the screen
            snprintf(buf, 17, "Time: %ds   ", remaining_s);
            LCD_GoTo(1, 0);
            LCD_SendStr(buf);

            // Check if rest period is over
            if (remaining_s <= 0) {
                currentStepIndex++; // Move to the next exercise
                if (currentStepIndex >= EXERCISE_ROUTINE_STEPS) {
                    // Routine is finished
                    exerciseState = EXERCISE_STATE_DONE;
                    stepStartTime = now; // Use timer for "Done" message display time
                    LCD_SendCmd(0x01);
                    LCD_SendStr("Routine Complete");
                    LCD_GoTo(1,0);
                    LCD_SendStr("Well done! :)");
                } else {
                    // Start the next exercise
                    exerciseState = EXERCISE_STATE_ACTIVE;
                    stepStartTime = now;
                    LCD_SendCmd(0x01);
                    LCD_SendStr((char*)routine[currentStepIndex].instruction);
                }
            }
            break;
        }

        case EXERCISE_STATE_DONE:
            // After showing the completion message for 5 seconds, reset
            if (now - stepStartTime > 5000) {
                showExerciseScreen();
            }
            break;
    }
}

// ------------------------------------------ Coin Flip Game --------------------------------------------------

/**
 * @brief  Prints coin flip animation to the LCD screen
 * @retval None
 */
void showFlippingAnimation(void) {
    const char* coinFrames = "\\|/-";
    LCD_SendCmd(0x01);
    HAL_Delay(2);
    LCD_SendCmd(0x80);
    LCD_SendStr("Flipping...");

    for (int i = 0; i < 8; i++) {
        LCD_SendCmd(0xC0);
        char coinFlip[2] = { coinFrames[i % 4], '\0' };
        LCD_SendStr(coinFlip);
        HAL_Delay(150);
    }
    LCD_SendCmd(0x01);
}

/**
 * @brief  Initalises the Coin Flip game
 * 		   This function presents start screen
 * @retval None
 */
void showCoinFlipScreen(void) {
    LCD_SendCmd(0x01);
    LCD_SendStr("Heads or Tails?");
}

/**
 * @brief  Processes the Coin Flip game
 * 		   This function randomises the result and then presents
 * 		   a flipping animation and reveals the result
 * @retval None
 */
void processCoinFlip(void) {
    GPIO_PinState flip = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

    if (flip == GPIO_PIN_SET) {
    	int result = rand() % 2;
        HAL_Delay(2);

        showFlippingAnimation();

        LCD_SendCmd(0xC0);
        if (result == 0) {
            LCD_SendStr("Heads");
        } else {
            LCD_SendStr("Tails");
        }

//        LCD_SendCmd(0xC0);
//        LCD_SendStr("Heads or Tails?");
    }
}

// ------------------------------------------ Magic 8 Ball Game --------------------------------------------------

// Array storage for different Magic 8 Ball possible return responses
const char* high[]   = { "Yes", "Absolutely", "Without a doubt" };
const char* medium[] = { "Maybe", "Ask again", "Unclear" };
const char* low[]    = { "No", "Highly unlikely", "Never ever" };

/**
 * @brief  Prints magic 8 ball shaking animation to the LCD screen
 * @retval None
 */
void showShakingAnimation(void)
{
    LCD_SendCmd(0x01);
    LCD_SendCmd(0x80);
    LCD_SendStr("Shaking...");

    for (int i = 0; i < 1; i++) {
        for (int j = 6; j <= 8; j++) {
            char line2[17] = "                ";
             line2[j] = 'o';
            LCD_SendCmd(0xC0);
            LCD_SendStr(line2);
            HAL_Delay(5);
        }
        for (int j = 8; j >= 6; j--) {
            char line2[17] = "                ";
            line2[j] = 'o';
            LCD_SendCmd(0xC0);
            LCD_SendStr(line2);
            HAL_Delay(5);
        }
    }

    LCD_SendCmd(0x01);
}

/**
 * @brief  Initialises the Magic 8 Ball Game
 * 		   This function presents the game start screen
 * @retval None
 */
void showMagicBallScreen(void) {
    LCD_SendCmd(0x01);
    LCD_SendStr("Magic 8 Ball");
    LCD_SendCmd(0xC0);
    LCD_SendStr("Shake?");

}

/**
 * @brief  Processes the Magic 8 Ball Game
 * 		   This function displays a shaking animation, calculates a
 * 		   random result and displays the result on the LCD screen
 * @retval None
 */
void processMagicBall(void) {
	GPIO_PinState shake = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

	if (shake == GPIO_PIN_SET) {
        showShakingAnimation();
        HAL_Delay(2);
        LCD_SendCmd(0x01);

		int probability = rand() % 100;
		int index = rand() % 3;
		const char* response;

		if (probability < 30) {
			response = low[index];
		} else if (probability < 70) {
			response = medium[index];
		} else {
			response = high[index];
		}

        LCD_SendCmd(0x01);
        LCD_SendStr("8 Ball Says...");
        LCD_SendCmd(0xC0);
        LCD_SendStr(response);
	}
}

// ------------------------------------------ Score Tracker --------------------------------------------------

// Constants for Score Tracker
#define LONG_PRESS 500 // ms for long press on SW4

// Global Variables for Score Tracker Game
uint8_t scores[2] = {0, 0}; // All player's score = 0 on start
int8_t selectedPlayer = 0; // 0 = PLAYER1, 1 = PLAYER2

/**
 * @brief  Initalises the Score Tracker
 * 		   This function presents the Score Tracker start screen
 * 		   and initalises all players scores and default player to 0
 * @retval None
 */
void showScoreTrackerScreen(void) {
    LCD_SendCmd(0x01);
    LCD_SendStr("Score Tracker");
    selectedPlayer = 0;
    scores[0] = 0;
    scores[1] = 0;
}

/**
 * @brief  Processes the Score Tracker
 * 		   This function processes the score tracker
 * 		   On SW4 tap, the selected player alternates between PLAYER1 and PLAYER2
 * 		   On SW4 long press, the selected player's score increments
 * @retval None
 */
void processScoreTracker(void) {
    char line1[17];
	char line2[17];

    static int8_t lastSelectedPlayer = 0;
    static int lastDisplayedScores[2] = {-1, -1};

    uint32_t now = HAL_GetTick();
    static uint32_t pressStartTime = 0;

    static GPIO_PinState lastButtonState = GPIO_PIN_SET;
    GPIO_PinState currentButtonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);

    if (lastButtonState == GPIO_PIN_SET && currentButtonState == GPIO_PIN_RESET) {
        pressStartTime = now;
    } else if (lastButtonState == GPIO_PIN_RESET && currentButtonState == GPIO_PIN_SET) {
        uint32_t pressDuration = now - pressStartTime;

        // on short press, increments selected player's score
        if (pressDuration < LONG_PRESS) {
            scores[selectedPlayer]++;

        // on long press, changes selected player
        } else {
            selectedPlayer ^= 1;
        }
    }

    lastButtonState = currentButtonState;

    // If score or selected players changes, update display
    if (scores[0] != lastDisplayedScores[0] ||
    	scores[1] != lastDisplayedScores[1] ||
		lastSelectedPlayer != selectedPlayer) {
        if (selectedPlayer == 0) {
            sprintf(line1, ">Player1     %d", scores[0]);
            sprintf(line2, " Player2     %d", scores[1]);
        } else {
            sprintf(line1, " Player1     %d", scores[0]);
            sprintf(line2, ">Player2     %d", scores[1]);
        }

        LCD_SendCmd(0x80);
        LCD_SendStr(line1);
        LCD_SendCmd(0xC0);
        LCD_SendStr(line2);

        lastDisplayedScores[0] = scores[0];
        lastDisplayedScores[1] = scores[1];
        lastSelectedPlayer = selectedPlayer;
    }
}
