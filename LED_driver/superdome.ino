/*
 LED driver for RTI Dome 7 "SuperDome"
 Winter 2016
 Graeme Bragg
 g.bragg@ecs.soton.ac.uk

 Modified from LED driver for RTI domes ma rk >3
 Autumn 2013
 Philip Basford
 pjbasford@ieee.org
 
 WARNING: All arrays are bytes so sizeof = length
*/

/* ----------------------------------- Compile-time settings ---------------------------------- */  
#define DEBUG                     1     // Define whether to output debug info on debug serial port
#define HAS_SCREEN                1     // Define whether a serial screen is connected to Serial2
#define LED_BURN_CHECK            1     // Define whether to enable LED_BURN_CHECK
#define BUTTONS                   5     // The number of buttons connected to the controller
#define BUTTON_TIMEOUT            45000 // Timeout for buttons. 15000 is 0.96s
#define BUTTON_DEBOUNCE_TIMEOUT   15000 // ~1s
#define DEFAULT_NUM_LEDS          128   // The default number of LEDs connected. Currently only 76 and 128 are supported.

//#define OVERWRITE_NUM_LEDS        128   // Compile-time overwite value for num_leds. This should be set, flashed, commented out and then re-flashed.

/* -------------------------------------------------------------------------------------------- */

/* --------------------------------------- Focus Config --------------------------------------- */  
#define FOCUS_TIMEOUT             45000 // Timeout for focus.
#define FOCUS_LIMIT               60   // Number of loops, 3 minutes ~ 60, 5 minutes ~ 100
#define FOCUS_BANK_AC             136   // Two rows on - 8 + 128, same LEDs in all quarters
#define FOCUS_BANK_B              33    // first and sixth LED in each of the top banks.
#define EXPOSUE_SET_TIME          500   // Leave light on for 500ms when setting exposure.

/* -------------------------------------------------------------------------------------------- */

/* -------------------------------- System Config Definitions --------------------------------- */  
#include <pins_arduino.h>

// EEPROM Config
#include <EEPROM.h>
#define ADDR_NUM_LEDS             0
#define ADDR_SHUTTER_KEY          7

// Serial Port Assignments
#define DEBUG_SERIAL              Serial
#define SCREEN                    Serial2
#define CONSOLE                   Serial3

//OUTPUT BANKS
#define MAX_LEDS                  128 // Maximum number of LEDs supported by the controller
#define LED_BANKS                 3
#define A                         0
#define B                         1
#define C                         2

// Hardware Config
#define DEBUG_LED                 13  // 
#define CAMERA_SHUTTER            41  // Package pin 51: output to trigger camera
#define TRIGGER                   39  // Package pin 70: Input to start automated capture
#define AUTOMATED_RUNNING_LED     40  // Package pin 52

//Button Config
#define BUTTON_0                  62  // Package pin 89 - RED Has a pin-change interrupt associated for the STOP command.
#define BUTTON_1                  63  // Package pin 88 - GREEN Used to trigger autorun
#define BUTTON_2                  64  // Package pin 87 - UP button used for changing exposure
#define BUTTON_3                  65  // Package pin 86 - DOWN button used for chainging exposure
#define BUTTON_4                  66  // Package pin 85 - used to turn on top light for FOCUSing

#define STOP                      BUTTON_0
#define GO                        BUTTON_1
#define UP                        BUTTON_2
#define DOWN                      BUTTON_3
#define FOCUS                     BUTTON_4

// Default LED setting sanity check
#if ((DEFAULT_NUM_LEDS != 76) & (DEFAULT_NUM_LEDS != 128))
  #error "Unsupported Default Dome Size."
#endif

//LED Variables
uint8_t num_leds;                       // Number of LEDs connected to the controller

byte AUTORUN_LEDS[MAX_LEDS][LED_BANKS]; // Array to hold autorun sequence

//Automated running
#define LIGHT_SLACK_TIME          10    // Slack time added to shutter times
#define PRE_ON_DELAY              10    // LED "warm up" delay
#define SHUTTER_ACTUATION_TIME    70    // 0.056s from http://www.imaging-resource.com/PRODS/nikon-d810/nikon-d810A6.HTM
#define BETWEEN_SHOT_DELAY        1000  // The time between shots to allow writing to card, etc.

#define MAX_SHUTTER               16    // Number of shutter speed entries
#define DEFAULT_SHUTTER_KEY       10    // Default to half second exposures if EEPROM value corrupt/missing
uint8_t shutter_key;                    // Key for the position in the shutter speed table - this is stored in EEPROM

