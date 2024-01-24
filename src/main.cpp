#include <Arduino.h>

#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "PetFeeder"
#endif

#include "controlWiFi.h"

#include "MQTT_task.h"

#include <esp_task_wdt.h>
#define WDT_TIMEOUT 60

#define LED_BUILTIN 22

#define MOTION_INPUT_PIN 13

bool IsMotion = false;

hw_timer_t *Timer0_Cfg = NULL;


//Interrupt handler for motion detect
void IRAM_ATTR move_HANDLER() {
  IsMotion = true;
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

  //Setup Hardware Timer for MQTT publish
  Timer0_Cfg = timerBegin(0, 200, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 10000000, true);
  timerAlarmEnable(Timer0_Cfg);

  // Setup motion sensor pin and assign to interrupt
  pinMode(MOTION_INPUT_PIN, INPUT);
  gpio_set_intr_type(MOTION_INPUT_PIN, GPIO_INTR_POSEDGE);
  gpio_isr_handler_add(MOTION_INPUT_PIN, move_HANDLER, NULL);
}


void loop()
{
  if (IsMotion)
  {
    digitalWrite(LED_BUILTIN, LOW);
    MQTTMessageCallback(IsMotion);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    MQTTListernLoop();
  }
  IsMotion = false;
  esp_task_wdt_reset();
}
