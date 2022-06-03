//main file.

#include <Arduino.h>
#include <Wire.h>
#include "wifi_stuff.h"
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "ina219_stuff.h"
#include "SD_Card_stuff.h"
#include "OLED_stuff.h"

JSONVar readings;
// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
bool send_f = false;
String getSensorReadings()
{
  ina219values();
  readings["time"] = String(timeClient.getEpochTime());
  readings["voltage"] = String(loadvoltage);
  readings["current"] = String(current_mA);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

void setup()
{
  // Serial port for debugging purposes

  Serial.begin(115200);

  SPIClass spi = SPIClass(VSPI);
  spi.begin(SCK, MISO, MOSI, CS);

  initSDCard();
  initSPIFFS();
  initINA219();
  initWiFi();

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  

  initTime();

  // oled stuff to display ip
  oled.clearDisplay();
  oled.setFont(&FreeSans9pt7b);
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 17);

  oled.println(WiFi.localIP());
  oled.display();
  delay(2000);
  /// end oled stuff

  f_out = String("/") + String("Res_") + get_date_string() + ".csv";
  deleteFile(SD, f_out.c_str());
  if (SD.exists(f_out.c_str()))
  {
    appendFile(SD, f_out.c_str(), "\n<br\>\n<br\>");
    appendFile(SD, f_out.c_str(), f_out.c_str()); // + " time,  current, voltage \n<br\>");
    appendFile(SD, f_out.c_str(), "\n<br\>");
  }
  else
  {
    writeFile(SD, f_out.c_str(), f_out.c_str());
    appendFile(SD, f_out.c_str(), "\n<br\>");
    appendFile(SD, f_out.c_str(), "time,  current, voltage \n<br\>");
  }

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.serveStatic("/", SPIFFS, "/");

  server.on("/data_file", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(SPIFFS, "/data_file.html", "text/html");
    Serial.println("request");
    send_f = true;
    // Serial.println(f_out.c_str());
   

          
        // AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [&file](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
          
        //   //Write up to "maxLen" bytes into "buffer" and return the amount written.
        //   //index equals the amount of bytes that have been already sent
        //   //You will be asked for more data until 0 is returned
        //   //Keep in mind that you can not delay or yield waiting for more data!
        //   //maxLen
        //   // File filelambda = file;
        //   // Serial.println(maxLen);
        //   size_t nn = file.read(buffer,maxLen);
        //   Serial.println(nn);
        //   return nn;
        //   // if(file.available()) {return file.read(buffer, maxLen);}
        //   // else {return 0;}
        //         });

        //code borrowed from here: https://github.com/jendakol/bomb-game/blob/master/src/networking/WebServer.h#L22
        // AsyncWebServerResponse *response = request->beginResponse(
        //             "text/html",
        //             file.size(),
        //             [file](uint8_t *buffer, size_t maxLen, size_t total) mutable -> size_t {
        //                 int bytes = file.read(buffer, maxLen);

        //                 // close file at the end
        //                 if (bytes + total == file.size()) file.close();

        //                 return max(0, bytes); // return 0 even when no bytes were loaded
        //             }
        //     );          

        // // response->addHeader("Server","ESP Async Web Server");
        // try {request->send(response);} 
        // catch(int i){}
        // request->send(SD, f_out.c_str(), "text/html"); 
      });
    // if(file) try{file.close();} catch(int i){Serial.println("thrown here");} });

  server.serveStatic("/", SD, "/");
  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String json = getSensorReadings();
              request->send(200, "application/json", json);
              // Serial.println(json);
              json = String(); });
  events.onConnect([](AsyncEventSourceClient *client)
                   {
              if(client->lastId()){
              Serial.printf("Client reconnected! Last message ID that it got is: %u\n",
              client->lastId());
              }
              // send event with message "hello!", id current millis
              // and set reconnect delay to 1 second
              client->send("hello!", NULL, millis(), 10000); });

  server.addHandler(&events);
  lastTime = millis();
  // Start server
  server.begin();
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping", NULL, millis());
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();

    // Serial.println(getSensorReadings().c_str());
    events.send(getSensorReadings().c_str(), "new_readings", millis());

    String formattedTime = timeClient.getFormattedTime();
    if (send_f){
      send_file(SD,f_out.c_str());
      send_f = false;
    }
    // events.send("hello", "get_file", millis());
    // previousMillis = currentMillis;
    dataMessage = String(formattedTime) + ", " + String(current_mA) + ", " + String(loadvoltage) + "\r\n<br/>";
    appendFile(SD, f_out.c_str(), dataMessage.c_str());
    displaydata();
    lastTime = millis();
  }
  // put your main code here, to run repeatedly:
}