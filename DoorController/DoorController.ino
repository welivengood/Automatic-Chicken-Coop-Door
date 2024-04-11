#include <Wire.h> 
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include "string.h"


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the two-dimensional array on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad pad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Defines the number of steps per rotation
const int stepsPerRevolution = 2038;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 13, 11, 12, 10);

// uRTCLib rtc; Creates RTC information
uRTCLib rtc(0x68);
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#define STATE_MAIN_MENU 0
#define STATE_DOOR_MENU 1
#define STATE_CLOCK_MENU 2
#define STATE_OPENING 3
#define STATE_CLOSING 4
#define STATE_OPEN_CLOCK 5
#define STATE_CLOSE_CLOCK 6

int currentState;
bool isOpen;

int UPPER_HOUR = 12;
int UPPER_MIN = 59;
int AM_TO_PM = 12;

int openTimeHour;
int openTimeMin;
int openTimeSecond = 0;

int closeTimeHour;
int closeTimeMin;
int closeTimeSec = 0;

void setup() {
	// initialize the LCD, 
	lcd.begin();
 	lcd.backlight();

  //Starts RTC
  URTCLIB_WIRE.begin();

  //welcome screen
  lcd.setCursor(4,0);
  lcd.print("The Coop");
  lcd.setCursor(0,1);
  lcd.print("* For Main Menu");
  delay(5000);

  isOpen = false;

  //set state to main menu 
  currentState = STATE_MAIN_MENU;
}

void loop() {
  rtc.refresh();

  if (rtc.hour() == openTimeHour && rtc.minute() == openTimeMin && !isOpen) {
    lcd.clear();
    lcd.print("Opening Time");
    delay(5000);
    open();
  }

  if (rtc.hour() == closeTimeHour && rtc.minute() == closeTimeMin && isOpen) {
    lcd.clear();
    lcd.print("Closing Time");
    delay(5000);
    close();    
  }

  /*lcd.clear();
  lcd.print("Time: ");
  lcd.print(rtc.hour());
  lcd.print(':');
  lcd.print(rtc.minute());
  lcd.print(':');
  lcd.println(rtc.second());
  delay(1000); */

  switch (currentState) {
        case STATE_MAIN_MENU:
            mainMenu();
            break;
        case STATE_DOOR_MENU:
            doorMenu();
            break;
        case STATE_CLOCK_MENU:
            clockMenu();
            break;
        case STATE_OPEN_CLOCK:
            openClock();
            break;
        case STATE_CLOSE_CLOCK:
            closeClock();
            break;
    } 
/*
  if(key == '1') {
  	// Rotate CW slowly at 5 RPM
	  myStepper.setSpeed(5);
	  myStepper.step(stepsPerRevolution);
	  delay(1000);
	
	  // Rotate CCW quickly at 10 RPM
	  myStepper.setSpeed(10);
	  myStepper.step(-stepsPerRevolution);
	  delay(1000);

  }*/
}

void mainMenu() {
  lcd.setCursor(0,0);
  lcd.print("Door - Press 1");
  lcd.setCursor(0,1);
  lcd.print("Clock - Press 2");

  char key = pad.getKey();
  switch (key) {
    case '1':
      currentState = STATE_DOOR_MENU;
      break;
    case '2':
      currentState = STATE_CLOCK_MENU;
      break;
    case 'A':
      lcd.backlight();
      lcd.display();
      break;
    case 'B':
      lcd.noBacklight();
      lcd.noDisplay();
      break;
  }
}

void doorMenu() {
  lcd.setCursor(0, 0);
  lcd.print("Open - Press 1");
  lcd.setCursor(0, 1);
  lcd.print("Close - Press 2");

  char key = pad.getKey();
  switch (key) {
    case '1':
        currentState = STATE_OPENING;
        open(); // Call the open() function to open the door
        break;
    case '2':
        currentState = STATE_CLOSING;
        close(); // Call the close() function to close the door
        break;
    case '*':
        currentState = STATE_MAIN_MENU;
        mainMenu(); // Go back to the main menu
        break;
    default:
        // Handle other keys or no key pressed
        break;
  }
}

