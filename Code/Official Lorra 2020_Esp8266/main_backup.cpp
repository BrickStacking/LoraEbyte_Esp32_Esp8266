#include "Arduino.h"
#include "LoRa_E32.h"
#include <Wire.h>

LoRa_E32 E32_TTL_1W(D1, D2);        // RX, TX

void setup()
{
  Serial.begin(115200);
  delay(500);
  pinMode(2, OUTPUT);                                        
  digitalWrite(2, 1);
  pinMode(0,INPUT_PULLUP);
  pinMode(14,INPUT_PULLUP);
  pinMode(12,INPUT_PULLUP);
  pinMode(13,INPUT_PULLUP);
  Serial.println("Hi, I'm going to send message!");
  
  // Wire.begin(D7, D6);
  // Startup ll pins and UART
  E32_TTL_1W.begin();

  // Send message
  ResponseStatus rs = E32_TTL_1W.sendMessage("Hello, Day la Thien");
  // Check If there is some problem of succesfully send 
  Serial.println(rs.getResponseDescription());
  digitalWrite(2, 0);
}

void loop()
{
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
  if (Serial.available())
  {
    String input = Serial.readString();
    E32_TTL_1W.sendMessage(input);
  }
  Serial.print(digitalRead(0));
  Serial.print(digitalRead(12));
  Serial.print(digitalRead(13));
  Serial.println(digitalRead(14));
  
}