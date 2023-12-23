#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// constants for keypad
#define ROWS 4
#define COLS 4
// constants for 74hc595
char count = 0;
String CurrententeredNumber = ""; // Variable to store entered number
String enteredNumber = "";

const char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Define GPIO pins for rows and columns
const char ROW_PINS[ROWS] = {25, 24, 23, 22}; // Change these values according to your wiring
const char COL_PINS[COLS] = {12, 13, 14, 15}; // Change these values according to your wiring

// Set the LCD address (you might need to change this)
LiquidCrystal_I2C lcd(0x27, 20, 4);

char scanKeypad()
{
  char k = 0;
  for (int row = 0; row < ROWS; ++row)
  {
    digitalWrite(ROW_PINS[row], LOW);

    for (int col = 0; col < COLS; ++col)
    {
      if (digitalRead(COL_PINS[col]) == LOW)
      {
        k = keys[row][col];
        while (digitalRead(COL_PINS[col]) == LOW)
          ; // Wait for key release
      }
    }

    digitalWrite(ROW_PINS[row], HIGH);
  }
  return k;
}

void setup()
{
  Serial.begin(115200);
  for (int i = 0; i < ROWS; ++i)
  {
    pinMode(ROW_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);
  }
  for (int i = 0; i < COLS; ++i)
  {
    pinMode(COL_PINS[i], INPUT_PULLUP);
  }
  // Initialize the LCD with the number of columns and rows
  lcd.begin();
  // Clear the LCD
  lcd.clear();
  // Turn on the backlight (if available)
  lcd.backlight();

  lcd.setCursor(0, 0);
  // Print a message to the LCD
  lcd.print("Enter The Number :");
}

void loop()
{
  char key = scanKeypad();
  delay(100); // Adjust the delay according to your needs
  if (key != 0)
  {
    if (key == 'A')
    {
      // If '#' key is pressed, clear the entered number
      lcd.setCursor(0, 1); // Set cursor to the beginning of the 2nd row
      lcd.print("                    ");
      count = 1;
      enteredNumber = CurrententeredNumber;
      CurrententeredNumber = "";
    }
    else
    {
      // Concatenate the pressed key to the entered number
      CurrententeredNumber += key;
      lcd.setCursor(8, 1); // Set cursor to the beginning of the 2nd row
      lcd.print(CurrententeredNumber);
    }
  }
  if (count == 1)
  {
    if (enteredNumber == "1234")
    {
    }
    count = 0;
  }
}
