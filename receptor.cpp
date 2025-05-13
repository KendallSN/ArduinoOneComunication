// ===== RECEPTOR ===== //
byte origen = 0x02; // ID del receptor (inverso al transmisor)

void setup() {
  Serial.begin(9600);
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
    for(int i=0; i<255; i++){
      Serial.print(buffer[i]);
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
      // === Capa de APLICACIÓN ("App Datos") ===
      // Mostrar mensaje recibido
      Serial.print("[App Datos] Mensaje recibido: ");
      for (int k = 0; k < longitud; k++) {
        Serial.write(buffer[3 + k]);
      }
      Serial.println();

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
      Serial.println();

    } else {
      Serial.println("[ERROR] CRC incorrecto. Trama descartada.");
    }
  }
}

// Función CRC (debe estar también en el receptor)
byte calcularCRC(byte* datos, int longitud) {
  byte crc = 0;
  for (int i = 0; i < longitud; i++) {
    crc ^= datos[i];
  }
  return crc;
}