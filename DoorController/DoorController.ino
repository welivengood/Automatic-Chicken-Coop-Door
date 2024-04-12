/**
 * @file DoorController.cpp
 * @brief Implementation of a automatic chicken coop door controller system.
 *
 * This file contains the implementation of a door controller system using Arduino.
 * It includes functionality to control a stepper motor to open and close a door,
 * interact with a keypad and LCD for user input and display, and set opening and
 * closing times for the door based on an RTC (Real-Time Clock) module.
 * The system operates in a finite state machine with various states such as the
 * main menu, door menu, clock menu, opening, and closing states.
 *
 * @author William Livengood
 * @author Dylan Barrett
 * @date 04/07/2024
 */

#include <Wire.h> 
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include "string.h"

// 4x4 Keypad:
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
// Initialize an instance of class NewKeypad
Keypad pad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


// LCD:
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);


// Stepper Motor:
// Defines the number of steps per rotation
const int stepsPerRevolution = 2038;
// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper stepper = Stepper(stepsPerRevolution, 13, 11, 12, 10);
const int openSteps = 19000;
const int closeSteps = -18000;


// RTC:
// uRTCLib rtc; Creates RTC information
uRTCLib rtc(0x68);
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Defines states for DoorController Finite State Machine
#define STATE_MAIN_MENU 0
#define STATE_DOOR_MENU 1
#define STATE_CLOCK_MENU 2
#define STATE_OPENING 3
#define STATE_CLOSING 4
#define STATE_OPEN_CLOCK 5
#define STATE_CLOSE_CLOCK 6

// Set to one of the STATE_* constants
int currentState;

// Is open is updated with open() and close() 
bool isOpen;

// Define constants for time verification 
const int UPPER_HOUR = 12;
const int UPPER_MIN = 59;
const int AM_TO_PM = 12;

// Variables which can be set in STATE_OPEN_CLOCK  
int openTimeHour;
int openTimeMin;
int openTimeSecond = 0;

// Variables which can be set in STATE_CLOSE_CLOCK
int closeTimeHour;
int closeTimeMin;
int closeTimeSec = 0;


/**
 * @brief Initializes the system.
 *
 * This function initializes the LCD, RTC, and sets up the initial state of the system.
 * It displays a welcome message on the LCD and sets the initial state to the main menu.
 */
 void setup() {
	// Initialize the LCD, 
	lcd.begin();
 	lcd.backlight();

  // Starts RTC
  URTCLIB_WIRE.begin();

  // Welcome screen
  lcd.setCursor(4,0);
  lcd.print("The Coop");
  lcd.setCursor(0,1);
  lcd.print("* For Main Menu");
  delay(5000);

  // The door should be closed when the DoorController is first powered
  isOpen = false;

  // Set state to main menu 
  currentState = STATE_MAIN_MENU;
}

/**
 * @brief Main program loop.
 *
 * This function is the main program loop. It refreshes the RTC, checks if it's time to open or close the door,
 * and handles the current state of the system by calling corresponding functions based on the currentState variable.
 */
void loop() {
  //
  rtc.refresh();

  // Checks if the current time is the opening time
  if (rtc.hour() == openTimeHour && rtc.minute() == openTimeMin && !isOpen) {
    lcd.clear();
    lcd.print("Opening Time");
    delay(5000);
    open();
  }

  // Checks if the current time is the closing time
  if (rtc.hour() == closeTimeHour && rtc.minute() == closeTimeMin && isOpen) {
    lcd.clear();
    lcd.print("Closing Time");
    delay(5000);
    close();    
  }

  // Based on the the DoorController's state, different menu methods are called
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
}

/**
 * @brief Displays the main menu on the LCD and handles user input.
 *
 * This function displays the main menu options on the LCD screen and waits for user input.
 * If the user presses '1', it sets the current state to door menu.
 * If the user presses '2', it sets the current state to clock menu.
 * If the user presses 'A', it turns on the LCD backlight and display.
 * If the user presses 'B', it turns off the LCD backlight and display.
 */
