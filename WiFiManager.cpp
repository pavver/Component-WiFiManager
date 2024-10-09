#include "WiFiManager.h"
#include "EventManager/WiFiManager.h"
#include "WiFiConfig.h"
#include "freertos/event_groups.h"
#include "lwip/ip4_addr.h"
#include "Core.h"
#include "cJSON.h"
#include "esp_phy_init.h"
#include "esp_event.h"

const static char *WiFiSsidKey = "ssid";

const static char *wifiTag = "WiFi";
static int s_retry_num;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    esp_wifi_connect();
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (s_retry_num < 5)
    {
      esp_wifi_connect();
      s_retry_num++;
      // ESP_LOGI(wifiTag, "retry to connect to the AP");
    }
    else
      xEventGroupSetBits(s_wifi_event_group, BIT1);
    // ESP_LOGI(wifiTag, "connect to the AP fail");
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    // ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    //  ESP_LOGI(wifiTag, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, BIT0);
  }
}

void WiFiManager::Startup()
{
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifiAP = esp_netif_create_default_wifi_ap();
  esp_netif_ip_info_t ipInfo;
  IP4_ADDR(&ipInfo.ip, 192, 168, 1, 1);
  IP4_ADDR(&ipInfo.gw, 192, 168, 1, 1);
  IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
  esp_netif_dhcps_stop(wifiAP);
  esp_netif_set_ip_info(wifiAP, &ipInfo);
  esp_netif_dhcps_start(wifiAP);

  char *host = WiFiConfig->Get_AP_hide_ssid();

  esp_netif_set_hostname(wifiAP, host);

  wifiSTA = esp_netif_create_default_wifi_sta();
  esp_netif_set_hostname(wifiSTA, host);

  free(host);

  ESP_LOGI(wifiTag, "wifi init start");

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler, NULL, &instance_got_ip));

  _subscriber = eventManager->Subscribe("WiFi", EventTypeWiFI, 0);

  ESP_ERROR_CHECK(reloadApSettings());
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(reloadStaSettings());
}

