// ============================================================
// FirebaseRT.hpp - Firebase Realtime Database (sin libreria externa)
// Usa solo HTTPClient y WiFiClientSecure nativos del ESP32 core
// ============================================================

#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ====== Clientes SSL ======
WiFiClientSecure fbClientPUT;   // Para envio de datos (PUT puntual)
WiFiClientSecure fbClientSSE;   // Para recepcion de comandos (SSE persistente)

// ====== Buffer SSE ======
#define SSE_BUFFER_SIZE 512
char sseBuffer[SSE_BUFFER_SIZE];
int sseBufferPos = 0;
String currentEventType = "";

// ====== Obtener MAC sin ":" y sin ultimos 2 hex ======
// 08:D1:F9:29:E4:A0 -> 08D1F929E4
String getCleanMAC() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char id[11];
  sprintf(id, "%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4]);
  return String(id);
}

// ====== Construir URL base ======
String firebaseURL(String path) {
  String url = "https://" + String(FIREBASE_HOST) + path + ".json";
  if (strlen(FIREBASE_AUTH) > 0) {
    url += "?auth=" + String(FIREBASE_AUTH);
  }
  return url;
}

// ====== INICIALIZACION ======
void firebaseInit() {
  deviceMAC = getCleanMAC();

  // SSL sin verificacion de certificado (desarrollo)
  fbClientPUT.setInsecure();
  fbClientSSE.setInsecure();

  // Timeouts (milisegundos)
  fbClientPUT.setTimeout(10000);   // 10 segundos
  fbClientSSE.setTimeout(30000);   // 30 segundos

  // Forzar primer envio inmediato
  lastFirebaseSend = millis() - FB_SEND_INTERVAL;

  firebaseConectado = true;

  Serial.print("[Firebase] Device MAC: ");
  Serial.println(deviceMAC);
  Serial.printf("[Firebase] Heap libre: %d bytes\n", ESP.getFreeHeap());
}

// ============================================================
//                    ENVIO DE DATOS (PUT)
// ============================================================

// ====== Enviar datos iniciales (una sola vez en setup) ======
bool firebaseSendInitData() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = firebaseURL("/ESP32/" + deviceMAC);

  char json[96];
  snprintf(json, sizeof(json),
    "{\"st\":{\".sv\":\"timestamp\"},"
    "\"rs\":%d,"
    "\"fw\":\"%s\"}",
    WiFi.RSSI(),
    FW_VERSION
  );

  Serial.println("[Firebase] === INIT DATA ===");
  Serial.print("[Firebase] Payload: ");
  Serial.println(json);

  http.begin(fbClientPUT, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PATCH(String(json));
  Serial.printf("[Firebase] Init HTTP Code: %d\n", httpCode);
  http.end();
  return (httpCode == 200);
}

// ====== Construir JSON solo de sensores (periodico) ======
String buildSensorJSON() {
  char json[128];
  snprintf(json, sizeof(json),
    "{\"s\":{\"t\":%.1f,\"p\":%.2f,\"d\":%d,\"tb\":%.2f,\"na\":%d}}",
    temperaturaCelcius,
    ph,
    tds,
    turbidez,
    nivelAgua.estadoFlotador
  );
  return String(json);
}

// ====== Enviar datos a Firebase ======
bool firebaseSendData() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = firebaseURL("/ESP32/" + deviceMAC);

  Serial.println("[Firebase] === ENVIO ===");
  Serial.print("[Firebase] URL: ");
  Serial.println(url);

  http.begin(fbClientPUT, url);
  http.addHeader("Content-Type", "application/json");

  String payload = buildSensorJSON();
  Serial.print("[Firebase] Payload: ");
  Serial.println(payload);

  int httpCode = http.PATCH(payload);

  bool success = (httpCode == 200);
  Serial.printf("[Firebase] HTTP Code: %d | Heap: %d\n", httpCode, ESP.getFreeHeap());

  http.end();
  return success;
}

