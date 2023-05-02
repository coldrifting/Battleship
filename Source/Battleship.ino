//// Libraries
#include "Classes.h"

//// Arduino Pin-Outs
#define MATRIX_PIN      2
#define BUZZER_PIN      3 // Needs PWM Output Pin (3, 5, 6, 9, 10, 11)
#define BUTTON_A_PIN    4
#define BUTTON_B_PIN    5
#define JOYSTICK_X_PIN  14 // A0
#define JOYSTICK_Y_PIN  15 // A1
#define RANDOM_SEED_PIN 16 // A2;

//// Game Constants
#define GRID_SIZE  10
#define DELAY_TIME 1000

//// Debug Flags
#define USE_VIRTUAL_MATRIX false // Draw on the serial monitor instead of the RGB Matrix
#define SHOW_ENEMY_SHIPS   false // Cheat Mode for debugging

//// Game variables
bool isGameOver = false;
bool isWinner = false;
bool isPlayerGridShown = true;
int8_t enemyShipsSunk = 0;
int8_t playerShipsSunk = 0;
int8_t numTurns = 0;

Ship playerShips[5] = { 5, 4, 3, 3, 2 };
Ship enemyShips[5] =  { 5, 4, 3, 3, 2 };
const char shipNames[][11] = {"Carrier", "Battleship", "Cruiser", "Submarine", "Destroyer"};

//// Game grid variables
int8_t enemyGrid[GRID_SIZE][GRID_SIZE];
int8_t playerGrid[GRID_SIZE][GRID_SIZE];
#define GRID_EMPTY 0
#define GRID_MISS  1
#define GRID_HIT   2
#define GRID_SUNK  4

//// Point related variables
Point selection = Point(0,0);
Point shipPlaceStart = Point(-1,-1);

void setup() {
    // Start logging
    Serial.begin(115200);

    // Initialize our components
    setupIO();
    setupLCD();
    setupLEDMatrix();
    
    // Begin the game
    startGame();
}

void loop() {
}

// Starts the game by controlling the order of the gameplay componenets
void startGame() {
    placeEnemyShips();
    placePlayerShips();
    while (!isGameOver) {
        playerTurn();
        computerTurn();
        numTurns++;
    }
    gameOver();
}

// Player's Turn
void playerTurn() {
    if (isGameOver)
        return;

    isPlayerGridShown = false;
    refreshMatrix();
    
    prompt_P(PSTR("Select a square"), PSTR("to Attack"));
    
    Point guess = getSquare();
    refreshMatrix(false);

    // Check for a hit
    if (isEnemyShipHit(guess)) {
        // Mark spot on grid
        enemyGrid[guess.x][guess.y] = GRID_HIT;
        refreshMatrix(false);

        // Play Hit Sound
        buzzValid();

        // If this hit sunk the ship
        if (isEnemyShipSunk(guess)) {
            sinkEnemyShip(guess);
            refreshMatrix(false);

            // Sunk Ship Prompt - Add Exclamation Point
            char shipName[17] = "";
            strcpy(shipName, shipNames[getEnemyShipIndex(guess)]);
            strncat(shipName, "!", 17);
            
            prompt_P1(PSTR("You Sunk Their"), shipName);

            // Game Over condition
            if (enemyShipsSunk == 5) {
                delay(DELAY_TIME);
                isWinner = true;
                isGameOver = true;
                return;
            }
        } 
        else {
            // Hit but not a sunk ship
            prompt_P(PSTR("You Hit!"));
        }
    } 
    else {
        // Miss
        enemyGrid[guess.x][guess.y] = GRID_MISS;
        refreshMatrix(false);
        
        prompt_P(PSTR("You Missed!"));
    }
    delay(DELAY_TIME);
}

