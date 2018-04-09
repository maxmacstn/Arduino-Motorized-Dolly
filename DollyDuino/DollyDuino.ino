/**
    @file    DollyDuino
    @author  Sitinut Waisara (maxmacstn)
    @date    April,7 2018
    @version 2.0

    Automated camera dolly control using Arduino.
    https://github.com/maxmacstn/Arduino-Motorized-Dolly
    https://maxmacstn.wordpress.com/2018/03/09/dollyduino/


*/


#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define enA 2                       //PWM Motor control
#define in1 24                      //Motor rotational control1
#define in2 26                      //Motor rotational control2
#define limSW_L 28                  //Left limit switch
#define limSW_R 30                  //Right limit switch
#define shutter 32                  //Camera shutter trigger pin
#define btnRIGHT  0 
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define brightnessPin 10            //LCD Brightness control (Connected from the LCD Keypad shield)


const char* menuItems[] = {"     VIDEO             ", "   TIMELAPSE           ", "    SETTINGS   "};
const unsigned int menuItemsNumber = 3;
const unsigned int sleepTime[] = {0, 10, 30, 120, 300};
unsigned long lastActiveTime = 0;
byte lastPotValue = 255;
byte currentBrightness = 255;
byte motorDir = 0;     // positive = motor is on the left of the controller
byte cameraDelay = 0;  //camera delay between shots (Seconds)
byte dimChoice = 0;    //choice of time until screen was dim.

byte updn_arrow[] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B11111,
  B01110,
  B00100,
  B00000
};

byte left_arrow[] = {

  B00010,
  B00110,
  B01110,
  B11110,
  B01110,
  B00110,
  B00010,
  B00000
};

byte right_arrow[] = {
  B10000,
  B11000,
  B11100,
  B11110,
  B11100,
  B11000,
  B10000,
  B00000
};

byte move_left[] = {
  B00010,
  B00100,
  B01000,
  B10000,
  B01000,
  B00100,
  B00010,
  B00000
};

byte move_right[] = {
  B01000,
  B00100,
  B00010,
  B00001,
  B00010,
  B00100,
  B01000,
  B00000
};



// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
volatile int rotDirection = 0;
int pressed = false;

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  resetDisplaySleep();

  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;

  return btnNONE;  // when all others fail, return this...
}

void video_mode() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Speed");
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
  lcd.setCursor(1, 1);
  lcd.print("stop");

  resetDisplaySleep();
  checkDisplaySleep();
  bool dir;

  while (1) {

    int potValue = analogRead(A15); // Read potentiometer value
    int pwmOutput = map(potValue, 0, 1023, 0 , 255); // Map the potentiometer value from 0 to 255
    int pwmPercent = map(potValue, 0, 1023, 0, 100); // Map pot to percent for display
    Serial.println(potValue);
    analogWrite(enA, pwmOutput); // Send PWM signal to L298N Enable pin

    // Check limit switch - Debounce
    if (!digitalRead(limSW_L)) {
      rotDirection = 1;
    }
    if (!digitalRead(limSW_R)) {
      rotDirection = 0;
    }
    if (read_LCD_buttons() == btnLEFT) {
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      lcd.clear();
      return;
    }
    if (read_LCD_buttons() == btnSELECT) {
      while (read_LCD_buttons() == btnSELECT) {}
      rotDirection = !rotDirection;
    }
    delay(20);
    // If button is pressed - change rotation direction
    if (rotDirection == 0) {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      delay(20);
    }
    // If button is pressed - change rotation direction
    if (rotDirection == 1) {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      delay(20);
    }

    //Update LCD
    if (pwmPercent >= 100) {
      lcd.setCursor(9, 0);
      lcd.print(pwmPercent);
    }
    else if (pwmPercent >= 10) {
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(10, 0);
      lcd.print(pwmPercent);

    } else {
      lcd.setCursor(9, 0);
      lcd.print("  ");
      lcd.setCursor(11, 0);
      lcd.print(pwmPercent);
    }

    lcd.setCursor(12, 0);
    lcd.print("%");

    lcd.setCursor(0, 0);
    lcd.print("   ");
    lcd.setCursor(13, 0);
    lcd.print("   ");

    //Arrow animation speed.
    int anim_shift = millis() / 1000 % 3;

    if (pwmPercent >= 90) {
      anim_shift = millis() / 500 % 3;
    }

    if (pwmPercent <= 40) {
      anim_shift = millis() / 1500 % 3;
    }
    if (pwmPercent <= 10) {
      anim_shift = millis() / 2500 % 3;
    }

    dir = rotDirection;
    if (motorDir)
      dir = !dir;

    if (!dir) {    //Motor is on the left
      lcd.setCursor(2 - anim_shift, 0);
      lcd.write((byte)3);
    }
    else {
      lcd.setCursor(13 + anim_shift, 0);
      lcd.write((byte)4);
    }

    checkDisplaySleep();

  }
}

