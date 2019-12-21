#pragma once

#include <stdint.h>

#if defined(esp8266)||defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#if defined(esp32)||defined(ESP32)
  #include <WiFi.h>
#endif

#include "NTPtimeESP.h"

/* ------------------------------------------------------------ */

class CClockData
{
public:
  CClockData();

public:
  bool      m_bSynchronized;
  uint16_t  m_year;
  uint8_t   m_month;
  uint8_t   m_day;
  uint8_t   m_hour;
  uint8_t   m_minutes;
  uint8_t   m_seconds;
  uint8_t   m_dow;
};

/* ------------------------------------------------------------ */

class CRTCClock
{
public:
  CRTCClock(CClockData &clockdata);

  enum EUType { ENone, ETime, EDate };

  EUType Update();

private:
  static const int days[];
  CClockData& m_clockdata;
  unsigned long m_lasttime;
};

/* ------------------------------------------------------------ */

class CNTPFsm
{
public:
  CNTPFsm(const char *pszNTPServer, float offset, int timezone, int timeout_ms, int resynch_ms, CClockData &clockdata);

  void HandleNTP();

private:
  float       m_offset;
  int         m_timezone;
  NTPtime     m_ntptime;
  CClockData& m_clockdata;
};

/* ------------------------------------------------------------ */

class CNetworkFsm
{
public:
  CNetworkFsm(const char *pszSSID, const char *pszPassword);

  enum EConnStatus { EConnStarting, EConnConnecting, EConnRunning };

  String GetStatus();

  EConnStatus HandleNetwork();

  void SignalError();

private:
  EConnStatus m_status;
  const char *m_pszSSID;
  const char *m_pszPassword;
  WiFiClass   m_wifi;
  bool        m_bError;
};
