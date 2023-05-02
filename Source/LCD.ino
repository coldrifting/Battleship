//// LCD Related functions and variables
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
hd44780_I2Cexp lcd; // declare lcd object: auto locate & config display for hd44780 chip

// Store the previously displayed line here
// So we only update the screen when the new line differs
char lcdLine1[17] = "";
char lcdLine2[17] = "";

// Used for printing with the virtual matrix
char lcdLastMsg1[17] = "";
char lcdLastMsg2[17] = "";

// Initialize LCD
void setupLCD() {
    lcd.begin(16,2);
    lcd.clear();
}

// Display a one line prompt on the LCD display
void prompt(const char *msg) {
    prompt(msg, "");
}

// Display a prompt on the LCD Display using both lines
void prompt(const char *msg1, const char *msg2) {
    // The prompt hasn't changed, no need to update LCD
    if (strcmp(lcdLine1,msg1) == 0 && strcmp(lcdLine2,msg2) == 0)
        return;

    strcpy(lcdLine1, msg1);
    strcpy(lcdLine2, msg2);
    lcdWrite(msg1, msg2);
}

// Display a prompt on the LCD display
// Adapted to show a string stored in program memory
// Method signature is the same, so a different method name is required
void prompt_P(const char *pstr) {
    prompt_P1(pstr, "");
}

// Displays a prompt on the LCD Display using both lines
// Used for displaying constants stored in Program Memory space (Saves dynamic memory)
void prompt_P(const char *pstr1, const char *pstr2) {
    char msg1Buffer[17] = "";
    char msg2Buffer[17] = "";

    strlcpy_P(msg1Buffer, pstr1, 17);
    strlcpy_P(msg2Buffer, pstr2, 17);

    prompt(msg1Buffer, msg2Buffer);
}

// Display a prompt on the LCD when only the first line is stored in program memory
void prompt_P1(const char *pstr1, const char *msg2) {
    char msg1Buffer[17] = "";
    
    strlcpy_P(msg1Buffer, pstr1, 17);

    prompt(msg1Buffer, msg2);
}

// Display a prompt on the LCD when only the last line is stored in program memory
void prompt_P2(const char *msg1, const char *pstr2) {
    char msg2Buffer[17] = "";
    
    strlcpy_P(msg2Buffer, pstr2, 17);

    prompt(msg1, msg2Buffer);
}

// Display a notification on the LCD, then restore the previous message
void promptNotify(const char *msg, const int waitTime) {
    promptNotify(msg, "", waitTime);
}

// Display a notification on the LCD using both lines, then restore the previous message
void promptNotify(const char *msg1, const char *msg2, const int waitTime) {
    lcdWrite(msg1, msg2);
    delay(waitTime);
    
    // Restore Previous Message
    lcdWrite(lcdLine1, lcdLine2);
}

// Display a notification on the LCD, then restore the previous message
// Use when displaying one line stored in Program Memory
void promptNotify_P(const char *pstr, const int waitTime) {
    promptNotify_P1(pstr, "", waitTime);
}

// Display a notification on the LCD, then restore the previous message
// Use when displaying two lines stored in program memory
void promptNotify_P(const char *pstr1, const char *pstr2, const int waitTime) {
    char msg1Buffer[17] = "";
    char msg2Buffer[17] = "";

    strlcpy_P(msg1Buffer, pstr1, 17);
    strlcpy_P(msg2Buffer, pstr2, 17);

    promptNotify(msg1Buffer, msg2Buffer, waitTime);
}

// Display a notification on the LCD, then Restore the previous message
// Use when only the first string is stored in program memory, but not the second
void promptNotify_P1(const char *pstr, const char *msg2, const int waitTime) {
    char msg1Buffer[17] = "";
    strlcpy_P(msg1Buffer, pstr, 17);
    promptNotify(msg1Buffer, msg2, waitTime);
}

// Display a notification on the LCD, then Restore the previous message
// Use when only the second string is stored in program memory, but not the first
void promptNotify_P2(const char *msg1, const char *pstr2, const int waitTime) {
    char msg2Buffer[17] = "";
    strlcpy_P(msg2Buffer, pstr2, 17);
    promptNotify(msg1, msg2Buffer, waitTime);
}

// Write only to the top line of the lcd
// Calls the two line version to reduce code duplication
void lcdWrite(const char *msg) {
    lcdWrite(msg, "");
}

// clears the lcd screen and writes two lines to it
// Faster than using the lcd.clear() method
void lcdWrite(const char *msg1, const char *msg2) {
    // Print the last message under the vitual matrix when it's used
    if (USE_VIRTUAL_MATRIX) {
        strlcpy(lcdLastMsg1, msg1, 17);
        strlcpy(lcdLastMsg2, msg2, 17);
        refreshMatrix();
    }
    
    lcd.setCursor(0,0);
    lcd.print(F("                ")); // Store blank string in PROGMEM to save memory
    lcd.setCursor(0,1);
    lcd.print(F("                ")); // Store blank string in PROGMEM to save memory
    lcd.setCursor(0,0);
    lcd.print(msg1);
    lcd.setCursor(0,1);
    lcd.print(msg2);
}
