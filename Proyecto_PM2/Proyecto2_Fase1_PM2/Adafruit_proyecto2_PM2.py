from Adafruit_IO import MQTTClient
import serial
import time

ADAFRUIT_IO_USERNAME = "AnaLu_11"
ADAFRUIT_IO_KEY = "aio_HJZB83hYrtYfQpjfnup69jvab2Xr" # No es la contraseña del perfil xd, me confundí con eso

FEEDS = ["feed1", "feed2", "feed3", "feed4"]  # Solo feeds para leer datos (Tx del Adafruit según lo hizo Pedro)

# Configura tu puerto
ser = serial.Serial("COM4", 9600, timeout=1) # Recuerden cambiar su número de COM
time.sleep(2)

def connected(client):
    print("Conectado!")
    for feed in FEEDS:
        print(f"Suscribiendo a {feed}")
        client.subscribe(feed)

def message(client, feed_id, payload):
    print(f"{feed_id}: {payload}")

    # Enviar formato: servo1:90
    msg = f"{feed_id}:{payload}\n"
    print(f"Recibido de Adafruit: {repr(msg)}")  # <-- agrega
    ser.write(msg.encode())
    print(f"Enviado al serial")                   # <-- agrega

client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)

client.on_connect = connected
client.on_message = message

client.connect()
client.loop_blocking()  