/*
 LED driver for RTI domes mark >3
 Autumn 2013
 Philip Basford
 pjbasford@ieee.org
 
 WARNING: All arrays are bytes so sizeof = length
*/

	//Setup variables
	const byte CAMERA_SHUTTER = 41; //51 output to trigger camera
	const byte TRIGGER = 39; // 70Input to start automated capture
	const byte AUTOMATED_RUNNING_LED = 40; //52
	const byte DEBUG_LED = 13;
	//OUTPUT BANKS
	const byte A = 0;
	const byte B = 1;
	const byte C = 2;
	
	//LED OUTPUTS
	byte leds[3][8];
	
	//Automated running
	int LIGHT_ON_TIME = 1500;
	int PRE_ON_DELAY = 100;
	int SHUTTER_ACTUATION_TIME = 100;
	int BETWEEN_SHOT_DELAY = 100;
	byte AUTORUN_LEDS[76][3];
	
	//enable watchdog checks
	int ledburncheck = 1;
	
void debug(String output){
	//Serial3.println(output);
}

void setup() {
	
	leds[A][0] = 22; //78
	leds[A][1] = 23; //77
	leds[A][2] = 24; //76
	leds[A][3] = 25; //75
	leds[A][4] = 26; //74
	leds[A][5] = 27; //73
	leds[A][6] = 28; //72
	leds[A][7] = 29; //71
	
	leds[B][0] = 37;  //53
	leds[B][1] = 36;  //54
	leds[B][2] = 35;	//55
	leds[B][3] = 34;	//56
	leds[B][4] = 33;	//57
	leds[B][5] = 32;	//58
	leds[B][6] = 31;	//59
	leds[B][7] = 30;	//60
	
	leds[C][0] = 49;	//35
	leds[C][1] = 48;	//36
	leds[C][2] = 47;	//37
	leds[C][3] = 46;	//38
	leds[C][4] = 45;	//39
	leds[C][5] = 44;	//40
	leds[C][6] = 43;	//41
	leds[C][7] = 42; 	//42
	 
	AUTORUN_LEDS[0][0] = 1;
	AUTORUN_LEDS[0][1] = 8;
	AUTORUN_LEDS[0][2] = 0; 
	AUTORUN_LEDS[1][0] = 1;
	AUTORUN_LEDS[1][1] = 128;
	AUTORUN_LEDS[1][2] = 0;
	AUTORUN_LEDS[2][0] = 2;
	AUTORUN_LEDS[2][1] = 128;
	AUTORUN_LEDS[2][2] = 0;
	AUTORUN_LEDS[3][0] = 4;
	AUTORUN_LEDS[3][1] = 128;
	AUTORUN_LEDS[3][2] = 0;
	AUTORUN_LEDS[4][0] = 8;
	AUTORUN_LEDS[4][1] = 128;
	AUTORUN_LEDS[4][2] = 0;
	AUTORUN_LEDS[5][0] = 1;
	AUTORUN_LEDS[5][1] = 16;
	AUTORUN_LEDS[5][2] = 0;
	AUTORUN_LEDS[6][0] = 1;
	AUTORUN_LEDS[6][1] = 64;
	AUTORUN_LEDS[6][2] = 0;
	AUTORUN_LEDS[7][0] = 2;
	AUTORUN_LEDS[7][1] = 64;
	AUTORUN_LEDS[7][2] = 0;
	AUTORUN_LEDS[8][0] = 4;
	AUTORUN_LEDS[8][1] = 64;
	AUTORUN_LEDS[8][2] = 0;
	AUTORUN_LEDS[9][0] = 8;
	AUTORUN_LEDS[9][1] = 64;
	AUTORUN_LEDS[9][2] = 0;
	AUTORUN_LEDS[10][0] = 1;
	AUTORUN_LEDS[10][1] = 32;
	AUTORUN_LEDS[10][2] = 0;
	AUTORUN_LEDS[11][0] = 2;
	AUTORUN_LEDS[11][1] = 32;
	AUTORUN_LEDS[11][2] = 0;
	AUTORUN_LEDS[12][0] = 4;
	AUTORUN_LEDS[12][1] = 32;
	AUTORUN_LEDS[12][2] = 0;
	AUTORUN_LEDS[13][0] = 8;
	AUTORUN_LEDS[13][1] = 32;
	AUTORUN_LEDS[13][2] = 0;
	AUTORUN_LEDS[14][0] = 2;
	AUTORUN_LEDS[14][1] = 16;
	AUTORUN_LEDS[14][2] = 0;
	AUTORUN_LEDS[15][0] = 4;
	AUTORUN_LEDS[15][1] = 16;
	AUTORUN_LEDS[15][2] = 0;
	AUTORUN_LEDS[16][0] = 8;
	AUTORUN_LEDS[16][1] = 16;
	AUTORUN_LEDS[16][2] = 0;
	AUTORUN_LEDS[17][0] = 4;
	AUTORUN_LEDS[17][1] = 8;
	AUTORUN_LEDS[17][2] = 0;
	AUTORUN_LEDS[18][0] = 8;
	AUTORUN_LEDS[18][1] = 8;
	AUTORUN_LEDS[18][2] = 0;

	AUTORUN_LEDS[19][0] = 16;
	AUTORUN_LEDS[19][1] = 8;
	AUTORUN_LEDS[19][2] = 0;
	AUTORUN_LEDS[20][0] = 16;
	AUTORUN_LEDS[20][1] = 128;
	AUTORUN_LEDS[20][2] = 0;
	AUTORUN_LEDS[21][0] = 32;
	AUTORUN_LEDS[21][1] = 128;
	AUTORUN_LEDS[21][2] = 0;
	AUTORUN_LEDS[22][0] = 64;
	AUTORUN_LEDS[22][1] = 128;
	AUTORUN_LEDS[22][2] = 0;
	AUTORUN_LEDS[23][0] = 128;
	AUTORUN_LEDS[23][1] = 128;
	AUTORUN_LEDS[23][2] = 0;
	AUTORUN_LEDS[24][0] = 16;
	AUTORUN_LEDS[24][1] = 16;
	AUTORUN_LEDS[24][2] = 0;
	AUTORUN_LEDS[25][0] = 16;
	AUTORUN_LEDS[25][1] = 64;
	AUTORUN_LEDS[25][2] = 0;
	AUTORUN_LEDS[26][0] = 32;
	AUTORUN_LEDS[26][1] = 64;
	AUTORUN_LEDS[26][2] = 0;
	AUTORUN_LEDS[27][0] = 64;
	AUTORUN_LEDS[27][1] = 64;
	AUTORUN_LEDS[27][2] = 0;
	AUTORUN_LEDS[28][0] = 128;
	AUTORUN_LEDS[28][1] = 128;
	AUTORUN_LEDS[28][2] = 0;
	AUTORUN_LEDS[29][0] = 16;
	AUTORUN_LEDS[29][1] = 32;
	AUTORUN_LEDS[29][2] = 0;
	AUTORUN_LEDS[30][0] = 32;
	AUTORUN_LEDS[30][1] = 32;
	AUTORUN_LEDS[30][2] = 0;
	AUTORUN_LEDS[31][0] = 64;
	AUTORUN_LEDS[31][1] = 32;
	AUTORUN_LEDS[31][2] = 0;
	AUTORUN_LEDS[32][0] = 128;
	AUTORUN_LEDS[32][1] = 32;
	AUTORUN_LEDS[32][2] = 0;
	AUTORUN_LEDS[33][0] = 32;
	AUTORUN_LEDS[33][1] = 16;
	AUTORUN_LEDS[33][2] = 0;
	AUTORUN_LEDS[34][0] = 64;
	AUTORUN_LEDS[34][1] = 16;
	AUTORUN_LEDS[34][2] = 0;
	AUTORUN_LEDS[35][0] = 128;
	AUTORUN_LEDS[35][1] = 16;
	AUTORUN_LEDS[35][2] = 0;
	AUTORUN_LEDS[36][0] = 64;
	AUTORUN_LEDS[36][1] = 8;
	AUTORUN_LEDS[36][2] = 0;
	AUTORUN_LEDS[37][0] = 128;
	AUTORUN_LEDS[37][1] = 8;
	AUTORUN_LEDS[37][2] = 0;

	AUTORUN_LEDS[38][0] = 0;
	AUTORUN_LEDS[38][1]= 8;
	AUTORUN_LEDS[38][2] = 1;
	AUTORUN_LEDS[39][0] = 0;
	AUTORUN_LEDS[39][1] = 128;
	AUTORUN_LEDS[39][2] = 1;
	AUTORUN_LEDS[40][0] = 0;
	AUTORUN_LEDS[40][1] = 128;
	AUTORUN_LEDS[40][2] = 2;
	AUTORUN_LEDS[41][0] = 0;
	AUTORUN_LEDS[41][1] = 128;
	AUTORUN_LEDS[41][2] = 4;
	AUTORUN_LEDS[42][0] = 0;
	AUTORUN_LEDS[42][1] = 128;
	AUTORUN_LEDS[42][2] = 8;
	AUTORUN_LEDS[43][0] = 0;
	AUTORUN_LEDS[43][1] = 16;
	AUTORUN_LEDS[43][2] = 1;
	AUTORUN_LEDS[44][0] = 0;
	AUTORUN_LEDS[44][1] = 64;
	AUTORUN_LEDS[44][2] = 1;
	AUTORUN_LEDS[45][0] = 0;
	AUTORUN_LEDS[45][1] = 64;
	AUTORUN_LEDS[45][2] = 2;
	AUTORUN_LEDS[46][0] = 0;
	AUTORUN_LEDS[46][1] = 64;
	AUTORUN_LEDS[46][2] = 4;
	AUTORUN_LEDS[47][0] = 0;
	AUTORUN_LEDS[47][1] = 64;
	AUTORUN_LEDS[47][2] = 8;
	AUTORUN_LEDS[48][0] = 0;
	AUTORUN_LEDS[48][1] = 32;
	AUTORUN_LEDS[48][2] = 1;
	AUTORUN_LEDS[59][0] = 0;
	AUTORUN_LEDS[49][1] = 32;
	AUTORUN_LEDS[49][2] = 2;
	AUTORUN_LEDS[50][0] = 0;
	AUTORUN_LEDS[50][1] = 32;
	AUTORUN_LEDS[50][2] = 4;
	AUTORUN_LEDS[51][0] = 0;
	AUTORUN_LEDS[51][1] = 32;
	AUTORUN_LEDS[51][2] = 8;
	AUTORUN_LEDS[52][0] = 0;
	AUTORUN_LEDS[52][1] = 16;
	AUTORUN_LEDS[52][2] = 2;
	AUTORUN_LEDS[53][0] = 0;
	AUTORUN_LEDS[53][1] = 16;
	AUTORUN_LEDS[53][2] = 4;
	AUTORUN_LEDS[54][0] = 0;
	AUTORUN_LEDS[54][1] = 16;
	AUTORUN_LEDS[54][2] = 8;
	AUTORUN_LEDS[55][0] = 0;
	AUTORUN_LEDS[55][1] = 8;
	AUTORUN_LEDS[55][2] = 4;
	AUTORUN_LEDS[56][0] = 0;
	AUTORUN_LEDS[56][1] = 8;
	AUTORUN_LEDS[56][2] = 8;

	AUTORUN_LEDS[57][0] = 0;
	AUTORUN_LEDS[57][1] = 8;
	AUTORUN_LEDS[57][2] = 16;
	AUTORUN_LEDS[58][0] = 0;
	AUTORUN_LEDS[58][1] = 128;
	AUTORUN_LEDS[58][2] = 16;
	AUTORUN_LEDS[59][0] = 0;
	AUTORUN_LEDS[59][1] = 128;
	AUTORUN_LEDS[59][2] = 32;
	AUTORUN_LEDS[60][0] = 0;
	AUTORUN_LEDS[60][1] = 128;
	AUTORUN_LEDS[60][2] = 64;
	AUTORUN_LEDS[61][0] = 0;
	AUTORUN_LEDS[61][1] = 128;
	AUTORUN_LEDS[61][2] = 128;
	AUTORUN_LEDS[62][0] = 0;
	AUTORUN_LEDS[62][1] = 16;
	AUTORUN_LEDS[62][2] = 16;
	AUTORUN_LEDS[63][0] = 0;
	AUTORUN_LEDS[63][1] = 64;
	AUTORUN_LEDS[63][2] = 16;
	AUTORUN_LEDS[64][0] = 0;
	AUTORUN_LEDS[64][1] = 64;
	AUTORUN_LEDS[64][2] = 32;
	AUTORUN_LEDS[65][0] = 0;
	AUTORUN_LEDS[65][1] = 64;
	AUTORUN_LEDS[65][2] = 64;
	AUTORUN_LEDS[66][0] = 0;
	AUTORUN_LEDS[66][1] = 64;
	AUTORUN_LEDS[66][2] = 128;
	AUTORUN_LEDS[67][0] = 0;
	AUTORUN_LEDS[67][1] = 32;
	AUTORUN_LEDS[67][2] = 16;
	AUTORUN_LEDS[68][0] = 0;
	AUTORUN_LEDS[68][1] = 32;
	AUTORUN_LEDS[68][2] = 32;
	AUTORUN_LEDS[69][0] = 0;
	AUTORUN_LEDS[69][1] = 32;
	AUTORUN_LEDS[69][2] = 64;
	AUTORUN_LEDS[70][0] = 0;
	AUTORUN_LEDS[70][1] = 32;
	AUTORUN_LEDS[70][2] = 128;
	AUTORUN_LEDS[71][0] = 0;
	AUTORUN_LEDS[71][1] = 16;
	AUTORUN_LEDS[71][2] = 32;
	AUTORUN_LEDS[72][0] = 0;
	AUTORUN_LEDS[72][1] = 16;
	AUTORUN_LEDS[72][2] = 64;
	AUTORUN_LEDS[73][0] = 0;
	AUTORUN_LEDS[73][1] = 16;
	AUTORUN_LEDS[73][2] = 128;
	AUTORUN_LEDS[74][0] = 0;
	AUTORUN_LEDS[74][1] = 8;
	AUTORUN_LEDS[74][2] = 64;
	AUTORUN_LEDS[75][0] = 0;
	AUTORUN_LEDS[75][1] = 8;
	AUTORUN_LEDS[75][2] = 128;
	
	Serial3.begin(38400); //init serial port
	Serial3.setTimeout(100);
	Serial3.println("RTI DOME Controller v0.3");

	//Setup IO
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
	watchdoginit();
	
	Serial3.println("Init Complete\r\n");
}

