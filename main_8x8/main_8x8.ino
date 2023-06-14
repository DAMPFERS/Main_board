#include<SoftwareSerial.h> // Версия 8 x 8

#define IN1 2 
#define IN2 3
#define IN3 4
#define STEP 8
#define COUNTER 16
#define NAMBER_TABLE 3
#define RX 10
#define TX 11
#define PSE 7

void sendingScreen();
void gameTact();
int connectCheck(byte pin, bool adc);
bool findTrueTable(byte seg, byte table);
void comandEnd();
void SendInt(String dev, int dat);
void SendInt(String dev, int dat, bool bin);
void NextionKarno(int count);
void NextionButton();

SoftwareSerial mySerial(RX,TX);

byte karno_table[NAMBER_TABLE][4] = {{0,0,0,0}, {11,12,0,1}, {10,9,8,2}};
int game_table[NAMBER_TABLE][COUNTER];
int values[NAMBER_TABLE + 1] = {0,0,0,0};
const byte input[NAMBER_TABLE] = {IN1, IN2, IN3};
byte count_table = 0; 


void setup(){
  for (int i = 0; i < NAMBER_TABLE; i++)
    pinMode(input[i], INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(STEP, OUTPUT);
  pinMode(PSE, OUTPUT);
  digitalWrite(STEP, LOW);
  digitalWrite(PSE, LOW);
  Serial.begin(9600);
  mySerial.begin(9600);
  NextionKarno(count_table);
  SendInt("Game.pn.val", values[NAMBER_TABLE + 1], false);
}

void loop(){
  if (Serial.available()){
    while (Serial.available()) Serial.read();
    gameTact();
     
  }
  NextionButton();
  
}

int connectCheck(byte pin, bool adc){
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
      game_table[j][i] = connectCheck(input[j],0);
      if (game_table[j][i] == findTrueTable(i,j))
        ++values[j];
    }
    digitalWrite(STEP,HIGH);
    delay(50);
    digitalWrite(STEP,LOW);
  }
  int summ = 0;
  for(int i = 0; i < NAMBER_TABLE; i++){
    summ += values[i];
    Serial.println("-----------------------------------------");
    for(int j = 0; j < COUNTER; j++){
      Serial.print(game_table[i][j]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  values[NAMBER_TABLE] = summ;
  Serial.println(values[0]);
  Serial.println(values[1]);
  Serial.println(values[2]);
  SendInt("Game.pn.val", values[NAMBER_TABLE], false);
  return;
}
bool findTrueTable(byte seg, byte table){
  byte even = 0;
  byte uneven = 0;
  byte line;
  byte column;
  for(int i = 0; i < 4; i++){
    if (i % 2){
      uneven += bitRead(seg,i);
      uneven = uneven << 1;
    }
    else{
      even += bitRead(seg,i);
      even = even << 1;
    }
    }
    even = even >> 1;
    uneven = uneven >> 1;
    switch(even){
      case 0b00000000:
        line = 0;
        break;
      case 0b00000001:
        line = 1;
        break;
      case 0b00000011:
        line = 2;
        break;
      case 0b00000010:
        line = 3;
        break;
    }
    switch(uneven){
      case 0b00000000:
        column = 0;
        break;
      case 0b00000001:
        column = 1;
        break;
      case 0b00000011:
        column = 2;
        break;
      case 0b00000010:
        column = 3;
        break;
    }
    return bitRead(karno_table[table][line], column);
  
}
void comandEnd(){ 
  for(int k = 0; k < 3; k++)
    mySerial.write(0xff);
  return; 
}
void SendInt(String dev, int dat, bool bin){
   mySerial.print(dev);
   mySerial.print("=");
   if (bin)
    mySerial.print(dat,BIN);
   else
    mySerial.print(dat);
   comandEnd();
   return;
}
void NextionKarno(byte count){
  SendInt("Game.tb.val", count, false);
  SendInt("Game.n0.val", karno_table[count][0], true);
  SendInt("Game.n1.val", karno_table[count][1], true);
  SendInt("Game.n2.val", karno_table[count][2], true);
  SendInt("Game.n3.val", karno_table[count][3], true);
  return;
}
void NextionButton(){
  if(mySerial.available()){
    byte a[5];
    while (mySerial.available()) mySerial.readBytes(a,5);
    if(count_table < NAMBER_TABLE-1)
      count_table++;
    else
      count_table = 0;
    NextionKarno(count_table);
  }
  return;
}
