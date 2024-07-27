#include <Arduino.h>
#include "pitches.h"
#include <LCD_1602_RUS.h>
#include <microDS18B20.h>

LCD_1602_RUS lcd(0x27, 16, 2); // Устанавливаем дисплей (адрес, и т.д.)

MicroDS18B20<2> sensor_HW;  // датчик температуры горячей воды (D2)
MicroDS18B20<9> sensor_CW;  // датчик температуры холодной воды (D9)
MicroDS18B20<7> sensor_PRN; // датчик температуры парной (D7)

const int Button = 5;  // Номер пина подключения кнопки
const int SSR100 = 10; // Номер пина подключения SSR100
const int Heater = 12; // Номер выхода нагреватель счётчика
const int Alarm = 13;  // Номер выхода Alarm
const int melody[] = {NOTE_A7, NOTE_G7, NOTE_E7, NOTE_C7, NOTE_D7, NOTE_B7, NOTE_F7, NOTE_C7};

void successfulInclusion();
void btnClickSound();
void btnClick();
void HWscreenWrite();
void CWscreenWrite();
void PRNscreenWrite();

int nScreen = 0;

void setup()
{
  Serial.begin(115200);

  lcd.init(); // инициализация LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(L"start"); // Выводим сообщение.
  delay(500);

  pinMode(Button, INPUT);   // Инициализируем пин, подключенный к кнопке, как вход (кнопка)INPUT_PULLUP
  pinMode(SSR100, OUTPUT);  // Инициализируем пин, подключенный к светодиоду, как выход (SSR100)
  pinMode(Heater, OUTPUT);  // Инициализируем пин, подключенный к светодиоду, как выход (нагреватель счётчика)
  pinMode(Alarm, OUTPUT);   // Инициализируем пин, подключенный к светодиоду, как выход (Alarm)
  digitalWrite(Alarm, LOW); // выключаем Alarm

  successfulInclusion(); // звук "Успешное включение"

  lcd.clear();
}

void loop()
{
  if (digitalRead(Button) == 1) btnClick();
  HWscreenWrite();
  delay(100);
}

void successfulInclusion()
{
  for (int i = 0; i < 8; i++)
  {
    tone(Alarm, melody[i]);
    delay(100);
  }
  noTone(Alarm);
  Serial.println("\nstart");
}

void btnClick() {
  btnClickSound();
  nScreen++;
  if (nScreen == 3) nScreen = 0;
  delay(250);
  Serial.println("\nbutton cliked");
}

void btnClickSound()
{
  tone(Alarm, NOTE_C7);
  delay(50);
  noTone(Alarm);
}

void HWscreenWrite() {
  float t = sensor_HW.getTemp();
  String str1 = "Горячая вода"; 
  String str2 = String(t) + " °C";

  
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(str1);
  lcd.setCursor(4, 1);
  lcd.print(str2);
  
  Serial.print("\nstr1: ");
  Serial.println(str1);
  Serial.print("str2: ");
  Serial.println(str2);
  Serial.print("Temperature: ");
  Serial.println(t);
}

void CWscreenWrite() {

}

void PRNscreenWrite() {

}