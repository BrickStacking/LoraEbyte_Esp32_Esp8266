///////// ------------Include Libs------------------//////////////
#include "Arduino.h"
#include "LoRa_E32.h"
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
WiFiClient client;
#if defined(ESP8266)
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#else
#include <WiFi.h> //https://github.com/esp8266/Arduino
#endif
//needed for library
#include <DNSServer.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <Ticker.h>
Ticker ticker, sent_speed;
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define BUILTIN_LED 2

LoRa_E32 E32_TTL_1W(&Serial2); // RX, TX
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // use 8883 for SSL
#define AIO_USERNAME "thienlc11"
#define AIO_KEY "aio_TDBW34PiPRg0ad3OCGYodDhTeMH1"
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish sen1_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen1");
Adafruit_MQTT_Publish sen2_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen2");
Adafruit_MQTT_Publish sen3_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen3");
Adafruit_MQTT_Publish sen4_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen4");
Adafruit_MQTT_Publish sen5_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen5"); //Sen 5-8 is from Lora2
Adafruit_MQTT_Publish sen6_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen6");
Adafruit_MQTT_Publish sen7_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen7");
Adafruit_MQTT_Publish sen8_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Sen8");
Adafruit_MQTT_Publish display_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Display");
Adafruit_MQTT_Publish display2_publish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Display2");

///////// ------------Ending Include Libs------------------//////////////

/******** DECLARE FOR RTOS HANDLER **********************/
TaskHandle_t Task1;

/******** ENDING DECLARE **********************/
///////// ------------Declare Variables------------------//////////////
// struct Message
// {
//   int ID = 0;
//   int sen1 = 0;
//   int sen2 = 0;
//   int sen3 = 0;
//   int sen4 = 0;
// } message;
unsigned long time1 = 0;
struct Message
{
  int32_t ID = 1;
  int32_t sen1 = 1;
  int32_t sen2 = 1;
  int32_t sen3 = 1;
  int32_t sen4 = 1;
} message;
Message message1, message2, message1_last, message2_last; // Struct data for lora 1 and lora 2
///////// ------------End Declare ------------------//////////////

///////// ------------Define Functions------------------//////////////
void update_server_lora1();
void update_server_lora2();
void MQTT_Pull_Data();
void MQTT_connect();
void Task1code(void *pvParameters);
void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED); // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);    // set pin to the opposite state
}
void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

///////////----------Ending Functions-----------//////////////////////////////////////
void setup()
{

  Serial.begin(115200);
  delay(500);
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);
  // ticker.attach(0.6, tick);
  ////LCD begin/////
  lcd.clear();
  lcd.init();
  lcd.backlight(); //Bật đèn nền
  lcd.home();
  lcd.setCursor(2, 0);
  lcd.print("Staring Lora !");
  lcd.setCursor(1, 1);
  for (int i = 0; i < 12; i++)
  {
    lcd.print(".");
    delay(70);
  }

  Serial.println("");
  Serial.println("Hi, I'm going to send message!");

  // Wire.begin(D7, D6);
  // Startup ll pins and UART
  E32_TTL_1W.begin();

  // Send message
  // ResponseStatus rs = E32_TTL_1W.sendMessage("Hello, Day la Thien");
  // // Check If there is some problem of succesfully send
  // Serial.println(rs.getResponseDescription());
  // digitalWrite(2, 0);
  delay(1000);

  //Wifi Begin///
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect("LoraProgram", "123456789"))
  {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  Serial.println("Ok I'am Done");
  lcd.setCursor(0, 0);
  lcd.print("Config Lora Done");
  delay(2000);
  // ticker.detach();
  // pinMode(0,INPUT_PULLUP);
  // pinMode(14,INPUT_PULLUP);
  // pinMode(12,INPUT_PULLUP);
  // pinMode(13,INPUT_PULLUP);
  // Serial.println("First data:");
  // Serial.println("Content data:");
  // Serial.println(message.ID);
  // Serial.println(message.sen1);
  // Serial.println(message.sen2);
  // Serial.println(message.sen3);
  // Serial.println(message.sen4);
  // Serial.println("Done packet");
  time1 = millis();
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      1);        /* pin task to core 0 */
}

