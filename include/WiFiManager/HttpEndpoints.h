#pragma once

#include "EventManager.h"
#include "EventManager/WiFiManager.h"
#include "HttpEvents.h"
#include "cJSON.h"
#include "esp_err.h"

// ╔════════════════════════════════════════════╗
// ║                    WiFi                    ║
// ╚════════════════════════════════════════════╝

/// @brief Список WiFi мереж
static esp_err_t post_api_aps_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_GetAPs, 0, true);
}

/// @brief Підєднатись до WiFi мережі
static esp_err_t post_api_connect_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_Connect, 128, true);
}

/// @brief Змінити налаштування WiFi точки доступу
static esp_err_t post_api_apsettings_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_ApSettings, 128, true);
}

/// @brief Вимкнути WiFi точку доступу
static esp_err_t post_api_apoff_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_ApOff);
}

/// @brief Поточний режим роботи
static esp_err_t post_api_status_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_Status, 0, true);
}

// ╔════════════════════════════════════════════╗
// ║                    HTTP                    ║
// ╚════════════════════════════════════════════╝

static esp_err_t post_wifi_handler(httpd_req_t *req)
{
  httpd_resp_set_hdr(req, AccessControlAllowOrigin, "*");
  httpd_resp_set_type(req, "application/json");

  esp_err_t err = ESP_FAIL;

  API_METHOD("status", &post_api_status_handler);
  API_METHOD("aps", &post_api_aps_handler);
  API_METHOD("connect", &post_api_connect_handler);
  API_METHOD("apsettings", &post_api_apsettings_handler);
  API_METHOD("apoff", &post_api_apoff_handler);

  httpd_resp_send_404(req);

  return ESP_ERR_NOT_SUPPORTED;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

const static char *apiwifi = "/api/wifi/*";

static const httpd_uri_t http_server_post_wifi_request = {
    .uri = apiwifi,
    .method = HTTP_POST,
    .handler = post_wifi_handler};

static const httpd_uri_t http_server_options_wifi_request = {
    .uri = apiwifi,
    .method = HTTP_OPTIONS,
    .handler = options_handler};

#pragma GCC diagnostic pop

static esp_err_t register_wifi_command_handler(httpd_handle_t handle)
{
  esp_err_t err = httpd_register_uri_handler(handle, &http_server_post_wifi_request);
  if (err != ESP_OK)
    return err;
  err = httpd_register_uri_handler(handle, &http_server_options_wifi_request);
  return err;
}