//Load user-saved settings from EEPROM into global variable
void loadSaveData() {
  byte loadedMotorDir = EEPROM.read(0);
  byte loadedCameraDelay = EEPROM.read(1);
  byte loadedDimChoice = EEPROM.read(2);

  // Reset EEPROM Value to 0 if the loaded value is not valid.
  if ( loadedMotorDir > 1) {
    loadedMotorDir = 0;
    EEPROM.write(0, 0);
  }
  if ( loadedCameraDelay > 20) {
    loadedCameraDelay = 0;
    EEPROM.write(1, 0);
  }

  if ( loadedDimChoice > 4) {
    loadedDimChoice = 0;
    EEPROM.write(2, 0);
  }

  motorDir = loadedMotorDir;
  cameraDelay = loadedCameraDelay;
  dimChoice = loadedDimChoice;
}

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(shutter, OUTPUT);

  // Init Limit sensor and set built-in pullup
  pinMode(limSW_L, INPUT);
  pinMode(limSW_R, INPUT);
  digitalWrite(limSW_L, HIGH);
  digitalWrite(limSW_R, HIGH);

  // Set initial rotation direction
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  pinMode(brightnessPin, OUTPUT);
  digitalWrite(brightnessPin, HIGH);

  //Serial.begin(9600);

  //Create custom characters
  lcd.createChar(0, left_arrow);
  lcd.createChar(1, right_arrow);
  lcd.createChar(2, updn_arrow);
  lcd.createChar(3, move_left);
  lcd.createChar(4, move_right);

  //init led screen
  lcd.begin(16, 2);              // start the library
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" DollyDuino 2.0"); // print a Welcome message
  lcd.setCursor(3, 1);
  lcd.print("Starting...."); // print a Welcome message
  loadSaveData();
  delay(1000);
}

void loop() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Main Menu");
  int currentMenu = 0;
  lcd.setCursor(0, 1);
  lcd.print(menuItems[0]);
  Serial.println("MainLoop");
  lcd.setCursor(15, 1);
  lcd.write((byte)1);
  resetDisplaySleep();
  while (1) {
    if (read_LCD_buttons() == btnRIGHT && currentMenu != menuItemsNumber - 1 ) {
      while (read_LCD_buttons() == btnRIGHT) {};
      update_lcd_menu(currentMenu + 1, true);
      currentMenu++;

    }
    if (read_LCD_buttons() == btnLEFT && currentMenu != 0) {
      while (read_LCD_buttons() == btnLEFT) {};
      update_lcd_menu(currentMenu - 1, false);
      currentMenu--;
    }
    if (read_LCD_buttons() == btnSELECT) {
      while (read_LCD_buttons() == btnSELECT) {};
      switch (currentMenu) {
        case 0: video_mode(); break;
        case 1: time_lapse(); break;
        case 2: setting_page(); break;
      }
      break;
    }

    checkDisplaySleep();

  }




}