void clockMenu() {
  lcd.setCursor(0,0);
  lcd.print("Open - Press 1");
  lcd.setCursor(0,1);
  lcd.print("Close - Press 2");

  char key = pad.getKey();
  switch (key) {
    case '1':
      currentState = STATE_OPEN_CLOCK;
      openClock();
      break;
    case '2':
      currentState = STATE_CLOSE_CLOCK;
      closeClock();
      break;
    case '*':
      currentState = STATE_MAIN_MENU;
  }

}

void openClock() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  lcd.setCursor(0,1);
  lcd.print("# Back D Done");
  lcd.setCursor(8,0);
  lcd.print(":");

  String time = "";
  while (time.length() < 4) {
    char key = pad.getKey();
    if (isdigit(key)) {
      time += key; // Append the digit to the time string
      lcd.setCursor(6, 0);
        switch (time.length()) {
        case 1:
        lcd.print("  : " + time);
        break;
        case 2:
        lcd.print("  :" + time);    
        break; 
        case 3:
        lcd.print(" " + time.substring(0, 1) + ":" + time.substring(1, time.length()));            
        break; 
        case 4:
        lcd.print(time.substring(0,2) + ":" + time.substring(2, time.length()));  
        break; 
        }
    } else if (key == '#') { // Backspace functionality
      if (time.length() > 0){
        time = time.substring(0,time.length() - 1);
        lcd.setCursor(6, 0);
        switch (time.length()) {
        case 0:
        lcd.setCursor(6, 0);
        lcd.print("  :  ");
        break;
        case 1:
        lcd.print("  : " + time);
        break;
        case 2:
        lcd.print("  :" + time);    
        break; 
        case 3:
        lcd.print(" " + time.substring(0, 1) + ":" + time.substring(1, time.length()));            
        break; 
        case 4:
        lcd.print(time.substring(0,2) + ":" + time.substring(2, time.length()));  
        break; 
        }
      }
    } else if (key == '*') {
        currentState = STATE_MAIN_MENU;
        mainMenu(); // Go back to the main menu
        break; // Exit the loop to prevent further input
    } else if (key == 'D') {
        break;
    }
  }

  delay(2000);
  if (!isTimeValid(time)) {
    lcd.setCursor(6, 0);
    lcd.print("Invalid.");
    delay(2000);
    currentState = STATE_MAIN_MENU;
    mainMenu();
    return;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("AM - Press 1");
  lcd.setCursor(0, 1);
  lcd.print("PM - Press 2");

  int timeInt = atoi(time.c_str());
  int hours = timeInt / 100;
  int minutes = timeInt % 100;

  char key2 = pad.waitForKey();
    switch (key2) {
    case '1':
      setOpenTime(hours, minutes);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Opening Time");
      lcd.setCursor(0, 1);
      lcd.print("Set to ");
      lcd.print(hours);
      lcd.print(":");
      lcd.print(minutes);
      lcd.print("AM");
      delay(5000);
      currentState = STATE_MAIN_MENU;
      mainMenu(); // Go back to the main menu
      break;
    case '2':
      setOpenTime(hours + 12 , minutes);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Opening Time");
      lcd.setCursor(0, 1);
      lcd.print("Set to ");
      lcd.print(hours);
      lcd.print(":");
      lcd.print(minutes);
      lcd.print("PM");
      delay(5000);
      currentState = STATE_MAIN_MENU;
      mainMenu(); // Go back to the main menu
      break;
    case '*':
      currentState = STATE_MAIN_MENU;
      mainMenu(); // Go back to the main menu
      break; // Exit the loop to prevent further input
  }


  //int timeVal = std::stoi(time);

  //need to get time string as an int and calculate with rtc

  delay(100);
}

