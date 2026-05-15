from Adafruit_IO import MQTTClient
import serial
import time
import threading

# Credenciales Adafruit
ADAFRUIT_IO_USERNAME = "username"
ADAFRUIT_IO_KEY      = "adafruit_user_key"

# Feeds
FEEDS_TX = ["servo1_TX", "servo2_TX", "servo3_TX", "servo4_TX"]  # Adafruit -> AVR
FEEDS_RX = ["servo1_RX", "servo2_RX", "servo3_RX", "servo4_RX"]  # AVR -> Adafruit

# Puerto serial
ser = serial.Serial("COMX", 9600, timeout=1)  # Cambiar COMX
time.sleep(2)

#########################################
# Callbacks MQTT                        #
#########################################
def connected(client):
    print("Conectado a Adafruit IO!")
    for feed in FEEDS_TX:
        print(f"Suscribiendo a {feed}")
        client.subscribe(feed)

def message(client, feed_id, payload):
    print(f"Recibido {feed_id}: {payload}")

    # Extraer número de servo: "servo1_TX" -> "1"
    servo_num = feed_id.replace("servo", "").replace("_TX", "")

    # Formatear y enviar al AVR
    angulo  = int(float(payload))
    angulo  = max(0, min(180, angulo))      # clamp 0-180
    comando = f"S{servo_num}:{angulo:03d}\n"

    print(f"Enviando al AVR: {comando.strip()}")
    ser.write(comando.encode())

#########################################
# Hilo: leer UART y publicar en RX      #
#########################################
def leerUART():
    while True:
        if ser.in_waiting > 0:
            linea = ser.readline().decode("utf-8").strip()

            # Validar formato R1:090
            if len(linea) == 6 and linea[0] == 'R' and linea[2] == ':':
                try:
                    servo_num = linea[1]                    # "1"-"4"
                    angulo    = int(linea[3:6])             # "090" -> 90
                    feed      = f"servo{servo_num}_RX"

                    if feed in FEEDS_RX and 0 <= angulo <= 180:
                        print(f"Publicando {feed}: {angulo}")
                        client.publish(feed, angulo)

                except ValueError:
                    pass    # línea malformada, ignorar

        time.sleep(0.05)    # 50ms para no saturar el CPU

#########################################
# Main                                  #
#########################################
client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
client.on_connect = connected
client.on_message = message
client.connect()

# Leer UART en hilo separado para no bloquear el loop MQTT
hilo = threading.Thread(target=leerUART, daemon=True)
hilo.start()

client.loop_blocking()