void update_lcd_menu(int targetMenu, boolean isIncrement) {
  checkDisplaySleep();

  int scroll_speed = 10;

  //Text scroll animation
  if (isIncrement == true) {                  //Right to left
    //Move previous text out of screen
    for (int i = 0; i > -16; i--) {

      lcd.setCursor(0, 1);
      //lcd.print("                ");
      lcd.setCursor(i, 1);
      lcd.print(menuItems[targetMenu - 1]);
      delay(scroll_speed);
    }
    for (int i = 16; i >= 0; i--) {
      lcd.setCursor(0, 1);
      //lcd.print("                ");
      lcd.setCursor(i, 1);
      lcd.print(menuItems[targetMenu]);
      delay(scroll_speed);
    }
  }
  else {                                      //Left to right
    //Move previous text out of screen
    for (int i = 0; i < 16; i++) {
      lcd.setCursor(0, 1);
      //lcd.print("                ");
      lcd.setCursor(i, 1);
      lcd.print(menuItems[targetMenu + 1]);
      delay(scroll_speed);
    }
    for (int i = -16; i <= 0; i++) {
      lcd.setCursor(0, 1);
      //lcd.print("                ");
      lcd.setCursor(i, 1);
      lcd.print(menuItems[targetMenu]);
      delay(scroll_speed);
    }

  }

  //Draw arrow
  if (targetMenu != 0) {
    lcd.setCursor(0, 1);
    lcd.write((byte)0);
  }
  if (targetMenu != menuItemsNumber - 1) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }

  return;

}


void time_lapse() {
  resetDisplaySleep();
  checkDisplaySleep();
  lcd.clear();
  byte int_sec = 1;
  int step_sec = 500;
  while (1) {
    lcd.setCursor(0, 0);
    lcd.print("Set interval");
    lcd.setCursor(15, 0);
    lcd.print("s");
    lcd.setCursor(0, 1);
    lcd.write((byte)0);
    lcd.setCursor(1, 1);
    lcd.print("Back");

    lcd.setCursor(6, 1);
    lcd.write((byte)2);
    lcd.setCursor(7, 1);
    lcd.print("Set");

    lcd.setCursor(11, 1);
    lcd.print("Next");
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
    boolean value_update = false;
    if (read_LCD_buttons() == btnUP) {
      while (read_LCD_buttons() == btnUP) {}
      if (int_sec < 100) {
        int_sec++;
        value_update = true;
      }
    }
    if (read_LCD_buttons() == btnDOWN) {
      while (read_LCD_buttons() == btnDOWN) {}
      if (int_sec > 1) {
        int_sec--;
        value_update = true;
      }
    }
    if (read_LCD_buttons() == btnLEFT) {
      lcd.clear();
      return;
    }

    //Blink and update LCD
    if (millis() % 1000 > 500 || value_update == true) {
      if (int_sec >= 10) {
        lcd.setCursor(13, 0);
      }
      else {
        lcd.setCursor(13, 0);
        lcd.print(" ");
        lcd.setCursor(14, 0);
      }
      lcd.print(int_sec);
    }
    else {
      lcd.setCursor(13, 0);
      lcd.print("  ");
    }

    //Continue to step 2
    if (read_LCD_buttons() == btnRIGHT) {
      lcd.clear();
      while (read_LCD_buttons() == btnRIGHT) {};
      while (1) {
        lcd.setCursor(0, 0);
        lcd.print("Set move");
        lcd.setCursor(14, 0);
        lcd.print("ms");
        lcd.setCursor(0, 1);
        lcd.write((byte)0);
        lcd.setCursor(1, 1);
        lcd.print("Back");

        lcd.setCursor(6, 1);
        lcd.write((byte)2);
        lcd.setCursor(7, 1);
        lcd.print("Set");

        lcd.setCursor(12, 1);
        lcd.print("Run");
        lcd.setCursor(15, 1);
        lcd.write((byte)1);
        boolean value_update = false;
        if (read_LCD_buttons() == btnUP) {
          while (read_LCD_buttons() == btnUP) {}
          if (step_sec < 9000) {
            step_sec += 100;
            value_update = true;
          }
        }
        if (read_LCD_buttons() == btnDOWN) {
          while (read_LCD_buttons() == btnDOWN) {}
          if (step_sec > 100) {
            step_sec -= 100;
            value_update = true;
          }
        }

        //Blink and update LCD
        if (millis() % 1000 > 500 || value_update == true) {
          if (step_sec >= 1000) {
            lcd.setCursor(10, 0);
          }
          else {
            lcd.setCursor(10, 0);
            lcd.print("   ");
            lcd.setCursor(11, 0);
          }
          lcd.print(step_sec);
        }
        else {
          lcd.setCursor(10, 0);
          lcd.print("    ");
        }
        if (read_LCD_buttons() == btnLEFT) {
          while (read_LCD_buttons() == btnLEFT) {};
          lcd.clear();
          break;
        }
        if (read_LCD_buttons() == btnRIGHT) {
          lcd.clear();
          do_timeLapse(int_sec + cameraDelay, step_sec);
        }
        checkDisplaySleep();

      }
      checkDisplaySleep();

    }//End step2
    checkDisplaySleep();
  }

}

