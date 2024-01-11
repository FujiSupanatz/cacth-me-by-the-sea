  #include <Wire.h>
  #include <EEPROM.h>
  #include <LiquidCrystal_I2C.h>
  #include <Keypad.h>

  String phoneNumber = "";
  bool continueCheckSensors = false;
  unsigned long previousMillis = 0;
  const long interval = 400;  // Interval between sensor readings in milliseconds
int sensor1Pin = 2;
int sensor2Pin = 3;
int sensor3Pin = 12;


int sensor1Value, sensor2Value, sensor3Value;
  bool objectDetected = false;
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

  #define MAX_PHONES 10

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

  void updatePhoneNumber(String oldNumber, String newNumber) {
    int oldIndex = findPhoneNumberIndex(oldNumber);
    int newIndex = findPhoneNumberIndex(newNumber);

    if (oldIndex != -1 && newIndex == -1) {
        // Old phone number found, new phone number not found
        phoneData[oldIndex].phoneNumber = newNumber;
        int address = oldIndex * sizeof(PhoneData);
        EEPROM.put(address, phoneData[oldIndex]);
        Serial.println("Phone number updated!");
    } else {
        if (newIndex != -1) {
            Serial.println("New phone number already exists!");
        } else {
            Serial.println("Old phone number not found!");
        }
    }
}


  void updatePhoneData(String number, int count) {
    int phoneIndex = findPhoneNumberIndex(number);

    if (phoneIndex != -1) {
        // Phone number found, update the count
        phoneData[phoneIndex].count = count;
        int address = phoneIndex * sizeof(PhoneData);
        EEPROM.put(address, phoneData[phoneIndex]);
    } else {
        if (phoneDataSize < MAX_PHONES) {
            // Phone number not found, add it to EEPROM
            phoneData[phoneDataSize].phoneNumber = number;
            phoneData[phoneDataSize].count = count;
            int address = phoneDataSize * sizeof(PhoneData);
            EEPROM.put(address, phoneData[phoneDataSize]);
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
    pinMode(sensor1Pin, INPUT); 
    pinMode(sensor2Pin, INPUT); 
    pinMode(sensor3Pin, INPUT); 
    lcd.setCursor(0, 0);
    lcd.print("Enter your phone:");
    readCountFromEEPROM();
  }

 void loop() {
    char key = keypad.getKey();

    if (key != NO_KEY) {
        if (key == 'B') {
            Serial.println("B key pressed");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Bottle count: ");
            lcd.print(bottleCount);
            checkSensors();
            continueCheckSensors = true;
        } else if (key == 'C') {
            checkPoints();
        } else {
            if (isDigit(key) && phoneNumber.length() < 10) { // จำกัดความยาวของเบอร์โทรศัพท์ที่รับเข้ามาไม่เกิน 10 ตัว
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
                if (validatePhoneNumber(phoneNumber)) {
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
                } else {
                    Serial.println("Invalid phone number entered!");
                    lcd.clear();
                    lcd.print("Invalid phone number");
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
    while (true) {
        char key = keypad.getKey(); // Move this line inside the loop

        // Read sensor values inside the loop
        sensor1Value = digitalRead(sensor1Pin);
        sensor2Value = digitalRead(sensor2Pin);
        sensor3Value = digitalRead(sensor3Pin);

        // Read sensor 1
        if (sensor1Value == LOW) {
            Serial.println("------------------------");
            Serial.println("Sensor1 Object Detected!");
            Serial.println("------------------------");
            delay(250);
            bottleCount++;
            Serial.print("Count = ");
            Serial.println(bottleCount);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Bottle count: ");
            lcd.print(bottleCount);

            // Save bottle count to EEPROM with unique address based on the phone number
            addCountToEEPROM(bottleCount, phoneNumber);
            updatePhoneData(phoneNumber, bottleCount);
        }

        if (sensor2Value == LOW) {
            Serial.println("------------------------");
            Serial.println("Sensor2 Object Detected!");
            Serial.println("------------------------");
            delay(250);
            bottleCount++;
            Serial.print("Count = ");
            Serial.println(bottleCount);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Bottle count: ");
            lcd.print(bottleCount);

            // Save bottle count to EEPROM with unique address based on the phone number
            addCountToEEPROM(bottleCount, phoneNumber);
            updatePhoneData(phoneNumber, bottleCount);
        }

        if (sensor3Value == LOW) {
            Serial.println("------------------------");
            Serial.println("Sensor3 Object Detected!");
            Serial.println("------------------------");
            delay(250);
            bottleCount++;
            Serial.print("Count = ");
            Serial.println(bottleCount);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Bottle count: ");
            lcd.print(bottleCount);

            // Save bottle count to EEPROM with unique address based on the phone number
            addCountToEEPROM(bottleCount, phoneNumber);
            updatePhoneData(phoneNumber, bottleCount);
        }

        delay(350);

        if (key != NO_KEY && key == '#') {
            lcd.clear();
            bottleCount = 0;
            lcd.print("Enter your phone:");
            lcd.setCursor(0, 1);
            phoneNumber = "";
            Serial.println("Clear Display");
            break; // Break out of the loop
        }
    }
}


void saveCountToEEPROM(int count, String number) {
    int phoneIndex = findPhoneNumberIndex(number);

    if (phoneIndex != -1) {
        // Phone number found, add the count to the existing count
        phoneData[phoneIndex].count += count;
        int address = phoneIndex * sizeof(PhoneData);
        EEPROM.put(address, phoneData[phoneIndex]);
    } else {
        // Phone number not found, create a new entry
        if (phoneDataSize < MAX_PHONES) {
            phoneData[phoneDataSize].phoneNumber = number;
            phoneData[phoneDataSize].count = count;
            int address = phoneDataSize * sizeof(PhoneData);
            EEPROM.put(address, phoneData[phoneDataSize]);
            phoneDataSize++;
        } else {
            Serial.println("Maximum phone numbers reached!");
        }
    }
}




void addScoreToPhoneNumber(String number, int newScore) {
    int index = findPhoneNumberIndex(number);

    if (index != -1) {
        // Phone number found, add the new score to the existing score
        phoneData[index].count += newScore;
        int address = index * sizeof(PhoneData);
        EEPROM.put(address, phoneData[index]);
        Serial.println("Score added to existing phone number!");
    } else {
        Serial.println("Phone number not found!");
    }
}



// This function is missing in the provided code
void addCountToEEPROM(int count, String number) {
    int phoneIndex = findPhoneNumberIndex(number);

    if (phoneIndex != -1) {
        // Phone number found, add the count to the existing count
        phoneData[phoneIndex].count += count;
        int address = phoneIndex * sizeof(PhoneData);
        EEPROM.put(address, phoneData[phoneIndex]);
    } else {
        // Phone number not found, create a new entry
        if (phoneDataSize < MAX_PHONES) {
            phoneData[phoneDataSize].phoneNumber = number;
            phoneData[phoneDataSize].count = count;
            int address = phoneDataSize * sizeof(PhoneData);
            EEPROM.put(address, phoneData[phoneDataSize]);
            phoneDataSize++;
        } else {
            Serial.println("Maximum phone numbers reached!");
        }
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

 void displayTotalCountOnLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Phone:");

    for (int i = 0; i < phoneDataSize; ++i) {
        lcd.setCursor(6, i);
        lcd.print(phoneData[i].phoneNumber);
        lcd.setCursor(0, i + 1);
        lcd.print("Count: ");
        lcd.print(phoneData[i].count);
    }

    delay(7500);
    lcd.clear();
    lcd.print("Enter your phone");
    lcd.setCursor(0, 1);
    phoneNumber = "";
}

int getTotalCount(String number) {
    int total = 0;
    for (int i = 0; i < phoneDataSize; ++i) {
        if (phoneData[i].phoneNumber == number) {
            total += phoneData[i].count;
        }
    }
    return total;
}


 void checkPoints() {
    int index = findPhoneNumberIndex(phoneNumber);

    if (validatePhoneNumber(phoneNumber)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Phone:");
        lcd.print(phoneNumber);

        lcd.setCursor(0, 1);
        lcd.print("Total Count: ");

        // อัปเดตข้อมูลจาก EEPROM
        readCountFromEEPROM();
        
        // นับคะแนนทั้งหมด
        int total = getTotalCount(phoneNumber);
        lcd.print(total);

        Serial.print("Phone: ");
        Serial.print(phoneNumber);
        Serial.print(", Total Count: ");
        Serial.println(total);

        delay(5000);
        lcd.clear();
        lcd.print("Enter your phone");
        Serial.println("Clear Check");
        phoneNumber = "";
    } else {
        Serial.println("Invalid phone number entered!");
        lcd.clear();
        lcd.print("Invalid phone number");
        delay(2000);
        lcd.clear();
        lcd.print("Enter your phone");
    }
}


bool validatePhoneNumber(String number) {
    // ตรวจสอบว่าเบอร์โทรศัพท์มีความยาว 10 ตัวและประกอบไปด้วยตัวเลขเท่านั้น
    return (number.length() == 10 && number.toInt() != 0);
}