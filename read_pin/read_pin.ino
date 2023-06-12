#include<SoftwareSerial.h>

#define IN1 2
#define IN2 3
#define IN3 4
#define STEP 8
#define COUNTER 16
#define NAMBER_TABLE 3
#define RX 10
#define TX 11

void sendingScreen();
void gameTact();
int connectCheck(int pin, bool adc);
bool findTrueTable(byte seg, byte table);
void comandEnd();
void SendInt(String dev, int dat);

SoftwareSerial mySerial(RX,TX);

byte karno_table[NAMBER_TABLE][4] = {{1,2,3,4}, {11,12,0,1}, {10,9,8,2}};
int game_table[NAMBER_TABLE][COUNTER];
int values[NAMBER_TABLE + 1] = {0,0,0,0};
const char input[] = "123456"; 


void setup(){
  for (int i = 0; i < NAMBER_TABLE; i++)
    pinMode(input[i], INPUT);
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
  for(byte i = 0; i < COUNTER; i++){
    for(byte j = 0; j < NAMBER_TABLE; j++){
      game_table[0][i] = connectCheck(input[j],0);
      if (game_table[0][i] == findTrueTable(i,0))
        ++values[i];
    }
    digitalWrite(STEP,HIGH);
    delay(50);
    digitalWrite(STEP,LOW);
  }
  for(int i = 0; i < NAMBER_TABLE; i++){
    values[NAMBER_TABLE + 1] += values[i];
    Serial.println("-----------------------------------------");
    for(int j = 0; j < COUNTER; j++){
      Serial.print(game_table[i][j]);
      Serial.print(" ");
    }
  }
  return;
}
bool findTrueTable(byte seg, byte table){
  byte even = 0;
  byte uneven = 0;
  byte line;
  byte column;
  for(int i = 3; i >= 0; i--){
    byte bite = bitRead(seg,i);
    if (i % 2){
      uneven += bite;
      uneven = uneven << 1;
    }
    else{
      even += bite;
      even = even << 1;
    }
    switch(even){
      case 0b00000000:
        line = 0;
        break;
      case 0b00000010:
        line = 1;
        break;
      case 0b00000011:
        line = 2;
        break;
      case 0b00000001:
        line = 3;
        break;
    }
    switch(uneven){
      case 0b00000000:
        column = 0;
        break;
      case 0b00000010:
        column = 1;
        break;
      case 0b00000011:
        column = 2;
        break;
      case 0b00000001:
        column = 3;
        break;
    }
    return bitRead(karno_table[table][line], column);
  }
}
void comandEnd(){ 
  for(int k = 0; k < 3; k++)
    mySerial.write(0xff);
  return; 
}
void SendInt(String dev, int dat){
   Serial.print(dev);
   Serial.print("=");
   Serial.print(dat);
   comandEnd();
   return;
}
