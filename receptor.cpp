#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Dirección 0x27 es común, pantalla de 16x2 caracteres
LiquidCrystal_I2C lcd(0x27, 16, 2);


// ===== RECEPTOR =====
byte origen = 0x02;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Esperando msg...");

}

void loop() {
  if (Serial.available()) {
    byte buffer[256];
    int i = 0;

    // Leer trama entera (con breve retraso para sincronización)
    delay(50);
    while (Serial.available()) {
      buffer[i++] = Serial.read();
    }
    // === Capa de ENLACE ("Trama Link") ===
    // Verificar CRC
    byte longitud = buffer[2];
    byte crc_calculado = 0;
    for (int j = 0; j < 3 + longitud; j++) {
      crc_calculado ^= buffer[j];
    }
    byte crc_recibido = buffer[3 + longitud];
    
    if (crc_calculado == crc_recibido) {

      char mensaje[17]; // LCD 16x2, máximo 16 caracteres por línea
      memcpy(mensaje, &buffer[3], min(16, longitud));
      mensaje[min(16, longitud)] = '\0';

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mensaje Recibido:");
      lcd.setCursor(0, 1);
      lcd.print(mensaje);

      // === Capa de APLICACIÓN ("App Datos") ===
      // Mostrar mensaje recibido
      //Serial.print("[App Datos] Mensaje recibido: ");
      
      //Serial.println();

      // Responder "OK" (mismo proceso que el transmisor)
      String respuesta = "OK";
      byte trama_respuesta[256];
      trama_respuesta[0] = origen;
      trama_respuesta[1] = buffer[0]; // ORIGEN original ahora es DESTINO
      trama_respuesta[2] = respuesta.length();

      for (int l = 0; l < respuesta.length(); l++) {
        trama_respuesta[3 + l] = respuesta[l];
      }

      byte crc_respuesta = calcularCRC(trama_respuesta, 3 + respuesta.length());
      trama_respuesta[3 + respuesta.length()] = crc_respuesta;

      // Enviar respuesta
      for (int m = 0; m < 4 + respuesta.length(); m++) {
        Serial.write(trama_respuesta[m]);
      }
      
      Serial.print("Respuesta a:" );
      for (int k = 0; k < longitud; k++) {
        Serial.write(buffer[3 + k]);
      }
      Serial.println();
    } else {
      //Serial.println("[ERROR] CRC incorrecto. Trama descartada.");
    }
  }
}

byte calcularCRC(byte* datos, int longitud) {
  byte crc = 0;
  for (int i = 0; i < longitud; i++) {
    crc ^= datos[i];
  }
  return crc;
}
