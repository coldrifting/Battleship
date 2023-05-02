//// Functions for the ship placement part of the game
// Let players place their own ships on the board in any order
void placePlayerShips() {
    isPlayerGridShown = true;
    refreshMatrix();

    int8_t shipsPlaced = 0;
    bool isReady = false;
    bool isAlmostReady = false;
    while (!isReady) {
        if (isAlmostReady) {
            prompt_P(PSTR("A:Done"), PSTR("B:Edit Layout"));
            if (isButtonAPressed()) {
                isReady = true;
            }
            if (isButtonBPressed()) {
                isAlmostReady = false;
                refreshMatrix();
            }
            delay(100);
            continue;
        }

        prompt_P(PSTR("A:Place Ship"), PSTR("B:Delete Ship"));
        updateCursor();
        
        if (isButtonAPressed()) {
            if (tryPlacePlayerShip()) {
                shipsPlaced++;
                delay(100);
                if (shipsPlaced == 5) {
                    isAlmostReady = true;
                    while(isButtonAPressed()) {
                        delay(10);
                    }
                    refreshMatrix(false);
                    continue;
                }
            }
        }
        
        if (isButtonBPressed()) {
            if (deletePlayerShip()) {
                shipsPlaced--;
                promptNotify_P(PSTR("Ship Deleted"), DELAY_TIME);
            }
        }
        
        delay(100);
    }
}

// Makes sure the ship can be placed
bool tryPlacePlayerShip() {
    int shipIndex = getPlayerShipIndex(selection);
    if (shipIndex != -1) {
        promptNotify_P(PSTR("Invalid Spot"), PSTR("For Ship Start"), DELAY_TIME);
        return false;
    }
    
    prompt_P(PSTR("Select Ship"), PSTR("Endpoint"));
    shipPlaceStart = selection;
    refreshMatrix();
    delay(50);
    while (true) {
        ShipPlaceMainLoop:
        updateCursor();
        if (isButtonBPressed()) {
            shipPlaceStart = Point(-1,-1);
            refreshMatrix();
            return false;
        }
        if (isButtonAPressed()) {
            // Disallow 1 square long ships
            if (selection == shipPlaceStart) {
                delay(100);
                continue;
            }

            // Force ships to be orthogonal
            if (selection.x != shipPlaceStart.x && selection.y != shipPlaceStart.y) {
                buzzInvalid();
                promptNotify_P(PSTR("Ship Not"), PSTR("Aligned"), DELAY_TIME);
                continue;
            }

            // Force ships to be 5 squares long at maximum
            int8_t dist = selection.getDistance(shipPlaceStart);
            if (dist > 4) {
                buzzInvalid();
                promptNotify_P(PSTR("Invalid Size"), DELAY_TIME);
                continue;
            }

            // Make sure ships can't collide
            Point p = selection;
            Point direction = selection.getVector(shipPlaceStart);
            for (int8_t i = 0; i < dist; i++) {
                shipIndex = getPlayerShipIndex(p);
                // if ship index is non-negative, then the point is occupied by a ship
                if (shipIndex != -1) {
                    buzzInvalid();
                    promptNotify_P(PSTR("Collision"), PSTR("Detected"), DELAY_TIME);
                    goto ShipPlaceMainLoop; // Un-avoidable goto - break would only break the first loop
                }
                p = p + direction;
            }

            // Check ship sizes and place if a match
            for (int8_t i = 0; i < 5; i++) {
                if (dist + 1 == playerShips[i].size) {
                    if (!playerShips[i].isPlaced) {
                        // Place ship
                        placeShip(playerShips[i], shipPlaceStart, selection);
                        return true;
                    }  else  {
                        if (dist + 1 == 3) {
                            if (!playerShips[3].isPlaced) {
                                placeShip(playerShips[3], shipPlaceStart, selection);
                                return true;
                            } else {
                                buzzInvalid();
                                promptNotify_P2(shipNames[3], PSTR("Already Placed"), DELAY_TIME);
                                break;
                            }
                        }
                        buzzInvalid();
                        promptNotify_P2(shipNames[i], PSTR("Already Placed"), DELAY_TIME);
                    }
                }
            }
        }
        delay(100);
    }
}

// Places a ship
void placeShip(Ship &ship, const Point start, const Point end) {
    Point direction = start.getVector(end);
    Point current = start;

    for (int8_t i = 0; i < ship.size; i++) {
        ship.points[i] = current;
        current = current + direction;
    }
    
    ship.isPlaced = true;
    
    shipPlaceStart = Point(-1,-1);
    refreshMatrix();
}

// Deletes a ship
bool deletePlayerShip() {
    int8_t shipIndex = getPlayerShipIndex(selection);
    if (shipIndex == -1)
        return false;
    else {
        playerShips[shipIndex].isPlaced = false;
        refreshMatrix();
        return true;
    }
}

// Place the enemy ships in random positions and orientations
void placeEnemyShips() {
    prompt_P(PSTR("Please Wait"), PSTR("Placing Ships"));
    for (int i = 0; i < 5; i++) {
        int shipSize = enemyShips[i].size;
        int offset = (GRID_SIZE - shipSize) + 1;

        randomSeed(analogRead(RANDOM_SEED_PIN));
        bool isVertical = random(0,2);
        if (isVertical)
            tryPlaceEnemyShip(GRID_SIZE, offset, i, DOWN);
        else
            tryPlaceEnemyShip(offset, GRID_SIZE, i, RIGHT);
    }
    delay(1000);
    prompt_P(PSTR("                "));
}

// Try to place an enemy ship within a specified spot
void tryPlaceEnemyShip(const int xBounds, const int yBounds, const int shipIndex, const Point direction) {
    int shipSize = enemyShips[shipIndex].size;
    // Possibly naive approach. Tries random squares until one works
    while (true) {
        // Generate a random start point for the ship
        int x = random(0,xBounds);
        int y = random(0,yBounds);
        
        Point startPoint = Point(x,y);

        // If spot is valid, place the ship
        if(isEnemyShipPlacementValid(startPoint, direction, shipIndex)) {
            Point endPoint = startPoint;
            for (int i = 0; i < enemyShips[shipIndex].size; i++) {
                endPoint = endPoint + direction;
            }
            placeShip(enemyShips[shipIndex], startPoint, endPoint);
            return;
        }
    }
}

// Check if a specified ship can be placed at a given spot
bool isEnemyShipPlacementValid(const Point startPoint, const Point direction, const int shipIndex) {
    Point p = startPoint;
    for (int i = 0; i < enemyShips[shipIndex].size; i++) {
        // We can't place ships that go off the grid
        if (!p.isInBounds(GRID_SIZE))
            return false;
        // Check that the ship doesn't overlap any placed ship
        if (getEnemyShipIndex(p) != -1)
            return false;
        p = p + direction;
    }
    return true;
}
