#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

String phoneNumber = "";

const int trigPin = 3; // Trig pin of the ultrasonic sensor
const int echoPin = 2; // Echo pin of the ultrasonic sensor
int bottleCount = 1; // Variable to count bottles
const byte ROWS = 4;
const byte COLS = 4;

void saveCountToEEPROM(int count, String number);
void readCountFromEEPROM();
void checkSensors();

char customKeymap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

LiquidCrystal_I2C lcd(0x27, 16, 2);
byte rowPins[ROWS] = {11, 10, 9, 8}; // Replace with actual row pin numbers (connected to pins 4 to 11)
byte colPins[COLS] = {7, 6, 5, 4};   // Replace with actual column pin numbers
Keypad keypad = Keypad(makeKeymap(customKeymap), rowPins, colPins, ROWS, COLS);

bool isBKeyPressed = false; // Flag to indicate if 'B' key is pressed

void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight of the LCD
  pinMode(trigPin, OUTPUT); // Set trigPin as an output
  pinMode(echoPin, INPUT); // Set echoPin as an input
  lcd.setCursor(0, 0);
  lcd.print("Enter your phone:");
  // Read bottle count and phone number from EEPROM on startup
  readCountFromEEPROM();
}

void loop() {
  char key = keypad.getKey();

  if (key != NO_KEY) {
    if (key == '*') {
      // Stop the 'B' key loop if '*' key is pressed
      isBKeyPressed = false;
      lcd.clear();
      lcd.print("Enter your phone:");
      lcd.setCursor(0, 1);
      phoneNumber = "";
      Serial.println("Clear Display");
    } else if (key == 'B' && !isBKeyPressed) {
      // Start 'B' key loop if 'B' is pressed and loop is not already running
      isBKeyPressed = true;
      Serial.println("B key pressed");
    } else if (key != '*' && key != '#' && key != 'B') {
      // Handle keypad input except for '*', '#', and 'B'
      if (isBKeyPressed) {
        // If 'B' key loop is active
        if (key == '#') {
          isBKeyPressed = false; // Stop the loop when '#' is pressed
          Serial.println("Loop stopped");
        } else {
          // Execute your desired loop functionality here
          // For example:
          // delay(1000);
        }
      } else {
        // Handle regular keypad input
        if (isDigit(key)) {
          phoneNumber += key;
          lcd.setCursor(phoneNumber.length() - 1, 1);
          lcd.print(key);
          delay(10);
          Serial.println(key);
        } else if (key == 'D') {
          if (phoneNumber.length() > 0) {
            phoneNumber.remove(phoneNumber.length() - 1);
            lcd.setCursor(phoneNumber.length(), 1);
            lcd.print(" ");
            lcd.setCursor(phoneNumber.length(), 1);
            Serial.println("Deleted last digit");
          }
        } else if (key == 'A') {
          if (phoneNumber.length() > 0) {
            // Process the entered phone number
            Serial.print("Entered phone number: ");
            Serial.println(phoneNumber);
            lcd.clear();
            lcd.print("Complete");
            delay(2000);
            lcd.clear();
            lcd.print("Enter your phone");
            lcd.setCursor(0, 1);
            phoneNumber = "";
          }
        } else if (key == 'C') {
          // Display the count and phone number on LCD
          displayInfoOnLCD();
        }
      }
    }
  }

  // Flag to indicate whether sensors should be checked
  bool shouldRunCheckSensors = (key == 'B' && isBKeyPressed); // Only run checkSensors() when 'B' key is pressed and 'B' key loop is active

  if (shouldRunCheckSensors) {
    checkSensors();
  }
}

void checkSensors() {
  for (int i = 0; i < 90; i++){
  unsigned long duration, distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

 

    if (distance > 0 && distance < 20) { // Assuming bottles are detected within 20 cm
      bottleCount++;
      Serial.print("Bottle count: ");
      Serial.println(bottleCount);
      Serial.println("run ran run");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Bottle count: ");
      lcd.print(bottleCount);

      delay(1000);

      // Save bottle count to EEPROM
      saveCountToEEPROM(bottleCount, phoneNumber);
    }
  }
}


void saveCountToEEPROM(int count, String number) {
  int addressCount = 0; // Change this address to a specific address in EEPROM
  EEPROM.put(addressCount, count);
  int addressPhone = addressCount + sizeof(int); // Increment the address by the size of int
  for (unsigned int i = 0; i < number.length(); ++i) {
    EEPROM.put(addressPhone + i, number[i]);
  }
  EEPROM.put(addressPhone + number.length(), '\0');
}

void readCountFromEEPROM() {
  int addressCount = 0; // Change this address to the same address used in saveCountToEEPROM
  EEPROM.get(addressCount, bottleCount);
  int addressPhone = addressCount + sizeof(int); // Increment the address by the size of int
  String tempPhone = "";
  char ch;
  do {
    EEPROM.get(addressPhone + tempPhone.length(), ch);
    if (ch != '\0') {
      tempPhone += ch;
    }
  } while (ch != '\0');
  phoneNumber = tempPhone;

  Serial.print("Stored Count: ");
  Serial.println(bottleCount);
  Serial.print("Stored Phone Number: ");
  Serial.println(phoneNumber);
}

void displayInfoOnLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Count: ");
  lcd.print(bottleCount);
  Serial.print("Count: ");	
  Serial.print(bottleCount);
  
   
  lcd.setCursor(0, 1);
  lcd.print("Phone: ");
  lcd.print(phoneNumber);
  delay(7500);
  lcd.clear();
  lcd.print("Enter your phone");
  	
}