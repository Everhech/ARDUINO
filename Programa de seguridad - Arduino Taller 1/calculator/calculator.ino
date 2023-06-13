/**
   Arduino Calculator

   Copyright (C) 2020, Uri Shaked.
   Released under the MIT License.
*/

/*
PARTICIPANTES:
- Edwin Orlando Restrepo Mosquera
- Juan Pablo Rivera Portilla
- Diego Alejandro Piamba Escobar
*/

/*Solo documentar las funciones.
 construir un sistema de seguridad de contraseña, máximo de 3 intentos
 cuando llegue a 3 intentos, escribir sistema bloqueado
  Si el password es correcto escribe passwod correcto
 si el password es incorrecto escribe password incorrecto*/

#include <LiquidCrystal.h>
#include <Keypad.h>

/* Display */
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);  //Prueba en el Wokwi
const int Led_rojo = 22;
const int Led_verde = 24;
const int Led_azul = 28;
/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
const int max_intentos = 3;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A4, A3, A2, A1};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'#', '0', '=', 'D'}
};

//password min= 4 max = 8
char password[9] = "";
unsigned char idx = 0;
char contrasenia[9] = "56A";
int intentos = 0;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16,2);
  pinMode(Led_rojo,OUTPUT);
  pinMode(Led_verde,OUTPUT);
  pinMode(Led_azul,OUTPUT);
  lcd.print("Hello, world!");
  delay (2000); // 2 segundos
  lcd.clear();
  lcd.print("CONTRASENA: ");
  lcd.setCursor(0,1);
}

void loop() {
  char key = keypad.getKey();
  if (key){
    limite(key);
  }
}


//Procedimiento "limite" encargado del límite de escritura
//para la contraseña, además de permitir comprobar
//la contraseña.
void limite(char key){
  if(key == '='){
    comprobar();
    return;
  }
  if(strlen(password) < sizeof(password)-1){
    password[idx++] = key;
    lcd.print("*");
  }  
}


//Procedimiento para comprobar si la contraseña
//digitada es correcta o no, teniendo 3 intentos
void comprobar(){
  if(strcmp(contrasenia, password) == 0){
    lcd.clear();
    digitalWrite(Led_verde, HIGH);
    lcd.setCursor(0,0);
    lcd.print("Password");
    lcd.setCursor(0,1);
    lcd.print("Succesfully");    
    delay(2000);
    lcd.clear();
    lcd.print("Entraste");
    digitalWrite(Led_verde, LOW);
    return;
  }else{
    lcd.clear();
    digitalWrite(Led_rojo, HIGH);
    lcd.setCursor(0,0);
    lcd.print("Password");
    lcd.setCursor(0,1);
    lcd.print("incorrect");
    digitalWrite(Led_rojo, LOW);
    intentos++;
    idx = 0;

    if(intentos >= max_intentos){
      lcd.clear();
      digitalWrite(Led_azul, HIGH);
      lcd.setCursor(0,0);
      lcd.print("Sistema");
      lcd.setCursor(0,1);
      lcd.print("Bloqueado");
      exit(0);
    }
  }  
  delay(2000);
  lcd.clear();
  borrar_contrasenia();
}


//Procedimiento para vaciar el array de la contraseña
//para así permitir el ingreso de nueva información
//para la nueva clave.
void borrar_contrasenia(){
  for(int i = 0; i < sizeof(password); i++){
    password[i] = '\0';
  }
}