void mainMenu() {
  // Prints the main menu
  lcd.setCursor(0,0);
  lcd.print("Door - Press 1");
  lcd.setCursor(0,1);
  lcd.print("Clock - Press 2");

  // Waits for user input to change the state
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

/**
 * @brief Displays the door menu on the LCD and handles user input.
 *
 * This function displays the options to open or close the door on the LCD screen and waits for user input.
 * If the user presses '1', it sets the current state to opening and calls the open() function to open the door.
 * If the user presses '2', it sets the current state to closing and calls the close() function to close the door.
 * If the user presses '*', it returns to the main menu.
 */
void doorMenu() {
  // Prints the door menu
  lcd.setCursor(0, 0);
  lcd.print("Open - Press 1");
  lcd.setCursor(0, 1);
  lcd.print("Close - Press 2");

  //Waits for user input to move the door or to go back to main menu
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


/**
 * @brief Displays the clock menu on the LCD and handles user input.
 *
 * This function displays the options to open or close the clock menu on the LCD screen and waits for user input.
 * If the user presses '1', it sets the current state to open clock and calls the openClock() function to set the opening time.
 * If the user presses '2', it sets the current state to close clock and calls the closeClock() function to close the clock.
 * If the user presses '*', it returns to the main menu.
 */
void clockMenu() {
  // Prints the clock menu
  lcd.setCursor(0,0);
  lcd.print("Open - Press 1");
  lcd.setCursor(0,1);
  lcd.print("Close - Press 2");

  // Waits for user input to change the state and go to a new menu
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

/**
 * @brief Allows the user to set the opening time for the door.
 *
 * This function prompts the user to input the opening time for the door on the LCD screen.
 * The user can enter the time using the keypad, with options for hours (0-23) and minutes (0-59).
 * The user can backspace by pressing '#'. The user confirms the time by pressing 'D'.
 * After entering the time, the user can choose between AM or PM.
 * Once the opening time is set, it displays a confirmation message and returns to the main menu.
 * If the input time is invalid, it displays an error message and returns to the main menu.
 */
void openClock() {

  // Prints the openClock menu
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  lcd.setCursor(0,1);
  lcd.print("# Back D Done");
  lcd.setCursor(8,0);
  lcd.print(":");

  // Instanstiates the time string as a empty string
  String time = getTimeString();
  delay(2000);

  // Prints the AM or PM selection menu
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("AM - Press 1");
  lcd.setCursor(0, 1);
  lcd.print("PM - Press 2");

  // Gets the time string as an integer
  int timeInt = atoi(time.c_str());
  int hours = timeInt / 100; 
  int minutes = timeInt % 100;

  // Waits for user to choose AM or PM, or return to the main menu 
  char key2 = pad.waitForKey();
    switch (key2) {
    // Sets opening time to teh inputted time and displays confirmation 
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
    // Sets opening time to the inputted time + 12 hours (military time) and displays confirmation
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
  delay(100);
}

/**
 * @brief Allows the user to set the closing time for the door.
 *
 * This function prompts the user to input the closing time for the door on the LCD screen.
 * The user can enter the time using the keypad, with options for hours (0-23) and minutes (0-59).
 * The user can backspace by pressing '#'. The user confirms the time by pressing 'D'.
 * After entering the time, the user can choose between AM or PM.
 * Once the closing time is set, it displays a confirmation message and returns to the main menu.
 * If the input time is invalid, it displays an error message and returns to the main menu.
 */
void closeClock() {
  // Prints the closeClock  menu
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  lcd.setCursor(0,1);
  lcd.print("# Back D Done");
  lcd.setCursor(8,0);
  lcd.print(":");

  string time = getTimeString();
  delay(2000);

  // Prints the AM or PM selection menu
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("AM - Press 1");
  lcd.setCursor(0, 1);
  lcd.print("PM - Press 2");

  // Gets the time string as an integer
  int timeInt = atoi(time.c_str());
  int hours = timeInt / 100;
  int minutes = timeInt % 100;

  // Waits for user to choose AM or PM, or return to the main menu 
  char key2 = pad.waitForKey();
    switch (key2) {
    // Sets opening time to teh inputted time and displays confirmation 
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
    // Sets opening time to the inputted time + 12 hours (military time) and displays confirmation
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
  delay(100);
}

/**
 * @brief Retrieves the time string entered by the user.
 * 
 * This function prompts the user to input a time string using the keypad.
 * The user can input digits (0-9) to construct the time string.
 * The time string represents hours and minutes in 24-hour format (HHMM).
 * The user can use the '#' key to delete the last entered digit.
 * The user confirms the time string by pressing the 'D' key.
 * If the user presses '*', the function returns to the main menu.
 * 
 * @return The time string entered by the user.
 */
string getTimeString() {
  // Instanstiates the time string as a empty string
  String time = "";
  while (time.length() < 4) {
    char key = pad.getKey();
    // If it's a digit it gets added to the string
    if (isdigit(key)) {
      time += key; // Append the digit to the time string
      lcd.setCursor(6, 0);
        // Based on the length of the time string it is displayed on the screen differently relative to the colon
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
        // Removes the last character from the time string
        time = time.substring(0,time.length() - 1);
        lcd.setCursor(6, 0);
        // Based on the length of the time string it is displayed on the screen differently relative to the colon
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
    } else if (key == 'D') { // If key is 'D' the time is entered
        break;
    }
  }

  // Checks if the time entered is valid, if not it returns to main menu
  if (!isTimeValid(time)) {
    lcd.setCursor(6, 0);
    lcd.print("Invalid.");
    delay(2000);
    currentState = STATE_MAIN_MENU;
    mainMenu();
    return;
  }

  return time;
}

/**
 * @brief Opens the door if it is not already open.
 *
 * This function checks if the door is already open. If it is closed, it opens the door by
 * rotating the stepper motor in a clockwise direction. After opening the door, it updates the
 * state variable 'isOpen' to true and returns to the main menu state.
 */
void open() {
  if (!isOpen) {
    // Clear the LCD display and show "Opening..." message
    lcd.clear();
    lcd.print("Opening...");
    delay(300); // Delay for clearing

    // Set the speed of the stepper motor and rotate it clockwise to open the door
    stepper.setSpeed(10);
    stepper.step(openSteps); // Clockwise rotation
    delay(1000);

    // Update the state variable to indicate that the door is now open
    isOpen = true;
  }

  // Set the current state back to the main menu
  currentState = STATE_MAIN_MENU;
}


/**
 * @brief Closes the door if it is not already closed.
 *
 * This function checks if the door is already closed. If it is open, it closes the door by
 * rotating the stepper motor in a counterclockwise direction. After closing the door, it updates the
 * state variable 'isOpen' to false and returns to the main menu state.
 */
void close() {
  if (isOpen) {
    // Clear the LCD display and show "Closing..." message
    lcd.clear();
    lcd.print("Closing...");
    delay(300); // Delay for clearing

    // Set the speed of the stepper motor and rotate it counterclockwise to close the door
    stepper.setSpeed(10);
    stepper.step(closeSteps); // Counterclockwise rotation
    delay(1000);

    // Update the state variable to indicate that the door is now closed
    isOpen = false;
  }

  // Set the current state back to the main menu
  currentState = STATE_MAIN_MENU;
}


/**
 * @brief Checks if the given time string is valid.
 *
 * This function validates the given time string by checking if it is empty, and then converting it
 * to an integer representing hours and minutes. It ensures that hours are within the valid range
 * (greater than 0 and not exceeding the defined upper hour limit) and that minutes are within the
 * valid range (between 0 and the defined upper minute limit). If the time is valid, it returns true;
 * otherwise, it returns false.
 * 
 * @param timeString The time string to validate.
 * @return True if the time is valid, false otherwise.
 */
bool isTimeValid(String timeString) {
  if (timeString.length() == 0) {
    return false;
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
  return true;
}

/**
 * @brief Sets the opening time for the door.
 *
 * This function sets the opening time for the door by updating the global variables 'openTimeHour'
 * and 'openTimeMin' with the provided hours and minutes.
 * 
 * @param hours The hour component of the opening time.
 * @param min The minute component of the opening time.
 */
void setOpenTime(int hours, int min) {
  openTimeHour = hours;
  openTimeMin = min;
}

/**
 * @brief Sets the closing time for the door.
 *
 * This function sets the closing time for the door by updating the global variables 'closeTimeHour'
 * and 'closeTimeMin' with the provided hours and minutes.
 * 
 * @param hours The hour component of the closing time.
 * @param min The minute component of the closing time.
 */
void setCloseTime(int hours, int min) {
  closeTimeHour = hours;
  closeTimeMin = min;
}


