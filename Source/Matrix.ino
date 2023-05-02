//// LED Matrix related functions and variables
#include <Adafruit_NeoPixel.h>
#define COLOR_HIT        strip.Color(255,   0,   0) // Red
#define COLOR_MISS       strip.Color(255, 255, 255) // White
#define COLOR_SHIP       strip.Color(  0, 255,  30) // Green (To stand out from a miss)
#define COLOR_SHIP_START strip.Color(255,   0, 255) // Magenta (Subject to change)
#define COLOR_CURSOR     strip.Color(255, 128,   0) // Amber
#define COLOR_NONE       strip.Color(  0,   8,  32) // Dark Blue
#define COLOR_SUNK       strip.Color( 32,   8,   0) // Dark Red

#define BRIGHTNESS 24
#define NUM_LEDS   100

Adafruit_NeoPixel strip(NUM_LEDS, MATRIX_PIN);
bool drawCursor;

// Sets up the LCD Matrix
void setupLEDMatrix() {
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.show();

    // Draw the inital game board
    refreshMatrix();
}

void refreshMatrix() {
    refreshMatrix(true);
}

// Update the display
void refreshMatrix(const bool isCursorVisible) {
    drawCursor = isCursorVisible;

    if (USE_VIRTUAL_MATRIX) {
        drawSerial();
    } else {
        drawMatrix();
    }
}

void drawMatrix() {
    // Fill background
    strip.fill(COLOR_NONE, 0);
    
    // Draw ships
    if (isPlayerGridShown) {
        for (int i = 0; i < 5; i++) {
            if (playerShips[i].isPlaced) {
                for (int j = 0; j < playerShips[i].size; j++) {
                    Point p = playerShips[i].points[j];
                    strip.setPixelColor(XY(p.x,p.y), COLOR_SHIP);
                }
            }
        }
    } else {
        if (SHOW_ENEMY_SHIPS) {
            for (int i = 0; i < 5; i++) {
                if (enemyShips[i].isPlaced) {
                    for (int j = 0; j < enemyShips[i].size; j++) {
                        Point p = enemyShips[i].points[j];
                        strip.setPixelColor(XY(p.x,p.y), COLOR_SHIP);
                    }
                }
            }
        }
    }

    // Draw hits & misses
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            int value;
            if (isPlayerGridShown) {
                value = playerGrid[row][col];
            } 
            else {
                value = enemyGrid[row][col];
            }
            if (value == GRID_SUNK)
                strip.setPixelColor(XY(row, col), COLOR_SUNK);
            if (value == GRID_HIT)
                strip.setPixelColor(XY(row, col), COLOR_HIT);
            if (value == GRID_MISS)
                strip.setPixelColor(XY(row, col), COLOR_MISS);
        }
    }

    // Draw cursors
    if (drawCursor) {
        // Show cursors on top of everything else
        strip.setPixelColor(XY(selection.x, selection.y), COLOR_CURSOR);
        
        if (shipPlaceStart.x >= 0 && shipPlaceStart.y >= 0)
            strip.setPixelColor(XY(shipPlaceStart.x, shipPlaceStart.y), COLOR_SHIP_START);
    }

    // Show changes
    strip.show();
}

// Turn off the matrix display
void matrixOff() {
    strip.clear();
    strip.show();
}

// Draw the game board on the serial monitor instead of the RGB Matrix
void drawSerial() {
    const char S_EMPTY      = '+';
    const char S_MISS       = 'O';
    const char S_HIT        = 'X';
    const char S_SHIP       = 'S';
    const char S_SUNK       = '$';
    const char S_CURSOR     = '*';
    const char S_SHIP_START = 'P';

    // +1 for null terminator
    char serialMatrix[GRID_SIZE][GRID_SIZE + 1];

    // Draw empty grid, hits, and misses
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            serialMatrix[i][j] = S_EMPTY;
            int value;
            if (isPlayerGridShown)
                value = playerGrid[i][j];
            else
                value = enemyGrid[i][j];
            if (value == GRID_MISS) {
                serialMatrix[i][j] = S_MISS;
                continue;
            }
            if (value == GRID_HIT) {
                serialMatrix[i][j] = S_HIT;
            }
            if (value == GRID_SUNK) {
                serialMatrix[i][j] = S_SUNK;
            }
        }
        serialMatrix[i][10] = '\0';
    }

    // Draw ships
    if (isPlayerGridShown) {
        for (int i = 0; i < 5; i++) {
            if (playerShips[i].isPlaced) {
                for (int j = 0; j < playerShips[i].size; j++) {
                    Point p = playerShips[i].points[j];
                    if (playerGrid[p.x][p.y] != GRID_HIT)
                        serialMatrix[p.x][p.y] = S_SHIP;
                }
            }
        }
    } else {
        if (SHOW_ENEMY_SHIPS) {
            for (int i = 0; i < 5; i++) {
                if (enemyShips[i].isPlaced) {
                    for (int j = 0; j < enemyShips[i].size; j++) {
                        Point p = enemyShips[i].points[j];
                        if (enemyGrid[p.x][p.y] != GRID_HIT)
                            serialMatrix[p.x][p.y] = S_SHIP;
                    }
                }
            }
        }
    }

    // Draw cursors
    if (drawCursor) {
        serialMatrix[selection.x][selection.y] = S_CURSOR;

        if (shipPlaceStart.x >= 0 && shipPlaceStart.y >= 0)
            serialMatrix[shipPlaceStart.x][shipPlaceStart.y] = S_SHIP_START;
    }

    // Print results to serial console
    for (int i = 0; i < GRID_SIZE; i++) {
        Serial.println(serialMatrix[i]);
    }

    Serial.println(lcdLastMsg1);
    Serial.println(lcdLastMsg2);

    // Added whitespace to give the illusion of updating the console
    for (int i = 0; i < 32; i++) {
        Serial.println();
    }
}

// Convert row-column indicies to match the serpentine layout of the RGB Matrix
int XY(const int row, const int col) {
    uint16_t index;
    if (row & 0x01) {
        // Odd rows run backwards
        int reverseX = (GRID_SIZE - 1) - col;
        index = (row * GRID_SIZE) + reverseX;
    } else {
        // Even rows run forwards
        index = (row * GRID_SIZE) + col;
    }

    return index;
}
