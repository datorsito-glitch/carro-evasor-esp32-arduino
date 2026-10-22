# Carro Evasor de Obstáculos Inteligente con ESP32 y Arduino Uno

Este proyecto consiste en un vehículo robótico capaz de evadir obstáculos de forma autónoma y comunicarse con el usuario mediante audio y una interfaz web local. El sistema utiliza una arquitectura de doble microcontrolador (**ESP32** para conectividad y audio, y **Arduino Uno R3** para el control de hardware y motores).

##  Características
- **Evasión Autónoma:** Detección de objetos mediante un sensor ultrasónico para evitar colisiones.
- **Control Remoto Web:** Interfaz web interna (servida por el punto de acceso del ESP32) para controlar el movimiento manualmente.
- **Interacción por Voz/Texto:** Al escribir frases en la web (ej. "Buenos días"), el robot procesa el comando y responde reproduciendo un audio pregrabado.
- **Red Local Autónoma:** El ESP32 crea su propia red Wi-Fi interna, por lo que no depende de internet externo.

##  Arquitectura y Componentes
El proyecto se divide en dos módulos principales conectados por comunicación serial (UART):

1. **ESP32 (Servidor Web, Audio e Interacción):**
   - Crea el Punto de Acceso Wi-Fi (AP).
   - Aloja la página web interna (HTML/CSS/JS).
   - Almacena y reproduce los audios pregrabados.
   - Envía comandos seriales al Arduino según lo recibido en la web.

2. **Arduino Uno R3 (Control de Motores y Sensores):**
   - Lee el sensor de distancia (ultrasónico).
   - Controla el Driver de Motores (ej. L298N) para mover las ruedas.
   - Ejecuta la lógica de evasión autónoma cuando no está en modo manual.

### Componentes Utilizados:
- ESP32 (NodeMCU o similar)
- Arduino Uno R3
- Sensor Ultrasónico HC-SR04
- Driver de motores L298N (o similar)
- Motores DC con ruedas
- Módulo de Audio / Altavoz (ej. DFPlayer Mini o DAC interno)
- Servomotor (MG90S)
- Amplificador de audio (ej. LM386)
- Cables DuPont
- Altavoz / Bocina (8 ohms, 0.5W o similar)
- Cable USB y Fuente de alimentación (5V)
- Protoboard (Placa de pruebas)

##  Estructura del Repositorio
- `/esp32_code`: Contiene el sketch de Arduino IDE para el ESP32 y los archivos de la interfaz web.
- `/arduino_code`: Contiene el código fuente que corre en el Arduino Uno R3.
- `/schematics`: Diagramas de conexión eléctrica entre ambos microcontroladores y sensores.

##  Instalación y Configuración

### 1. Preparación del hardware
1. Comunicación Inter-Microcontrolador (UART Serial)
*Es obligatorio cruzar las líneas de datos y unificar las referencias de tierra para que se puedan enviar comandos de la web al sistema de motores:*
* **ESP32 TX (Pin 17 / GPIO17)** ➡️ **Arduino Uno RX (Pin 0)** *(Nota: Se recomienda colocar un divisor de voltaje con resistencias de 1kΩ y 2.2kΩ en esta línea para bajar los 5V del Arduino a los 3.3V tolerantes del ESP32).*
* **ESP32 RX (Pin 16 / GPIO16)** ➡️ **Arduino Uno TX (Pin 1)**
* **ESP32 GND** ➡️ **Arduino Uno GND** (Tierra común)
 3. Etapa de Sensores y Motores (Conectados al Arduino Uno R3)
*El Arduino se encarga de la telemetría del entorno y de la fuerza bruta para mover el chasis:*
* **Sensor Ultrasónico HC-SR04 (Evasor):**
  - **VCC** ➡️ Arduino 5V *(Se sugiere un capacitor cerámico de 100nF en paralelo entre VCC y GND del sensor para estabilizar lecturas).*
  - **Trigger** ➡️ Arduino Pin 2
  - **Echo** ➡️ Arduino Pin 3
  - **GND** ➡️ Arduino GND
