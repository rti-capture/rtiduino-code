//sets serial lcd splash screen

// 1)make sure tx of lcd is unplugged
// 2)change lines to whatever you want (see below)
// 3)upload sketch to board
// 4)power down board and plug in tx of lcd
// 5)power up board
// 6)reset to test after the screen goes blank (old splash - new splash - blank screen - reset to test)

void setup()
{
 Serial2.begin(9600);
 backlightOn();
 delay(100);
 clearLCD();
 selectLineOne();
 delay(100);
 Serial2.write("Custom Imaging  "); //type in the first line of the splash here (16 char max)
 selectLineTwo();
 delay(100);
 Serial2.write("RTI Controller  "); //type the second line of the splash here (16 char max)
 delay(500);
 Serial2.write(0x7C); //these lines...
 Serial2.write(10); //set the splash to memory (this is the <control> j char or line feed
 delay(100);
 clearLCD();
}

void loop()
{  
 delay(100);
}

void selectLineOne(){  //puts the cursor at line 0 char 0.
  Serial2.write(0xFE);   //command flag
  Serial2.write(128);    //position
}
void selectLineTwo(){  //puts the cursor at line 0 char 0.
  Serial2.write(0xFE);   //command flag
  Serial2.write(192);    //position
}
void clearLCD(){
  Serial2.write(0xFE);   //command flag
  Serial2.write(0x01);   //clear command.
}
void backlightOn(){  //turns on the backlight
   Serial2.write(0x7C);   //command flag for backlight stuff
   Serial2.write(157);    //light level.
}
void backlightOff(){  //turns off the backlight
   Serial2.write(0x7C);   //command flag for backlight stuff
   Serial2.write(128);     //light level for off.
}
void serCommand(){   //a general function to call the command flag for issuing all other commands  
 Serial2.write(0xFE);
}