void do_timeLapse(char int_sec, int step_sec) {
  unsigned long startShootMillis = 0;
  unsigned long startMoveMillis = 0;
  boolean isShot = false;
  boolean isShooting = false;
  boolean isFirstEnter = true;
  unsigned int shotCount = 0;
  lcd.setCursor(0, 0);
  lcd.print("TimeLapse");
  lcd.setCursor(13, 0);
  lcd.print("img");
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
  lcd.setCursor(1, 1);
  lcd.print("Stop");

  resetDisplaySleep();
  checkDisplaySleep();

  while (1) {
    lcd.setCursor(10, 0);
    lcd.print(shotCount);
    int potValue = analogRead(A15); // Read potentiometer value
    int pwmOutput = map(potValue, 0, 1023, 0 , 255); // Map the potentiometer value from 0 to 255
    unsigned long currentMillis = millis();
    Serial.println(potValue);
    analogWrite(enA, pwmOutput); // Send PWM signal to L298N Enable pin

    // Check limit switch - Debounce
    if (!digitalRead(limSW_L)) {
      rotDirection = 1;
    }
    if (!digitalRead(limSW_R)) {
      rotDirection = 0;
    }
    if (read_LCD_buttons() == btnLEFT) {
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      lcd.clear();
      return;
    }
    Serial.print("startShootMillis = ");
    Serial.println(startShootMillis);
    Serial.print("startMoveMillis = ");
    Serial.println(startMoveMillis);
    Serial.print("CurrentMillis = ");
    Serial.println(currentMillis);

    // exceed delay time of move
    if (currentMillis - startMoveMillis >= step_sec) {
      Serial.println("Exceed move time");
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      if (isFirstEnter == true) {
        startShootMillis = millis();
        currentMillis = millis();
        isFirstEnter = false;
      }
      // exceed delay time of shooting
      if (currentMillis - startShootMillis  >= int_sec * 1000) {
        startMoveMillis = millis();
        Serial.println("Exceed shoot time ...................");
        Serial.println(startShootMillis -  currentMillis);
        isFirstEnter = true;
        isShot = true;

      } else if (isShot == true) {
        delay(100);
        digitalWrite(shutter, HIGH);
        delay(100);
        digitalWrite(shutter, LOW);
        isShot = false;
        shotCount++;
      }

    } else {  // Move motor
      if (rotDirection == 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        // delay(20);
      }
      // If button is pressed - change rotation direction
      if (rotDirection == 1) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        //delay(20);
      }

      checkDisplaySleep();
    }
    checkDisplaySleep();
  }
}

