#include <Arduino.h>

#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "PetFeeder"
#endif

#include "controlWiFi.h"

#include "MQTT_task.h"

#include <esp_task_wdt.h>
#define WDT_TIMEOUT 60

#define LED_BUILTIN 22

#define MOTION_INPUT_PIN 25

#define ENGINE_OUTPUT_PIN 26

bool IsMotion = false;

hw_timer_t *Timer0_Cfg = NULL;


//Interrupt handler for motion detect
void IRAM_ATTR move_HANDLER() {
  IsMotion = true;
}

void FeedEngineOff()
{
  Serial.println("Stop engine.");
  timerWrite(Timer0_Cfg, 0);
  timerAlarmDisable(Timer0_Cfg);
  timerDetachInterrupt(Timer0_Cfg);
  digitalWrite(ENGINE_OUTPUT_PIN, LOW);
}

void FeedEngineOn(int FeedLoad)
{
  if(FeedLoad > 0) {
    Serial.print("Feed command has been detected. Switching on engine for ");
    Serial.print(FeedLoad);
    Serial.println(" sec.");
    digitalWrite(ENGINE_OUTPUT_PIN, HIGH);
    timerAlarmWrite(Timer0_Cfg, FeedLoad * 1000 * 10, false);
    timerAttachInterrupt(Timer0_Cfg, &FeedEngineOff, true);
    timerAlarmEnable(Timer0_Cfg);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");

  pinMode(LED_BUILTIN, OUTPUT);

  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_POWERON:
      Serial.println("Reset due to power-on event");
    break;
  
    case ESP_RST_SW:
      Serial.println("Software reset via esp_restart");
    break;

    case ESP_RST_WDT:
      Serial.println("Rebooted by Watchdog!");
    break;
  }

  //Set witchdog timeout for 32 seconds
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  while (esp_task_wdt_status(NULL) != ESP_OK) {
    // LED blinks indefinitely
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }

  Serial.print("Start WiFi on ");
  Serial.println(DEVICE_BOARD_NAME);
  
  initializeWiFi(DEVICE_BOARD_NAME);
  
  establishWiFi();

  // you're connected now, so print out the data
  printWifiStatus();

  initMQTT();

  //Setup Hardware Timer for motor 
  Timer0_Cfg = timerBegin(0, 8000, true);
  
  // Setup motion sensor pin and assign to interrupt
  pinMode(MOTION_INPUT_PIN, INPUT);
  attachInterrupt(MOTION_INPUT_PIN, move_HANDLER, RISING);

  pinMode(ENGINE_OUTPUT_PIN, OUTPUT);
}


void loop()
{
  if (IsMotion)
  {
    Serial.println("Motion is Detected. Sending mqtt message...");
    digitalWrite(LED_BUILTIN, LOW);
    MQTTMessageCallback(IsMotion);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    IsMotion = false;
  } else {
    MQTTListernLoop();
  }

  FeedEngineOn(FeedCommandCallback());
  delay(50);
  esp_task_wdt_reset();
}
