#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {4, 5, 6, 7};    // Pines conectados a R1-R4
byte colPins[COLS] = {8, 9, 10, 11};  // Pines conectados a C1-C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Dirección 0x27 es común, pantalla de 16x2 caracteres
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== TRANSMISOR =====
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
    lcd.setCursor(0, 0);
    lcd.setCursor(0, 0);
    lcd.print("Mensaje:");
    lcd.setCursor(0, 1);
    lcd.print(mensaje);
}

void loop() {

  char tecla = keypad.getKey();

  if (tecla) {
    if (tecla == '*') {
      mensaje = ""; // Reinicia mensaje con '*'
    } else {
      mensaje += tecla;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mensaje enviado:");
    lcd.setCursor(0, 1);
    lcd.print(mensaje);
  }

  if (mensaje.length() > 0 && digitalRead(pinBoton) == LOW && !botonPresionado) {
    botonPresionado = true;

    // === Capa de Aplicación ===
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

    // === Enviar trama (NRZ implícito por UART) ===
    for (int i = 0; i < 4 + longitud; i++) {
      Serial.write(trama[i]);
    }

    //Serial.println("Emisor: Mensaje enviado.");

    // === Leer respuesta ===
    delay(100);
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
        Serial.print("Emisor: Respuesta del receptor: ");
        for (int j = 0; j < len; j++) {
          Serial.write(buffer[3 + j]);
        }
        Serial.println();
      } else {
        Serial.println("Emisor: CRC de respuesta inválido.");
      }
    } else {
      Serial.println("Emisor: No se recibió respuesta.");
    }

  } else if (digitalRead(pinBoton) == HIGH) {
    botonPresionado = false;
  }
}
