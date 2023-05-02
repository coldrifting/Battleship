// Probablity heat map. The higher the number for a given value, 
// the more likely that the corrosponding point on the playerGrid has a ship
int chances[10][10];

// Get's the computer guess based on where ships are likely to be
Point getComputerGuess() {
    
    
    generateChances();
    return getMostLikelyPoint();
}

// Calculate probabilities
void generateChances() {
    // Reset chances for each iteration
    resetChances();

    // Calculate chanes for each ship that is still in play
    for (int s = 0; s < 5; s++) {
        if (!playerShips[s].isSunk()) {
            calculateShipProbability(playerShips[s].size, DOWN);
            calculateShipProbability(playerShips[s].size, RIGHT);
        }
    }

    // Print current probablity values for debugging
    //printChances();
}

// Reset every square on the probability grid back to zero
void resetChances() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            chances[i][j] = 0;
        }
    }
}

// Print the chances for each square to contain a hit
// Used for debugging
void printChances() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int value = chances[i][j];
            if (value < 10);
                Serial.print(" ");
                
            Serial.print(value);
            Serial.print(" ");
        }
        Serial.println();
    }
}

// Find each spot where a ship could exist and account for this in the chances heat map
void calculateShipProbability(Ship ship, Point direction) {
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            Point p = Point(x,y);
            if (isDestroyMode()) {
                if (isShipPossible(p, ship.size, direction)) {
                    if (isShipOverAHit(p, ship.size, direction)) {
                        addChances(p, ship.size, direction, 10);
                    }
                }
            }
            else {
                if (isShipPossible(p, ship.size, direction)) {
                    // Add probablilty factor
                    addChances(p, ship.size, direction, 1);
                }
            }
            if (playerGrid[x][y] != GRID_EMPTY) {
                chances[x][y] = 0;
            }
        }
    }
}

// Check if a ship has been hit but not sunk yet
bool isDestroyMode() {
    for (int s = 0; s < 5; s++) {
        if ((!playerShips[s].isSunk()) && playerShips[s].hits > 0)
            return true;
    }
    return false;
}

// Add a ship layout to the probability matrix
void addChances(Point startPoint, int size, Point direction, int value) {
    Point p = startPoint;
    for (int i = 0; i < size; i++) {
        chances[p.x][p.y] += value;
        p += direction;
    }
}

// Check if a ship could possibly occupy a space based on current hit/miss info
bool isShipPossible(Point startPoint, int size, Point direction) {
    Point p = startPoint;
    for (int i = 0; i < size; i++) {
        if (!p.isInBounds(GRID_SIZE)) {
            return false;
        }
        if (playerGrid[p.x][p.y] == GRID_MISS) {
            return false;  
        }
        if (playerGrid[p.x][p.y] == GRID_SUNK) {
            return false;
        }
        p += direction;
    }
    return true;
}

// Check if a possible ship overlaps a hit on an un-sunk ship (This makes any such squares much more likely to contain a ship)
bool isShipOverAHit(Point startPoint, int size, Point direction) {
    Point p = startPoint;
    bool valid = false;
    for (int i = 0; i < size; i++) {
        if (!p.isInBounds(GRID_SIZE)) {
            return false;
        }

        if (playerGrid[p.x][p.y] == GRID_HIT) {
            valid = true;
        }
        p = p + direction;
    }
    return valid;
}

// Finds the most likely point on the grid to contain a ship, based on the chances array.
// Ignores squares that aren't on the parity checkerboard pattern.
Point getMostLikelyPoint() {
    // Default values
    Point guess = Point(-1,-1);
    uint8_t bestChanceValue = 0;

    // Find highest value
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (playerGrid[x][y] == GRID_EMPTY) {
                if (chances[x][y] > bestChanceValue) {
                    bestChanceValue = chances[x][y];
                }
            }
        }
    }

    int8_t bestSquares[100];
    int8_t numBestSquares = 0;
    // Store all squares that share the highest value in an array
    for (int8_t x = 0; x < GRID_SIZE; x++) {
        for (int8_t y = 0; y < GRID_SIZE; y++) {
            if (chances[x][y] == bestChanceValue) {
                Point p = Point(x,y);
                int8_t coordinate1D = p.convertTo1D(GRID_SIZE);
                bestSquares[numBestSquares++] = coordinate1D;
            }
        }
    }

    // Get one of the best squares at random if there is more than one
    int8_t bestSquaresIndex = random(0,numBestSquares++);
    int8_t coordinate1D = bestSquares[bestSquaresIndex];
    int8_t x = Point::getRow(coordinate1D, GRID_SIZE);
    int8_t y = Point::getCol(coordinate1D, GRID_SIZE);
    
    return Point(x,y);
}
