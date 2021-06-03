// заметки
// myswitch убираем у нас радио на Serial1
// SD убираем у нас оно на Serial2
// остальное вроде нормально

//Импортируем библиотеки
#include "SD.h"
#include <Wire.h> 
#include <MS5611.h>
#include "MAX6675_Thermocouple.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <RCSwitch.h>

//Ввод данных
int loops = 0;
//Радио модуль
RCSwitch mySwitch = RCSwitch();
//Акселеромерт
MPU6050 mpu;
//Термопара на 500
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
MAX6675_Thermocouple* thermocouple = NULL;
//Термопара 1000
int thermoDO_1 = 7;
int thermoCS_1 = 8;
int thermoCLK_1 = 9;
MAX6675_Thermocouple* thermocouple_1 = NULL;
MS5611 ms5611;
double referencePressure;


void setup() {
   pinMode(10, OUTPUT); 
   pinMode(11, OUTPUT);
   pinMode(12, OUTPUT);
   pinMode(13, OUTPUT);
   //Объявляем пины термопары 1
   thermocouple = new MAX6675_Thermocouple(thermoDO, thermoCS, thermoCLK);
   //Объявляем пины термопары 2
   thermocouple_1 = new MAX6675_Thermocouple(thermoDO_1, thermoCS_1, thermoCLK_1);
   // получаем начальное давление для правильных вычислений 0_0
   referencePressure = ms5611.readPressure();
   //Радио модуль
   mySwitch.enableTransmit(1);
   mySwitch.enableReceive(0);
}

void loop() {
    // Получение предворительных данных
  uint32_t rawTemp = ms5611.readRawTemperature();
  uint32_t rawPressure = ms5611.readRawPressure();
  // Получение настоящих данных
  double realTemperature = ms5611.readTemperature();
  long realPressure = ms5611.readPressure();
  // Подсчет и получение
  float absoluteAltitude = ms5611.getAltitude(realPressure);
  float relativeAltitude = ms5611.getAltitude(realPressure, referencePressure);
  const double celsius = thermocouple->readCelsius();
  const double celsius_1 = thermocouple_1->readCelsius();
  
  //Включаем 2 реле
  int value = mySwitch.getReceivedValue();
  if (value == "St" or loops == 100){
    digitalWrite(10, HIGH);
    digitalWrite(12, HIGH);
  
  //Если 1 реле не работает, влючаем второе
  if (celsius < 500){
    digitalWrite(10, HIGH);
    if (digitalRead(10) == LOW){
       digitalWrite(11, HIGH);
      }else{
        digitalWrite(11, LOW);
        }
    }
  if (celsius_1 < 1000){
    digitalWrite(12, HIGH);
    if (digitalRead(12) == LOW){
       digitalWrite(13, HIGH);
      }else{
        digitalWrite(13, LOW);
        }
    }  
    loops = 99
  }

  mySwitch.send("; R1:", digitalRead(10));
  mySwitch.send("; R2:", digitalRead(11));
  mySwitch.send("; R3:", digitalRead(12));
  mySwitch.send("; R4", digitalRead(13));
  mySwitch.send("; C:", celsius);
  mySwitch.send("; C_1:", celsius_1);
    
  //Сохранение на SD курту
  //Получение данных с акселерометра
  Vector rawAccel = mpu.readRawAccel();
  Vector rawGyro = mpu.readRawGyro();
  //Открываем файл
  File dataFile = SD.open("LoggerCD.txt", FILE_WRITE);
  // если файл доступен, записываем в него
  if (dataFile) {
    //Значение с реле
    dataFile.println("R1:"); 
    dataFile.print(digitalRead(10));
    dataFile.print("; R2:"); 
    dataFile.print(digitalRead(11));
    dataFile.print("; R3:"); 
    dataFile.print(digitalRead(12));
    dataFile.print("; R4:"); 
    dataFile.print(digitalRead(13));
    //Температура с термопар
    dataFile.print("; C:"); 
    dataFile.print(celsius);
    dataFile.print("; C_1:"); 
    dataFile.print(celsius_1);
    //Модуль датчиков
    //Акси
    dataFile.print("; AX:");
    dataFile.print(rawAccel.XAxis);
    dataFile.print("; AY");
    dataFile.print(rawAccel.YAxis);
    dataFile.print("; AZ:");
    dataFile.print(rawAccel.ZAxis);
    //Гиро
    dataFile.print("; GX");
    dataFile.print(rawGyro.XAxis);
    dataFile.print("; GY");
    dataFile.print(rawGyro.YAxis);
    dataFile.print("; GZ");
    dataFile.println(rawGyro.ZAxis);
    
    dataFile.print("; rT:"); 
    dataFile.print(rawTemp);
    dataFile.print("; rP:"); 
    dataFile.print(rawPressure);
    dataFile.print("; T:"); 
    dataFile.print(realTemperature);
    dataFile.print("; P:"); 
    dataFile.print(realPressure);
    dataFile.print("; A:"); 
    dataFile.print(absoluteAltitude);
    dataFile.close();
  }
  loops = loops + 1;
  delay(5000);
}
