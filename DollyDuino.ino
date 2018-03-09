#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define enA 2
#define in1 24
#define in2 26
#define limSW_L 28
#define limSW_R 30
#define shutter 32
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

const char* menuItems[]={"     VIDEO             ", "   TIMELAPSE           ", "    SETTING    "};
const unsigned int menuItemsNumber = 3;

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
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  

 return btnNONE;  // when all others fail, return this...
}

void video_mode(){
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Speed");
  lcd.setCursor(0,1);
  lcd.write((byte)0);
  lcd.setCursor(1,1);
  lcd.print("stop");
  while(1){
    int potValue = analogRead(A15); // Read potentiometer value
    int pwmOutput = map(potValue, 0, 1023, 0 , 255); // Map the potentiometer value from 0 to 255
    int pwmPercent = map(potValue, 0, 1023, 0, 100); // Map pot to percent for display
    Serial.println(potValue);
    analogWrite(enA, pwmOutput); // Send PWM signal to L298N Enable pin
    
    // Check limit switch - Debounce
    if (!digitalRead(limSW_L)) {
      rotDirection = 1;
    }
    if (!digitalRead(limSW_R)){
      rotDirection = 0;
    }
    if (read_LCD_buttons() == btnLEFT){
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      lcd.clear();
      return;
    }
    if (read_LCD_buttons() == btnSELECT){
      while(read_LCD_buttons() == btnSELECT){}
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
    if (pwmPercent >= 100){
     lcd.setCursor(9,0);
     lcd.print(pwmPercent);
    }
    else if(pwmPercent >= 10){
     lcd.setCursor(9,0);
     lcd.print(" ");
     lcd.setCursor(10,0);
     lcd.print(pwmPercent);

    }else{
     lcd.setCursor(9,0);
     lcd.print("  ");
     lcd.setCursor(11,0);
     lcd.print(pwmPercent);
    }

     lcd.setCursor(12,0);
     lcd.print("%");

      lcd.setCursor(0,0);
      lcd.print("   ");
      lcd.setCursor(13,0);
      lcd.print("   ");
      int anim_shift = millis() / 1000 %3;
      if (rotDirection == 0) {
        lcd.setCursor(2 - anim_shift,0);
        lcd.write((byte)3);
      }if (rotDirection == 1) {
        lcd.setCursor(13 + anim_shift,0);
        lcd.write((byte)4);
      }
      
    
    }
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
 
  Serial.begin(9600);

  //Create custom characters
  lcd.createChar(0, left_arrow);
  lcd.createChar(1, right_arrow);
  lcd.createChar(2, updn_arrow);
  lcd.createChar(3, move_left);
  lcd.createChar(4, move_right);
  
  //init led screen
  lcd.begin(16, 2);              // start the library
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("DollyDuino"); // print a Welcome message
  lcd.setCursor(3,1);
  lcd.print("Starting...."); // print a Welcome message

  delay(1000);
  
}
void loop() {
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Main Menu");
  int currentMenu = 0;
  lcd.setCursor(0,1);
  lcd.print(menuItems[0]);
  Serial.println("MainLoop");
  lcd.setCursor(15,1);
  lcd.write((byte)1);
  while(1){
    if (read_LCD_buttons() == btnRIGHT && currentMenu != menuItemsNumber -1 ){
      while(read_LCD_buttons() == btnRIGHT){};
      update_lcd_menu(currentMenu+1,true);
      currentMenu++;
    }
    if (read_LCD_buttons() == btnLEFT && currentMenu != 0){
      while(read_LCD_buttons() == btnLEFT){};
      update_lcd_menu(currentMenu-1,false);
      currentMenu--;
    }
    if (read_LCD_buttons() == btnSELECT){
      while(read_LCD_buttons() == btnSELECT){};
      switch(currentMenu){
        case 0: video_mode(); break;
        case 1: time_lapse(); break;
      }
      break;
    }
    
  }
  //video_mode();
  //time_lapse();
  
}

void update_lcd_menu(int targetMenu, boolean isIncrement){
    int scroll_speed = 20;

    //Text scroll animation
    if(isIncrement == true){                    //Right to left
      //Move previous text out of screen
      for(int i = 0; i > -16; i--){
        
        lcd.setCursor(0,1);
        //lcd.print("                ");
        lcd.setCursor(i,1);
        lcd.print(menuItems[targetMenu-1]);
        delay(scroll_speed);
      }
      for(int i = 16; i >= 0; i--){
        lcd.setCursor(0,1);
        //lcd.print("                ");
        lcd.setCursor(i,1);
        lcd.print(menuItems[targetMenu]);
        delay(scroll_speed);
      }}
      else{                                       //Left to right
      //Move previous text out of screen
      for(int i = 0; i < 16; i++){
        lcd.setCursor(0,1);
        //lcd.print("                ");
        lcd.setCursor(i,1);
        lcd.print(menuItems[targetMenu+1]);
        delay(scroll_speed);
      }
      for(int i = -16; i <= 0; i++){
        lcd.setCursor(0,1);
        //lcd.print("                ");
        lcd.setCursor(i,1);
        lcd.print(menuItems[targetMenu]);
        delay(scroll_speed);
      }
     
    }

      //Draw arrow
      if (targetMenu != 0){
        lcd.setCursor(0,1);
        lcd.write((byte)0);
      }
      if (targetMenu != menuItemsNumber -1){
        lcd.setCursor(15,1);
         lcd.write((byte)1);
      }

    return;
  
}


void time_lapse(){
  lcd.clear();
  byte int_sec = 1;
  int step_sec = 500;
  while(1){
    lcd.setCursor(0,0);
    lcd.print("Set interval");
    lcd.setCursor(15,0);
    lcd.print("s");
    lcd.setCursor(0,1);
    lcd.write((byte)0);
    lcd.setCursor(1,1);
    lcd.print("Back");
  
    lcd.setCursor(6,1);
    lcd.write((byte)2);
    lcd.setCursor(7,1);
    lcd.print("Set");
  
    lcd.setCursor(11,1);
    lcd.print("Next");
    lcd.setCursor(15,1);
    lcd.write((byte)1);
    boolean value_update = false;
    if(read_LCD_buttons() == btnUP){
      while(read_LCD_buttons() == btnUP){}
      if(int_sec <100){
      int_sec++;
      value_update = true;
      }
    }
    if(read_LCD_buttons() == btnDOWN){
      while(read_LCD_buttons() == btnDOWN){}
      if(int_sec >1){
        int_sec--;
        value_update = true;
      }
    }
    if(read_LCD_buttons() == btnLEFT){
            lcd.clear();
            return;
    }

     //Blink and update LCD
    if(millis()%1000> 500 || value_update == true){
      if(int_sec >=10){
        lcd.setCursor(13,0);
      }
      else{
        lcd.setCursor(13,0);
        lcd.print(" ");
        lcd.setCursor(14,0);
      }
      lcd.print(int_sec);
      }
    else{
      lcd.setCursor(13,0);
      lcd.print("  ");
      }

    //Continue to step 2
    if(read_LCD_buttons() == btnRIGHT){
      lcd.clear();
      while(read_LCD_buttons() == btnRIGHT){};
        while(1){
            lcd.setCursor(0,0);
            lcd.print("Set move");
            lcd.setCursor(14,0);
            lcd.print("ms");
            lcd.setCursor(0,1);
            lcd.write((byte)0);
            lcd.setCursor(1,1);
            lcd.print("Back");
          
            lcd.setCursor(6,1);
            lcd.write((byte)2);
            lcd.setCursor(7,1);
            lcd.print("Set");
          
            lcd.setCursor(12,1);
            lcd.print("Run");
            lcd.setCursor(15,1);
            lcd.write((byte)1);
            boolean value_update = false;
            if(read_LCD_buttons() == btnUP){
              while(read_LCD_buttons() == btnUP){}
              if(step_sec <9000){
              step_sec+= 100;
              value_update = true;
              }
            }
            if(read_LCD_buttons() == btnDOWN){
              while(read_LCD_buttons() == btnDOWN){}
                  if(step_sec >100){
                    step_sec-= 100;
                    value_update = true;
                  }
                }
            
                 //Blink and update LCD
                if(millis()%1000> 500 || value_update == true){
                  if(step_sec >=1000){
                    lcd.setCursor(10,0);
                  }
                  else{
                    lcd.setCursor(10,0);
                    lcd.print("   ");
                    lcd.setCursor(11,0);
                  }
                  lcd.print(step_sec);
                  }
                else{
                  lcd.setCursor(10,0);
                  lcd.print("    ");
                  }
           if(read_LCD_buttons() == btnLEFT){
            while(read_LCD_buttons() == btnLEFT){};
            lcd.clear();
            break;
           }
           if(read_LCD_buttons() == btnRIGHT){
           lcd.clear();
              do_timeLapse(int_sec,step_sec);
           }
          }
          
        }//End step2
  }
 
}

void do_timeLapse(char int_sec, int step_sec){
  unsigned long startShootMillis = 0;
  unsigned long startMoveMillis = 0;
  boolean isShot = false;
  boolean isShooting = false;
  boolean isFirstEnter = true;
  unsigned int shotCount = 0;
  lcd.setCursor(0,0);
  lcd.print("TimeLapse");
  lcd.setCursor(13,0);
  lcd.print("img");
  lcd.setCursor(0,1);
  lcd.write((byte)0);
  lcd.setCursor(1,1);
  lcd.print("Stop");
  while(1){
    lcd.setCursor(10,0);
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
    if (!digitalRead(limSW_R)){
      rotDirection = 0;
    }
    if (read_LCD_buttons() == btnLEFT){
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
    if(currentMillis - startMoveMillis >= step_sec){
      Serial.println("Exceed move time");
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      if (isFirstEnter == true){
        startShootMillis = millis();
        currentMillis = millis();
        isFirstEnter = false;
      }
      // exceed delay time of shooting
      if(currentMillis - startShootMillis  >= int_sec*1000){
        startMoveMillis = millis();
        Serial.println("Exceed shoot time ...................");
        Serial.println(startShootMillis -  currentMillis);
        isFirstEnter = true;
        isShot = true;
        
      }else if (isShot == true){
          delay(100);
          digitalWrite(shutter, HIGH);
          delay(100);
          digitalWrite(shutter, LOW);
          isShot = false;
          shotCount++;
      }
      
    }else{   // Move motor 
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
    /*
    if(isShot == true || currentMillis - startShootMillis < int_sec*1000){
      if(isShooting == false){
        startShootMillis = millis();
        digitalWrite(shutter, HIGH);
        delay(20);
        digitalWrite(shutter, LOW);
        isShooting = true
      }
    }else if( currentMillis - startShootMillis >=  int_sec*1000){

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
      }*/
    }
  }
}



