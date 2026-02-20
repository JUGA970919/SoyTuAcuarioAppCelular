# Especificacion Tecnica BLE - App Ionic SoyTuAcuario

## Resumen

Este documento describe el protocolo BLE para la configuracion WiFi del dispositivo ESP32-C3 "SoyTuAcuario". La app debe permitir al usuario configurar las credenciales WiFi del dispositivo mediante Bluetooth Low Energy.

---

## 1. Identificacion del Dispositivo

### Nombre del Dispositivo
```
Patron: SoyTuAcuario_XXXX
Ejemplo: SoyTuAcuario_A1B2
```

- `XXXX` = Ultimos 4 caracteres hexadecimales del MAC address del ESP32
- Cada dispositivo tiene un nombre unico
- **Filtrar** dispositivos BLE por nombre que comience con `SoyTuAcuario_`

### Ejemplo de Filtro (Ionic/Capacitor)
```typescript
// Filtrar por prefijo de nombre
const devices = await BleClient.requestDevice({
  namePrefix: 'SoyTuAcuario_',
  optionalServices: ['12345678-1234-1234-1234-123456789abc']
});
```

---

## 2. UUIDs del Servicio BLE

### Service UUID (Principal)
```
12345678-1234-1234-1234-123456789abc
```

### Characteristics UUIDs

| UUID Completo | Alias | Tipo | Descripcion |
|---------------|-------|------|-------------|
| `12345678-1234-1234-1234-123456789001` | REDES | READ | Lista de redes WiFi disponibles |
| `12345678-1234-1234-1234-123456789002` | CREDENCIALES | WRITE | Enviar SSID y password |
| `12345678-1234-1234-1234-123456789003` | ESTADO | READ + NOTIFY | Estado de la conexion |

---

## 3. Caracteristica REDES (UUID: ...9001)

### Propiedades
- **Tipo**: READ
- **Formato**: String UTF-8
- **Separador**: Pipe `|`

### Formato de Datos
```
Red1|Red2|Red3|Red4|Red5
```

### Ejemplo de Respuesta
```
IZZI-740C|Totalplay-2.4G|INFINITUM_A123|Vecino_WiFi|Casa_Lopez
```

### Notas
- Maximo 10 redes
- Las redes estan ordenadas por intensidad de senal (mejor primero)
- No incluye redes ocultas
- SSID maximo: 32 caracteres

### Codigo Ionic/Capacitor
```typescript
const SERVICE_UUID = '12345678-1234-1234-1234-123456789abc';
const CHAR_REDES = '12345678-1234-1234-1234-123456789001';

// Leer lista de redes
const result = await BleClient.read(
  deviceId,
  SERVICE_UUID,
  CHAR_REDES
);

// Decodificar respuesta
const redesString = new TextDecoder().decode(result);
const redesArray = redesString.split('|');

console.log(redesArray);
// ['IZZI-740C', 'Totalplay-2.4G', 'INFINITUM_A123', ...]
```

---

## 4. Caracteristica CREDENCIALES (UUID: ...9002)

### Propiedades
- **Tipo**: WRITE
- **Formato**: String UTF-8
- **Separador**: Dos puntos `:`

### Formato de Envio
```
SSID:PASSWORD
```

### Ejemplos
```
IZZI-740C:miPassword123
Totalplay-2.4G:clave_segura_456
Mi Red WiFi:password con espacios
```

### Restricciones
| Campo | Maximo | Caracteres Permitidos |
|-------|--------|----------------------|
| SSID | 32 chars | Cualquiera (viene del scan) |
| Password | 63 chars | ASCII imprimibles |

### Notas Importantes
- El SSID debe ser **exactamente** como aparece en la lista de redes
- El password es case-sensitive
- Si el password contiene `:`, solo el primer `:` es el separador

### Codigo Ionic/Capacitor
```typescript
const CHAR_CREDENCIALES = '12345678-1234-1234-1234-123456789002';

// Preparar credenciales
const ssid = 'IZZI-740C';
const password = 'miPassword123';
const credenciales = `${ssid}:${password}`;

// Enviar al dispositivo
await BleClient.write(
  deviceId,
  SERVICE_UUID,
  CHAR_CREDENCIALES,
  new TextEncoder().encode(credenciales)
);

console.log('Credenciales enviadas, esperando respuesta...');
```

---

## 5. Caracteristica ESTADO (UUID: ...9003)

### Propiedades
- **Tipo**: READ + NOTIFY
- **Formato**: String UTF-8

### Estados Posibles

| Estado | Descripcion | Accion Recomendada |
|--------|-------------|-------------------|
| `ESPERANDO` | Dispositivo listo, esperando credenciales | Mostrar formulario |
| `CONECTANDO` | Probando conexion WiFi | Mostrar spinner/loading |
| `CONECTADO` | Conexion exitosa, dispositivo se reiniciara | Mostrar exito, cerrar conexion |
| `NO_CONECTADO` | Password incorrecto o red no disponible | Mostrar error, permitir reintentar |
| `ERROR_FORMATO` | Formato de credenciales invalido | Mostrar error de formato |

