// ===== TRANSMISOR ===== //
byte origen = 0x01;      // ID del transmisor
byte destino = 0x02;     // ID del receptor
String mensaje = "HOLA"; // Mensaje a enviar

const int pinBoton = 2;  // Pin donde está conectado el botón
bool botonPresionado = false;

// Función para calcular CRC (XOR de todos los bytes)
byte calcularCRC(byte* datos, int longitud) {
  byte crc = 0;
  for (int i = 0; i < longitud; i++) {
    crc ^= datos[i];
  }
  return crc;
}

void setup() {
  Serial.begin(9600);         // Inicia comunicación serial
  pinMode(pinBoton, INPUT_PULLUP); // Configura el pin del botón con resistencia pull-up interna
}

void loop() {
  if (digitalRead(pinBoton) == LOW) { // Botón presionado (activo en bajo)
    if (!botonPresionado) { // Detectar flanco descendente (una sola vez por pulsación)
      botonPresionado = true;

      // === Capa de APLICACIÓN ===
      byte longitud = mensaje.length();
      byte trama[256];

      // Construir trama
      trama[0] = origen;
      trama[1] = destino;
      trama[2] = longitud;

      for (int i = 0; i < longitud; i++) {
        trama[3 + i] = mensaje[i];
      }

      byte crc = calcularCRC(trama, 3 + longitud);
      trama[3 + longitud] = crc;

      // === Capa FÍSICA ===
      for (int i = 0; i < 4 + longitud; i++) {
        Serial.write(trama[i]);
      }
      Serial.println(); // Fin de trama

      Serial.println("Emisor: Mensaje enviado.");
    }
  } else {
    botonPresionado = false; // Reinicia el estado cuando se suelta el botón
  }
}
