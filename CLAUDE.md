# SoyTuAcuario - Sistema de Monitoreo de Acuarios

## Descripcion del Proyecto

Sistema embebido de monitoreo de acuarios en tiempo real basado en **ESP32-C3 SuperMini**. Mide y visualiza parametros criticos del agua para mantener la salud del ecosistema acuatico.

### Sensores Implementados

| Sensor | Tipo | Protocolo | Rango |
|--------|------|-----------|-------|
| pH | Analogico | ADS1115 (CH0) | 0-14 pH |
| TDS | Analogico | ADS1115 (CH1) | 0-2000+ ppm |
| Temperatura | Digital | OneWire DS18B20 | -55 a +125C |
| Turbidez | Analogico | ADS1115 (CH2) | 0-5V |
| Nivel (Flotador) | Digital | GPIO4 | Alto/Bajo |

---

## Stack Tecnologico

- **Hardware**: ESP32-C3 SuperMini (RISC-V, 160 MHz, 4MB Flash)
- **IDE**: Arduino IDE 2.x
- **Lenguaje**: C++ (Arduino)
- **Comunicacion**: WiFi 2.4GHz, BLE, Serial 115200 bps
- **Display**: OLED SSD1306 128x64 (I2C)
- **ADC Externo**: ADS1115 16-bit

---

## Estructura del Proyecto

```
SoyTuAcuarioAppCelular/
├── .gitignore
├── CLAUDE.md                                      # Este archivo
└── Soy_tu_AcuarioAppCelular_ESP32-C3/             # Carpeta del sketch
    ├── Soy_tu_AcuarioAppCelular_ESP32-C3.ino     # Archivo principal
    ├── Config.h                                   # Configuracion global
    ├── Structs.h                                  # Estructuras de datos
    ├── ConfigBLE.hpp                              # Configuracion WiFi via BLE
    ├── ConexionWifi.hpp                           # Gestion WiFi
    ├── SolicitudDatos.hpp                         # Protocolo calibracion
    ├── FiltroSensor.hpp                           # Filtros EMA
    ├── Pantalla.hpp                               # Control OLED
    ├── SensorPh.hpp                               # Sensor pH
    ├── SensorTemperatura.hpp                      # Sensor DS18B20
    ├── SensorTds.hpp                              # Sensor TDS
    ├── SensorFlotador.hpp                         # Sensor nivel
    └── SensorTurbidez.hpp                         # Sensor turbidez
```

---

## Pines del ESP32-C3

| Pin | Funcion | Tipo |
|-----|---------|------|
| 0 | TX UART | Serial |
| 1 | RX UART | Serial |
| 2 | OneWire (DS18B20) | Digital In |
| 4 | Flotador | Digital In (Pull-up) |
| 5 | Control MUX TDS | Digital Out |
| 8 | I2C SDA | I2C |
| 9 | I2C SCL | I2C |

---

## Modos de Operacion

### MODO_NORMAL (0)
Modo de sensado continuo. Ejecuta la maquina de estados para leer sensores.

### MODO_CONFIG_BLE (1)
Modo de configuracion WiFi via Bluetooth Low Energy.

---

## Configuracion WiFi via BLE

### Flujo de Configuracion

```
                    ┌─────────────────┐
                    │   POWER ON      │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │ Hay credenciales│
                    │  en Preferences?│
                    └────────┬────────┘
                             │
              ┌──────────────┴──────────────┐
              │ NO                          │ SI
              ▼                             ▼
    ┌─────────────────┐           ┌─────────────────┐
    │ Modo Config BLE │           │ Conectar WiFi   │
    │  (automatico)   │           └────────┬────────┘
    └────────┬────────┘                    │
             │                    ┌────────▼────────┐
             │                    │ Conexion OK?    │
             │                    └────────┬────────┘
             │                             │
             │              ┌──────────────┴──────────────┐
             │              │ NO                          │ SI
             │              ▼                             ▼
             │    ┌─────────────────┐           ┌─────────────────┐
             └───►│ Modo Config BLE │           │  MODO NORMAL    │
                  └─────────────────┘           └─────────────────┘
```

### Servicio BLE

| UUID | Tipo | Descripcion |
|------|------|-------------|
| `12345678-1234-1234-1234-123456789abc` | Service | Servicio de configuracion WiFi |
| `...9001` | READ | Lista de redes WiFi (separadas por \|) |
| `...9002` | WRITE | Credenciales (formato: `SSID:PASSWORD`) |
| `...9003` | NOTIFY | Estado: ESPERANDO, CONECTANDO, CONECTADO, NO_CONECTADO |

### Nombre BLE
`SoyTuAcuario_XXXX` donde XXXX son los ultimos 4 digitos del MAC address.

### Timeout
5 minutos de inactividad. Si hay dispositivo conectado, NO reinicia.

---

## Arquitectura y Patrones

### 1. Maquina de Estados (Tiempo Compartido)
Alterna cada 5 segundos entre dos estados:
- **Estado 1** (D5=LOW): Lee pH, Turbidez
- **Estado 2** (D5=HIGH): Lee TDS, Temperatura

### 2. Filtro EMA (Exponential Moving Average)
Reduce ruido analogico con factor alfa=0.03:
```cpp
yk = (nuevaLectura * 0.03) + (0.97 * ultimaLectura)
```

