/*
 * Programa: código-fonte do dispositivo IoT do mini-curso
 * Autor: Pedro Bertoleti
 */

#include <stdio.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

/* Definicoes gerais */
#define TEMPO_ENVIO_INFORMACOES    5000 //ms

/* Definicoes do sensor de temperatura */
#define DHTPIN  D1   /* GPIO que o pino 2 do sensor é conectado */

/* A biblioteca serve para os sensores DHT11, DHT22 e DHT21.
   No nosso caso, usaremos o DHT22, porém se você desejar utilizar
   algum dos outros disponíveis, basta descomentar a linha correspondente.
*/
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

/* Definicoes da UART de debug */
#define DEBUG_UART_BAUDRATE               115200

/* MQTT definitions */
#define MQTT_PUB_TOPIC "tago/data/post"
#define MQTT_ID        "secomp_iot_device_id"       
#define MQTT_USERNAME  "secomp_iot_device"  /* Coloque aqui qualquer valor */
#define MQTT_PASSWORD  " "  /* coloque aqui o Device Token do seu dispositivo no Tago.io */

/* WIFI */
const char* ssid_wifi = " ";     /*  WI-FI network SSID (name) you want to connect */
const char* password_wifi = " "; /*  WI-FI network password */
WiFiClient espClient;     

/* MQTT */
const char* broker_mqtt = "mqtt.tago.io"; /* MQTT broker URL */
int broker_port = 1883;                      /* MQTT broker port */
PubSubClient MQTT(espClient); 

/*
 * Variáveis e objetos globais
 */
/* objeto para comunicação com sensor DHT22  */
DHT dht(DHTPIN, DHTTYPE);

/* Prototypes */
void init_wifi(void);
void init_MQTT(void);
void connect_MQTT(void);
void connect_wifi(void);
void verify_wifi_connection(void);
void send_data_iot_platform(void);

/* Funcao: inicializa conexao wi-fi
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void init_wifi(void) 
{
    delay(10);
    Serial.println("------WI-FI -----");
    Serial.print("Tentando se conectar a rede wi-fi ");
    Serial.println(ssid_wifi);
    Serial.println("Aguardando conexao");    
    connect_wifi();
}

/* Funcao: inicializa variaveis do MQTT para conexao com broker
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void init_MQTT(void)
{
    MQTT.setServer(broker_mqtt, broker_port);
}

/* Funcao: conecta com broker MQTT (se nao ha conexao ativa)
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void connect_MQTT(void) 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao broker MQTT: ");
        Serial.println(broker_mqtt);
        
        if (MQTT.connect(MQTT_ID, MQTT_USERNAME, MQTT_PASSWORD)) 
        {
            Serial.println("Conectado ao broker MQTT com sucesso!");
        } 
        else 
        {
            Serial.println("Falha na tentativa de conexao com broker MQTT.");
            Serial.println("Nova tentativa em 2s...");
            delay(2000);
        }
    }
}

/* Funcao: conexao a uma rede wi-fi
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void connect_wifi(void) 
{
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(ssid_wifi, password_wifi);
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Conectado com sucesso a rede wi-fi: ");
    Serial.println(ssid_wifi);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

/* Funcao: verifica e garante conexao wi-fi
 * Parametros: nenhum
 * Retorno: nenhum 
 */
void verify_wifi_connection(void)
{
    connect_wifi(); 
}

/* Funcao: envia informacoes para plataforma IoT (Tago.io) via MQTT
 * Parametros: nenhum
 * Retorno: nenhum
 */
void send_data_iot_platform(void)
{
   StaticJsonDocument<250> tago_json_temperature;
   StaticJsonDocument<250> tago_json_humidity;
   char json_string[250] = {0};
   float temperatura_lida = dht.readTemperature();
   float umidade_lida = dht.readHumidity();
   int i;

   /* Imprime medicoes de temperatura e umidade (para debug) */
   for (i=0; i<80; i++)
       Serial.println(" ");
       
   Serial.println("----------");
   Serial.print("Temperatura: ");
   Serial.print(temperatura_lida);
   Serial.println("C");
   Serial.print("Umidade: ");
   Serial.print(umidade_lida);
   Serial.println("%");

   /* Envio da temperatura */
   tago_json_temperature["variable"] = "temperatura";
   tago_json_temperature["unit"] = "C";
   tago_json_temperature["value"] = temperatura_lida;
   memset(json_string, 0, sizeof(json_string));
   serializeJson(tago_json_temperature, json_string);
   MQTT.publish(MQTT_PUB_TOPIC, json_string);

   /* Envio da umidade */
   tago_json_humidity["variable"] = "umidade";
   tago_json_humidity["unit"] = "%";
   tago_json_humidity["value"] = umidade_lida;
   memset(json_string, 0, sizeof(json_string));
   serializeJson(tago_json_humidity, json_string);
   MQTT.publish(MQTT_PUB_TOPIC, json_string);
}

void setup() 
{
    /* UARTs setup */  
    Serial.begin(DEBUG_UART_BAUDRATE);

    /* Inicializa comunicacao com sensor DHT22 */
    dht.begin();

    /* Inicializa wi-fi */
    init_wifi();

    /* Inicializa MQTT e faz conexao ao broker MQTT */
    init_MQTT();
    connect_MQTT();
}

void loop() 
{
    /* Faz o envio da temperatura e umidade para a plataforma IoT (Tago.io) */
    send_data_iot_platform();

    /* Aguarda o tempo definido em TEMPO_ENVIO_INFORMACOES para o proximo envio */
    delay(TEMPO_ENVIO_INFORMACOES);
}