void setting_page() {
  lcd.clear();
  byte newMotorDir = motorDir;
  while (1) {
    lcd.setCursor(0, 0);
    lcd.print("Motor pos.");




    lcd.setCursor(0, 1);
    lcd.write((byte)2);
    lcd.setCursor(1, 1);
    lcd.print("Set");

    lcd.setCursor(11, 1);
    lcd.print("Next");
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
    boolean value_update = false;
    if (read_LCD_buttons() == btnUP || read_LCD_buttons() == btnDOWN ) {
      while (read_LCD_buttons() == btnUP || read_LCD_buttons() == btnDOWN) {}
      newMotorDir = !newMotorDir;
      value_update = true;

    }
    if (read_LCD_buttons() == btnLEFT) {
      lcd.clear();
      return;
    }

    //Blink and update LCD
    if (millis() % 1000 > 500 || value_update == true) {
      lcd.setCursor(11, 0);
      if (newMotorDir == 0)
        lcd.print("LEFT ");
      else
        lcd.print("RIGHT");
    }
    else {
      lcd.setCursor(11, 0);
      lcd.print("     ");
    }

    //Continue to step 2
    if (read_LCD_buttons() == btnRIGHT) {
      lcd.clear();
      byte newValue = cameraDelay;
      while (read_LCD_buttons() == btnRIGHT) {};
      while (1) {
        lcd.setCursor(0, 0);
        lcd.print("TL. Delay");
        lcd.setCursor(15, 0);
        lcd.print("s");



        lcd.setCursor(0, 1);
        lcd.write((byte)2);
        lcd.setCursor(1, 1);
        lcd.print("Set");

        lcd.setCursor(11, 1);
        lcd.print("Next");
        lcd.setCursor(15, 1);
        lcd.write((byte)1);
        boolean value_update = false;

        if (read_LCD_buttons() == btnUP) {
          while (read_LCD_buttons() == btnUP) {}
          if (newValue < 20)
            newValue++;
          value_update = true;
        }
        if (read_LCD_buttons() == btnDOWN) {
          while (read_LCD_buttons() == btnDOWN) {}
          if (newValue > 0)
            newValue--;
        }

        //Blink and update LCD
        if (millis() % 1000 > 500 || value_update == true) {
          lcd.setCursor(12, 0);
          lcd.print(newValue);
        }
        else {
          lcd.setCursor(12, 0);
          lcd.print("   ");
        }

        //set Auto Dim
        if (read_LCD_buttons() == btnRIGHT) {
          while (read_LCD_buttons() == btnRIGHT) {}
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Screen dim");
          lcd.setCursor(15, 0);
          lcd.print("s");



          lcd.setCursor(0, 1);
          lcd.write((byte)2);
          lcd.setCursor(1, 1);
          lcd.print("Set");

          lcd.setCursor(11, 1);
          lcd.print("SAVE");
          lcd.setCursor(15, 1);
          lcd.write((byte)1);
          int newDimChoice = dimChoice;

          while (1) {
            boolean value_update = false;
            if (read_LCD_buttons() == btnUP) {
              while (read_LCD_buttons() == btnUP) {}
              if (newDimChoice < 4)
                newDimChoice++;
              value_update = true;
            }
            if (read_LCD_buttons() == btnDOWN) {
              while (read_LCD_buttons() == btnDOWN) {}
              if (newDimChoice > 0)
                newDimChoice--;
            }

            //Blink and update LCD
            if (millis() % 1000 > 500 || value_update == true) {
              lcd.setCursor(12, 0);
              if (!newDimChoice){
                lcd.print("--");
              }
              else
                lcd.print(sleepTime[newDimChoice]);
            }
            else {
              lcd.setCursor(12, 0);
              lcd.print("   ");
            }

            if (read_LCD_buttons() == btnRIGHT) {
              while (read_LCD_buttons() == btnRIGHT) {}
              lcd.clear();
              EEPROM.write(0, newMotorDir);
              EEPROM.write(1, newValue);
              EEPROM.write(2, newDimChoice);
              motorDir = newMotorDir;
              cameraDelay = newValue;
              dimChoice = newDimChoice;
              lcd.setCursor(5, 0);
              lcd.print("Saved!");
              delay(1000);
              return;
            }
            checkDisplaySleep();

          }
        }
        checkDisplaySleep();
      }
      checkDisplaySleep();
    }//End step2
    checkDisplaySleep();
  }
}

void checkDisplaySleep() {
  if (dimChoice == 0){
    analogWrite(brightnessPin, 255);
    return;
  }
  int potValue = analogRead(A15); // Read potentiometer value
  int pwmOutput = map(potValue, 0, 1023, 0 , 255); // Map the potentiometer value from 0 to 255

  if (abs(pwmOutput - lastPotValue) > 5) {
    lastPotValue = pwmOutput;
    resetDisplaySleep();
  }

  if (millis() - lastActiveTime > sleepTime[dimChoice] * 1000){
    for (int i = currentBrightness; i >=10 ; i--){
       analogWrite(brightnessPin, i);
       delay(1);

    }
    currentBrightness = 10;
  }
  else {
    for (int i = currentBrightness; i <= 255 ; i++){
       analogWrite(brightnessPin, i);
       delay(1);

    }
    currentBrightness = 255;
  }


}

void resetDisplaySleep() {
  lastActiveTime = millis();
}