### Suscripcion a Notificaciones
```typescript
const CHAR_ESTADO = '12345678-1234-1234-1234-123456789003';

// Suscribirse a cambios de estado
await BleClient.startNotifications(
  deviceId,
  SERVICE_UUID,
  CHAR_ESTADO,
  (value) => {
    const estado = new TextDecoder().decode(value);
    console.log('Estado recibido:', estado);

    switch(estado) {
      case 'ESPERANDO':
        // Mostrar formulario
        break;
      case 'CONECTANDO':
        // Mostrar loading
        break;
      case 'CONECTADO':
        // Exito! El dispositivo se reiniciara
        showSuccess('WiFi configurado correctamente');
        disconnectDevice();
        break;
      case 'NO_CONECTADO':
        // Error de credenciales
        showError('Password incorrecto. Intenta de nuevo.');
        break;
      case 'ERROR_FORMATO':
        // Error de formato
        showError('Formato invalido. Usa SSID:PASSWORD');
        break;
    }
  }
);
```

---

## 6. Flujo Completo de la App

```
┌─────────────────────────────────────────────────────────────┐
│                    PANTALLA INICIO                          │
│                                                             │
│  [Boton: Buscar Dispositivos]                               │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                 ESCANEANDO BLE...                           │
│                                                             │
│  Buscando dispositivos SoyTuAcuario_*                       │
│  [Spinner]                                                  │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              DISPOSITIVOS ENCONTRADOS                       │
│                                                             │
│  ○ SoyTuAcuario_A1B2                                        │
│  ○ SoyTuAcuario_C3D4                                        │
│                                                             │
│  [Boton: Conectar]                                          │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  CONECTANDO...                              │
│                                                             │
│  Conectando a SoyTuAcuario_A1B2                             │
│  [Spinner]                                                  │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              SELECCIONAR RED WiFi                           │
│                                                             │
│  ● IZZI-740C                                                │
│  ○ Totalplay-2.4G                                           │
│  ○ INFINITUM_A123                                           │
│  ○ Vecino_WiFi                                              │
│                                                             │
│  Password: [________________]                               │
│                                                             │
│  [Boton: Configurar WiFi]                                   │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                 CONFIGURANDO...                             │
│                                                             │
│  Probando conexion a IZZI-740C                              │
│  [Spinner]                                                  │
│                                                             │
│  Estado: CONECTANDO                                         │
└─────────────────────────────────────────────────────────────┘
                            │
              ┌─────────────┴─────────────┐
              ▼                           ▼
┌──────────────────────────┐  ┌──────────────────────────┐
│        EXITO!            │  │         ERROR            │
│                          │  │                          │
│  ✓ WiFi configurado      │  │  ✗ Password incorrecto   │
│                          │  │                          │
│  El dispositivo se       │  │  [Boton: Reintentar]     │
│  reiniciara...           │  │                          │
│                          │  │                          │
│  [Boton: Finalizar]      │  └──────────────────────────┘
└──────────────────────────┘
```

---

## 7. Ejemplo Completo Ionic/Capacitor

### Instalacion
```bash
npm install @capacitor-community/bluetooth-le
npx cap sync
```