boolean multiple_leds(byte input){
	//check that more than 1 bit is set in the input - if so then more than 1 LED
	// will be on so overheating is not an issue
	return !(input == 0) && (input&(input -1));
}

void watchdoginit() {
	cli();//stop interrupts
	//set timer1 interrupt at 1Hz (15624)
	TCCR1A = 0;// set entire TCCR1A register to 0
	TCCR1B = 0;// same for TCCR1B
	TCNT1  = 0;//initialize counter value to 0
	// set compare match register for 1hz increments (sets Timeout)
	OCR1A = 65535; // = (16*10^6) / (1*1024) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12 and CS10 bits for 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10);  
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
	sei();//allow interrupts
 }
 
 void watchdogstop() {
	debug("dog stop");
	//digitalWrite(DEBUG_LED, LOW);
	// Set CS12 and CS10 bits ZERO to stop timer 
	TCCR1B &= !((1 << CS12) | (1 << CS10)); 
}

void watchdogstart() {
	debug("dog start");
	TCNT1  = 0;//initialize counter value to 0
	// Set CS12 and CS10 bits for 1024 prescaler and start
	TCCR1B |= (1 << CS12) | (1 << CS10);  
}

void watchdogrestart(){
	watchdogstop();
	watchdogstart();
}


void flash_debug(int time){
	digitalWrite(DEBUG_LED, HIGH);
	delay(time);
	digitalWrite(DEBUG_LED, LOW);
}