#define SCREEN_CMD_DELAY          10    // Delay after some screen commands. Prevents blanking/non-response issues.
 
#define STATE_AUTORUN             0x01  // State mask to indicate autorun is in progress
#define STATE_AUTORUN_STOP        0x02  // State mask to indicate that autorun should stop
uint8_t status_byte = 0;                // Status byte to indicate run state.
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------- Static arrays --------------------------------------- */  
const byte leds[LED_BANKS][8] = {     /* Pin allocations for LED banks */
  {22, 23, 24, 25, 26, 27, 28, 29},   /* LED Bank A, Rows 0-7 (+28V) */
  {37, 36, 35, 34, 33, 32, 31, 30},   /* LED Bank B, Columns 0-7 (GND) */
  {49, 48, 47, 46, 45, 44, 43, 42}    /* LED Bank C, Rows 8-15 (+28V) */
};

const uint16_t light_on_time[MAX_SHUTTER + 1] = {     /* Array of strings to hold screen text */
  0,
  67 + LIGHT_SLACK_TIME,              /* 1/15 exposure */
  77 + LIGHT_SLACK_TIME,              /* 1/13 exposure */
  100 + LIGHT_SLACK_TIME,             /* 1/10 exposure */
  125 + LIGHT_SLACK_TIME,             /* 1/8 exposure */
  167 + LIGHT_SLACK_TIME,             /* 1/6 exposure */
  200 + LIGHT_SLACK_TIME,             /* 1/5 exposure */
  250 + LIGHT_SLACK_TIME,             /* 1/4 exposure */
  334 + LIGHT_SLACK_TIME,             /* 1/3 exposure */
  400 + LIGHT_SLACK_TIME,             /* 1/2.5 exposure */
  500 + LIGHT_SLACK_TIME,             /* 1/2 exposure */
  625 + LIGHT_SLACK_TIME,             /* 1/1.6 exposure */
  770 + LIGHT_SLACK_TIME,             /* 1/1.3 exposure */
  1000 + LIGHT_SLACK_TIME,            /* 1s exposure */
  1300 + LIGHT_SLACK_TIME,             /* 1.3s exposure */
  1600 + LIGHT_SLACK_TIME,             /* 1.6s exposure */
  2000 + LIGHT_SLACK_TIME             /* 2s exposure */
};

const char *light_menu_strs[MAX_SHUTTER + 1] = {      /* Array of strings to hold screen text */
  "",
  " 1/15",        /* 1/15 exposure */
  " 1/13",        /* 1/13 exposure */
  " 1/10",        /* 1/10 exposure */
  " 1/8",         /* 1/8 exposure */
  " 1/6",         /* 1/6 exposure */
  " 1/5",         /* 1/5 exposure */
  " 1/4",         /* 1/4 exposure */
  " 1/3",         /* 1/3 exposure */
  "1/2.5",        /* 1/2.5 exposure */
  " 1/2",         /* 1/2 exposure */
  "1/1.6",        /* 1/1.6 exposure */
  "1/1.3",        /* 1/1.3 exposure */
  " 1\"",         /* 1s exposure */
  " 1.3\"",       /* 1.3s exposure */
  " 1.6\"",       /* 1.6s exposure */
  " 2\""          /* 2s exposure */
};
/* -------------------------------------------------------------------------------------------- */

  
void debug(String output){
#if DEBUG
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.println(output);
#else
  CONSOLE.println(output);
#endif
#endif
}