* **Driver de Motores L298N (Puente H):**
  - **IN1** y **IN2** (Dirección Motor Izquierdo) ➡️ Arduino Pines 4 y 5
  - **IN3** y **IN4** (Dirección Motor Derecho) ➡️ Arduino Pines 6 y 7
  - **ENA** y **ENB** (Control de velocidad mediante puentes fijos a 5V o cables a pines PWM del Arduino si se desea regular la velocidad).

4. Sistema de Alimentación y Distribución de Potencia
*Para evitar bucles de masa y ruidos provocados por los motores DC, se utiliza una configuración de distribución de energía en estrella:*
* **Batería (Ej. 2 celdas Li-Ion 18650 de 7.4V o LiPo):**
  - **Polo (+)** ➡️ Entrada de **12V/Vcc del Driver L298N** y al regulador de voltaje externo (o pin Vin del Arduino).
  - **Polo (-)** ➡️ Entrada de **GND del Driver L298N**.
 Sistema de Amplificación de Audio (LM386 y Parlante de 8 Ohms)
El ESP32 genera la señal de audio (mediante DAC o PWM), pero debido a que la señal es muy débil, se implementó una etapa de amplificación analógica utilizando el circuito integrado **LM386** para alimentar un parlante pesado de computadora vieja de 8 Ohms. 

Para limpiar el ruido y estabilizar el sonido se usaron los siguientes capacitores críticos:

* **Capacitor de desacoplo de alimentación (100µF a 250µF - Electrolítico):** Conectado físicamente entre el pin 6 (Vcc) y el pin 4 (GND) del LM386. Absorbe los picos de corriente que exige el parlante de 8 ohms, evitando que el consumo de audio resetee el ESP32.
* **Capacitor de acoplamiento de salida (220µF a 470µF - Electrolítico):** Conectado en serie entre la salida del amplificador (Pin 5) y el polo positivo del parlante. Bloquea la corriente directa (DC) para que al parlante solo le llegue la señal de audio pura (AC) y no se queme la bobina.
* **Filtro del pin de Bypass (10µF - Electrolítico):** Conectado entre el pin 7 y tierra (GND) para aislar la etapa de entrada del amplificador del ruido eléctrico proveniente de la fuente de poder del robot.
* **Red de Snubber / Zobel (Capacitor cerámico de 47nF/100nF + Resistencia de 10 Ohms):** Conectados en serie desde el pin 5 a tierra. Evita las oscilaciones de alta frecuencia (silbidos agudos) generadas por la carga inductiva del parlante pesado de computadora.

- Realiza las conexiones. Asegúrate de conectar el pin `TX` del ESP32 al `RX` del Arduino (usando divisor de voltaje si es necesario) y compartir la misma línea de tierra (`GND`).

### 2. Cargar el código en el Arduino Uno
1. Abre `/arduino_code/arduino_code.ino` en el Arduino IDE.
2. Selecciona la placa **Arduino Uno** y el puerto COM correcto.
3. Sube el código.

### 3. Cargar el código en el ESP32
1. Abre `/esp32_code/esp32_code.ino` en el Arduino IDE.
2. Instala las librerías necesarias (ESP32Servo.h, XT_DAC_Audio.h, WebServer.h y WiFi.h).
3. Configura el SSID y contraseña deseados para la red interna en las variables del código.
4. Sube el código a la placa ESP32.

##  Cómo usarlo
1. Enciende el robot.
2. Desde tu teléfono o computadora, conéctate a la red Wi-Fi generada por el ESP32.
3. Abre tu navegador web e ingresa a la dirección IP asignada (por defecto suele ser `192.168.4.1`).
4. Interactúa con el robot o contrólalo manualmente desde los botones