void loop()
{
  // // If something available
  // if (millis() - time1 > 10000)
  // {
  //   Serial.print(F("\nSending test text to feed "));
  //   Serial.print("...");

  //   if (!display_publish.publish("Test state"))
  //   {
  //     Serial.println(F("Failed"));
  //   }
  //   else
  //   {
  //     Serial.println(F("OK!"));
  //   }
  //   time1 = millis();
  // }

  if (E32_TTL_1W.available() > 1)
  {
    ResponseStructContainer rsc = E32_TTL_1W.receiveMessage(sizeof(message));
    struct Message message = *(Message *)rsc.data;
    Serial.println("Content data:");
    Serial.println(message.ID);
    if (message.ID == 33619969)
    {
      Serial.println("Frome Lora 2");
      ////Sen 1
      if (message.sen1 == 16777216)
      {
        message2.sen1 = 1;
      }
      else
      {
        message2.sen1 = 0;
      }

      ////Sen 2
      if (message.sen2 == 16777216)
      {
        message2.sen2 = 1;
      }
      else
      {
        message2.sen2 = 0;
      }

      ////Sen 3
      if (message.sen3 == 16777216)
      {
        message2.sen3 = 1;
      }
      else
      {
        message2.sen3 = 0;
      }

      ////Sen 4
      if (message.sen4 == 16777216)
      {
        message2.sen4 = 1;
      }
      else
      {
        message2.sen4 = 0;
      }

      //Update to server if anything change
      update_server_lora2();
      Serial.println(message2.sen1);
      Serial.println(message2.sen2);
      Serial.println(message2.sen3);
      Serial.println(message2.sen4);
      Serial.println("Done packet 2");
    }
    ////////
    if (message.ID == 50397185)
    {
      Serial.println("Frome Lora 1");
      ////Sen 1
      if (message.sen1 == 16777216)
      {
        message1.sen1 = 1;
      }
      else
      {
        message1.sen1 = 0;
      }

      ////Sen 2
      if (message.sen2 == 16777216)
      {
        message1.sen2 = 1;
      }
      else
      {
        message1.sen2 = 0;
      }

      ////Sen 3
      if (message.sen3 == 16777216)
      {
        message1.sen3 = 1;
      }
      else
      {
        message1.sen3 = 0;
      }

      ////Sen 4
      if (message.sen4 == 16777216)
      {
        message1.sen4 = 1;
      }
      else
      {
        message1.sen4 = 0;
      }
      //Update to server if anything change
      update_server_lora1();
      Serial.println(message1.sen1);
      Serial.println(message1.sen2);
      Serial.println(message1.sen3);
      Serial.println(message1.sen4);
      Serial.println("Done packet 1");
    }

    for (int i = 0; i < 6; i++)
    {
      digitalWrite(2, 0);
      delay(50);
      digitalWrite(2, 1);
      delay(50);
    }
    free(rsc.data);
  }

  // Serial.print(digitalRead(0));
  // Serial.print(digitalRead(12));
  // Serial.print(digitalRead(13));
  // Serial.println(digitalRead(14));
}

void Task1code(void *pvParameters)
{

  for (;;)
  {
    // Serial.print("Task1 running on core ");
    // Serial.println(xPortGetCoreID());
    MQTT_connect();
    MQTT_Pull_Data();
  }
}

void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(1000); // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}

void MQTT_Pull_Data()
{
  // ping the server to keep the mqtt connection alive
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }
}

