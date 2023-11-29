#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

String phoneNumber = "";
bool continueCheckSensors = false;

const int trigPin = 3;
const int echoPin = 2;
int bottleCount = 0;
const byte ROWS = 4;
const byte COLS = 4;

void saveCountToEEPROM(int count, String number);
void readCountFromEEPROM();
void checkSensors();
void displayInfoOnLCD();
void checkPoints(); // Added function

char customKeymap[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

LiquidCrystal_I2C lcd(0x27, 16, 2);
byte rowPins[ROWS] = {11, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 4};
Keypad keypad = Keypad(makeKeymap(customKeymap), rowPins, colPins, ROWS, COLS);

#define MAX_PHONES 10000000000000000000000

struct PhoneData {
  String phoneNumber;
  int count;
};

PhoneData phoneData[MAX_PHONES];
int phoneDataSize = 0;

int findPhoneNumberIndex(String number) {
  for (int i = 0; i < phoneDataSize; ++i) {
    if (phoneData[i].phoneNumber == number) {
      return i;
    }
  }
  return -1;
}

void updatePhoneData(String number, int count) {
  int index = findPhoneNumberIndex(number);
  if (index != -1) {
    phoneData[index].count += count;
  } else {
    if (phoneDataSize < MAX_PHONES) {  // Fix the condition here
      phoneData[phoneDataSize].phoneNumber = number;
      phoneData[phoneDataSize].count = count;
      phoneDataSize++;
    } else {
      Serial.println("Maximum phone numbers reached!");
    }
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  lcd.setCursor(0, 0);
  lcd.print("Enter your phone:");
  readCountFromEEPROM();
}

void loop() {
  char key = keypad.getKey();

  if (key != NO_KEY) {
    if (key == 'B') {
      Serial.println("B key pressed");
      continueCheckSensors = true;
      checkSensors();
    } else if (key == '#') {
      displayInfoOnLCD();
    } else if (key == 'C') {
      checkPoints(); // Invoke checkPoints function
    } else {
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
          Serial.print("Entered phone number: ");
          Serial.println(phoneNumber);

          // Save the entered phone number directly without concatenating
          saveCountToEEPROM(1, phoneNumber);

          lcd.clear();
          lcd.print("Complete");
          delay(2000);
          lcd.clear();

          lcd.print("Enter your phone");
          lcd.setCursor(0, 1);
          phoneNumber = "";
        }
      }
    }
  }
}

void checkSensors() {
  while (continueCheckSensors) {
    char key = keypad.getKey(); // Move this line inside the loop

    unsigned long duration, distance;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;

    if (distance > 0 && distance < 50) {
      bottleCount++;
      Serial.print("Bottle count: ");
      Serial.println(bottleCount);
      Serial.println("run ran run");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Bottle count: ");
      lcd.print(bottleCount);

      delay(350);
      if (bottleCount > 0) {
        // Save bottle count to EEPROM with unique address based on the phone number
        saveCountToEEPROM(bottleCount, phoneNumber);
        updatePhoneData(phoneNumber, bottleCount);
      }
    }

    if (key != NO_KEY) {
      if (key == '*') {
        lcd.clear();
        bottleCount = 0;
        lcd.print("Enter your phone:");
        lcd.setCursor(0, 1);
        phoneNumber = "";
        Serial.println("Clear Display");
        continueCheckSensors = false;
        break;
        bottleCount = 0;
      } else {
        continueCheckSensors = true;
      }
    }
  }
}


void saveCountToEEPROM(int count, String number) {
  int phoneIndex = findPhoneNumberIndex(number);

  if (phoneIndex == -1) {
    // Phone number not found, add it to EEPROM
    if (phoneDataSize < MAX_PHONES) {
      int address = phoneDataSize * sizeof(PhoneData);
      EEPROM.put(address, PhoneData{number, count});
      phoneDataSize++;
    } else {
      Serial.println("Maximum phone numbers reached!");
    }
  } else {
    // Phone number found, update the count
    int address = phoneIndex * sizeof(PhoneData);
    EEPROM.get(address, phoneData[phoneIndex]);
    phoneData[phoneIndex].count += count;
    EEPROM.put(address, phoneData[phoneIndex]);
  }
}

void readCountFromEEPROM() {
  for (int i = 0; i < phoneDataSize; ++i) {
    int address = i * sizeof(PhoneData);
    EEPROM.get(address, phoneData[i]);
  }

  Serial.println("Stored Data:");
  for (int i = 0; i < phoneDataSize; ++i) {
    Serial.print("Phone Number: ");
    Serial.print(phoneData[i].phoneNumber);
    Serial.print(", Count: ");
    Serial.println(phoneData[i].count);
  }
}

void displayInfoOnLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Phone: ");
  lcd.print(phoneNumber);

  lcd.setCursor(0, 1);
  lcd.print("Total Count: ");
  int total = 0;
  for (int i = 0; i < phoneDataSize; ++i) {
    if (phoneData[i].phoneNumber == phoneNumber) {
      total = phoneData[i].count;
      break;
    }
  }
  lcd.print(total);

  Serial.print("Phone: ");
  Serial.print(phoneNumber);
  Serial.print(", Total Count: ");
  Serial.println(total);

  delay(7500);
  lcd.clear();
  lcd.print("Enter your phone");
}

void checkPoints() {
  // Display total count for the entered phone number
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Phone: ");
  lcd.print(phoneNumber);

  lcd.setCursor(0, 1);
  lcd.print("Total Count: ");
  int total = 0;
  for (int i = 0; i < phoneDataSize; ++i) {
    if (phoneData[i].phoneNumber == phoneNumber) {
      total = phoneData[i].count;
      break;
    }
  }
  lcd.print(total);

  Serial.print("Phone: ");
  Serial.print(phoneNumber);
  Serial.print(", Total Count: ");
  Serial.println(total);

  delay(5000);
  lcd.clear();
  Serial.println("Clear Check");
  phoneNumber = "";
  lcd.print("Enter your phone");
}