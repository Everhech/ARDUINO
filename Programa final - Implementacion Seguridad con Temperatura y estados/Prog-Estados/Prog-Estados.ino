#include <StateMachine.h>
#include "AsyncTaskLib.h"
#include <LiquidCrystal.h>
#include <Keypad.h>
#include "DHTStable.h"
//#include "DHT.h"

// Inicialización del objeto LiquidCrystal
LiquidCrystal lcd(23, 25, 27, 29, 31, 33);

// Configuración del teclado
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {41, 43, 45, 47}; 
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', '*'},
  {'.', '0', '=', '/'}
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

#define DHT11_PIN 6

// Inicialización del objeto DHTStable
DHTStable DHT;

// Definición de constantes y pines
#define BEZ_SENSOR 13
const int photocellPin = A7;
const int Led_rojo = 49;
const int Led_verde = 51;
const int Led_azul = 53;
#define PULSADOR 8		// pulsador en pin 2

// Variables de contraseña y bloqueo
char password[6];
char realpass[6] = "00000";
int idx = 0;
int bloq = 3;
boolean enter = false;
int outputValue = 0;

// Declaración de funciones de los estados
void securityEntryState(void);
void eventDoorsAndWindowsState(void);
void environmentalMonitoringState(void);
void environmentalAlarmState(void);
void handleSecurityAlert(void);

// Declaración de funciones auxiliares para lectura de sensores
void readPhotoresistor(void);
void readTemperature(void);

// Declaración de tareas asíncronas
AsyncTask taskEnvironmentalMonitoring(2000, true, environmentalMonitoringState);
AsyncTask taskBeforeEnvironmentalMonitoring(3000, true, eventDoorsAndWindowsState);
AsyncTask taskAlarmAlert(3000, true, handleSecurityAlert);
AsyncTask taskEventAlarm(3000, true, eventDoorsAndWindowsState);

// Declaración de la máquina de estados y sus estados
StateMachine machine = StateMachine();
State* stSecurityEntry = machine.addState(&securityEntryState);
State* stEventDoorsAndWindows = machine.addState(&eventDoorsAndWindowsState);
State* stEnvironmentalMonitoring = machine.addState(&environmentalMonitoringState);
State* stEnvironmentalAlarm = machine.addState(&environmentalAlarmState);
State* stHandleSecurityAlert = machine.addState(&handleSecurityAlert);

void setup() {
  Serial.begin(115200);
  pinMode(Led_rojo, OUTPUT);
  pinMode(Led_verde, OUTPUT);
  pinMode(Led_azul, OUTPUT);

  // Agregar transiciones entre estados
  stSecurityEntry->addTransition(&checkCorrectPassword, stEventDoorsAndWindows);
  stEventDoorsAndWindows->addTransition(&checkTimeout2sec, stEnvironmentalMonitoring);
  stHandleSecurityAlert->addTransition(&checkTimeout6sec, stEventDoorsAndWindows);
  stEnvironmentalMonitoring->addTransition(&checkTemperatureOver32, stEnvironmentalAlarm);
  stEnvironmentalMonitoring->addTransition(&checkTimeout10sec, stEventDoorsAndWindows);
  stEnvironmentalAlarm->addTransition(&checkTemperatureOver32_5s, stHandleSecurityAlert);
  stEnvironmentalAlarm->addTransition(&checkTemperatureBelow30, stEnvironmentalMonitoring);

  // Inicialización del objeto LiquidCrystal
  lcd.begin(16, 2);

  lcd.setCursor(3, 0);
  lcd.print("Bienvenido");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ingrese la clave");
  lcd.setCursor(5, 1);
}

void loop() {
  machine.run();
}

// Estado de ingreso de seguridad
void securityEntryState() {
  Serial.println("Ingreso de Seguridad");
  char key = keypad.getKey();

  if (key) {
    password[idx] = key;
    idx++;
    lcd.print("*");
  }

  if (idx == 5) {
    bool isCorrectPassword = true;
    for (int i = 0; i < 5; i++) {
      if (password[i] != realpass[i]) {
        isCorrectPassword = false;
        break;
      }
    }

    lcd.clear();
    processing();
    delay(300);
    lcd.clear();

    if (isCorrectPassword) {
      digitalWrite(Led_verde, HIGH);
      lcd.setCursor(2, 1);
      lcd.print("Bienvenido");
      digitalWrite(Led_verde, LOW);
    } else {
      bloq--;
      if (bloq > 0) {
        digitalWrite(Led_rojo, HIGH);
        lcd.setCursor(2, 0);
        lcd.print("cont.Incorrecta");
        delay(300);
        digitalWrite(Led_rojo, LOW);
        lcd.setCursor(0, 1);
        lcd.print("digite de nuevo");
        delay(2000);
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Contrasenia");
        lcd.setCursor(5, 1);
      } else {
        lcd.setCursor(0, 0);
        lcd.println("Sistema");
        lcd.setCursor(0, 1);
        lcd.println("Bloqueado");
        digitalWrite(Led_azul, HIGH);
        exit(0);
      }
    }

    idx = 0;
  }
}