void loop() {
	char input[30];
	if(digitalRead(TRIGGER) == LOW){
		Serial3.write("Starting autorun\r\n");
		autorun();
		Serial3.write("Autorun complete\r\n");
	}
	if(Serial3.available() >= 1){
		if(Serial3.peek() == '?'){  
			// This is the software querying to make sure it's got the correct device attached
			Serial3.read();
			spoofResponse();
		}else if(Serial3.peek() == '!'){
			//This is the software trying to init the system, can just be thrown away
			char null[11];
			Serial3.readBytes(null, 9);
		} else if (Serial3.peek() == 'A'){
			
			Serial3.readBytes(input,Serial3.available());
	 		if(input[0] == 'A' && input[2] == 'B' && input[4] == 'C'){
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
					watchdogrestart();
				}
				process(A, Astate);
				process(B, Bstate);
				process(C, Cstate);
			}else{
				flash_debug(100);
				Serial3.write("Started promising");
			}
		}else{
			//didn't get the expected amount of data from the serial link before timeout 
			flash_debug(100);
			char got = Serial3.read();
			Serial3.print("Unexpected begining char");
			Serial3.println(got);
		 
		}
	}
}


void autorun(){
	// Perform an automated capture sequence 
	digitalWrite(AUTOMATED_RUNNING_LED, HIGH);
	for(int i = 0; i < sizeof(AUTORUN_LEDS); i++){
		process(A, char(AUTORUN_LEDS[i][0]));
		process(B, char(AUTORUN_LEDS[i][1]));
		process(C, char(AUTORUN_LEDS[i][2]));
		delay(PRE_ON_DELAY);
		digitalWrite(CAMERA_SHUTTER, HIGH);
		delay(SHUTTER_ACTUATION_TIME);
		digitalWrite(CAMERA_SHUTTER, LOW);
		delay(LIGHT_ON_TIME);
		process(A, char(0));
		process(B, char(0));
		process(C, char(0));
		delay(BETWEEN_SHOT_DELAY);   
	}
	digitalWrite(AUTOMATED_RUNNING_LED, LOW);
}

void spoofResponse(){
	// Spoof the response from the USB IO device
	Serial3.println("USB I/O 24R1"); 
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


ISR(TIMER1_COMPA_vect){//timer0 interrupt 2kHz toggles pin 8
//generates pulse wave of frequency 2kHz/2 = 1kHz (takes two cycles for full wave- toggle high then toggle low)
	if (ledburncheck){
		process(A,char(0));
		process(B,char(0));
		process(C,char(0));
		debug("dog");
		watchdogstop();
		digitalWrite(DEBUG_LED, LOW);
		flash_debug(1000);
	}
}
