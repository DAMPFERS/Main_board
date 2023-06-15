// Версия 8 x 8 //

#include<SoftwareSerial.h> // 

#define IN1 2         // Входы игры
#define IN2 3
#define IN3 4
#define STEP 8       // Пин - шаг счетчика
#define COUNTER 64   // Размер таблици Карно
#define NAMBER_TABLE 3 // Кол-во таблиц Карно
#define RX 10  // Пин нового порта
#define TX 11  // Пин нового порта
#define PSE 7  // Сброс счетчика
#define NUMBER_OF_VARIABLES 6// Количество перемнных

void gameTact();                                   // Игровой такт
int connectCheck(byte pin, bool adc);              // Проверка подключения пина (С АЦП и без)
bool findTrueTable(byte seg, byte table);          // Преобразование Карно в таблицу истинности
void comandEnd();                                  // Конец строки команды для экрана
void SendInt(String dev, int dat, bool bin);       // Отправка команд на экран
void NextionKarno(byte count);                     // Отправка на экран таблицы Карно под номером count
void NextionButton();                              // Обработка кнопки на экране

SoftwareSerial mySerial(RX,TX);

byte karno_table[NAMBER_TABLE][8] = {{0,0,0,0,0,0,0,0}, {255,112,200,100,126,34,34,34}, {200,199,180,234,255,250,255,255}};
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
    delay(20);
    digitalWrite(STEP,LOW);
  }
  byte summ = 0;
  for(byte i = 0; i < NAMBER_TABLE; i++){
    summ += values[i];
    Serial.println("-----------------------------------------");
    for(byte j = 0; j < COUNTER; j++){
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
  for(int i = 0; i < NUMBER_OF_VARIABLES; i++){
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
      case 0b00000110:
        line = 4;
        break;
      case 0b00000111:
        line = 5;
        break;
      case 0b00000101:
        line = 6;
        break;
      case 0b00000100:
        line = 7;
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
      case 0b00000110:
        column = 4;
        break;
      case 0b00000111:
        column = 5;
        break;
      case 0b00000101:
        column = 6;
        break;
      case 0b00000100:
        column = 7;
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
