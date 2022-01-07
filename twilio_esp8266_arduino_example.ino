/*
 * Twilio SMS and MMS on ESP8266 Example.
 */

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>

#include "twilio.hpp"

// Use software serial for debugging?
#define USE_SOFTWARE_SERIAL 0

// Print debug messages over serial?
#define USE_SERIAL 1

// Your network SSID and password
const char* ssid = "MovuinoTest";
const char* password = "MovuinoTest";

// Find the api.twilio.com SHA1 fingerprint, this one was valid as 
// of July 2020. This will change, please see 
// https://www.twilio.com/docs/sms/tutorials/how-to-send-sms-messages-esp8266-cpp
// to see how to update the fingerprint.
const char fingerprint[] = "0E 15 35 A9 DD DB 63 DF CE D3 02 2E A8 6A 5A E0 28 9B FF 26";

// Twilio account specific details, from https://twilio.com/console
// Please see the article: 
// https://www.twilio.com/docs/guides/receive-and-reply-sms-and-mms-messages-esp8266-c-and-ngrok

// If this device is deployed in the field you should only deploy a revocable
// key. This code is only suitable for prototyping or if you retain physical
// control of the installation.
const char* account_sid = "AC1640975185c8210dc9b0dfb66b330265";
const char* auth_token = "2f08c2d277ce755a53b6ea8f9528ca51";

// Details for the SMS we'll send with Twilio.  Should be a number you own 
// (check the console, link above).
String to_number    = "+33676958636";
String from_number = "+12242040481";
String message_body    = "I AM IN DANGER! NEED HELP!";

// The 'authorized number' to text the ESP8266 for our example
String master_number    = "+18005551212";

// Optional - a url to an image.  See 'MediaUrl' here: 
// https://www.twilio.com/docs/api/rest/sending-messages
String media_url = "";

// Global twilio objects
Twilio *twilio;
ESP8266WebServer twilio_server(8000);

//  Optional software serial debugging
#if USE_SOFTWARE_SERIAL == 1
#include <SoftwareSerial.h>
// You'll need to set pin numbers to match your setup if you
// do use Software Serial
extern SoftwareSerial swSer(14, 4, false, 256);
#else
#define swSer Serial
#endif

// constants won't change. They*re used here to set pin numbers:
const int buttonPin = 12; // the number of the pushbutton pin
const int buzzerPin = 13;
const int resetPin = 14;

// variables will change:
int buttonState = 0; // variable for reading the pushbutton status
int buzzerState = 0;
int resetState = 0;

bool buttonopen = false;
bool resetpressed = false;
bool buzzeropen = false;
int  chrono = 0;

int countSMS = 0;


/*
 * Callback function when we hit the /message route with a webhook.
 * Use the global 'twilio_server' object to respond.
 */
 void handle_message() {
        #if USE_SERIAL == 1
        swSer.println("Incoming connection!  Printing body:");
        #endif
        bool authorized = false;
        char command = '\0';

        // Parse Twilio's request to the ESP
        for (int i = 0; i < twilio_server.args(); ++i) {
                #if USE_SERIAL == 1
                swSer.print(twilio_server.argName(i));
                swSer.print(": ");
                swSer.println(twilio_server.arg(i));
                #endif

                if (twilio_server.argName(i) == "From" and 
                    twilio_server.arg(i) == master_number) {
                    authorized = true;
                } else if (twilio_server.argName(i) == "Body") {
                        if (twilio_server.arg(i) == "?" or
                            twilio_server.arg(i) == "0" or
                            twilio_server.arg(i) == "1") {
                                command = twilio_server.arg(i)[0];
                        }
                }
        } // end for loop parsing Twilio's request

        // Logic to handle the incoming SMS
        // (Some board are active low so the light will have inverse logic)
        String response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        if (command != '\0') {
                if (authorized) {
                        switch (command) {
                        case '0':
                                digitalWrite(LED_BUILTIN, LOW);
                                response += "<Response><Message>"
                                "Turning light off!"
                                "</Message></Response>";
                                break;
                        case '1':
                                digitalWrite(LED_BUILTIN, HIGH);
                                response += "<Response><Message>"
                                "Turning light on!"
                                "</Message></Response>";
                                break;
                        case '?':
                        default:
                                response += "<Response><Message>"
                                "0 - Light off, 1 - Light On, "
                                "? - Help\n"
                                "The light is currently: ";
                                response += digitalRead(LED_BUILTIN);
                                response += "</Message></Response>";
                                break;               
                        }
                } else {
                        response += "<Response><Message>"
                        "Unauthorized!"
                        "</Message></Response>";
                }

        } else {
                response += "<Response><Message>"
                "Look: a SMS response from an ESP8266!"
                "</Message></Response>";
        }

        twilio_server.send(200, "application/xml", response);
}

/*
 * Setup function for ESP8266 Twilio Example.
 * 
 * Here we connect to a friendly wireless network, set the time, instantiate 
 * our twilio object, optionally set up software serial, then send a SMS 
 * or MMS message.
 */
void setup() {
  //Serial.begin(115200);
//        WiFi.begin(ssid, password);
//        twilio = new Twilio(account_sid, auth_token, fingerprint);
//
//        #if USE_SERIAL == 1
//        swSer.begin(115200);
//        while (WiFi.status() != WL_CONNECTED) {
//                delay(1000);
//                swSer.print(".");
//        }
//        swSer.println("");
//        swSer.println("Connected to WiFi, IP address: ");
//        swSer.println(WiFi.localIP());
//        #else
//        while (WiFi.status() != WL_CONNECTED) delay(1000);
//        #endif
//
//        // Response will be filled with connection info and Twilio API responses
//        // from this initial SMS send.
//        String response;
//        bool success = twilio->send_message(
//                to_number,
//                from_number,
//                message_body,
//                response,
//                media_url
//        );
//
//        // Set up a route to /message which will be the webhook url
//        twilio_server.on("/message", handle_message);
//        twilio_server.begin();
//
//        // Use LED_BUILTIN to find the LED pin and set the GPIO to output
//        pinMode(LED_BUILTIN, OUTPUT);
//
//        #if USE_SERIAL == 1
//        swSer.println(response);
//        #endif

Serial.begin(115200);
  //delay(2000);
  pinMode (buzzerPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode (buttonPin, INPUT);
  pinMode (resetPin, INPUT);

  //twilio_function ();
}


/* 
 *  In our main loop, we listen for connections from Twilio in handleClient().
 */
void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead (buttonPin) ;
  buzzerState = digitalRead (buzzerPin) ;
  resetState =  digitalRead (resetPin) ;

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
  //.println("On");
  } else {
    Serial.println("bouton ouvert");
    buttonopen = true;
    chrono = millis();
  }

   if (resetState == LOW) {
    Serial.println("reset on");
  } else {
    Serial.println("reset off");
    resetpressed = true;
    buttonopen = false;
    buzzeropen = false;
  }

  if (buttonopen == true && millis() - chrono >= 10000) {
    digitalWrite(13, HIGH);

//    if(countSMS == 0){
      twilio_function();
      delay(5000);
//      }else{
//        countSMS = 0;
//      }
  }

  if (buttonopen == false) {
    digitalWrite (13,LOW);
 
 }
  if (buzzeropen == true && millis() - chrono >= 15000){
//    Serial.println("Send SMS");
//      twilio_function();
//      delay(1000);
//      twilio_server.handleClient();
    
}

  //twilio_server.handleClient();
}