// ====== Escribir un valor individual en Firebase ======
bool firebaseSetValue(String path, String value) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = firebaseURL(path);

  http.begin(fbClientPUT, url);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.PUT(value);
  http.end();
  return (httpCode == 200);
}

// ============================================================
//              RECEPCION DE COMANDOS (SSE)
// ============================================================

// ====== Parser JSON simple - extraer int ======
int extractIntValue(const char* json, const char* key) {
  const char* pos = strstr(json, key);
  if (!pos) return 0;
  pos += strlen(key);
  // Saltar espacios
  while (*pos == ' ') pos++;
  // Leer numero
  int val = 0;
  int sign = 1;
  if (*pos == '-') { sign = -1; pos++; }
  while (*pos >= '0' && *pos <= '9') {
    val = val * 10 + (*pos - '0');
    pos++;
  }
  return val * sign;
}

// ====== Ejecutar comandos recibidos ======
void executeCommands() {
  // Reset remoto
  if (fbCmd.reset == 1) {
    Serial.println("[Firebase] Comando: REINICIAR");
    firebaseSetValue("/ESP32/" + deviceMAC + "/c/r", "0");
    delay(500);
    ESP.restart();
  }

  // Calibracion pH - vincular con variable existente
  calibracionph = fbCmd.calph;

  // luz, bomba, feeder se procesan en el loop principal
  // segun los GPIOs que se asignen
}

// ====== Procesar comando SSE ======
void processSSECommand(String path, String data) {
  Serial.printf("[SSE] cmd path=%s data=%s\n", path.c_str(), data.c_str());

  if (path == "/") {
    // Objeto completo de /c
    fbCmd.luz    = extractIntValue(data.c_str(), "\"l\":");
    fbCmd.bomba  = extractIntValue(data.c_str(), "\"b\":");
    fbCmd.feeder = extractIntValue(data.c_str(), "\"f\":");
    fbCmd.reset  = extractIntValue(data.c_str(), "\"r\":");
    fbCmd.calph  = extractIntValue(data.c_str(), "\"cp\":");
  }
  else if (path == "/l")  fbCmd.luz    = data.toInt();
  else if (path == "/b")  fbCmd.bomba  = data.toInt();
  else if (path == "/f")  fbCmd.feeder = data.toInt();
  else if (path == "/r")  fbCmd.reset  = data.toInt();
  else if (path == "/cp") fbCmd.calph  = data.toInt();

  executeCommands();
}

// ====== Procesar linea SSE ======
void sseProcessLine(String line) {
  line.trim();

  if (line.startsWith("event:")) {
    currentEventType = line.substring(7);
    currentEventType.trim();
  }
  else if (line.startsWith("data:")) {
    String data = line.substring(6);
    data.trim();

    if (currentEventType == "put" || currentEventType == "patch") {
      // Formato: {"path":"/l","data":1}
      int pathStart = data.indexOf("\"path\":\"") + 8;
      int pathEnd = data.indexOf("\"", pathStart);
      String path = data.substring(pathStart, pathEnd);

      int dataStart = data.indexOf("\"data\":") + 7;
      String dataValue = data.substring(dataStart);
      // Quitar } final del wrapper
      if (dataValue.endsWith("}")) {
        dataValue = dataValue.substring(0, dataValue.length() - 1);
      }
      dataValue.trim();

      processSSECommand(path, dataValue);
    }
    else if (currentEventType == "keep-alive") {
      // Conexion viva, nada que hacer
    }
    else if (currentEventType == "cancel" || currentEventType == "auth_revoked") {
      Serial.printf("[SSE] Stream %s\n", currentEventType.c_str());
      fbClientSSE.stop();
      sseConectado = false;
    }

    currentEventType = "";
  }
}

