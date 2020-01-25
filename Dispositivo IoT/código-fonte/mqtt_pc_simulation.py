#IMPORTANTE: 
#1) Utilize python 3
#2) Instale a biblioteca de MQTT (paho-mqtt com o comando: sudo pip3 install paho-mqtt
import paho.mqtt.client as mqtt
import sys
import random
import time
import json

# Definicoes:
device_token = # Aqui vai o token do device, entre aspas
broker = "mqtt.tago.io"
porta_broker = 1883
keep_alive_broker = 60
timeout_conexao = 15 # Timeout da conexao com broker
topico_publish = "tago/data/post"
mqtt_username = # Um nome qualquer, entre aspas
mqtt_password = device_token

# Callback - conexao ao Broker realizada
def on_connect(client, userdata, flags, rc):
    print("[STATUS] Conectado ao Broker. Resultado de conexao: " + str(rc))

# Programa principal
try:
    print("[STATUS] Inicializando MQTT...")
    client = mqtt.Client() #inicializa MQTT:
    client.username_pw_set(mqtt_username, mqtt_password)
    client.on_connect = on_connect
    client.connect(broker, porta_broker, keep_alive_broker)
    
    while True:
        # Garante a reconexao ao broker caso caia
        status_conexao_broker_tago_io = client.loop(timeout_conexao)
        if (status_conexao_broker_tago_io > 0):
            client.connect(broker, porta_broker, keep_alive_broker)

        # Gera temperatura e umidade aleatorias
        temperatura = random.randint(20,40)
        umidade = random.randint(20,70)
        print("Temperatura gerada: " + str(temperatura) + "C")
        print("Umidade gerada: " + str(umidade) + "%")
            
        # Prepara a formatacao dos dados coletados
        temperatura_json = {"variable": "temperatura", "unit": "C", "value": temperatura}
        umidade_json = {"variable": "umidade", "unit": "%", "value": umidade}
        temperatura_json_string = json.dumps(temperatura_json)
        umidade_json_string = json.dumps(umidade_json)
            
        # Envia dados corretamente formatados para o tago.io
        client.publish(topico_publish, temperatura_json_string)
        client.publish(topico_publish, umidade_json_string)

        # Aguarda 05 segundos para proximo envio
        time.sleep(5)

# Encerramento do programa (pressiona CTRL+C)
except KeyboardInterrupt:
    print("\nCtrl+C pressionado, encerrando aplicacao e saindo...")
    sys.exit(0)