void WiFiManager::Loop()
{
  EventData *event = _subscriber->Next();
  if (event == nullptr)
  {
    Sleep(100);
    return;
  }

  if (event->isSubtype(EventSubtypeWiFI_ApOff))
  {
    WiFiConfig->Set_AP_ssid(nullptr);
    WiFiConfig->Set_AP_pass(nullptr);
    WiFiConfig->Set_AP_Configured(false);
    reloadApSettings();
    _subscriber->Done();
  }
  httpd_req_t *req = (httpd_req_t *)event->getAdditionalValue();

  if (event->isSubtype(EventSubtypeWiFI_ApSettings))
  {
    cJSON *jsonNada = cJSON_Parse((char *)event->getValue());
    cJSON *tmpSsid = cJSON_GetObjectItem(jsonNada, WiFiSsidKey);
    cJSON *tmpPass = cJSON_GetObjectItem(jsonNada, "pass");
    int status = 0;

    if (cJSON_IsString(tmpSsid) &&
        cJSON_IsString(tmpPass) &&
        strlen(tmpSsid->valuestring) < 32 &&
        strlen(tmpSsid->valuestring) >= 6 &&
        strlen(tmpPass->valuestring) < 64 &&
        strlen(tmpPass->valuestring) >= 8)
    {
      WiFiConfig->Set_AP_ssid(tmpSsid->valuestring);
      WiFiConfig->Set_AP_pass(tmpPass->valuestring);
      WiFiConfig->Set_AP_Configured(true);
      reloadApSettings();
      status = 1;
    }

    cJSON_Delete(jsonNada);

    httpd_resp_sendstr_chunk(req, "{\"status\":");
    int len = snprintf(NULL, 0, "%d", status) + 1;
    char *status_buffer = (char *)malloc(sizeof(char) * len);
    snprintf(status_buffer, len, "%d", status);
    httpd_resp_sendstr_chunk(req, status_buffer);
    free(status_buffer);
    httpd_resp_sendstr_chunk(req, "}");

    _subscriber->Done();
  }

  else if (event->isSubtype(EventSubtypeWiFI_Status))
  {
    esp_netif_ip_info_t ip_info;
    httpd_resp_sendstr_chunk(req, "{\"sta\":{");

    char *name = WiFiConfig->Get_AP_hide_ssid();
    httpd_resp_sendstr_chunk(req, "\"name\":\"");
    httpd_resp_sendstr_chunk(req, name);
    httpd_resp_sendstr_chunk(req, "\",");
    free(name);

    if (WiFiConfig->Get_STA_Configured())
    {
      esp_netif_get_ip_info(wifiSTA, &ip_info);
      char staip[16];
      snprintf(staip, sizeof(staip), IPSTR, IP2STR(&ip_info.ip));
      httpd_resp_sendstr_chunk(req, "\"ip\":\"");
      httpd_resp_sendstr_chunk(req, staip);
      httpd_resp_sendstr_chunk(req, "\",");

      char *stassid = WiFiConfig->Get_STA_ssid();
      httpd_resp_sendstr_chunk(req, "\"ssid\":\"");
      httpd_resp_sendstr_chunk(req, stassid);
      httpd_resp_sendstr_chunk(req, "\",");
      free(stassid);

      wifi_ap_record_t wifidata;
      int8_t wifi_status = (esp_wifi_sta_get_ap_info(&wifidata) == ESP_OK) ? 1 : 0;
      int len = snprintf(NULL, 0, "%d", wifi_status) + 1;
      char *status_buffer = (char *)malloc(sizeof(char) * len);
      snprintf(status_buffer, len, "%d", wifi_status);
      httpd_resp_sendstr_chunk(req, "\"status\":");
      httpd_resp_sendstr_chunk(req, status_buffer);
      free(status_buffer);

      if (wifi_status == 1)
      {
        len = snprintf(NULL, 0, "%d", wifidata.rssi) + 1;
        char *rssi_buffer = (char *)malloc(sizeof(char) * len);
        snprintf(rssi_buffer, len, "%d", wifidata.rssi);
        httpd_resp_sendstr_chunk(req, ",\"rssi\":");
        httpd_resp_sendstr_chunk(req, rssi_buffer);
        free(rssi_buffer);
      }
    }
    else
    {
      httpd_resp_sendstr_chunk(req, "\"status\":-1");
    }

    httpd_resp_sendstr_chunk(req, "},\"ap\":{");

    char *apssid = WiFiConfig->Get_AP_ssid();
    httpd_resp_sendstr_chunk(req, "\"ssid\":\"");
    httpd_resp_sendstr_chunk(req, apssid);
    httpd_resp_sendstr_chunk(req, "\",");
    free(apssid);

    char *appass = WiFiConfig->Get_AP_pass();
    httpd_resp_sendstr_chunk(req, "\"pass\":\"");
    httpd_resp_sendstr_chunk(req, appass);
    httpd_resp_sendstr_chunk(req, "\",");
    free(appass);

    int8_t ap_status = WiFiConfig->Get_AP_Configured() ? 1 : 0;
    int len = snprintf(NULL, 0, "%d", ap_status) + 1;
    char *ap_status_buffer = (char *)malloc(sizeof(char) * len);
    snprintf(ap_status_buffer, len, "%d", ap_status);
    httpd_resp_sendstr_chunk(req, "\"status\":");
    httpd_resp_sendstr_chunk(req, ap_status_buffer);
    free(ap_status_buffer);

    esp_netif_get_ip_info(wifiAP, &ip_info);
    char apip[16];
    snprintf(apip, sizeof(apip), IPSTR, IP2STR(&ip_info.ip));
    httpd_resp_sendstr_chunk(req, ",\"ip\":\"");
    httpd_resp_sendstr_chunk(req, apip);
    httpd_resp_sendstr_chunk(req, "\"}}");

    _subscriber->Done();
  }

  else if (event->isSubtype(EventSubtypeWiFI_GetAPs))
  {
    uint16_t ap_count;
    wifi_ap_record_t *ap_info = wifi_scan(ap_count);

    httpd_resp_sendstr_chunk(req, "[");
    for (int i = 0; i < ap_count; i++)
    {
      if (i > 0)
        httpd_resp_sendstr_chunk(req, ",");

      httpd_resp_sendstr_chunk(req, "{\"ssid\":\"");
      httpd_resp_sendstr_chunk(req, (char *)ap_info[i].ssid);
      httpd_resp_sendstr_chunk(req, "\",\"rssi\":");

      int len = snprintf(NULL, 0, "%d", ap_info[i].rssi) + 1;
      char *rssi_buffer = (char *)malloc(sizeof(char) * len);
      snprintf(rssi_buffer, len, "%d", ap_info[i].rssi);
      httpd_resp_sendstr_chunk(req, rssi_buffer);
      free(rssi_buffer);

      httpd_resp_sendstr_chunk(req, ",\"authmode\":");
      len = snprintf(NULL, 0, "%d", ap_info[i].authmode) + 1;
      char *authmode_buffer = (char *)malloc(sizeof(char) * len);
      snprintf(authmode_buffer, len, "%d", ap_info[i].authmode);
      httpd_resp_sendstr_chunk(req, authmode_buffer);
      free(authmode_buffer);

      httpd_resp_sendstr_chunk(req, "}");
    }
    httpd_resp_sendstr_chunk(req, "]");

    free(ap_info);
    _subscriber->Done();
  }

  else if (event->isSubtype(EventSubtypeWiFI_Connect))
  {
    cJSON *jsonNada = cJSON_Parse((char *)event->getValue());
    cJSON *tmpSsid = cJSON_GetObjectItem(jsonNada, WiFiSsidKey);
    cJSON *tmpPass = cJSON_GetObjectItem(jsonNada, "pass");

    if (cJSON_IsString(tmpSsid) &&
        cJSON_IsString(tmpPass) &&
        ConnectTo(tmpSsid->valuestring, tmpPass->valuestring) == ESP_OK)
    {
      WiFiConfig->Set_STA_ssid(tmpSsid->valuestring);
      WiFiConfig->Set_STA_pass(tmpPass->valuestring);
      WiFiConfig->Set_STA_Configured(true);

      esp_netif_ip_info_t ip_info;
      esp_netif_get_ip_info(wifiSTA, &ip_info);
      char staip[16];
      snprintf(staip, sizeof(staip), IPSTR, IP2STR(&ip_info.ip));

      httpd_resp_sendstr_chunk(req, "{\"status\":1,\"ip\":\"");
      httpd_resp_sendstr_chunk(req, staip);
      httpd_resp_sendstr_chunk(req, "\"}");
    }
    else
    {
      WiFiConfig->Set_STA_Configured(false);
      httpd_resp_sendstr_chunk(req, "{\"status\":0}");
    }

    cJSON_Delete(jsonNada);
    _subscriber->Done();
  }
}