// ====== Conectar SSE a Firebase ======
bool sseConnect() {
  if (fbClientSSE.connected()) return true;
  if (WiFi.status() != WL_CONNECTED) return false;

  String host = String(FIREBASE_HOST);
  String path = "/ESP32/" + deviceMAC + "/c.json";
  if (strlen(FIREBASE_AUTH) > 0) {
    path += "?auth=" + String(FIREBASE_AUTH);
  }

  Serial.println("[SSE] === CONEXION ===");
  Serial.print("[SSE] Host: ");
  Serial.println(host);
  Serial.print("[SSE] Path: ");
  Serial.println(path);

  if (!fbClientSSE.connect(host.c_str(), 443)) {
    Serial.println("[SSE] Fallo conexion TLS");
    return false;
  }

  // Enviar request HTTP con headers SSE
  fbClientSSE.print("GET " + path + " HTTP/1.1\r\n");
  fbClientSSE.print("Host: " + host + "\r\n");
  fbClientSSE.print("Accept: text/event-stream\r\n");
  fbClientSSE.print("Connection: keep-alive\r\n");
  fbClientSSE.print("\r\n");

  // Leer status line
  String statusLine = fbClientSSE.readStringUntil('\n');
  statusLine.trim();
  Serial.print("[SSE] Status: ");
  Serial.println(statusLine);

  // Manejar redirect 307 (Firebase lo hace frecuentemente)
  if (statusLine.indexOf("307") != -1) {
    String redirectHost = "";
    while (fbClientSSE.connected()) {
      String header = fbClientSSE.readStringUntil('\n');
      header.trim();
      if (header.startsWith("Location:")) {
        int start = header.indexOf("://") + 3;
        int end = header.indexOf("/", start);
        redirectHost = header.substring(start, end);
      }
      if (header.length() == 0) break;
    }
    fbClientSSE.stop();

    if (redirectHost.length() > 0) {
      Serial.printf("[SSE] Redirect -> %s\n", redirectHost.c_str());
      if (!fbClientSSE.connect(redirectHost.c_str(), 443)) return false;

      fbClientSSE.print("GET " + path + " HTTP/1.1\r\n");
      fbClientSSE.print("Host: " + redirectHost + "\r\n");
      fbClientSSE.print("Accept: text/event-stream\r\n");
      fbClientSSE.print("Connection: keep-alive\r\n");
      fbClientSSE.print("\r\n");

      statusLine = fbClientSSE.readStringUntil('\n');
    }
  }

  // Verificar status 200
  if (statusLine.indexOf("200") == -1) {
    Serial.printf("[SSE] Error HTTP: %s\n", statusLine.c_str());
    fbClientSSE.stop();
    return false;
  }

  // Consumir headers de respuesta
  while (fbClientSSE.connected()) {
    String header = fbClientSSE.readStringUntil('\n');
    header.trim();
    if (header.length() == 0) break;
  }

  sseConectado = true;
  sseBufferPos = 0;
  Serial.println("[SSE] Stream conectado");
  return true;
}

// ====== Lectura SSE no-bloqueante ======
void sseRead() {
  if (!fbClientSSE.connected()) {
    if (sseConectado) {
      Serial.println("[SSE] Conexion perdida");
      sseConectado = false;
      lastSSEReconnect = millis();
    }
    return;
  }

  // Leer solo bytes disponibles (no bloquea)
  while (fbClientSSE.available()) {
    char c = fbClientSSE.read();

    if (c == '\n') {
      sseBuffer[sseBufferPos] = '\0';
      String line = String(sseBuffer);
      sseBufferPos = 0;

      if (line.length() > 0) {
        sseProcessLine(line);
      }
    }
    else if (c != '\r' && sseBufferPos < SSE_BUFFER_SIZE - 1) {
      sseBuffer[sseBufferPos++] = c;
    }
  }
}

// ============================================================
//                    LOOP PRINCIPAL
// ============================================================

void firebaseLoop() {
  if (WiFi.status() != WL_CONNECTED) return;

  unsigned long now = millis();

  // === Envio periodico de datos ===
  if (now - lastFirebaseSend >= FB_SEND_INTERVAL) {
    lastFirebaseSend = now;
    firebaseSendData();
  }

  // === Mantener conexion SSE ===
  if (!sseConectado) {
    if (now - lastSSEReconnect >= FB_SSE_RECONNECT) {
      lastSSEReconnect = now;
      sseConnect();
    }
  } else {
    sseRead();
  }
}
