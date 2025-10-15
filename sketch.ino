#include <WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h> 

// --- Configurações de Rede ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";        

// --- Configurações do MQTT ---
const char* mqtt_server = "20.97.192.88"; 
const int mqtt_port = 1883;
const char* mqtt_topic = "/fiap/teste_erick_jooji_1espv/attrs"; // "Endpoint" para publicar os dados

// --- Instâncias dos Clientes ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- Pinos dos Sensores ---
int pinoLDR = 34;
int pinoDHT = 15;

DHTesp dhtSensor;

void setup() {
  Serial.begin(9600);
  
  // Inicializa o sensor DHT
  dhtSensor.setup(pinoDHT, DHTesp::DHT22);

  // Conecta ao Wi-Fi
  setup_wifi();
  
  // Configura o servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Se não estiver conectado ao MQTT, tenta reconectar
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop(); 

  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  int iluminacao = analogRead(pinoLDR);
  // Transforma/mapeia o valor da iluminação para de 0 a 100
  int iluminacao_porcento = map(iluminacao, 4095, 0, 0, 100);

  // Checa se a leitura dos sensores foi bem-sucedida
  if (dhtSensor.getStatus() == DHTesp::ERROR_NONE) {
    
  // --- Cria a mensagem q vai ser enviada  ---
  String payload = "t|";
  payload += String(data.temperature, 2);
  payload += "|h|";
  payload += String(data.humidity, 1);
  payload += "|l|";
  payload += String(iluminacao_porcento);

    // --- Print no serial para depuração ---
    Serial.println("---");
    Serial.println("Publicando dados via MQTT...");
    Serial.print("Tópico: ");
    Serial.println(mqtt_topic);
    Serial.print("Payload: ");
    Serial.println(payload);

    // --- Publica a mensagem no MQTT ---
    client.publish(mqtt_topic, payload.c_str());

  } else {
    Serial.println("Erro ao ler o sensor DHT.");
  }

  delay(5000); 
}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT Broker...");

    if (client.connect("ESP32Client-Agnello")) {
      
      Serial.println("conectado!");

    } else {

      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);

    }
  }
}