esp_err_t WiFiManager::reloadApSettings()
{
  wifi_config_t config = {};
  if (WiFiConfig->Get_AP_Configured())
  {
    char *ssid = WiFiConfig->Get_AP_ssid();
    char *pass = WiFiConfig->Get_AP_pass();
    ESP_LOGI(wifiTag, "AP ssid: %s, pass: %s", ssid, pass);
    config.ap.ssid_len = snprintf((char *)config.ap.ssid, 32, "%s", ssid);
    snprintf((char *)config.ap.password, 64, "%s", pass);
    free(pass);
    free(ssid);
  }
  else
  {
    char *ssid = WiFiConfig->Get_AP_hide_ssid();
    config.ap.ssid_len = snprintf((char *)config.ap.ssid, 32, "%s", ssid);
    config.ap.ssid_hidden = 1;
    char *pass = WiFiConfig->Get_AP_hide_pass();
    ESP_LOGI(wifiTag, "hide AP ssid: %s, pass: %s", config.ap.ssid, pass);
    snprintf((char *)config.ap.password, 64, "%s", pass);
    free(pass);
    free(ssid);
  }
  config.ap.authmode = WIFI_AUTH_WPA2_PSK;
  config.ap.max_connection = 10;
  return esp_wifi_set_config(WIFI_IF_AP, &config);
}

esp_err_t WiFiManager::reloadStaSettings()
{
  esp_err_t ret = ESP_FAIL;

  if (WiFiConfig->Get_STA_Configured())
  {
    char *ssid = WiFiConfig->Get_STA_ssid();
    char *password = WiFiConfig->Get_STA_pass();

    ret = ConnectTo(ssid, password);

    free(ssid);
    free(password);

    if (!WiFiConfig->Get_AP_Configured())
    {
      WiFiConfig->Set_AP_Configured(true);
      reloadApSettings();
    }
  }
  else
  {
    ret = ESP_OK; // no need to connect
    // esp_wifi_set_mode(WIFI_MODE_AP);
  }

  http->Stop();
  http->Start();
  return ret;
}

wifi_ap_record_t *WiFiManager::wifi_scan(uint16_t &ap_count)
{
  ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
  wifi_ap_record_t *ap_info = (wifi_ap_record_t *)calloc(sizeof(wifi_ap_record_t), ap_count);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));

  return ap_info;
}

esp_err_t WiFiManager::ConnectTo(char *ssid, char *password)
{
  ESP_LOGI(wifiTag, "ConnectTo ssid: %s, pass: %s", ssid, password);
  uint16_t ap_count;
  wifi_ap_record_t *ap_info = wifi_scan(ap_count);

  for (int i = 0; i < ap_count; i++)
  {
    if (cmpstr((char *)ssid, (char *)ap_info[i].ssid))
    {
      esp_err_t ret = ConnectTo(ssid, password, ap_info[i].authmode);
      free(ap_info);
      return ret;
    }
  }

  free(ap_info);

  return ESP_FAIL;
}

esp_err_t WiFiManager::ConnectTo(char *ssid, char *password, wifi_auth_mode_t authmode)
{
  esp_wifi_disconnect();

  s_retry_num = 0;

  wifi_config_t sta_config = {};

  sta_config.sta.threshold.authmode = authmode;
  strcpy((char *)sta_config.sta.ssid, ssid);
  if (authmode != WIFI_AUTH_OPEN)
    strcpy((char *)sta_config.sta.password, password);
  sta_config.sta.channel = 0;

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
  ESP_ERROR_CHECK(esp_wifi_connect());

  /* Waiting until either the connection is established (BIT0) or connection failed for the maximum
   * number of re-tries (BIT1). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         BIT0 | BIT1,
                                         pdFALSE,
                                         pdFALSE,
                                         portMAX_DELAY);

  esp_err_t ret = ESP_OK;

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
   * happened. */
  if (bits & BIT0)
  {
    ESP_LOGI(wifiTag, "connected to ap SSID:%s password:%s", ssid, password);
  }
  else if (bits & BIT1)
  {
    ESP_LOGI(wifiTag, "Failed to connect to SSID:%s, password:%s", ssid, password);
    ret = ESP_FAIL;
  }
  else
  {
    ESP_LOGE(wifiTag, "UNEXPECTED EVENT");
  }

  return ret;
}
