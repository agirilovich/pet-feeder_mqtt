#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "PetFeeder"
#endif

#ifndef MQTT_GENERAL_PREFIX
#  define MQTT_GENERAL_PREFIX "home"
#endif

#define MQTT_SENSORS_NAME "/binary_sensor/petfeedermotion"
#define MQTT_BUTTON_NAME "/button/petfeedermotion"
#define MQTT_CONFIG_PREFIX "homeassistant"
#define SENSOR_NAME "PetFeederMotion"

#define LIMIT_MQTT_FAILURE 10

#define MQTT_TOPIC_SENSOR_CONFIG MQTT_CONFIG_PREFIX MQTT_SENSORS_NAME "/config"
#define MQTT_TOPIC_BUTTON_CONFIG MQTT_CONFIG_PREFIX MQTT_BUTTON_NAME "/config"

#define MQTT_TOPIC_SENSOR_STATE MQTT_GENERAL_PREFIX "/" DEVICE_BOARD_NAME "/motion/state"

#define MQTT_TOPIC_CTRL MQTT_GENERAL_PREFIX "/" DEVICE_BOARD_NAME "/control"

void initMQTT();

void initializeMQTTTopic(const char *Topic, char *SensorConfig);

void subscribeMQTTTopic(const char *Topic);

void publishMQTTPayload(const char *Topic, char *PayloadMessage);

void MQTTMessageCallback(bool IsMotion);

void MQTTListernLoop();

int FeedCommandCallback();