// Computer turn
void computerTurn() {
    if (isGameOver)
        return;

    isPlayerGridShown = true;
    refreshMatrix(false);
    
    prompt_P(PSTR("Incoming!"));
    delay(DELAY_TIME);

    Point enemySelection = getComputerGuess();

    // Check for a hit
    if (isPlayerShipHit(enemySelection)) {
        // Mark on grid
        playerGrid[enemySelection.x][enemySelection.y] = GRID_HIT;
        refreshMatrix(false);

        // Play Hit Sound
        buzzInvalid();
        
        // If this hit sunk the ship
        if (isPlayerShipSunk(enemySelection)) {
            sinkPlayerShip(enemySelection);
            refreshMatrix(false);

            // Sunk Ship Prompt - Add Exclamation Point
            char shipName[17] = "";
            strcpy(shipName, shipNames[getPlayerShipIndex(enemySelection)]);
            strncat(shipName, "!", 17);
            
            prompt_P1(PSTR("They Sunk Your"), shipName);

            // Game Over condition
            if (playerShipsSunk == 5) {
                delay(DELAY_TIME);
                isWinner = false;
                isGameOver = true;
                return;
            }
        } 
        else {
            // Hit but not a sunk ship
            prompt_P(PSTR("They Hit!"));
        }
    } 
    else {
        // Miss
        playerGrid[enemySelection.x][enemySelection.y] = GRID_MISS;
        refreshMatrix(false);
        
        prompt_P(PSTR("They Missed!"));
    }
    delay(DELAY_TIME);
}

// Game Over routine
void gameOver() {
    char turnMsg[17];
    snprintf(turnMsg, 17, "Turns: %d", numTurns);
    
    // Show LCD Prompt
    if (isWinner) {
        prompt_P1(PSTR("You Won!"), turnMsg);
    } 
    else {
        prompt_P1(PSTR("You Lost!"), turnMsg);
    }
}



//// Helper functions
// Get enemy ship index
int8_t getEnemyShipIndex(const Point searchPoint) {
    return getShipIndex(searchPoint, enemyShips);
}

// Get player ship index
int8_t getPlayerShipIndex(const Point searchPoint) {
    return getShipIndex(searchPoint, playerShips);
}

// Check if a point is contained within a ship
// if so, return the ship index, if not, return -1
int8_t getShipIndex(const Point searchPoint, const Ship ships[]) {
    for (int8_t i = 0; i < 5; i++) {
        if (ships[i].isPlaced) {
            for (int8_t j = 0; j < ships[i].size; j++) {
                if (searchPoint == ships[i].points[j]) {
                    return i;
                }
            }
        }
    }
    // No match was found
    return -1;
}



// Check if an enemy ship at a point has been hit, and hit it if so
bool isEnemyShipHit(const Point searchPoint) {
    return isShipHit(searchPoint, enemyShips);
}

// Check if a player ship at a point has been hit, and hit it if so
bool isPlayerShipHit(const Point searchPoint) {
    return isShipHit(searchPoint, playerShips);
}

// Check if a ship at this point has been hit, and if so, increment the hit counter
bool isShipHit(const Point searchPoint, Ship ships[]) {
    int8_t index = getShipIndex(searchPoint, ships);
    if (index == -1)
        return false;

    ships[index].hits++;
    if (ships[index].hits == ships[index].size)
        ships[index].isSunk();

    return true;
}



// Check if enemy ship at a given Point is sunk
bool isEnemyShipSunk(const Point searchPoint) {
    return isShipSunk(searchPoint, enemyShips);
}

// Check if player ship at a given Point is sunk
bool isPlayerShipSunk(const Point searchPoint) {
    return isShipSunk(searchPoint, playerShips);
}

// Check if a ship at this point has been sunk
bool isShipSunk(const Point searchPoint, const Ship ships[]) {
    int8_t index = getShipIndex(searchPoint, ships);
    if (index == -1)
        return false;
        
    return ships[index].isSunk();
}



// Sink an enemy ship at a specified point
void sinkEnemyShip(const Point searchPoint) {
    sinkShip(searchPoint, enemyShips, enemyGrid);
    enemyShipsSunk++;
}

// Sink a player ship at a specified point
void sinkPlayerShip(const Point searchPoint) {
    sinkShip(searchPoint, playerShips, playerGrid);
    playerShipsSunk++;
}

// Sink a ship at a sepcified point and mark the grid positions as sunk
void sinkShip(const Point searchPoint, const Ship ships[], int8_t grid[][GRID_SIZE]) {
    int8_t index = getShipIndex(searchPoint, ships);
    if (index == -1)
        return false;
        
    for (int i = 0; i < ships[index].size; i++) {
        Point p = ships[index].points[i];
        grid[p.x][p.y] = GRID_SUNK;
    }
}