// Función para verificar la contraseña ingresada
bool checkCorrectPassword() {
  if (strcmp(password, realpass) == 0) {
    taskEnvironmentalMonitoring.Start();
    return true;
  }
  return false;
}

// Estado de puertas y ventanas
void eventDoorsAndWindowsState() {
  Serial.println("Puertas y Ventanas");
  taskEnvironmentalMonitoring.Update();
}

// Función para verificar el tiempo de espera de 2 segundos
bool checkTimeout2sec() {
  if (!taskEnvironmentalMonitoring.IsActive()) {
    taskEnvironmentalMonitoring.Stop();
    taskEnvironmentalMonitoring.Reset();
    return true;
  }
  return false;
}

// Función para verificar si se activó una puerta o ventana
bool checkDoorWindowActivated() {
  return true;
}

// Estado de monitoreo ambiental
void environmentalMonitoringState() {
  Serial.println("Monitor Ambiental");
  readTemperatureAndPhotoresistor();
  taskBeforeEnvironmentalMonitoring.Stop();
  taskBeforeEnvironmentalMonitoring.Reset();
}

// Función para verificar si la temperatura supera los 32 grados Celsius
bool checkTemperatureOver32() {
  if (DHT.getTemperature() > 26) {
    taskBeforeEnvironmentalMonitoring.Stop();
    taskBeforeEnvironmentalMonitoring.Reset();
    taskAlarmAlert.Start();
    return true;
  }
  return false;
}

// Función para verificar el tiempo de espera de 10 segundos
bool checkTimeout10sec() {
  if (!taskBeforeEnvironmentalMonitoring.IsActive()) {
    taskBeforeEnvironmentalMonitoring.Stop();
    taskBeforeEnvironmentalMonitoring.Reset();
    taskEnvironmentalMonitoring.Start();
    return true;
  }
  return false;
}

// Función para leer la temperatura y el valor del fotoresistor
void readTemperatureAndPhotoresistor() {
  readTemperature();
  readPhotoresistor();
}

// Estado de alarma ambiental
void environmentalAlarmState() {
  Serial.println("Alarma Ambiental");
  tone(BEZ_SENSOR, 262, 250);
  taskAlarmAlert.Update();
}

// Función para verificar si la temperatura supera los 32 grados Celsius durante 5 segundos
bool checkTemperatureOver32_5s() {
  if (!taskAlarmAlert.IsActive()) {
    taskAlarmAlert.Stop();
    taskAlarmAlert.Reset();
    taskEventAlarm.Reset();
    return true;
  }
  return false;
}

// Función para verificar si la temperatura está por debajo de los 30 grados Celsius
bool checkTemperatureBelow30() {
  if (DHT.getTemperature() < 30) {
    taskAlarmAlert.Reset();
    return true;
  }
  return false;
}

// Estado de manejo de la alerta de seguridad
void handleSecurityAlert() {
  Serial.println("Alerta de Seguridad");
  taskEventAlarm.Update();
}

// Función para verificar el tiempo de espera de 6 segundos
bool checkTimeout6sec() {
  if (!taskEventAlarm.IsActive()) {
    taskEventAlarm.Stop();
    taskEventAlarm.Reset();
    taskEnvironmentalMonitoring.Start();
    return true;
  }
  return false;
}

// Función para mostrar "Procesando" en la pantalla LCD
void processing() {
  for (int i = 0; i < 3; i++) {
    lcd.setCursor(2, 0);
    lcd.print("Procesando");
    lcd.setCursor(12, 0);
    delay(200);
    lcd.print(".");
    lcd.setCursor(13, 0);
    delay(200);
    lcd.print(".");
    lcd.setCursor(14, 0);
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.clear();
  }
}

// Función para leer la temperatura
void readTemperature() {
  int chk = DHT.read11(DHT11_PIN);

  const char* errorMessages[] = {
    "OK",
    "Checksum error",
    "Time out error",
    "Unknown error"
  };

  if (chk >= 0 && chk < sizeof(errorMessages) / sizeof(errorMessages[0])) {
    Serial.print(errorMessages[chk]);
  } else {
    Serial.print("Invalid error code");
  }

  float humidity = DHT.getHumidity();
  float temperature = DHT.getTemperature();

  lcd.setCursor(0, 1);
  lcd.print("H%:");
  lcd.setCursor(4, 1);
  lcd.print(humidity, 1);
  Serial.print(",\t");
  Serial.print(humidity, 1);
  lcd.setCursor(9, 1);
  lcd.print("T:");
  lcd.setCursor(12, 1);
  lcd.print(temperature, 1);
  Serial.print(temperature, 1);
  Serial.println();
}

// Función para leer el valor del fotoresistor
void readPhotoresistor() {
  int lightIntensity = analogRead(photocellPin);
  lcd.setCursor(0, 0);
  lcd.print("Luz:");
  lcd.setCursor(5, 0);
  lcd.print(lightIntensity);
  Serial.print("Luz = ");
  Serial.println(lightIntensity);

  if (lightIntensity > 100) {
    digitalWrite(Led_azul, HIGH);
  } else {
    digitalWrite(Led_azul, LOW);
  }
}