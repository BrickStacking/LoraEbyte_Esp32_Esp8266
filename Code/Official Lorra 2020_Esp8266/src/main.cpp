#include "Arduino.h"
#include "LoRa_E32.h"
#include <Wire.h>

LoRa_E32 E32_TTL_1W(D1, D2); // RX, TX
#define sen1_state 0         //D3
#define sen2_state 14        //D5
#define sen3_state 12        //D6
#define sen4_state 13        //D7

// struct Message
// {
//   int ID = 0;
//   int sen1 = 0;
//   int sen2 = 0;
//   int sen3 = 0;
//   int sen4 = 0;
// } message;
struct Message
{
  int32_t ID = 1;
  int32_t sen1 = 1;
  int32_t sen2 = 1;
  int32_t sen3 = 1;
  int32_t sen4 = 1;
} message;

int state_bt1 = 0, state_bt2 = 0, state_bt3 = 0, state_bt4 = 0;
int last_state_bt1 = 0, last_state_bt2 = 0, last_state_bt3 = 0, last_state_bt4 = 0;
bool enable_out = 0;
//-----Declare Functions------///
void check_state();
void send_data();
void setup()
{
  Serial.begin(115200);
  message.ID = 2; //ID 3 for lora 1. ID 2 for lora 2
  delay(500);
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);
  pinMode(0, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  Serial.println("Hi, I'm going to send structure message!");
  last_state_bt1 = digitalRead(sen1_state);
  last_state_bt2 = digitalRead(sen2_state);
  last_state_bt3 = digitalRead(sen3_state);
  last_state_bt4 = digitalRead(sen4_state);

  // Wire.begin(D7, D6);
  // Startup ll pins and UART
  E32_TTL_1W.begin();

  // Send message
  // ResponseStatus rs = E32_TTL_1W.sendMessage(&message);

  // ResponseStatus rs = E32_TTL_1W.sendFixedMessage(1, 0, 1, &message, sizeof(Message));
  // // Check If there is some problem of succesfully send
  // Serial.println(rs.getResponseDescription());

  digitalWrite(2, 0);
}

void loop()
{
  check_state();
  // If something available
  if (E32_TTL_1W.available() > 1)
  {
    // read the String message
    ResponseContainer rc = E32_TTL_1W.receiveMessage();
    // Is something goes wrong print error
    if (rc.status.code != 1)
    {
      rc.status.getResponseDescription();
    }
    else
    {
      // Print the data received
      Serial.print("Data get:");
      Serial.println(rc.data);
      for (int i = 0; i < 10; i++)
      {
        digitalWrite(2, 0);
        delay(50);
        digitalWrite(2, 1);
        delay(50);
      }
    }
  }

  if (enable_out == 1)
  {
    send_data();
    enable_out = 0;
  }
  // Serial.print(digitalRead(0));
  // Serial.print(digitalRead(12));
  // Serial.print(digitalRead(13));
  // Serial.println(digitalRead(14));
}

void check_state()
{

  if (last_state_bt1 != digitalRead(sen1_state))
  {
    Serial.println("Change State 1");
    delay(1000);
    if (last_state_bt1 != digitalRead(sen1_state))
    {
      Serial.println("Change State Sen1");
      message.sen1 = digitalRead(sen1_state);   //Update vao structure
      last_state_bt1 = digitalRead(sen1_state); //Update last_state
      enable_out = 1;
    }
  }

  if (last_state_bt2 != digitalRead(sen2_state))
  {
    Serial.println("Change State 2");
    delay(1000);
    if (last_state_bt2 != digitalRead(sen2_state))
    {
      Serial.println("Change State Sen2");
      message.sen2 = digitalRead(sen2_state);   //Update vao structure
      last_state_bt2 = digitalRead(sen2_state); //Update last_state
      enable_out = 1;
    }
  }

  if (last_state_bt3 != digitalRead(sen3_state))
  {
    Serial.println("Change State 3");
    delay(1000);
    if (last_state_bt3 != digitalRead(sen3_state))
    {
      Serial.println("Change State Sen3");
      message.sen3 = digitalRead(sen3_state);   //Update vao structure
      last_state_bt3 = digitalRead(sen3_state); //Update last_state
      enable_out = 1;
    }
  }

  if (last_state_bt4 != digitalRead(sen4_state))
  {
    Serial.println("Change State 4");
    delay(1000);
    if (last_state_bt4 != digitalRead(sen4_state))
    {
      Serial.println("Change State Sen4");
      message.sen4 = digitalRead(sen4_state);   //Update vao structure
      last_state_bt4 = digitalRead(sen4_state); //Update last_state
      enable_out = 1;
    }
  }
}

void send_data()
{
  Serial.println("Content data:");
  Serial.println(message.ID);
  Serial.println(message.sen1);
  Serial.println(message.sen2);
  Serial.println(message.sen3);
  Serial.println(message.sen4);
  Serial.println("Done packet");
  ResponseStatus rs = E32_TTL_1W.sendFixedMessage(1, 0, 1, &message, sizeof(message));
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(2, 0);
    delay(200);
    digitalWrite(2, 1);
    delay(200);
  }
  // Check If there is some problem of succesfully send
  Serial.println(rs.getResponseDescription());
}