### Servicio BLE (ble.service.ts)
```typescript
import { Injectable } from '@angular/core';
import { BleClient, BleDevice } from '@capacitor-community/bluetooth-le';

@Injectable({
  providedIn: 'root'
})
export class BleService {

  // UUIDs
  private readonly SERVICE_UUID = '12345678-1234-1234-1234-123456789abc';
  private readonly CHAR_REDES = '12345678-1234-1234-1234-123456789001';
  private readonly CHAR_CREDENCIALES = '12345678-1234-1234-1234-123456789002';
  private readonly CHAR_ESTADO = '12345678-1234-1234-1234-123456789003';

  private deviceId: string = '';

  // Inicializar BLE
  async initialize(): Promise<void> {
    await BleClient.initialize();
  }

  // Escanear dispositivos SoyTuAcuario
  async scanDevices(): Promise<BleDevice[]> {
    const devices: BleDevice[] = [];

    await BleClient.requestLEScan(
      { namePrefix: 'SoyTuAcuario_' },
      (result) => {
        if (result.device.name && !devices.find(d => d.deviceId === result.device.deviceId)) {
          devices.push(result.device);
        }
      }
    );

    // Escanear por 5 segundos
    await new Promise(resolve => setTimeout(resolve, 5000));
    await BleClient.stopLEScan();

    return devices;
  }

  // Conectar a dispositivo
  async connect(deviceId: string): Promise<void> {
    this.deviceId = deviceId;
    await BleClient.connect(deviceId, (disconnected) => {
      console.log('Dispositivo desconectado:', disconnected);
    });
  }

  // Obtener lista de redes WiFi
  async getWifiNetworks(): Promise<string[]> {
    const result = await BleClient.read(
      this.deviceId,
      this.SERVICE_UUID,
      this.CHAR_REDES
    );

    const networksString = new TextDecoder().decode(result);
    return networksString.split('|').filter(n => n.length > 0);
  }

  // Enviar credenciales WiFi
  async sendCredentials(ssid: string, password: string): Promise<void> {
    const credentials = `${ssid}:${password}`;

    await BleClient.write(
      this.deviceId,
      this.SERVICE_UUID,
      this.CHAR_CREDENCIALES,
      new TextEncoder().encode(credentials)
    );
  }

  // Suscribirse a estado
  async subscribeToStatus(callback: (status: string) => void): Promise<void> {
    await BleClient.startNotifications(
      this.deviceId,
      this.SERVICE_UUID,
      this.CHAR_ESTADO,
      (value) => {
        const status = new TextDecoder().decode(value);
        callback(status);
      }
    );
  }

  // Desconectar
  async disconnect(): Promise<void> {
    if (this.deviceId) {
      await BleClient.stopNotifications(this.deviceId, this.SERVICE_UUID, this.CHAR_ESTADO);
      await BleClient.disconnect(this.deviceId);
      this.deviceId = '';
    }
  }
}
```

### Componente de Configuracion (wifi-config.page.ts)
```typescript
import { Component } from '@angular/core';
import { BleService } from '../services/ble.service';

@Component({
  selector: 'app-wifi-config',
  templateUrl: './wifi-config.page.html'
})
export class WifiConfigPage {

  devices: any[] = [];
  networks: string[] = [];
  selectedNetwork: string = '';
  password: string = '';
  status: string = '';
  isLoading: boolean = false;
  isConnected: boolean = false;

  constructor(private bleService: BleService) {}

  async ngOnInit() {
    await this.bleService.initialize();
  }

  // Paso 1: Escanear dispositivos
  async scanDevices() {
    this.isLoading = true;
    this.status = 'Buscando dispositivos...';

    try {
      this.devices = await this.bleService.scanDevices();
      this.status = `${this.devices.length} dispositivo(s) encontrado(s)`;
    } catch (error) {
      this.status = 'Error al escanear';
      console.error(error);
    }

    this.isLoading = false;
  }

  // Paso 2: Conectar a dispositivo
  async connectToDevice(deviceId: string) {
    this.isLoading = true;
    this.status = 'Conectando...';

    try {
      await this.bleService.connect(deviceId);
      this.isConnected = true;

      // Suscribirse a estado
      await this.bleService.subscribeToStatus((status) => {
        this.handleStatus(status);
      });

      // Obtener redes WiFi
      this.networks = await this.bleService.getWifiNetworks();
      this.status = 'Selecciona una red WiFi';

    } catch (error) {
      this.status = 'Error al conectar';
      console.error(error);
    }

    this.isLoading = false;
  }

  // Paso 3: Enviar credenciales
  async configureWifi() {
    if (!this.selectedNetwork || !this.password) {
      this.status = 'Selecciona una red e ingresa la contraseña';
      return;
    }

    this.isLoading = true;
    this.status = 'Enviando credenciales...';

    try {
      await this.bleService.sendCredentials(this.selectedNetwork, this.password);
      // El estado se actualizara via notificacion
    } catch (error) {
      this.status = 'Error al enviar credenciales';
      console.error(error);
      this.isLoading = false;
    }
  }

  // Manejar estados del dispositivo
  handleStatus(status: string) {
    this.status = status;

    switch(status) {
      case 'ESPERANDO':
        this.isLoading = false;
        break;

      case 'CONECTANDO':
        this.isLoading = true;
        this.status = 'Probando conexion WiFi...';
        break;

      case 'CONECTADO':
        this.isLoading = false;
        this.status = '¡Configuracion exitosa! El dispositivo se reiniciara.';
        // Desconectar despues de 2 segundos
        setTimeout(() => this.disconnect(), 2000);
        break;

      case 'NO_CONECTADO':
        this.isLoading = false;
        this.status = 'Contraseña incorrecta. Intenta de nuevo.';
        this.password = '';
        break;

      case 'ERROR_FORMATO':
        this.isLoading = false;
        this.status = 'Error en formato de datos';
        break;
    }
  }

  async disconnect() {
    await this.bleService.disconnect();
    this.isConnected = false;
    this.networks = [];
    this.selectedNetwork = '';
    this.password = '';
  }
}
```

