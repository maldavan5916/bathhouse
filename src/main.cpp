//управление водоснабжения бани

#include "pitches.h"
#include <LCD_1602_RUS.h>
#include <microDS18B20.h>


LCD_1602_RUS lcd(0x27, 16, 2);  // Устанавливаем дисплей (адрес, и т.д.)

MicroDS18B20<2> sensor_HW;   //датчик температуры горячей воды (D2)
MicroDS18B20<9> sensor_CW;   //датчик температуры холодной воды (D9)
MicroDS18B20<7> sensor_PRN;  //датчик температуры парной (D7)


const int Button = 5;   // Номер пина подключения кнопки
const int SSR100 = 10;  // Номер пина подключения SSR100
const int Heater = 12;  // Номер выхода нагреватель счётчика
const int Alarm = 13;   // Номер выхода Alarm

//------------------------------------------------------
byte SEL = 1;   // переменная для выбора номера экрана
byte Flag = 0;  // переменная для выбора сотояния нагревателя
bool KEY;       // переменная для чтения состояния пина к которому подключена кнопка

void setup() {
  Serial.begin(115200);

  lcd.init();  // инициализация LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(L"Банька V4.0 beta");  // Выводим сообщение"Банька V-3.0 - по русски" на LCD.
  delay(500);
  
  pinMode(Button, INPUT);    // Инициализируем пин, подключенный к кнопке, как вход (кнопка)INPUT_PULLUP
  pinMode(SSR100, OUTPUT);   // Инициализируем пин, подключенный к светодиоду, как выход (SSR100)
  pinMode(Heater, OUTPUT);   // Инициализируем пин, подключенный к светодиоду, как выход (нагреватель счётчика)
  pinMode(Alarm, OUTPUT);    // Инициализируем пин, подключенный к светодиоду, как выход (Alarm)
  digitalWrite(Alarm, LOW);  // выключаем Alarm

  // звук "Успешное включение"
  tone(Alarm, NOTE_A7);
  delay(100);
  tone(Alarm, NOTE_G7);
  delay(100);
  tone(Alarm, NOTE_E7);
  delay(100);
  tone(Alarm, NOTE_C7);
  delay(100);
  tone(Alarm, NOTE_D7);
  delay(100);
  tone(Alarm, NOTE_B7);
  delay(100);
  tone(Alarm, NOTE_F7);
  delay(100);
  tone(Alarm, NOTE_C7);
  delay(100);
  noTone(Alarm);
  lcd.clear();
}

void loop() {
  //_______________ считываем значения с входов ардино_____________________________________________________
  {
    KEY = digitalRead(Button);  //кнопка
    sensor_HW.requestTemp();    //температура горячей воды
    sensor_CW.requestTemp();    // температура холодной воды
    sensor_PRN.requestTemp();   //температура парной
  }
  //_____________________Работа кнопки__________________________________________________________
  {
    // Фиксируем нажатие кнопки
    if (KEY == HIGH) {
      digitalWrite(Alarm, HIGH);  // включаем сигнал
      delay(50);
      SEL++;                     // переключаем на следующий экран
      digitalWrite(Alarm, LOW);
    }
  }
  //____________________Работа экрана___________________________________________________________
  {
    //Температура горячей воды
    if (SEL == 1 && sensor_HW.readTemp()) {
      lcd.setCursor(2, 0);
      lcd.print(L"Горячая вода");
      lcd.setCursor(4, 1);
      lcd.print(sensor_HW.getTemp());  //выводим на экран температуру горячей воды
      lcd.setCursor(10, 1);
      lcd.print(L"°C");
      sensor_HW.requestTemp();
    } else {
      Serial.print("error HW");
      //lcd.setCursor(2, 0);
      //lcd.print(L"Горячая вода");
      lcd.setCursor(5, 1);
      lcd.print(L"ERROR!");
      delay(1000);
      sensor_HW.requestTemp();
      
    }
  }
  // Температура холодной воды
  if (SEL == 2) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print(L"Холодная вода");
    lcd.setCursor(4, 1);
    lcd.print(sensor_CW.getTemp());  //выводим на экран температуру холодной воды
    lcd.setCursor(10, 1);
    lcd.print(L"°C");  //3
  }
  // Температура в парилке
  if (SEL == 3) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print(L"Темп. парной");
    lcd.setCursor(4, 1);
    lcd.print(sensor_PRN.getTemp());  //выводим на экран температуру парной
    lcd.setCursor(10, 1);
    lcd.print(L"°C");
  }


  if (SEL >= 4) {
    SEL = 1;
    lcd.clear();
  }
  //_________________Подогрев счетчика___________________________
  if (sensor_CW.getTemp() <= 3) {
    digitalWrite(Heater, HIGH);  // включаем нагреватель
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(L"ПРОГРЕВ СЧЁТЧИКА");
    lcd.setCursor(4, 1);
    lcd.print(sensor_CW.getTemp());
    lcd.setCursor(10, 1);
    lcd.print(L"°C");
    digitalWrite(Alarm, HIGH);  // включаем сигнал
    delay(50);
    digitalWrite(Alarm, LOW);  // выключаем сигнал
    delay(1000);
    lcd.clear();
  } else {
    Flag = 0;
  }


  if (sensor_CW.getTemp() >= 3) {
    digitalWrite(Heater, LOW);  // выключаем нагреватель
    Flag = 0;
  }


  if (sensor_CW.getTemp() >= 5) {
    digitalWrite(Heater, LOW);  // выключаем нагреватель
  }



  // вывод данных в порт
  {
    Serial.print("t°гор.вода ");
    Serial.println(sensor_HW.getTemp());
    Serial.print("t°хол.вода ");
    Serial.println(sensor_CW.getTemp());
    Serial.print("t° парилка ");
    Serial.println(sensor_PRN.getTemp());

    Serial.print("Кнопка ");
    Serial.println(KEY);

    Serial.print("Экран ");
    Serial.println(SEL);

    Serial.print("Прогр.");
    Serial.println(Flag);

    Serial.println("-------------------------");
    delay(1000);
  }
}







/*
#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}
*/