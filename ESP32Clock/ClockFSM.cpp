
#include "ClockFSM.h"
#include "SSD1306Wire.h"

#include <stdlib.h>

/* ------------------------------------------------------------ */

CClockData::CClockData()
  : m_bSynchronized(false)
  , m_year(2008)
  , m_month(12)
  , m_day(29)
  , m_hour(1)
  , m_minutes(59)
  , m_seconds(0)
  , m_dow(0)
{
}

/* ------------------------------------------------------------ */

const int CRTCClock::days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

CRTCClock::CRTCClock(CClockData &clockdata)
  : m_clockdata(clockdata)
  , m_lasttime(millis())
{
}

CRTCClock::EUType CRTCClock::Update()
{
  unsigned long now = millis();
  unsigned long elapsed = now - m_lasttime;

  if (elapsed < 1000)
    return CRTCClock::ENone;
    
  m_lasttime = now;

  if (++m_clockdata.m_seconds >= 60)
  {
    m_clockdata.m_seconds = 0;
    if (++m_clockdata.m_minutes >= 60)
    {
      m_clockdata.m_minutes = 0;
      if (++m_clockdata.m_hour >= 24)
      {
        m_clockdata.m_hour = 0;
        if (m_clockdata.m_day >= days[m_clockdata.m_month - 1])
        {
          m_clockdata.m_day = 1;
          m_clockdata.m_dow = (m_clockdata.m_dow + 1) % 7;
          if (++m_clockdata.m_month > 12)
          {
            m_clockdata.m_month = 1;
            m_clockdata.m_year++;
          }          
        }

        return CRTCClock::EDate;
      }
    }
  }
  
  return CRTCClock::ETime;
}

/* ------------------------------------------------------------ */

CNTPFsm::CNTPFsm(const char *pszNTPServer, float offset, int timezone, int timeout_ms, int resynch_ms, CClockData &clockdata)
  : m_offset(offset)
  , m_timezone(timezone)
  , m_ntptime(pszNTPServer)
  , m_clockdata(clockdata)
{
  m_ntptime.setSendInterval(resynch_ms);
  m_ntptime.setRecvTimeout(timeout_ms);
}

void CNTPFsm::HandleNTP()
{
    strDateTime dateTime = m_ntptime.getNTPtime(m_offset, m_timezone);
    if (dateTime.valid)
    {
      m_clockdata.m_year = dateTime.year;
      m_clockdata.m_month = dateTime.month;
      m_clockdata.m_day = dateTime.day;
      m_clockdata.m_hour = dateTime.hour; 
      m_clockdata.m_minutes = dateTime.minute;
      m_clockdata.m_seconds = dateTime.second;
      m_clockdata.m_dow= dateTime.dayofWeek;
      m_clockdata.m_bSynchronized = true;
    }
    else
      m_clockdata.m_bSynchronized = false;
}

/* ------------------------------------------------------------ */

CNetworkFsm::CNetworkFsm(const char *pszSSID, const char *pszPassword)
  : m_status(EConnStarting)
  , m_pszSSID(pszSSID)
  , m_pszPassword(pszPassword)
  , m_bError(false)
{
  m_wifi.mode(WIFI_STA);
}

CNetworkFsm::EConnStatus CNetworkFsm::HandleNetwork()
{
  switch (m_status)
  {
    case EConnStarting:
      {
        m_wifi.begin(m_pszSSID, m_pszPassword);
        m_status = EConnConnecting;
      }
      break;

    case EConnConnecting:
      {
        if (m_wifi.status() == WL_CONNECTED)
          m_status = EConnRunning;
      }
      break;
      
    case EConnRunning:
      {
        if (m_bError)
        {
          m_bError = false;
          m_status = EConnStarting;
        }
      }
      break;
  }

  return m_status;
}

void CNetworkFsm::SignalError()
{
  m_bError = true;
}

String CNetworkFsm::GetStatus()
{
  switch (m_status)
  {
    case EConnStarting:
      return "INI";

    case EConnConnecting:
      return "CON";
      
    case EConnRunning:
      return "ACT";
  }
}