### 3. Calibracion pH de 3 Puntos
Usa buffers 4.0, 6.86, 9.18 para calcular pendiente y offset:
```cpp
pH = slopeph * voltaje + offsetph
```

### 4. Debouncing Digital
El sensor flotador espera 3000ms antes de confirmar cambio de estado.

### 5. Almacenamiento Persistente
Usa `Preferences.h` (Flash NVS) para guardar:
- Credenciales WiFi (ssid, password)
- Calibracion pH (offset, slope)
- Ultimos valores de sensores

---

## Dependencias (Arduino Library Manager)

1. **Adafruit_ADS1X15** - Conversor ADC 16 bits
2. **OneWire** - Protocolo OneWire
3. **DallasTemperature** - Sensor DS18B20
4. **U8g2** - Display OLED SSD1306
5. **ESP32 BLE Arduino** - Incluido en ESP32 core

---

## Comunicacion Serial

### Salida de Datos (115200 bps)
```
ph,temperatura,turbidez,tds,nivel,
```

### Comandos de Calibracion
| Comando | Accion |
|---------|--------|
| `Calph1` | Calibra con buffer pH 4.0 |
| `Calph2` | Calibra con buffer pH 6.86 |
| `Calph3` | Calibra con buffer pH 9.18 |
| `GuardarCalibracion` | Guarda parametros en Flash |

---

## Flujo de Ejecucion

```
SETUP
├── Serial.begin(115200)
├── Wire.begin(SDA=8, SCL=9)
├── pantallaIni()
├── ¿Hay credenciales guardadas?
│   ├── NO → iniciarModoBLE() → return
│   └── SI → wifiInit()
│       ├── FALLO → iniciarModoBLE() → return
│       └── OK → setupModoNormal()
│           ├── AdsIni()
│           ├── InitAnalogFilter() x3
│           ├── InicioTemperatura()
│           ├── FlotadorIni()
│           └── pinMode(D5, OUTPUT)

LOOP
├── SI modoOperacion == MODO_CONFIG_BLE
│   └── loopModoBLE()
│       ├── resultado=1 → ESP.restart()
│       ├── resultado=-1 → ESP.restart()
│       └── resultado=0 → continuar BLE
│
└── SI modoOperacion == MODO_NORMAL
    ├── wifiLoop()
    ├── Setponts()
    ├── Maquina de estados (alterna cada 5s)
    │   ├── Estado 1: pH, Turbidez
    │   └── Estado 2: TDS, Temperatura
    ├── StatusButton() - Debouncing flotador
    ├── MostarPantalla()
    └── Serial.print() - Envia datos
```

---

## Variables Persistentes (Flash)

### Namespace "wifi"
| Variable | Descripcion |
|----------|-------------|
| `ssid` | SSID de la red WiFi |
| `password` | Contrasena WiFi |
| `configurado` | bool - indica si hay credenciales |

### Namespace "acuario" (existente)
| Variable | Descripcion | Default |
|----------|-------------|---------|
| `offsetph` | Ordenada al origen | 0.0 |
| `slopeph` | Pendiente | 1.0 |
| `ph` | Ultimo pH leido | 7.0 |
| `tds` | Ultimo TDS leido | 100 |

---

## Problemas Conocidos

### Codigo
- [ ] SensorTurbidez.hpp tiene formula NTU comentada (linea 8-9)
- [ ] Inconsistencia: include dice `Sensortds.hpp` pero archivo es `SensorTds.hpp`

---

## Mejoras Futuras

- [x] ~~Implementar configuracion WiFi via BLE~~
- [ ] Implementar servidor web local para configuracion avanzada
- [ ] Agregar MQTT para integracion con Home Assistant
- [ ] Implementar OTA (Over-The-Air updates)
- [ ] Agregar alertas cuando valores estan fuera de rango
- [ ] Implementar autocalibration del sensor de pH
- [ ] Agregar boton fisico para forzar modo config BLE

---

## App Movil (Pendiente)

La app movil debe:
1. Escanear dispositivos BLE filtrando por nombre `SoyTuAcuario_*`
2. Conectarse al dispositivo
3. Leer caracteristica de redes (UUID ...9001)
4. Mostrar checkbox con redes disponibles
5. Permitir seleccionar red e ingresar contrasena
6. Enviar `SSID:PASSWORD` a caracteristica (UUID ...9002)
7. Suscribirse a notificaciones de estado (UUID ...9003)
8. Mostrar resultado: CONECTADO o NO_CONECTADO

---

## Compilacion

1. Abrir `Soy_tu_AcuarioAppCelular_ESP32-C3.ino` en Arduino IDE
2. Seleccionar placa: **ESP32C3 Dev Module**
3. Configurar:
   - Flash Mode: QIO
   - Flash Size: 4MB
   - Partition Scheme: Default 4MB
4. Instalar dependencias desde Library Manager
5. Compilar y subir

---

## Notas para Desarrollo

- El codigo usa `inline` en funciones pequenas para optimizacion
- No hay validacion de rangos en algunas lecturas (puede retornar NaN)
- El buffer serial no tiene limite de tamano
- La pantalla OLED rota entre 5 vistas cada 3 segundos
- ESP32-C3 es single-core: WiFi y BLE no deben usarse simultaneamente

---

**Ultima actualizacion**: 2026-02-20
**Version**: 1.1 (Configuracion BLE implementada)
**Lineas de codigo**: ~700 lineas