void setup() {
/* ------------------------------------ Setup serial ports ------------------------------------ */  
  CONSOLE.begin(38400); //init serial port
  CONSOLE.setTimeout(100);

#if DEBUG
  DEBUG_SERIAL.begin(9600);
  DEBUG_SERIAL.setTimeout(100);
#endif

#if HAS_SCREEN
  SCREEN.begin(9600); //init serial port
  SCREEN.setTimeout(100);

  SCREEN.write(0x7C);           // Special Command Byte
  SCREEN.write(157);            // Backlight fully on
  delay(SCREEN_CMD_DELAY);
#endif
/* -------------------------------------------------------------------------------------------- */

/* --------------------------- Check & update stored number of LEDs --------------------------- */
  num_leds = EEPROM.read(ADDR_NUM_LEDS);    // Get the number of LEDs stored in EEPROM

#ifdef OVERWRITE_NUM_LEDS
  #warning Compile-Time overwrite set for LED Number. Please undef OVERWRITE_NUM_LEDS and reflash
#if ((OVERWRITE_NUM_LEDS != 76) & (OVERWRITE_NUM_LEDS != 128))
  #error "Unsupported Overwrite Dome Size."
#endif  
  num_leds = OVERWRITE_NUM_LEDS;
  EEPROM.put(ADDR_NUM_LEDS, OVERWRITE_NUM_LEDS);
#endif

  if((num_leds != 76) && (num_leds != 128)) {
    // Number of LEDs stored in EEPROM is corrupt. Reset to default.
#if DEBUG
    DEBUG_SERIAL.write("Error reading Number of LEDs from EEPROM: defaulting to ");
    DEBUG_SERIAL.print(DEFAULT_NUM_LEDS, DEC);
    DEBUG_SERIAL.write(" LEDs\r\n");
#endif    
    num_leds = DEFAULT_NUM_LEDS;
    EEPROM.put(ADDR_NUM_LEDS, DEFAULT_NUM_LEDS);
  }
/* -------------------------------------------------------------------------------------------- */

/* ---------------------------------- Get Stored Shutter Key ---------------------------------- */
  shutter_key = EEPROM.read(ADDR_SHUTTER_KEY);    // Get the shutter key stored in EEPROM

  if((shutter_key == 0) || (shutter_key > MAX_SHUTTER)) {
    // shutter_key EEPROM is corrupt. Reset to default.
#if DEBUG
    DEBUG_SERIAL.write("Error reading shutter_key from EEPROM: defaulting to ");
    DEBUG_SERIAL.print(DEFAULT_SHUTTER_KEY, DEC);
    DEBUG_SERIAL.write(" (");
    DEBUG_SERIAL.write(light_menu_strs[DEFAULT_SHUTTER_KEY]);
    DEBUG_SERIAL.write(" shutter time)\r\n");
#endif    

    shutter_key = DEFAULT_SHUTTER_KEY;
    EEPROM.put(ADDR_SHUTTER_KEY, DEFAULT_SHUTTER_KEY);
  }
/* -------------------------------------------------------------------------------------------- */

/* ------------------------------- Write initialisation strings ------------------------------- */ 
  if(num_leds == 76) {
    // Standard 76-LED Dome
    CONSOLE.write("RTI DOME Controller v0.2\r\n");

#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.write("RTI DOME Controller v0.2\r\n");
#endif

  } else if(num_leds == 128) {
    // 128-LED SuperDome
    CONSOLE.write("RTI SUPERDOME Controller v0.2\r\n");
  
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.write("RTI SUPERDOME Controller v0.2\r\n");
#endif
  }

  screenBanner();    // Print the screen banner

/* -------------------------------------------------------------------------------------------- */

/* -------------------------------- Setup autorun LED sequence -------------------------------- */
  if(num_leds == 76)  {  
    setup_autorun_dome();
  } else if (num_leds == 128) {
    setup_autorun_superdome();
  }
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------- Setup I/O pins -------------------------------------- */ 
  pinMode(TRIGGER, INPUT);
  pinMode(CAMERA_SHUTTER, OUTPUT); 
  pinMode(AUTOMATED_RUNNING_LED, OUTPUT);
  digitalWrite(DEBUG_LED, LOW);
  for( int i = 0; i < 3; i++){
      //iterate through the banks of leds
      for(int j = 0; j < 8; j++){
        pinMode(leds[i][j], OUTPUT); //set the pin for the LED as an output
      }
  }

#if BUTTONS > 0
  // Setup any configured buttons - assumes buttons are on consecutive pins.
  pinMode((BUTTON_0), INPUT);
  
  // Enable Pin Change Interrupt on BUTTON_0 as this will ALWAYS be Emergency-stop
  *digitalPinToPCMSK(BUTTON_0) |= bit (digitalPinToPCMSKbit(BUTTON_0));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(BUTTON_0));                        // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(BUTTON_0));                        // enable interrupt for the group 

#if BUTTONS > 1
  pinMode((BUTTON_1), INPUT);
#if BUTTONS > 3
  pinMode((BUTTON_2), INPUT);
  pinMode((BUTTON_3), INPUT);
#if BUTTONS == 5
  pinMode((BUTTON_4), INPUT);  
#endif  
#endif  
#endif

#endif

