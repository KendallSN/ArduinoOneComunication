// ===== EMISOR =====
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {4, 5, 6, 7};
byte colPins[COLS] = {8, 9, 10, 11};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);


byte origen = 0x01;
byte destino = 0x02;
String mensaje = "HOLA";

const int pinBoton = 2;
bool botonPresionado = false;

byte calcularCRC(byte* datos, int longitud) {
  byte crc = 0;
  for (int i = 0; i < longitud; i++) {
    crc ^= datos[i];
  }
  return crc;
}

void setup() {
  Serial.begin(9600);
  pinMode(pinBoton, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  // ===== SINCRONIZACION =====
  sincronizarConReceptor();

  lcd.setCursor(0, 0);
  lcd.print(mensaje);
}

void loop() {

  char tecla = keypad.getKey();
  // ===== Modificar Mensaje =====
  if (tecla) {
    if (tecla == '*') { // * reiniciar
      mensaje = "HOLA";
    } else {
      mensaje += tecla;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(mensaje);
  }

  if (mensaje.length() > 0 && digitalRead(pinBoton) == LOW && !botonPresionado) {
    botonPresionado = true;
    // ===== Actualizar para limpiar antiguas respuestas =====
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(mensaje);

    // ===== Capa de Aplicación =====
    byte longitud = mensaje.length();
    byte trama[256];
    trama[0] = origen;
    trama[1] = destino;
    trama[2] = longitud;
    for (int i = 0; i < longitud; i++) {
      trama[3 + i] = mensaje[i];
    }
    byte crc = calcularCRC(trama, 3 + longitud);
    trama[3 + longitud] = crc;

    // === Enviar trama (NRZ) ===
    for (int i = 0; i < 4 + longitud; i++) {
      Serial.write(trama[i]);
    }

    // ===== Leer respuesta =====
    delay(5);
    byte buffer[256];
    int i = 0;
    while (Serial.available()) {
      buffer[i++] = Serial.read();
      delay(5);
    }

    if (i >= 4) {
      byte len = buffer[2];
      byte crc_rx = buffer[3 + len];
      byte crc_calc = calcularCRC(buffer, 3 + len);
      if (crc_rx == crc_calc) {
        String respuesta = "";
        for (int j = 0; j < len; j++) {
          respuesta += (char)buffer[3 + j];
        }
        lcd.setCursor(0, 1);
        lcd.print(respuesta);
      } else {
        lcd.setCursor(0, 1);
        lcd.print("CRC R invalido");
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("No respuesta");
    }
  } else if (digitalRead(pinBoton) == HIGH) {
    botonPresionado = false;
  }
}

void sincronizarConReceptor() {
  bool ready = false;
  while (!ready) {
    delay(100);
    String mensajePing = "Ping";
    // ===== Capa de Aplicación =====
    byte longitud = mensajePing.length();
    byte trama[256];
    trama[0] = origen;
    trama[1] = destino;
    trama[2] = longitud;
    for (int i = 0; i < longitud; i++) {
      trama[3 + i] = mensajePing[i];
    }
    byte crc = calcularCRC(trama, 3 + longitud);
    trama[3 + longitud] = crc;

    // ===== Enviar trama (NRZ) =====
    for (int i = 0; i < 4 + longitud; i++) {
      Serial.write(trama[i]);
    }

    // ===== Leer respuesta =====
    delay(50);
    byte buffer[256];
    int i = 0;
    while (Serial.available()) {
      buffer[i++] = Serial.read();
      delay(5);
    }
    // ===== Validar CRC =====
    if (i >= 4) {
      byte len = buffer[2];
      byte crc_rx = buffer[3 + len];
      byte crc_calc = calcularCRC(buffer, 3 + len);
      if (crc_rx == crc_calc) {
        String respuesta = "";
        for (int j = 0; j < len; j++) {
          respuesta += (char)buffer[3 + j];
        }
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Ready");
        ready = true;
      } else {
        lcd.setCursor(0, 1);
        lcd.print("CRC invalido");

      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Sincronizando");
    }
  }
}