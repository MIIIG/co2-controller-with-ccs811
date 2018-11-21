#include "Arduino.h"
#include "Adafruit_CCS811.h" //CO2 official library
#include "SPI.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h> //Encoder button library by Paul Stoffregen
//#include "Timer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h> // Датчик влажности
#include <DHT_U.h> // DHT sensor library by Adafruit

Adafruit_CCS811 ccs;

#define DHTPIN            6 
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);

int tem;
int hum;

// Объект класса для дисплея
LiquidCrystal_I2C lcd(0x3F,16,2); // 0x27, 20, 4 - 0x5A - 0x3F
#define ENCODER_DO_NOT_USE_INTERRUPTS
Encoder myEnc(2, 3); //подключение энкодера
int menu = 1;
int interval = 1000;
int interval_enc = 400;
int interval_lcd = 250;
int interval_get_co2 = 15000;
int interval_get_temp = 2000;
int eCO2;
int TVOC;
int flag=0;
long newppm;
long newtemp;
long newhum;
long criticalppm;
long criticaltemp;
long criticalhum;
long time1;
long time2;
long time3;
long time4;
long time5;
long encoder_ppm;
long encoder_temp;
long encoder_hum;
void get_co2(void);
void get_temp(void);
void lcd_working(void);
void relay_working(void);
void encoder_work(void);

//Timer t; // Объект класса для энкодера для задания интервалов и измерений

void setup() {
Serial.begin(9600);

Serial.println("CO2, Temp, Hum Detecting And Control");

if(!ccs.begin()){
  Serial.println("Failed to start sensor! Please check your wiring.");
  while(1);
}

//calibrate temperature sensor
while(!ccs.available());//!ccs.available()
float temp = ccs.calculateTemperature();
ccs.setTempOffset(temp - 29.5);
// инициализруем датчик влажности и температуры
dht.begin();
// объект класса для работы с библиотекой адафруит
sensor_t sensor;
// снимаем показания с сенсора Темп-ы и Влаж-и
dht.temperature().getSensor(&sensor);
dht.humidity().getSensor(&sensor);
// подключение реле
pinMode(7,OUTPUT); 
digitalWrite(7,HIGH);
// подключаем кнопку энкодера
pinMode(14, INPUT); 
digitalWrite(14,HIGH);
// инициализация LCD
lcd.begin();
// темный фон, светлые буквы
lcd.backlight();
//lcd.print("Control debugger!");
// задаем время для таймера
//t.every(15000, get_co2);
//t.every(2000, get_temp);
}

