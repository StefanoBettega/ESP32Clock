/**
 *  ESP32Clock, a project by Stefano Bettega
 *  
 *  Version history
 *  2019.12.21  First release
 */

/* Include section -------------------------------------------- */

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "WiFiData.h"
#include "ClockFSM.h"
#include "SSD1306Wire.h"
#include "Version.h"

// #include "images.h"

/* Global variable section ------------------------------------ */

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 4, 15);

// Clock data
CClockData clockdata;
CRTCClock  rtcclock(clockdata);

// NTP synch object
CNTPFsm   ntpfsm(ntpsite, 1, 0, 1500, 60000, clockdata);
CNetworkFsm network(ssid, wpskey);

int nDelay = 1000;
enum EDisplayMode { Init, Hour, Date, Other } eDisplayMode;
 
/* Setup function --------------------------------------------- */

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // Reset display before starting
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  delay(50);
  digitalWrite(16,HIGH); 

  Serial.println("Init");
  
  display.init();

  display.flipScreenVertically();

  // Initialising the UI will init the display too.
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "ESP32 Clock");
  display.drawString(0, 10, "by Stefano Bettega");

  String version = "Version: " +
                   String(MajorVersion) + "." +
                   String(MinorVersion) + "." +
                   String(Build);

  display.drawString(0, 26, version);

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.display();

  eDisplayMode = Init;
  nDelay = 500;
}

void loop() {

  CNetworkFsm::EConnStatus stat = network.HandleNetwork();

  CRTCClock::EUType ut = rtcclock.Update();
  
  if (stat == CNetworkFsm::EConnRunning)
    ntpfsm.HandleNTP();

  if (--nDelay == 0)
  {
    nDelay = 500;
    switch (eDisplayMode)
    {
      case Init:
       eDisplayMode = Other;
       display.clear();
       break;
      
      case Other:
       eDisplayMode = Hour;
       ut = CRTCClock::ETime;
       break;
    
      case Hour:
       eDisplayMode = Date;
       ut = CRTCClock::EDate;
       break;
    
      case Date:
       eDisplayMode = Other;
       break;
    }    
  }

  switch (eDisplayMode)
  {
    case Hour:
      if (ut != CRTCClock::ENone)
      {
        char strclock[32];
        display.setColor(BLACK);
        display.fillRect(10, 0, 90, 16);
        display.setColor(WHITE);
        sprintf(strclock, "%02d:%02d:%02d", clockdata.m_hour, clockdata.m_minutes, clockdata.m_seconds);
        display.drawString(10, 0, strclock);    
        display.display();
      }
      break;
    
    case Date:
      {
        char strclock[32];
        display.setColor(BLACK);
        display.fillRect(10, 20, 90, 16);
        display.setColor(WHITE);
        sprintf(strclock, "%02d/%02d/%02d", clockdata.m_day, clockdata.m_month, clockdata.m_year);
        display.drawString(10, 20, strclock);    
        display.display();
      }    
      break;
    
    case Other:
      {
        display.setColor(BLACK);
        display.fillRect(10, 40, 90, 16);
        display.setColor(WHITE);
        String strclock = network.GetStatus();
        display.drawString(10, 40, strclock);    
        display.display();
      }    
      break;
  }
}
