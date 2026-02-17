#include <WiFi.h>

// ====== CREDENCIALES ======
const char* ssid     = "IZZI-740C";
const char* password = "mF3e6LRyeGfXLGPJkb";

// ====== CONTROL DE TIEMPO ======
unsigned long lastWifiCheck = 0;
const unsigned long wifiInterval = 5000; // 5 segundos

// ====== INICIALIZACIÓN WIFI ======
void wifiInit() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("📡 Conectando a WiFi...");
}

// ====== LOOP WIFI (RECONEXIÓN AUTOMÁTICA) ======
void wifiLoop() {
  unsigned long now = millis();

  if (now - lastWifiCheck >= wifiInterval) {
    lastWifiCheck = now;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠ WiFi desconectado, reconectando...");
      WiFi.disconnect();
      WiFi.reconnect();
    } 
    else {
      Serial.print("✅ WiFi conectado | IP: ");
      Serial.println(WiFi.localIP());
    }
  }
}