void update_server_lora2()
{
  ///-----Update to server------///////
  if (message2_last.sen1 != message2.sen1) //Check change sen5 to sent to server
  {
    if (message2.sen1 == 0)
    {
      if (!sen5_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 5 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 5 Lora2!"));
      }
    }
    if (message2.sen1 == 1)
    {
      if (!sen5_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 5 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 5 Lora2!"));
      }
    }
    message2_last.sen1 = message2.sen1; //Update variable
  }
  ////-----Ending Update-------/////
  ///-----Update to server------///////
  if (message2_last.sen2 != message2.sen2) //Check change sen5 to sent to server
  {
    if (message2.sen2 == 0)
    {
      if (!sen6_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 6 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 6 Lora2!"));
      }
    }
    if (message2.sen2 == 1)
    {
      if (!sen6_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 6 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 6 Lora2!"));
      }
    }
    message2_last.sen2 = message2.sen2; //Update variable
  }
  ////-----Ending Update-------/////
  ///-----Update to server------///////
  if (message2_last.sen3 != message2.sen3) //Check change sen5 to sent to server
  {
    if (message2.sen3 == 0)
    {
      if (!sen7_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 7 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 7 Lora2!"));
      }
    }
    if (message2.sen3 == 1)
    {
      if (!sen7_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 7 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 7 Lora2!"));
      }
    }
    message2_last.sen3 = message2.sen3; //Update variable
  }
  ////-----Ending Update-------/////
  ///-----Update to server------///////
  if (message2_last.sen4 != message2.sen4) //Check change sen5 to sent to server
  {
    if (message2.sen4 == 0)
    {
      if (!sen8_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 8 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 8 Lora2!"));
      }
    }
    if (message2.sen4 == 1)
    {
      if (!sen8_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 8 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 8 Lora2!"));
      }
    }
    message2_last.sen4 = message2.sen4; //Update variable
  }
  ////-----Ending Update-------/////
}

void update_server_lora1()
{
  ///-----Update to server------///////
  if (message1_last.sen1 != message1.sen1) //Check change sen5 to sent to server
  {
    if (message1.sen1 == 0)
    {
      if (!sen1_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 1 Lora1"));
      }
      else
      {
        Serial.println(F("OK publish sen 1 Lora1!"));
      }
    }

    if (message1.sen1 == 1)
    {
      if (!sen1_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 1 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 1 Lora1!"));
      }
    }
    message1_last.sen1 = message1.sen1; //Update variable
  }
  ////-----Ending Update-------/////
  ///-----Update to server------///////
  if (message1_last.sen2 != message1.sen2) //Check change sen5 to sent to server
  {
    if (message1.sen2 == 0)
    {
      if (!sen2_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 2 Lora1"));
      }
      else
      {
        Serial.println(F("OK publish sen 2 Lora1!"));
      }
    }

    if (message1.sen2 == 1)
    {
      if (!sen2_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 2 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 2 Lora1!"));
      }
    }
    message1_last.sen2 = message1.sen2; //Update variable
  }
  ////-----Ending Update-------/////
  ///-----Update to server------///////
  if (message1_last.sen3 != message1.sen3) //Check change sen5 to sent to server
  {
    if (message1.sen3 == 0)
    {
      if (!sen3_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 3 Lora1"));
      }
      else
      {
        Serial.println(F("OK publish sen 3 Lora1!"));
      }
    }

    if (message1.sen3 == 1)
    {
      if (!sen3_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 3 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 3 Lora1!"));
      }
    }
    message1_last.sen3 = message1.sen3; //Update variable
  }
  ////-----Ending Update-------/////
  ///-----Update to server------///////
  if (message1_last.sen4 != message1.sen4) //Check change sen5 to sent to server
  {
    if (message1.sen4 == 0)
    {
      if (!sen4_publish.publish("Occupy"))
      {
        Serial.println(F("Failed publish sen 4 Lora1"));
      }
      else
      {
        Serial.println(F("OK publish sen 4 Lora1!"));
      }
    }

    if (message1.sen4 == 1)
    {
      if (!sen4_publish.publish("Unoccupy"))
      {
        Serial.println(F("Failed publish sen 4 Lora2"));
      }
      else
      {
        Serial.println(F("OK publish sen 4 Lora1!"));
      }
    }
    message1_last.sen4 = message1.sen4; //Update variable
  }
  ////-----Ending Update-------/////
}