void closeClock() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  lcd.setCursor(0,1);
  lcd.print("# Back D Done");
  lcd.setCursor(8,0);
  lcd.print(":");

  String time = "";
  while (time.length() < 4) {
    char key = pad.getKey();
    if (isdigit(key)) {
      time += key; // Append the digit to the time string
      lcd.setCursor(6, 0);
        switch (time.length()) {
        case 1:
        lcd.print("  : " + time);
        break;
        case 2:
        lcd.print("  :" + time);    
        break; 
        case 3:
        lcd.print(" " + time.substring(0, 1) + ":" + time.substring(1, time.length()));            
        break; 
        case 4:
        lcd.print(time.substring(0,2) + ":" + time.substring(2, time.length()));  
        break; 
        }
    } else if (key == '#') { // Backspace functionality
      if (time.length() > 0){
        time = time.substring(0,time.length() - 1);
        lcd.setCursor(6, 0);
        switch (time.length()) {
        case 0:
        lcd.setCursor(6, 0);
        lcd.print("  :  ");
        break;
        case 1:
        lcd.print("  : " + time);
        break;
        case 2:
        lcd.print("  :" + time);    
        break; 
        case 3:
        lcd.print(" " + time.substring(0, 1) + ":" + time.substring(1, time.length()));            
        break; 
        case 4:
        lcd.print(time.substring(0,2) + ":" + time.substring(2, time.length()));  
        break; 
        }
      }
    } else if (key == '*') {
        currentState = STATE_MAIN_MENU;
        mainMenu(); // Go back to the main menu
        break; // Exit the loop to prevent further input
    } else if (key == 'D') {
        break;
    }
  }

  delay(2000);
  if (!isTimeValid(time)) {
    lcd.setCursor(6, 0);
    lcd.print("Invalid.");
    delay(2000);
    currentState = STATE_MAIN_MENU;
    mainMenu();
    return;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("AM - Press 1");
  lcd.setCursor(0, 1);
  lcd.print("PM - Press 2");

  int timeInt = atoi(time.c_str());
  int hours = timeInt / 100;
  int minutes = timeInt % 100;

  char key2 = pad.waitForKey();
    switch (key2) {
    case '1':
      setCloseTime(hours, minutes);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Closing Time");
      lcd.setCursor(0, 1);
      lcd.print("Set to ");
      lcd.print(hours);
      lcd.print(":");
      lcd.print(minutes);
      lcd.print("AM");
      delay(5000);
      currentState = STATE_MAIN_MENU;
      mainMenu(); // Go back to the main menu
      break;
    case '2':
      setCloseTime(hours + 12 , minutes);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Closing Time");
      lcd.setCursor(0, 1);
      lcd.print("Set to ");
      lcd.print(hours);
      lcd.print(":");
      lcd.print(minutes);
      lcd.print("PM");
      delay(5000);
      currentState = STATE_MAIN_MENU;
      mainMenu(); // Go back to the main menu
      break;
    case '*':
      currentState = STATE_MAIN_MENU;
      mainMenu(); // Go back to the main menu
      break; // Exit the loop to prevent further input
  }


  //int timeVal = std::stoi(time);

  //need to get time string as an int and calculate with rtc

  delay(100);
}


void open() {
  if (!isOpen) {
    lcd.clear();
    lcd.print("Opening...");
    delay(300); //delay for clearing
    myStepper.setSpeed(10);
    myStepper.step(19000); //clockwise
    delay(1000);
    isOpen = true;
  }

  //Back to main menu
  currentState = STATE_MAIN_MENU;
}

void close() {
  if (isOpen) {
    lcd.clear();
    lcd.print("Closing...");
    delay(300); //delay for clearing
    myStepper.setSpeed(10);
    myStepper.step(-18000); //counterclockwise
    delay(1000);
    isOpen = false;
  }

  //Back to main menu
  currentState = STATE_MAIN_MENU;
}

bool isTimeValid(String timeString) {
  if (timeString.length() == 0){
  return (false);

  }
  int time = atoi(timeString.c_str());
  int hours = time / 100;
  int minutes = time % 100;
  if (hours <= 0 || hours > UPPER_HOUR) {
    return false;
  }
  if (minutes < 0 || minutes > UPPER_MIN) {
    return false;
  }

  if (time > 0){
    return(true);
  }
}

void setOpenTime(int hours, int min) {
  openTimeHour = hours;
  openTimeMin = min;
}

void setCloseTime(int hours, int min) {
  closeTimeHour = hours;
  closeTimeMin = min;
}

