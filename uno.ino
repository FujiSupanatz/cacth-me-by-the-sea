
  #include <Key.h>
  #include <Keypad.h>
  #include <hd44780.h>
  #include <Wire.h>
  #include <EEPROM.h>
  #include <LiquidCrystal_I2C.h>
  #include <SoftwareSerial.h>
  #include <EEPROM.h> // Include EEPROM library if not already included

  int led = 13;
  int bottleCount = 0; // Variable declaration
  String phoneNumber = "";// Variable creation
  const int sensor1pin = 12;
 // sensor 3 pin A5
  int sensor1Value;


  void saveCountToEEPROM(int count, String number);
  void readCountFromEEPROM();
  void checkSensors();

  const byte ROWS = 4; //four rows  
  const byte COLS = 4; //three columns
  char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'}, // key pad 
    {'*', '0', '#', 'D'}
  };
  byte rowPins[ROWS] =  {4, 5, 6, 7};// Update with the new row pins (8, 9, 10, 11)
  byte colPins[COLS] = {8, 9, 10, 11}; // Update with the new column pins (4, 5, 6, 7)
  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  void setup() {
    pinMode(3, INPUT);
    pinMode(2, OUTPUT);
    pinMode(led, OUTPUT);
    pinMode(sensor1pin, INPUT);
  
    lcd.init();   
    Wire.begin();
    Serial.begin(9600);

    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Enter your phone:");

    Serial.println("phone : ");
    Serial.println(phoneNumber);
  }

  void loop() {
    char key = keypad.getKey();

if (key != NO_KEY) {
  if (isDigit(key)) {
    phoneNumber += key;
    lcd.setCursor(phoneNumber.length() - 1, 1); // Set cursor at the end of the phone number on the LCD
    lcd.print(key);
    delay(10);
    Serial.println(key);
  } else if (key == '*') {
    phoneNumber += key;
    // When 'C' is pressed, display the bottle count and phone number on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    char key = keypad.getKey();
    lcd.print("Enter your phone");
    lcd.setCursor(0, 1); // Set cursor to the second line
    phoneNumber = ""; // Reset phone number after displaying "Enter your phone"
  } else if (key == 'D') {
    if (phoneNumber.length() > 0) {
      phoneNumber.remove(phoneNumber.length() - 1); // Remove the last digit from the phone number
      lcd.setCursor(phoneNumber.length(), 1); // Set cursor to the updated position on the LCD
      lcd.print(" "); // Print a space to overwrite the deleted digit on the LCD
      lcd.setCursor(phoneNumber.length(), 1); // Set cursor back to the updated position
      Serial.println("Deleted last digit");
    }
  } else if (key == 'A') {
    if (phoneNumber.length() > 0) {
      // Process the entered phone number (e.g., make a call, store it, etc.)
      Serial.print("Entered phone number: ");
      Serial.print(phoneNumber);
      // Reset the phoneNumber variable for the next entry
      lcd.clear();
      lcd.print("Complete");
      delay(2000); // Display "Complete" for 2 seconds
      lcd.clear();
      lcd.print("Enter your phone");
      lcd.setCursor(0, 1); // Set cursor to the second line
      phoneNumber = ""; // Reset phone number after displaying "Enter your phone"
    }
  } else if (key == 'B') {
    if (phoneNumber.length() > 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Collect Points");
      lcd.setCursor(phoneNumber.length() - 4, 1);
      lcd.print("Phone:");
      lcd.print(phoneNumber);
      checkSensors();
    }
  } else if (key == 'C') {
    // When 'C' is pressed, display the bottle count and phone number on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Points: ");
    lcd.print(bottleCount);
    lcd.setCursor(phoneNumber.length() - 1, 1);
    lcd.print("Phone: ");
    lcd.print(phoneNumber);
    delay(5000); // Display for 7.5 seconds before returning to normal operation
    lcd.clear();
    lcd.print("Enter your phone");
    lcd.setCursor(0, 1); // Set cursor to the second line
    phoneNumber = ""; // Reset phone number after displaying "Enter your phone"
  }
}
      
    }

    
    

  

    
    

  void checkSensors() {

      if (sensor1Value == HIGH) {
          Serial.println("------------------------");
          Serial.println("Sensor1 Object Detected!");
          Serial.println("------------------------");
          delay(250); // if 1 print detect
          bottleCount++;
          saveCountToEEPROM(bottleCount, phoneNumber);
          readCountFromEEPROM();   
          Serial.print("Count = ");
          Serial.println(bottleCount);
        }
      }
    





  void saveCountToEEPROM(int count, String number) {
    int addressCount = 0;
    int addressPhone = sizeof(int);
    EEPROM.put(addressCount, count);
    for (unsigned int i = 0; i < number.length(); ++i) {
      EEPROM.put(addressPhone + i, number[i]);
    }
    EEPROM.put(addressPhone + number.length(), '\0');
  } // Added missing brace here

  void readCountFromEEPROM() {
    int addressCount = 0;
    int addressPhone = sizeof(int);
    EEPROM.get(addressCount, bottleCount);
    String tempPhone = "";
    char ch;
    do {
      EEPROM.get(addressPhone + tempPhone.length(), ch);
      if (ch != '\0') {
        tempPhone += ch;
      }
    } while (ch != '\0');
    // Assign the read values to global variables
    bottleCount = bottleCount; // Correct assignment
    phoneNumber = tempPhone; // Assign tempPhone to phoneNumber

    Serial.print("Stored Count: ");
    Serial.println(bottleCount);
    Serial.print("Stored Phone Number: ");
    Serial.println(phoneNumber);
  }
