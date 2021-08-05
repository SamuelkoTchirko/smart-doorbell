/*
 * Smart doorbell which can send emails and SMS messages
 * Made by Samuel Gabriel Tchir
 *
 * https://github.com/SamuelkoTchirko
 *
 */

#include "Arduino.h"

#include <EMailSender.h>
#include <EasyButton.h>
#include <NTPClient.h>

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define BUTTON_PIN 0
EasyButton button(BUTTON_PIN);

//Time client
const long utcOffsetInSeconds = 7200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

//Request for the SMS
String url = "here goes trigger URL";
const int httpsPort = 443;

String hours ;
String minutes ;
String seconds ;

String messageTime ;

const char* ssid = "here goes wifi ssid";
const char* password = "here goes wifi password";


uint8_t connection_state = 0;
uint16_t reconnect_interval = 10000;

//Email sending config
EMailSender emailSend("here goes your email acc with allowed third party devices", "here goes password to that email", "here goes the email of the sender", 
"here goes the name of the sender", "smtp.gmail.com", 465 );

//Wifi and http clients
WiFiClient client;
HTTPClient http;

//Reconnecting variables
unsigned long previousMillis = 0;
unsigned long interval = 240000;

void onPressed() {
    Serial.println("Button has been pressed!");
    
    timeClient.update();

    if(timeClient.getHours() < 10){
      hours = "0"+String(timeClient.getHours());
    }else{
      hours = String(timeClient.getHours());
    }

    if(timeClient.getMinutes() < 10){
      minutes = "0"+String(timeClient.getMinutes());
    }else{
      minutes = String(timeClient.getMinutes());
    }

    if(timeClient.getSeconds() < 10){
      seconds = "0"+String(timeClient.getSeconds());
    }else{
      seconds = String(timeClient.getSeconds());
    }

    messageTime = hours+":"+minutes+":"+seconds;
    
    EMailSender::EMailMessage message;
    message.subject = messageTime;
    message.message = "bolo zazvonene na zvoncek";

    EMailSender::Response resp = emailSend.send("here goes the recipients email", message);

    Serial.println("Sending status: ");

    Serial.println(resp.status);
    Serial.println(resp.code);
    Serial.println(resp.desc);

    http.begin(client, url);
    
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST("{\"value1\":\""+hours+"\",\"value2\":\""+minutes+"\",\"value3\":\""+seconds+"\"}");

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
}

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);
        Serial.println(nSSID);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void setup()
{
    Serial.begin(9600);

    connection_state = WiFiConnect(ssid, password);
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect

    // Initialize the button.
    button.begin();
    // Add the callback function to be called when the button is pressed.
    button.onPressed(onPressed);

    timeClient.begin();
}

void loop()
{
  button.read();
} 