### Template HTML (wifi-config.page.html)
```html
<ion-header>
  <ion-toolbar>
    <ion-title>Configurar WiFi</ion-title>
  </ion-toolbar>
</ion-header>

<ion-content class="ion-padding">

  <!-- Estado actual -->
  <ion-card>
    <ion-card-content>
      <ion-spinner *ngIf="isLoading"></ion-spinner>
      <p>{{ status }}</p>
    </ion-card-content>
  </ion-card>

  <!-- Paso 1: Buscar dispositivos -->
  <div *ngIf="!isConnected">
    <ion-button expand="block" (click)="scanDevices()" [disabled]="isLoading">
      Buscar Dispositivos
    </ion-button>

    <ion-list *ngIf="devices.length > 0">
      <ion-list-header>Dispositivos Encontrados</ion-list-header>
      <ion-item *ngFor="let device of devices" (click)="connectToDevice(device.deviceId)">
        <ion-icon name="bluetooth" slot="start"></ion-icon>
        <ion-label>{{ device.name }}</ion-label>
        <ion-icon name="chevron-forward" slot="end"></ion-icon>
      </ion-item>
    </ion-list>
  </div>

  <!-- Paso 2 y 3: Seleccionar red y password -->
  <div *ngIf="isConnected && networks.length > 0">
    <ion-list>
      <ion-list-header>Redes WiFi Disponibles</ion-list-header>
      <ion-radio-group [(ngModel)]="selectedNetwork">
        <ion-item *ngFor="let network of networks">
          <ion-radio slot="start" [value]="network"></ion-radio>
          <ion-label>{{ network }}</ion-label>
        </ion-item>
      </ion-radio-group>
    </ion-list>

    <ion-item>
      <ion-label position="floating">Contraseña WiFi</ion-label>
      <ion-input type="password" [(ngModel)]="password"></ion-input>
    </ion-item>

    <ion-button expand="block" (click)="configureWifi()" [disabled]="isLoading || !selectedNetwork">
      Configurar WiFi
    </ion-button>

    <ion-button expand="block" fill="outline" (click)="disconnect()">
      Cancelar
    </ion-button>
  </div>

</ion-content>
```

---

## 8. Permisos Requeridos

### Android (AndroidManifest.xml)
```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
```

### iOS (Info.plist)
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>Se necesita Bluetooth para configurar el dispositivo SoyTuAcuario</string>
<key>NSBluetoothPeripheralUsageDescription</key>
<string>Se necesita Bluetooth para configurar el dispositivo SoyTuAcuario</string>
```

---

## 9. Manejo de Errores

| Error | Causa Probable | Solucion |
|-------|---------------|----------|
| No encuentra dispositivos | BLE apagado o dispositivo fuera de rango | Verificar Bluetooth activo y cercania |
| Error de conexion | Dispositivo ocupado o fuera de rango | Reiniciar dispositivo ESP32 |
| Lista de redes vacia | Error en escaneo WiFi | El ESP32 reintenta al reiniciar |
| NO_CONECTADO | Password incorrecto | Verificar password y reintentar |
| ERROR_FORMATO | Falta SSID o password | Verificar formato SSID:PASSWORD |
| Timeout | Dispositivo se reinicio | Volver a escanear |

---

## 10. Notas Importantes

1. **El dispositivo se reinicia** despues de configuracion exitosa. La app debe manejar la desconexion automatica.

2. **Timeout de 5 minutos**: Si nadie se conecta en 5 minutos, el ESP32 se reinicia. Si hay un dispositivo conectado, el timeout se pausa.

3. **Una conexion a la vez**: El ESP32 solo acepta una conexion BLE simultanea.

4. **WiFi desactivado durante BLE**: El ESP32-C3 es single-core, por lo que WiFi esta apagado mientras BLE esta activo.

5. **Redes escaneadas al inicio**: Las redes se escanean ANTES de activar BLE. Si el usuario necesita actualizar la lista, debe reiniciar el dispositivo.

---

## 11. Testing

### Herramientas Recomendadas
- **nRF Connect** (Android/iOS) - Para probar BLE manualmente
- **LightBlue** (iOS) - Alternativa para iOS

### Pasos de Prueba Manual
1. Encender ESP32 sin credenciales guardadas
2. Abrir nRF Connect
3. Buscar `SoyTuAcuario_*`
4. Conectar
5. Navegar al servicio `12345678-...abc`
6. Leer caracteristica `...9001` (ver redes)
7. Escribir en `...9002`: `TuRed:TuPassword`
8. Observar notificacion en `...9003`

---

## Contacto

Para dudas sobre la implementacion del firmware ESP32:
- Revisar archivo `ConfigBLE.hpp` en el repositorio
- Documentacion completa en `CLAUDE.md`

---

**Version del Documento**: 1.0
**Fecha**: 2026-02-20
**Compatible con Firmware**: v1.1
