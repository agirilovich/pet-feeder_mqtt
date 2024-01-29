#include "MQTT_task.h"
#include "controlWiFi.h"
#include "Credentials.h"

char mqtt_command[8];
char mqtt_argument[8];

//Define MQTT Topic for HomeAssistant Motion Discovery
const char *MQTTMotionTopicConfig = MQTT_TOPIC_SENSOR_CONFIG;

//Define MQTT Topic for HomeAssistant Button Discovery
const char *MQTTButtonTopicConfig = MQTT_TOPIC_BUTTON_CONFIG;

//Define MQTT Topic for HomeAssistant Motion Sensor state
const char *MQTTMotionTopicState = MQTT_TOPIC_SENSOR_STATE;

//Define MQTT Topic for Manage Feed Engine
const char *MQTTTopicControl = MQTT_TOPIC_CTRL;


const char *mqtt_host = mqtt_server;
const int mqtt_port = 1883;
const char *mqtt_user = mqtt_username;
const char *mqtt_pass = mqtt_password;

//Define objects for MQTT messages in JSON format
#include <ArduinoJson.h>
StaticJsonDocument<512> JsonSensorConfig;
StaticJsonDocument<512> JsonButtonConfig;
char Buffer[512];

WiFiClient client;

#include <PubSubClient.h>
PubSubClient mqtt(client);

void MQTTListenCallback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Command received: ");
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);
  //Serial.println(doc);

  //mqtt_command = doc["Command"];
  strlcpy(mqtt_command, doc["Command"] | "", 8);
  Serial.println(mqtt_command);
  //mqtt_argument = doc["load"];
  strlcpy(mqtt_argument, doc["Load"] | "", 8);
  Serial.print("With argument: ");
  Serial.println(mqtt_argument);
} 


void initMQTT()
{
  Serial.print("Connecting to MQTT broker host: ");
  Serial.println(mqtt_host);
  while (!client.connect(mqtt_host, mqtt_port))
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected!");

  //Initialise MQTT autodiscovery topic and sensor
  mqtt.setServer(mqtt_host, mqtt_port);
  mqtt.setCallback(MQTTListenCallback);

  Serial.print("Testing connection to mqtt broker...");
  while (!mqtt.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass))
  {
    Serial.print(".");
    delay(1000);
  }

  if (mqtt.connected()) {
    Serial.println(" connected!");
  } 
  //Motion Sensor
  JsonSensorConfig["name"] = "Pet Feeder motion detector";
  JsonSensorConfig["device_class"] = "motion";
  JsonSensorConfig["state_class"] = "measurement";
  JsonSensorConfig["off_delay "] = "10";
  JsonSensorConfig["uniq_id"] = "petfeedermotion";
  JsonSensorConfig["state_topic"] = MQTTMotionTopicState;

  JsonObject device  = JsonSensorConfig.createNestedObject("device");
  device["identifiers"][0] = "petfeederw000";
  device["connections"][0][0] = "mac";
  device["connections"][0][1] = "D4:D4:DA:CF:64:D8";
  device["model"] = "PET-FEEDER-01";
  device["name"] = SENSOR_NAME;
  device["manufacturer"] = "Aliexpress"; 
  device["sw_version"] = "1.0";  
  serializeJson(JsonSensorConfig, Buffer);
  initializeMQTTTopic(MQTTMotionTopicConfig, Buffer);

  //Feed Button
  JsonButtonConfig["name"] = "Pet Feeder feed button";
  JsonButtonConfig["command_topic"] = MQTTTopicControl;
  JsonButtonConfig["payload_press"] = "{\"Command\":\"feed\", \"Load\":\"5\"}";
  JsonButtonConfig["uniq_id"] = "waterfiltertdsppn";
    
  device  = JsonButtonConfig.createNestedObject("device");
  device["identifiers"][0] = "petfeederw000";
  device["connections"][0][0] = "mac";
  device["connections"][0][1] = "D4:D4:DA:CF:64:D8";
  device["model"] = "PET-FEEDER-01";
  device["name"] = SENSOR_NAME;
  device["manufacturer"] = "Aliexpress"; 
  device["sw_version"] = "1.0";  

  serializeJson(JsonButtonConfig, Buffer);
  initializeMQTTTopic(MQTTButtonTopicConfig, Buffer);

  subscribeMQTTTopic(MQTTTopicControl);
}

void initializeMQTTTopic(const char *Topic, char *SensorConfig)
{
  
  Serial.println("Initialise MQTT autodiscovery topics and sensors...");
  Serial.println(Topic);
  //Serial.println(SensorConfig);

  //Publish message to AutoDiscovery topic
  if (mqtt.publish(Topic, SensorConfig, true)) {
    Serial.println("Done");
  } else {
    mqtt.publish(Topic, SensorConfig, true);
  }
  
  //Gracefully close connection to MQTT broker
}


void subscribeMQTTTopic(const char *Topic)
{
  
  Serial.println("Subscribe to MQTT ...");
  Serial.println(Topic);
  mqtt.subscribe(Topic);
    
  //Gracefully close connection to MQTT broker
}

void MQTTListernLoop()
{
  if (!mqtt.connected()) {
    initMQTT();
  } else {
    mqtt.loop();
  }
}

void MQTTMessageCallback(bool IsMotion)
{
  char MessageBuf[16];
  //Publish MQTT messages
  Serial.println("Publishing MQTT messages...");
  mqtt.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass);
  if (mqtt.connected()) {

    sprintf(MessageBuf, "%s", IsMotion?"ON":"OFF");
    mqtt.publish(MQTTMotionTopicState, MessageBuf, false);
  }
  else {
    Serial.println("Unable to connect to MQTT broker");
    Serial.println("Cycle is skipped");
  }
  //mqtt.disconnect();
  Serial.println("Done");
}

int FeedCommandCallback()
{
  int load = 0;
  if(strcmp(mqtt_command, "feed") == 0)
  {
    int i = 0;
    for (i = 0; mqtt_argument[i] != '\0'; i++)
    {
      load = load * 10 + (mqtt_argument[i] - '0');
    }
  }
  strlcpy(mqtt_command, "", 1);
  strlcpy(mqtt_argument, "", 1);

  return load;
}