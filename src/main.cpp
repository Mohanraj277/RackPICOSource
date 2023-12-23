#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// constants for keypad
#define ROWS 4
#define COLS 4

// constants for 74hc595
#define number_of_74hc595s 8
#define numOfRegisterPins number_of_74hc595s * 8
#define SER_Pin 7
#define RCLK_Pin 10
#define SRCLK_Pin 11
boolean registers[numOfRegisterPins];

String CurrententeredNumber = ""; // Variable to store entering ID number
String enteredNumber = "";        // Variable to store entered ID number

const char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Define GPIO pins for rows and columns
const char ROW_PINS[ROWS] = {25, 24, 23, 22}; // Change these values according to your wiring
const char COL_PINS[COLS] = {12, 13, 14, 15}; // Change these values according to your wiring

const int slaveSelectPin = 17; // You can choose any GPIO pin for SS/CS
const int interruptPin = 20;   // GPIO pin to trigger interrupt

volatile bool interruptFlag = false;
String sendData;

// variable to store url
String url = "https://erp.themaestro.in/webservice/getproductslocation?id=";

// Set the LCD address (you might need to change this)
LiquidCrystal_I2C lcd(0x27, 20, 4);

void handleInterrupt()
{
  interruptFlag = true;
}

void clearRegisters()
{
  /* function clearRegisters */
  // Clear registers variables
  for (int i = 0; i < numOfRegisterPins; i++)
  {
    registers[i] = LOW; // HIGH fro relay;
  }
}

void writeRegisters()
{
  /* function writeRegisters */
  // Write register after being set
  digitalWrite(RCLK_Pin, LOW);
  for (int i = 0; i < numOfRegisterPins; i++)
  {
    digitalWrite(SRCLK_Pin, LOW);
    int val = registers[i];
    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);
  }
  digitalWrite(RCLK_Pin, HIGH);
}

void setRegisterPin(int index, int value)
{
  /* function setRegisterPin */
  // Set register variable to HIGH or LOW
  registers[index] = value;
}

void writeGrpRelay(uint64_t data_out)
{
  /* function writeGrpRelay */
  for (int i = 0; i < numOfRegisterPins; i++)
  {
    if ((data_out >> i) & (0x01))
    {
      setRegisterPin(i, HIGH);
    }
    else
    {
      setRegisterPin(i, LOW);
    }
  }
  writeRegisters();
}

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
int getArrayLength(String arr[])
{
  int length = 0;
  Serial.println("in getArrayLength");
  // Iterate through the array until you find the end (assuming the array is terminated with a sentinel value)
  while (arr[length] != "null" && arr[length] != NULL)
  {
    Serial.println("in while");
    length++;
  }
  Serial.println(length);
  return length;
}
void match_fun(String str[])
{
  Serial.println("in match function");
  // Initialize a 2D array to store positions
  String grid[8][8];

  // Loop through rows and columns to generate positions
  for (int row = 0; row < 8; row++)
  {
    for (int col = 0; col < 8; col++)
    {
      // Create the position string using the row and column indices
      grid[row][col] = "R" + String(row + 1) + "C" + String(col + 1);
    }
  }

  // Calculate the number of elements in the array
  int arrayLength = getArrayLength(str);

  Serial.print("Number of elements in the array: ");
  Serial.println(arrayLength);

  // Initialize an array to store generated numbers for matched elements
  int matchedNumbers[arrayLength];

  // Match and generate numbers for matched elements
  Serial.println("match starts");
  for (int i = 0; i < arrayLength; i++)
  {
    for (int row = 0; row < 8; row++)
    {
      for (int col = 0; col < 8; col++)
      {
        if (grid[row][col] == str[i])
        {
          // Generate a number for the matched element
          int number = row * 8 + col + 1;
          Serial.print("Match found: ");
          Serial.print(str[i]);
          Serial.print(" corresponds to number ");
          Serial.println(number);

          // Store the generated number in the array
          matchedNumbers[i] = number;
        }
      }
    }
  }
  // Print the array of generated numbers
  Serial.print("Array of generated numbers for matched elements: ");
  for (int i = 0; i < arrayLength; i++)
  {
    Serial.print(matchedNumbers[i]);
    Serial.print(" ");
  }
  Serial.println();

  // Create an 8-byte value
  uint64_t result = 0;

  // Set bits at positions corresponding to matched numbers
  for (int i = 0; i < arrayLength; i++)
  {
    if (matchedNumbers[i] >= 1 && matchedNumbers[i] <= 64)
    {
      result |= (uint64_t(1) << (matchedNumbers[i] - 1));
    }
  }

  uint64_t binaryValue = 0;
  // Print the 8-byte value in binary
  Serial.print("8-byte value in binary: ");
  for (int i = 63; i >= 0; i--)
  {
    binaryValue = (result >> i) & 1;
    Serial.print(binaryValue);
    writeGrpRelay(binaryValue);
  }
  Serial.println();
}

