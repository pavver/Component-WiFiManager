#pragma once

#include "BaseTask.h"
#include "EventManager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "httpServer.h"
#include "Nvs.h"

class WiFiManager : public BaseTask
{
public:
  WiFiManager() : BaseTask("WiFiManager")
  {
  }
  WiFiManager(HttpServer *_http) : WiFiManager()
  {
    http = _http;
  }

  ~WiFiManager()
  {
    eventManager->UnSubscribe(_subscriber);
  }

  virtual void Startup() override;

  virtual void Loop() override;

  wifi_ap_record_t *wifi_scan(uint16_t &ap_count);

  /// @brief Reload/Apply AP configuration after change
  esp_err_t reloadApSettings();

  /// @brief Reload/Apply STA configuration after change
  esp_err_t reloadStaSettings();

  /// @brief try connecting to the AP without saving
  esp_err_t ConnectTo(char *ssid, char *password);

  /// @brief try connecting to the AP without saving
  esp_err_t ConnectTo(char *ssid, char *password, wifi_auth_mode_t authmode);

private:
  // EventManager subscriber
  Subscriber *_subscriber = nullptr;

  esp_netif_t *wifiAP;
  esp_netif_t *wifiSTA;

  HttpServer *http = nullptr;
};
