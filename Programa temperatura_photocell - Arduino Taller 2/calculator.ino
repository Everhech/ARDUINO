/***************************************************
Copyright (c) 2017 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

/*
Presentado por:
Edwin Orlando Restrepo Mosquera
Juan Pablo Rivera Portilla
Diego Alejandro Piamba Escobar
*/

#include "AsyncTaskLib.h"
#include "DHTStable.h"
#include <LiquidCrystal.h>
DHTStable DHT;
int outputValue = 0;
const int photocellPin = A0;
#define DEBUG(a) Serial.print(millis()); Serial.print(": "); Serial.println(a);
void readtemperature();
void readlight();
#define DHT11_PIN       5
AsyncTask asyncTask(1000, true,readlight);
AsyncTask asyncTaskTemp(2000, true, readtemperature);
const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting");
  asyncTaskTemp.Start();
  asyncTask.Start();
  lcd.begin(16,2);
 
}
void loop()
{
  asyncTask.Update();
  asyncTaskTemp.Update();
}
void readlight(){
  lcd.setCursor(0,0);
  lcd.print("Photocell:");
  outputValue = analogRead(photocellPin);//read the value of photoresistor
  lcd.println(outputValue); //print it in serial monitor
}
void readtemperature(){
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      Serial.print("OK, \t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error, \t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error, \t");
      break;
    default:
      Serial.print("Unknown error, \t");
      break;
  }
  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.print(DHT.getHumidity());
  lcd.print(", \t");
  lcd.setCursor(8,1);
  lcd.print("T:");
  lcd.print(DHT.getTemperature());
}