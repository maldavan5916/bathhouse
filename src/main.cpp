#include "pitches.h"
#include <Arduino.h>
#include <microDS18B20.h>
#include <Wire.h>
#include <LCD_1602_RUS.h>

//============================================Setings===================================================\\

LCD_1602_RUS lcd(0x27, 16, 2);

MicroDS18B20<2> sensor_HW;  // датчик температуры горячей воды (D2)
MicroDS18B20<9> sensor_CW;  // датчик температуры холодной воды (D9)
MicroDS18B20<7> sensor_PRN; // датчик температуры парной (D7)

const int Button = 5;  // Номер пина подключения кнопки
const int SSR100 = 10; // Номер пина подключения SSR100
const int Heater = 12; // Номер выхода нагреватель счётчика
const int Alarm = 13;  // Номер выхода Alarm
const long interval = 1000;      // Интервал опраса датчиков времени в миллисекундах (1 секунда)
const int melody[] = {NOTE_A7, NOTE_G7, NOTE_E7, NOTE_C7, NOTE_D7, NOTE_B7, NOTE_F7, NOTE_C7}; // мелоди при успешном включении

//======================================================================================================\\

void successfulInclusion();
void btnClickSound();
void btnClick();
void updateLCD(String, String);

int nScreen = 0;
String lastScreen;
unsigned long previousMillis = 0; // Переменная для хранения времени последнего вызова функции

void setup()
{
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(5, 0);
  lcd.print("Привет");
  
  delay(500);
  
  pinMode(Button, INPUT);   // Инициализируем пин, подключенный к кнопке, как вход (кнопка)INPUT_PULLUP
  pinMode(SSR100, OUTPUT);  // Инициализируем пин, подключенный к светодиоду, как выход (SSR100)
  pinMode(Heater, OUTPUT);  // Инициализируем пин, подключенный к светодиоду, как выход (нагреватель счётчика)
  pinMode(Alarm, OUTPUT);   // Инициализируем пин, подключенный к светодиоду, как выход (Alarm)
  digitalWrite(Alarm, LOW); // выключаем Alarm

  Serial.println("===START===");
  
  successfulInclusion(); // звук "Успешное включение"
  
  lcd.clear();
}

void loop()
{
  if (digitalRead(Button) == 1) btnClick();
  
  switch (nScreen)
  {
    case 0: { updateLCD("Горячая вода    ", String(sensor_HW.getTemp()) + " °C"); } break;
    case 1: { updateLCD("Холодная вода   ", String(sensor_CW.getTemp()) + " °C"); } break;
    case 2: { updateLCD("Темп. парной    ", String(sensor_PRN.getTemp()) + " °C"); } break;
    case 3: { updateLCD("ПРОГРЕВ СЧЁТЧИКА", String(sensor_CW.getTemp()) + " °C"); } break;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    sensor_HW.requestTemp();
    sensor_CW.requestTemp();
    sensor_PRN.requestTemp();
    
    Serial.println(
      " HW: " + String(sensor_HW.getTemp()) +
      " CW: " + String(sensor_CW.getTemp()) +
      " PRN: " + String(sensor_PRN.getTemp())
    );
  }

  if (sensor_CW.getTemp() < 30) {
    nScreen = 3;
    digitalWrite(Heater, HIGH);  // включаем нагреватель
    
    digitalWrite(Alarm, HIGH);  // включаем сигнал
    delay(50);
    digitalWrite(Alarm, LOW);  // выключаем сигнал
    
    Serial.println("\nWARMING THE COUNTER\n");
  } else {
    nScreen = 0;
  }
}

void successfulInclusion()
{
  for (int i = 0; i < 8; i++)
  {
    tone(Alarm, melody[i]);
    delay(100);
  }
  noTone(Alarm);
}

void btnClick() {
  btnClickSound();
  nScreen++;
  if (nScreen >= 3) nScreen = 0;
  delay(250);

  Serial.println("\nButton clicked\n");
}

void btnClickSound()
{
  digitalWrite(Alarm, HIGH);  // включаем сигнал
  delay(50);
  digitalWrite(Alarm, LOW);  // выключаем сигнал
}

void updateLCD(String str1, String str2) {
  if (lastScreen == str1 + str2) return; else lastScreen = str1 + str2;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str1);
  lcd.setCursor(0, 1);
  lcd.print(str2);
  
  Serial.println("\nLCD updated\n");
}