#include<SoftwareSerial.h>

#define IN1 2
#define IN2 3
#define IN3 4
#define STEP 8
#define COUNTER 16
#define RX 10
#define TX 11

void sendingScreen();
void gameTact();
int connectCheck(int pin, bool adc);

SoftwareSerial mySerial(RX,TX);

byte karno_table[3][4] = {{1,2,3,4}, {11,12,0,1}, {10,9,8,2}};
int game_table[3][COUNTER];


void setup(){
  pinMode(IN1, INPUT);
  pinMode(IN2, INPUT);
  pinMode(IN3, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(STEP, OUTPUT);
  digitalWrite(STEP, LOW);
  Serial.begin(9600);
}

void loop(){
  if (Serial.available()){
    gameTact();
  }
}

int connectCheck(int pin, bool adc){
  if (adc){
    if (digitalRead(pin) && analogRead(A0) > 900)
      return 1;
    else
      if (!digitalRead(pin) && analogRead(A0) < 100)
        return 0;
      else
        return -1;  
  }
  else
    return digitalRead(pin);
}
void gameTact(){
  for(int i = 0; i < COUNTER; i++){
    game_table[0][i] = connectCheck(IN1,0);
    game_table[1][i] = connectCheck(IN2,0);
    game_table[2][i] = connectCheck(IN3,0);
    digitalWrite(STEP,HIGH);
    delay(50);
    digitalWrite(STEP,LOW);
  }
  for(int i = 0; i < 3; i++){
    Serial.println("-----------------------------------------");
    for(int j = 0; j < COUNTER; j++){
      Serial.print(game_table[i][j]);
    }
  }
}
