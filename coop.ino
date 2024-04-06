#include <Wire.h> 
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Stepper.h>


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

#define STATE_MAIN_MENU 0
#define STATE_DOOR_MENU 1
#define STATE_CLOCK_MENU 2
#define STATE_OPENING 3
#define STATE_CLOSING 4
#define STATE_OPEN_CLOCK 5
#define STATE_CLOSE_CLOCK 6

int currentState;

void setup() {
	// initialize the LCD, 
	lcd.begin();
 	lcd.backlight();

  //welcome screen
  lcd.setCursor(4,0);
  lcd.print("The Coop");
  lcd.setCursor(0,1);
  lcd.print("* For Main Menu");
  delay(5000);

  //set state to main menu 
  currentState = STATE_MAIN_MENU;
}

void loop() {
  //key = pad.getKey();
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
        // More cases can be added
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
  lcd.print("Time: #Backspace");
  lcd.setCursor(0,1);

  String time = "";
  while (time.length() < 4) {
    char key = pad.getKey();
    if (isdigit(key)) {
      time += key; // Append the digit to the time string
    } else if (key == '#') { // Backspace functionality
      
    } else if (key == '*') {
        mainMenu(); // Go back to the main menu
        break; // Exit the loop to prevent further input
    }
}
  //int timeVal = std::stoi(time);

  //need to get time string as an int and calculate with rtc

  delay(100);
}

void closeClock() {
  lcd.setCursor(0,0);
  lcd.print("Insert Time:");
  lcd.setCursor(0,1);
}


void open() {
  lcd.clear();
  lcd.print("Opening...");
  delay(300); //delay for clearing
  myStepper.setSpeed(5);
  myStepper.step(stepsPerRevolution); //clockwise
	delay(1000);

  //Back to main menu
  currentState = STATE_MAIN_MENU;
}

void close() {
  lcd.clear();
  lcd.print("Closing...");
  delay(300); //delay for clearing
  myStepper.setSpeed(5);
  myStepper.step(-stepsPerRevolution); //counterclockwise
	delay(1000);

  //Back to main menu
  currentState = STATE_MAIN_MENU;
}