void split_fun(String inputString)
{
  // Set the maximum number of tokens
  const int maxTokens = 64;

  // Array to store tokens
  String tokens[maxTokens];

  int startIndex = 0;
  int endIndex = 0;
  int tokenCount = 0;

  while (startIndex < inputString.length() && tokenCount < maxTokens)
  {
    // Find the index of the next comma or the end of the string
    endIndex = inputString.indexOf(", ", startIndex);

    // If no comma is found, use the end of the string
    if (endIndex == -1)
    {
      endIndex = inputString.length();
    }

    // Extract the substring
    String token = inputString.substring(startIndex, endIndex);
    tokens[tokenCount] = token;
    tokenCount++;

    // Move the start index to the next character after the comma
    startIndex = endIndex + 2;
  }

  // Print the tokens
  for (int i = 0; i < tokenCount; i++)
  {
    Serial.println(tokens[i]);
  }
  match_fun(tokens);
}

void Json_parse_fun(String jsonString)
{
  // Parse JSON
  StaticJsonDocument<256> doc; // Adjust the size based on your JSON structure
  DeserializationError error = deserializeJson(doc, jsonString);

  // Check for parsing errors
  if (error)
  {
    Serial.print(F("JSON parsing failed: "));
    Serial.println(error.c_str());
    return;
  }
  // Access parsed values
  int status = doc["status"];
  String msg = doc["msg"];
  String location = doc["locations"];
  int noOfCells = doc["noOfCells"];

  // Use the parsed values as needed
  Serial.print(F("status: "));
  Serial.println(status);
  Serial.print(F("msg: "));
  Serial.println(msg);
  Serial.print(F("locations: "));
  Serial.println(location);
  Serial.print(F("noOfCells: "));
  Serial.println(noOfCells);
  split_fun(location);
}
void urlSendfun(String idstr)
{
  digitalWrite(slaveSelectPin, LOW);
  sendData = url + idstr;
  Serial.println();
  Serial.println(sendData);
  sendData += "    ";
  for (char c : sendData)
  {
    SPI.transfer(c);
  }
  digitalWrite(slaveSelectPin, HIGH);
}
void setup()
{
  Serial.begin(115200);
  SPI.begin();
  // Init register
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  clearRegisters();
  writeRegisters();
  delay(500);
  for (int i = 0; i < ROWS; ++i)
  {
    pinMode(ROW_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);
  }
  for (int i = 0; i < COLS; ++i)
  {
    pinMode(COL_PINS[i], INPUT_PULLUP);
  }

  pinMode(slaveSelectPin, OUTPUT);
  // Setup interrupt pin
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

  // Initialize the LCD with the number of columns and rows
  lcd.begin();
  // Clear the LCD
  lcd.clear();
  // Turn on the backlight (if available)
  lcd.backlight();
  lcd.setCursor(0, 0);
  // Print a message to the LCD
  lcd.print("Enter The Issue ID: ");
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
      // count = 1;
      enteredNumber = CurrententeredNumber;
      CurrententeredNumber = "";
      urlSendfun(enteredNumber);
    }
    else
    {
      // Concatenate the pressed key to the entered number
      CurrententeredNumber += key;
      lcd.setCursor(8, 1); // Set cursor to the beginning of the 2nd row
      lcd.print(CurrententeredNumber);
    }
  }
  if (interruptFlag)
  {
    interruptFlag = false;
    digitalWrite(slaveSelectPin, LOW);
    for (char c : "")
    {
      SPI.transfer(c);
    }
    digitalWrite(slaveSelectPin, HIGH);
    delay(100);
    digitalWrite(slaveSelectPin, LOW); // Select the slave
    // Receive string data from the slave
    String receivedData = "";
    while (true)
    {
      char c = SPI.transfer(0); // Send dummy byte to receive data
      if (c == '\0')
      {
        break; // Null character indicates the end of the string
      }
      receivedData += c;
    }
    digitalWrite(slaveSelectPin, HIGH); // Deselect the slave
    Serial.println("Received Data: " + receivedData);
    Json_parse_fun(receivedData);
  }
}
