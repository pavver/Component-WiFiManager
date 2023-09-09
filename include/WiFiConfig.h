#pragma once

#include <stdio.h>

class WiFiConfig_t
{
public:
  char *GetDeviceName();
  void SetDeviceName(char *deviceName);

  char *Get_STA_ssid();
  void Set_STA_ssid(const char *ssid);
  char *Get_STA_pass();
  void Set_STA_pass(const char *pass);

  char *Get_AP_ssid();
  void Set_AP_ssid(const char *ssid);
  char *Get_AP_pass();
  void Set_AP_pass(const char *pass);
  char *Get_AP_hide_ssid();
  char *Get_AP_hide_pass();

  bool Get_AP_Configured();
  void Set_AP_Configured(bool configured);
  bool Get_STA_Configured();
  void Set_STA_Configured(bool configured);
};

extern WiFiConfig_t WiFiConfig;
