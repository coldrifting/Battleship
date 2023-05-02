//// Functions and variables for button and joystick input and buzzer output
//// Joystick related variables
#define DEADZONE_MIN 512 - 384
#define DEADZONE_MAX 512 + 384
int joyX;
int joyY;

void setupIO() {
    // Use an analog pin as a Random Number Generator
    pinMode(RANDOM_SEED_PIN, INPUT);
    randomSeed(analogRead(RANDOM_SEED_PIN));
    
    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(JOYSTICK_Y_PIN, INPUT);

    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    pinMode(BUTTON_B_PIN, INPUT_PULLUP);
    
    pinMode(BUZZER_PIN, OUTPUT);
}

// Check if Button A is pressed.
bool isButtonAPressed() {
    return digitalRead(BUTTON_A_PIN) == LOW;
}

// Check if Button B is pressed.
bool isButtonBPressed() {
    return digitalRead(BUTTON_B_PIN) == LOW;
}

// Move the cursor based on joystick input
void updateCursor() {
    bool update = false;
    int8_t x = selection.x;
    int8_t y = selection.y;
    
    joyX = analogRead(JOYSTICK_X_PIN);
    if (joyX > DEADZONE_MAX) {
        update = true;
        x++;
    }
    if (joyX < DEADZONE_MIN) {
        update = true;
        x--;
    }
    joyY = analogRead(JOYSTICK_Y_PIN);
    if (joyY > DEADZONE_MAX) {
        update = true;
        y++;
    }
    if (joyY < DEADZONE_MIN) {
        update = true;
        y--;
    }

    if (update) {
        // Wraparound
        x = (x + GRID_SIZE) % GRID_SIZE;
        y = (y + GRID_SIZE) % GRID_SIZE;
        
        selection.x = x;
        selection.y = y;
        refreshMatrix();
    }
}

// Let the user select an empty square from the board
Point getSquare() {
    while (true) {
        updateCursor();
        long startTime = millis();
        // Wait 100 ms before moving the cursor, but still look for button inputs
        while (millis() - startTime < 100) {
            if (isButtonAPressed()) {
                if (enemyGrid[selection.x][selection.y] == GRID_EMPTY) {
                    return Point(selection.x, selection.y);
                } else {
                    buzzInvalid();
                    promptNotify_P(PSTR("Invalid square"), 1000);
                    // Restart the inner loop to check for cursor movement
                    break;
                }
            }
        }
    }
}

// Plan a buzzer sound that sounds like a confirmation
void buzzValid() {
    tone(BUZZER_PIN, 2000);
    delay(50);
    noTone(BUZZER_PIN);
}

// Play a buzzer sound that sounds like an error
void buzzInvalid() {
    tone(BUZZER_PIN, 700);
    delay(50);
    noTone(BUZZER_PIN);
}