/* -------------------------------------------------------------------------------------------- */

/* ------------------------------------- Initialise Timers ------------------------------------ */
  watchdoginit();
  
  buttonTimerInit();
/* -------------------------------------------------------------------------------------------- */

/* -------------------------------------- Setup I/O pins -------------------------------------- */   
  CONSOLE.write("Init Complete\r\n");
  
#ifdef DEBUG_SERIAL
    DEBUG_SERIAL.write("Init Complete\r\n");
#endif 
/* -------------------------------------------------------------------------------------------- */  
}

boolean multiple_leds(byte input){
	//check that more than 1 bit is set in the input - if so then more than 1 LED
	// will be on so overheating is not an issue
	return !(input == 0) && (input&(input -1));
}

void watchdoginit() {
  cli();//stop interrupts
  //set timer1 interrupt at 0.5Hz (31250)
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 1;//initialize counter value to 0
   // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  // set compare match register for 1hz increments (sets Timeout)
  OCR1A = 31250; // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  TCCR1B &= !((1 << CS12) | (1 << CS11) | (1 << CS10));
  // Set CS12 and CS10 bits for 1024 prescaler
  // TCCR1B |= (1 << CS12) | (1 << CS10);  

  sei();//allow interrupts
 }
 
void watchdogstop() {
  debug("\tWatchdog STOP\r\n");
  //digitalWrite(DEBUG_LED, LOW);
  // Set CS12 and CS10 bits ZERO to stop timer 
  TCCR1B &= !((1 << CS12) | (1 << CS10)); 
}

void watchdogstart() {
  debug("\tWatchdog START\r\n");
  digitalWrite(DEBUG_LED, HIGH);
  TCNT1  = 0;//initialize counter value to 0
  // Set CS12 and CS10 bits for 1024 prescaler and start
  TCCR1B |= (1 << CS12) | (1 << CS10);  
}


void flash_debug(int time){
  digitalWrite(DEBUG_LED, HIGH);
  delay(time);
  digitalWrite(DEBUG_LED, LOW);
}


void loop() {
#if BUTTONS > 1           // 2nd button (BUTTON_1) is always GREEN/trigger
  if((digitalRead(TRIGGER) == LOW) || (digitalRead(GO) == LOW)) {
#else
  if(digitalRead(TRIGGER) == LOW){
#endif
    CONSOLE.write("Starting autorun\r\n");
    autorun();
    CONSOLE.write("Autorun complete\r\n");
  }
#if BUTTONS > 3         // 3rd and 4th buttons connected - UP and DOWN respectively
  else if((digitalRead(UP) == LOW) || (digitalRead(DOWN) == LOW)) {
    // up/down button pressed, want to change set exposure.
    button_handler();    
    delay(50);
    screenBanner();
  }
#endif

#if BUTTONS > 4        // 5th button connected: FOCUS lights
  else if(digitalRead(FOCUS) == LOW) {
    focus_handler();
    delay(50);
    screenBanner();
  }
#endif

  if(CONSOLE.peek() == '?'){  
    // This is the software querying to make sure it's got the correct device attached
    CONSOLE.read();
    spoofResponse();
  }else if(CONSOLE.peek() == '!'){
    //This is the software trying to init the system, can just be thrown away
    char null[11];
    CONSOLE.readBytes(null, 9);
  } else if (CONSOLE.available() >=6){
    char input[10];
    CONSOLE.readBytes(input,6);
    //read the expected amount of data 
  	char Astate = input[1];
  	char Bstate = input[3];
  	char Cstate = input[5];
  	if (Bstate == 0 || ( Astate == 0 && Cstate == 0)){
  		//all LEDs are off
      debug("All off");
  		watchdogstop();
  	}else if(multiple_leds(Astate) || multiple_leds(Bstate & 0x1F)  || multiple_leds(Cstate) || (Astate != 0 && Cstate != 0 )){
  		//more than 1 LED will be lit
      debug("multiple leds");
  		watchdogstop();
  	}else{
      debug("kick dog");
      watchdogstop();
  		watchdogstart();
  	}
    process(A, Astate);
    process(B, Bstate);
    process(C, Cstate);
  }else{
   //didn't get the expected amount of data from the serial link before timeout 
  }
}


void autorun(){
  // Perform an automated capture sequence 
  digitalWrite(AUTOMATED_RUNNING_LED, HIGH);
  status_byte &= ~(STATE_AUTORUN_STOP);
  status_byte |= STATE_AUTORUN;
  
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x01);           // Clear screen
  delay(5);
  
  SCREEN.write("Autorun  Shutter");
  
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x80 + 67);
  SCREEN.write("/");
  SCREEN.print(num_leds, DEC);
  
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x80 + 71);
  SCREEN.write("    ");
  SCREEN.write(light_menu_strs[shutter_key]);
  
  
  for(int i = 0; i < num_leds; i++){
      SCREEN.write(0xFE);           // Command Byte
      SCREEN.write(0x80 + 64);
      SCREEN.write("   ");
      SCREEN.write(0xFE);           // Command Byte
      SCREEN.write(0x80 + 64);
      SCREEN.print(i, DEC);   
      
      if(status_byte & STATE_AUTORUN_STOP) {    // E-Stop has been pressed, stop running.
        break;
      }      
      
      watchdogstart();
      
      process(A, char(AUTORUN_LEDS[i][0]));
      process(B, char(AUTORUN_LEDS[i][1]));
      process(C, char(AUTORUN_LEDS[i][2]));
      delay(PRE_ON_DELAY);                      // LED "warmup" time
      digitalWrite(CAMERA_SHUTTER, HIGH);       // Actuate the shutter
      delay(SHUTTER_ACTUATION_TIME);            // Give the camera some time to process the shutter
      digitalWrite(CAMERA_SHUTTER, LOW);        // Disable shutter actuation
      delay(light_on_time[shutter_key]);        // Leave the LED on for the exposure time.
      process(A, char(0));
      process(B, char(0));
      process(C, char(0));
      
      watchdogstop();
      
      if(status_byte & STATE_AUTORUN_STOP) {    // E-Stop has been pressed, stop running.
        break;
      }
      
      delay(BETWEEN_SHOT_DELAY);   
  }
  digitalWrite(AUTOMATED_RUNNING_LED, LOW);
  
  screenBanner();    // Print the screen banner
  
  status_byte &= ~(STATE_AUTORUN_STOP | STATE_AUTORUN);
}

