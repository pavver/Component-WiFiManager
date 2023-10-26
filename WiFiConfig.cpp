
#include "WiFiConfig.h"
#include "esp_log.h"
#include "string.h"
#include "Core.h"

const static char *ApConfiguredKey = "apConf";
const static char *StaConfiguredKey = "staConf";
const static char *STA_ssidKey = "stassid";
const static char *STA_passKey = "stapass";
const static char *AP_ssidKey = "apssid";
const static char *AP_passKey = "appass";

char *WiFiConfig_t::Get_STA_ssid()
{
  return NVS->getCharArray(STA_ssidKey, "");
}
void WiFiConfig_t::Set_STA_ssid(const char *ssid)
{
  NVS->setCharArray(STA_ssidKey, ssid);
}

char *WiFiConfig_t::Get_STA_pass()
{
  return NVS->getCharArray(STA_passKey, nullptr);
}
void WiFiConfig_t::Set_STA_pass(const char *pass)
{
  NVS->setCharArray(STA_passKey, pass);
}

char *WiFiConfig_t::Get_AP_ssid()
{
  return NVS->getCharArray(AP_ssidKey, "esp");
}
void WiFiConfig_t::Set_AP_ssid(const char *ssid)
{
  if (ssid == nullptr)
    NVS->erase(AP_ssidKey);
  else
    NVS->setCharArray(AP_ssidKey, ssid);
}

char *WiFiConfig_t::Get_AP_pass()
{
  return NVS->getCharArray(AP_passKey, "password");
}
void WiFiConfig_t::Set_AP_pass(const char *pass)
{
  NVS->setCharArray(AP_passKey, pass);
}

char *WiFiConfig_t::Get_AP_hide_ssid()
{
  char *name = GetUniqueDeviceID();
  char *ret = (char *)malloc((sizeof(char) * 32));
  snprintf((char *)ret, 32, "pvLed_%s", name);
  free(name);
  return ret;
}

char *WiFiConfig_t::Get_AP_hide_pass()
{
  const static char *pattern = "%s%s";
  char *deviceName1 = GetUniqueDeviceID(8);
  char *deviceName2 = GetUniqueDeviceID(16);
  int len = snprintf(NULL, 0, pattern, deviceName1, deviceName2) + 1;
  char *ret = (char *)malloc(sizeof(char) * len);
  snprintf(ret, len, pattern, deviceName1, deviceName2);
  free(deviceName1);
  free(deviceName2);
  return ret;
}

bool WiFiConfig_t::Get_AP_Configured()
{
  return NVS->getBoolean(ApConfiguredKey, true);
}
void WiFiConfig_t::Set_AP_Configured(bool configured)
{
  NVS->setBoolean(ApConfiguredKey, configured);
}

bool WiFiConfig_t::Get_STA_Configured()
{
  return NVS->getBoolean(StaConfiguredKey, false);
}
void WiFiConfig_t::Set_STA_Configured(bool configured)
{
  NVS->setBoolean(StaConfiguredKey, configured);
}
