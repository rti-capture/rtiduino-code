
const byte  bankA[8] = {22, 23, 24, 25, 26, 27, 28, 29};
const byte  bankB[8] = {37, 36, 35, 34, 33, 32, 31, 30};
const byte bankC[8] = {49, 48, 47, 46, 45, 44, 43, 42};
// the setup function runs once when you press reset or power the board
void setup() {
  int i;

  // initialize digital pin LED_BUILTIN as an output.
  for( i = 0; i < 8; i++){
    pinMode(bankA[i], OUTPUT);
    pinMode(bankB[i], OUTPUT);
    pinMode(bankC[i], OUTPUT);
  }
  
}

// the loop function runs over and over again forever
void loop() {
  int i;
  for( i = 0; i < 8; i++){
      digitalWrite(bankA[i], HIGH); 
    digitalWrite(bankB[i], HIGH);
    digitalWrite(bankC[i], HIGH);    
  }
  delay(4000);
  for( i = 0; i < 8; i++){
      digitalWrite(bankA[i], LOW);  
    digitalWrite(bankB[i], LOW);
    digitalWrite(bankC[i], LOW);   
  }
  delay(1000);
}