void spoofResponse(){
  // Spoof the response from the USB IO device
  CONSOLE.println("USB I/O 24R1"); 
}

void process(byte bank, byte state_in){
  int state = state_in + 0;
  if (state & 1){
      digitalWrite(leds[bank][0], HIGH);
  }else{
    digitalWrite(leds[bank][0], LOW);
  }
  if (state & 2){
    digitalWrite(leds[bank][1], HIGH);
  }else{
    digitalWrite(leds[bank][1], LOW);
  } 
  if (state & 4){
    digitalWrite(leds[bank][2], HIGH);
  }else{
    digitalWrite(leds[bank][2], LOW);
  }
  if (state & 8){
    digitalWrite(leds[bank][3], HIGH);
  }else{
    digitalWrite(leds[bank][3], LOW);
  }
  if (state & 16){
    digitalWrite(leds[bank][4], HIGH);
  }else{
    digitalWrite(leds[bank][4], LOW);
  }
  if (state & 32){
    digitalWrite(leds[bank][5], HIGH);
  }else{
     digitalWrite(leds[bank][5], LOW);
  }
  if (state & 64){
    digitalWrite(leds[bank][6], HIGH);
  }else{
    digitalWrite(leds[bank][6], LOW);
  }
  if (state & 128){
    digitalWrite(leds[bank][7], HIGH);
  }else{
    digitalWrite(leds[bank][7], LOW);
  }
}

void buttonTimerInit(void) {
  // Initialise the button timer (Timer 5) and debounce timer (Timer 4)
  TCCR5A = 0;         // Set entire TCCR5A port to 0 - disable output pins 
  TCCR5B = 0;         // Set entire TCCR5B port to 0.
  TCCR5C = 0;         // Set entire TCCR5C port to 0
  TCNT5 = 0;          // Set counter value to 0

  TCCR4A = 0;         // Set entire TCCR4A port to 0 - disable output pins 
  TCCR4B = 0;         // Set entire TCCR4B port to 0.
  TCCR4C = 0;         // Set entire TCCR4C port to 0
  TCNT4 = 0;          // Set counter value to 0

  // Set CS12 and CS10 bits for 1024 prescaler
  // TCCR5B |= (1 << CS52) | (1 << CS50);  
}

