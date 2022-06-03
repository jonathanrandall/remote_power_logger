#ifndef WIFI_STUFF_
#define WIFI_STUFF_

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ssid_stuff.h" //i have my wife details here. So I don't accidently put them on github :)

// I've put mine in ssid_stuff.h file
// const char *ssid = "";
// const char *password = "";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");
// Json Variable to Hold Sensor Readings

// Timer variables
// unsigned long lastTime = 0;
// unsigned long timerDelay = 30000;

const long  gmtOffset_sec = 36000;
const int   daylightOffset_sec = 3600;

//const char* ntpServer = "pool.ntp.org";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


void initWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
        
    }
    Serial.println(WiFi.localIP());
}

void initTime(){
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(36000);
}

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  Serial.println(now);
  return now;
}

void send_file(fs::FS &fs, const char * path){
   File file = fs.open(path);
    uint8_t buf[512];
    size_t len = 0;
      if(!file){
       
        Serial.println("Failed to open file for appending");
        // return;
      } else {
          len = file.size();
          size_t flen = len;
          
          while(len){
            size_t toRead = len;
            if(toRead > 511){
              toRead = 511;
            }
            // file.read(buf, toRead);
            len -= toRead;
            memset(&buf[0],0,sizeof(buf));
            file.read(buf,toRead);
            Serial.println("new buffer");
            Serial.println(((char *)buf));
            // events.send("ping", NULL, millis());
            events.send((const char *) buf, "get_file", millis());
          }
          file.close();}
}

String get_date_string(){
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  String mth_pad = "";
  String day_pad = "";
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;

  if(monthDay < 10){ day_pad = "0";}
  if (currentMonth <10){  mth_pad = "0"; }
  
  return (String(currentYear) +String(mth_pad) + String(currentMonth) + String(day_pad) + String(monthDay));
}


#endif