void loop() {
  //t.update();
  get_co2();
  get_temp();
  lcd_working();
  relay_working();
  encoder_work();
}
//=================================================
void get_co2() {
  if (time1 + interval < millis()) {
    //Serial.println("get co2 handle");
    /*if(ccs.available()){
      if(!ccs.readData()){
        Serial.print("CO2: ");
        Serial.print(ccs.geteCO2());
        Serial.print("ppm, TVOC: ");
        Serial.print(ccs.getTVOC());
        Serial.print(" \n");
      }
      else{
        Serial.println("ERROR 1!");
        while(1);
      }
    }*/
    if(ccs.available()){
      //float temp = ccs.calculateTemperature();
      if(ccs.readData()){ // !css.readData()
        //Serial.println("DEBUG 2!");
        eCO2 = ccs.geteCO2();
        Serial.println(ccs.geteCO2());
      } else {
        //Serial.println("ERROR 2!");
        while(1);
      }
    }
    time1 = millis();
  }
}
//=================================================
void get_temp(){
  if (time2 + interval_get_temp < millis()) {
    //Serial.println("get temp handle");
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    tem=event.temperature;
    dht.humidity().getEvent(&event);
    hum=event.relative_humidity;

    time2 = millis();
  }
}
void lcd_working() {
  if (time3 + interval_enc < millis()) {
    //Serial.println("lcd work handle");
    switch (menu) {
      case 1:
        Serial.println("lcd menu 1");
        lcd.clear();
        break;
      case 2:
        Serial.println("lcd menu 2");
        lcd.setCursor(0, 1);
        lcd.print("CO2=");
        lcd.print(eCO2);lcd.print(" ");
        lcd.setCursor(9, 1);
        lcd.print("VC:");
        lcd.print(String(criticalppm) + "  ");
        break;
      case 3:
        lcd.clear();
        break;
      case 4:
        lcd.setCursor(0, 1);
        lcd.print("CO2=");
        //lcd.print(String(eCO2) + " ");
        lcd.print(eCO2);lcd.print(" ");
        lcd.setCursor(9, 1);
        lcd.print("VT:");
        lcd.print(String(criticaltemp) + "  ");
        break;
      case 5:
        lcd.clear();
        break;
      case 6:
        lcd.setCursor(0, 1);
        lcd.print("CO2=");
        lcd.print(eCO2);lcd.print(" ");
        lcd.setCursor(9, 1);
        lcd.print("VH:");
        lcd.print(String(criticalhum) + "  ");
        break;
    }
    lcd.setCursor(0, 0);
    lcd.print("T=");
    lcd.print(tem);
    lcd.print("C");
    lcd.setCursor(9, 0);
    lcd.print("H=");
    lcd.print(hum);
    lcd.print("%");

    time3 = millis();
  }
}
//=================================================
void encoder_work() {
  if (time4 + interval_enc < millis()) {
    //Serial.println("encoder work handle");
    if (digitalRead(14) == 0) {
      menu++;
    } else if (menu >= 7) {
      menu = 1;
    }

    switch (menu) {
      case 1:
      {
        //Serial.println("if ppm first");
        myEnc.write(encoder_ppm);
        //Serial.println(criticalppm);
        menu++;
      }break;
      case 2:
      {
        Serial.println("co2 menushka");
        Serial.println(String(criticalppm) + " " + "CO2");
        Serial.println(String(criticaltemp) + " " + "temp");
        Serial.println(String(criticalhum) + " " + "hum");

        encoder_ppm = myEnc.read() ;
        //old_position = current_position * 2.5;
        /*Serial.println(old_position);Serial.println("old_position");
        if (old_position != new_position) {
          new_position = old_position;
          Serial.println(new_position);Serial.println("new_position");
        }
        newppm = new_position;*/
        //newppm = myEnc.read();
        newppm = encoder_ppm * 2.5;
        criticalppm = -999;
          if (newppm != criticalppm) {
            criticalppm = newppm;
            //Serial.println(newppm);
          }
      }break;
      case 3:
      {
        //Serial.println("if temp first");
        myEnc.write(encoder_temp);
        //Serial.println(criticaltemp);
        menu++;
      }break;
      case 4:
      {
        //Serial.println("temp menushka");
        //Serial.println(String(criticalppm) + " " + "CO2");
        //Serial.println(String(criticaltemp) + " " + "temp");
        //Serial.println(String(criticalhum) + " " + "hum");
        encoder_temp = myEnc.read();
        newtemp = encoder_temp / 4;
        criticaltemp = -99;
        if (newtemp != criticaltemp) {
          criticaltemp = newtemp;
          //Serial.println(newtemp);
        }
      }break;
      case 5:
      {
        //Serial.println("if hum first");
        myEnc.write(encoder_hum);
        //Serial.println(criticalhum);
        menu++;
      }break;
      case 6:
      {
        //Serial.println("hum menushka");
        //Serial.println(String(criticalppm) + " " + "CO2");
        //Serial.println(String(criticaltemp) + " " + "temp");
        //Serial.println(String(criticalhum) + " " + "hum");
        encoder_hum = myEnc.read();
        newhum = encoder_hum / 4;
        criticalhum = -99;
        if (newhum != criticalhum) {
          criticalhum = newhum;
          //Serial.println(newhum);
        }
      }break;
    }
    time4 = millis();
  }
}
//=================================================
void relay_working() {
  if (time5 + interval < millis()) {
    //Serial.println("air working handle");
    if (criticalppm <= eCO2) {
      digitalWrite(7,LOW);
      //Serial.println("Turn off Air!");
    } else {
      digitalWrite(7,HIGH);
      //Serial.println("On Air!");
    }
    if (criticaltemp <= tem) {
      digitalWrite(8,LOW);
      //Serial.println("Turn off Air!");
    } else {
      digitalWrite(8,HIGH);
      //Serial.println("On Air!");
    }
    if (criticalhum <= hum) {
      digitalWrite(9,LOW);
      //Serial.println("Turn off Air!");
    } else {
      digitalWrite(9,HIGH);
      //Serial.println("On Air!");
    }
    time5 = millis();
  }
}