void buttonTimerReset(void) {
  TCCR5B = 0;                             // Stop the timer
  TCNT5 = 0;                              // Set counter value to 0
  TCCR5B |= (1 << CS52) | (1 << CS50);    // Set CS52 and CS50 bits for 1024 prescaler  
}

void buttonTimerStop(void) {
  TCCR5B = 0;                             // Stop the timer
}

uint16_t buttonTimerValue (void) {
  return (TCNT5);
}


void buttonDebounceReset(void) {
  TCCR4B = 0;                             // Stop the timer
  TCNT4 = 0;                              // Set counter value to 0
  TCCR4B |= (1 << CS42) | (1 << CS40);    // Set CS42 and CS40 bits for 1024 prescaler
}

void buttonDebounceStop(void) {
  TCCR4B = 0;                             // Stop the timer
}

uint16_t buttonDebounceValue (void) {
  return (TCNT4);
}


void button_handler(void) {
  uint8_t up_state, down_state = 0;

  screenShutter();              // Display the shutter banner
  buttonTimerReset();           // Start the button timeout 
  
  while (buttonTimerValue() < BUTTON_TIMEOUT) {   
    if(digitalRead(UP) == LOW) {              // UP button pressed
      if (up_state == 0) {                // if we are not in debounce, increment
        up_state = 1;             // Mark up debounce
        down_state = 0;           // Clear down debounce
        buttonDebounceReset();    // Reset debounce
        
        if (shutter_key < MAX_SHUTTER) {
          shutter_key++;
        } else if (shutter_key > MAX_SHUTTER) {   // sanity check
          shutter_key = MAX_SHUTTER;
        }

        // Update the displayed value
        SCREEN.write(0xFE);           // Command Byte
        SCREEN.write(0x80 + 69);      // Position 70
        SCREEN.write("      ");
        SCREEN.write(0xFE);           // Command Byte
        SCREEN.write(0x80 + 69);      // Position 70
        SCREEN.write(light_menu_strs[shutter_key]);
        delay(30);
      } 
      buttonTimerReset();       // Reset button timeout
    } else if(digitalRead(DOWN) == LOW) {     // Down button pressed
      if (down_state == 0) {              // if we are not in debounce, decrement
        down_state = 1;           // Mark down debounce
        up_state = 0;             // Clear up debounce
        buttonDebounceReset();    // Reset debounce
        
        if(shutter_key > 1) {
          shutter_key--;
        } else if (shutter_key < 1) {             // sanity check
          shutter_key = 1;
        }

        // Update the displayed value
        SCREEN.write(0xFE);           // Command Byte
        SCREEN.write(0x80 + 69);      // Position 70
        SCREEN.write("      ");
        SCREEN.write(0xFE);           // Command Byte
        SCREEN.write(0x80 + 69);      // Position 70
        SCREEN.write(light_menu_strs[shutter_key]);
        delay(30);
      }
      buttonTimerReset();       // Reset button timeout
    }

    if ((buttonDebounceValue() > BUTTON_DEBOUNCE_TIMEOUT) || ((digitalRead(UP) == HIGH) & (digitalRead(DOWN) == HIGH))) {    
      // Debounce timeout reached, clear markers
      up_state = 0;
      down_state = 0;
      buttonDebounceStop();
    }
  }


  // Write the new shutter_key to EEPROM
  EEPROM.put(ADDR_SHUTTER_KEY, shutter_key);

  //screenBanner();
}

void focus_handler(void) {
  // Turn on the top 4 lights to allow focusing.
  uint8_t focus_loop, row_key, col_key;
  
  status_byte &= ~(STATE_AUTORUN_STOP);
  status_byte |= STATE_AUTORUN;

  screenFocus();              // Display the focus banner

  // Turn on the LEDs for focusing.
  process(A, char(FOCUS_BANK_AC));
  process(B, char(FOCUS_BANK_B));
  process(C, char(FOCUS_BANK_AC));
  
  // Use debounce timer to delay between FOCUS button tests.
  buttonDebounceReset();
  while (buttonDebounceValue() < BUTTON_DEBOUNCE_TIMEOUT);

  // Focus timeout - about N minutes or until STOP button is pressed.   
  for(focus_loop = 0; focus_loop < FOCUS_LIMIT; focus_loop++) {
    buttonTimerReset();         // Start the button timeout
    while (buttonTimerValue() < FOCUS_TIMEOUT) {

      if(digitalRead(FOCUS) == LOW) {   
        // FOCUS button has been pressed again, start an autowalk to set exposure
        screenExposure();     // Display the exposure banner
        
        for(row_key = 0; row_key<4; row_key++) {
          switch (row_key) {
            // Set the row for autowalk
            case 0:   process(A, char(8));
                      process(C, char(0));
                      break;
                      
            case 1:   process(A, char(128));
                      process(C, char(0));
                      break;
                      
            case 2:   process(A, char(0));
                      process(C, char(128));
                      break;
                      
            case 3:   process(A, char(0));
                      process(C, char(8));
                      break;
                      
            deafult:  break;
          }
          
          for(col_key = 0; col_key < 8; col_key++) {
            //Cycle through the different columns
            watchdogstart();
            process(B, char(1 << col_key));
            delay(EXPOSUE_SET_TIME);
            process(B, char(0));
            watchdogstop();
            if(status_byte & STATE_AUTORUN_STOP) break; // E-Stop has been pressed, stop running.
          }
          if(status_byte & STATE_AUTORUN_STOP) break;   // E-Stop has been pressed, stop running.
        }
        
        status_byte |= STATE_AUTORUN_STOP;        // Reached the end of the autowalk, indicate that we should end.
        
      }
      if(status_byte & STATE_AUTORUN_STOP) break;      // E-Stop has been pressed, stop running.
    }
    if(status_byte & STATE_AUTORUN_STOP) break;       // E-Stop has been pressed, stop running.
  }
  
  // Turn off all of the LEDs
  process(A, char(0));
  process(B, char(0));
  process(C, char(0));
}

void screenBanner(void) {
  // Print the screen banner
#if HAS_SCREEN
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x01);           // Clear screen
  delay(SCREEN_CMD_DELAY);

  //SCREEN.write(0xFE);           // Command Byte
  //SCREEN.write(0x80);           // Position 0
  
  SCREEN.write("RTI ");

  if(num_leds == 76) {          // Standard 76-LED Dome
    SCREEN.write("DOME");
  } else if(num_leds == 128) {   // 128-LED SuperDome
    SCREEN.write("SUPERDOME");
  }
 
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x80 + 0x40);    // Position 64, start of line 2
  SCREEN.write("Controller v0.2");
#endif
}

void screenShutter(void) {
  // Print the Shutter change screen
#if HAS_SCREEN
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x01);           // Clear screen
  delay(SCREEN_CMD_DELAY);
  
  SCREEN.write("Shutter Speed:");
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x80 + 64);      // Position 64, start of line 2
  SCREEN.write("DWN: ");
  SCREEN.write(light_menu_strs[shutter_key]);
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x80 + 75);      // Position 64, start of line 2
  SCREEN.write(": UP ");
#endif  
}

void screenFocus(void) {
  // Print the Focus screen
#if HAS_SCREEN
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x01);           // Clear screen
  delay(SCREEN_CMD_DELAY);
  
  SCREEN.write("Focusing LEDs ON");
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x80 + 64);      // Position 64, start of line 2
  SCREEN.write("STOP to exit");
#endif  
}

void screenExposure(void) {
  // Print the Exposure autowalk screen
#if HAS_SCREEN
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x01);           // Clear screen
  delay(SCREEN_CMD_DELAY);
  
  SCREEN.write("Exposure Autowalk");
  SCREEN.write(0xFE);           // Command Byte
  SCREEN.write(0x80 + 64);      // Position 64, start of line 2
  SCREEN.write("STOP to exit");
#endif  
}

ISR(TIMER1_COMPA_vect){//timer0 interrupt 2kHz toggles pin 8
//generates pulse wave of frequency 2kHz/2 = 1kHz (takes two cycles for full wave- toggle high then toggle low)
  if (LED_BURN_CHECK){
   	process(A,char(0));
  	process(B,char(0));
	  process(C,char(0));
    debug("dog");
    watchdogstop();
    digitalWrite(DEBUG_LED, LOW);
    flash_debug(1000);
  }
}

ISR (PCINT2_vect) {     // Pin change interrupt for Port K
  if(status_byte & STATE_AUTORUN) {     // Only execute if we are currently autorunning
    PORTA = 0;                          // Kill Bank A
    PORTL = 0;                          // Kill Bank C
    PORTC = 0;                          // Kill Bank B
    digitalWrite(CAMERA_SHUTTER, LOW);  // Kill camera shutter
    status_byte |= STATE_AUTORUN_STOP;  // Mark that the STOP interrupt has fired  
  }
}
  
