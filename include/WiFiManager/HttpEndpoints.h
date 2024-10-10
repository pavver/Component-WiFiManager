#pragma once

#include "EventManager.h"
#include "EventManager/WiFiManager.h"
#include "HttpEvents.h"
#include "cJSON.h"
#include "esp_err.h"

// ╔════════════════════════════════════════════╗
// ║                    WiFi                    ║
// ╚════════════════════════════════════════════╝

/**
 * @brief Send a request event to get a list of available WiFi networks
 * @param req HTTP request
 * @returns ESP_OK if successful
 */
static esp_err_t post_api_aps_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_GetAPs, 0, true);
}

/**
 * @brief Send a request event to connect to a WiFi network
 * @param req HTTP request
 * @returns ESP_OK if successful
 */
static esp_err_t post_api_connect_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_Connect, 128, true);
}

/**
 * @brief Send a request event to set WiFi AP settings
 * @param req HTTP request
 * @returns ESP_OK if successful
 */
static esp_err_t post_api_apsettings_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_ApSettings, 128, true);
}

/**
 * @brief Send a request event to turn off WiFi AP
 * @param req HTTP request
 * @returns ESP_OK if successful
 */
static esp_err_t post_api_apoff_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_ApOff);
}

/**
 * @brief Send a request event to get WiFi status
 * @param req HTTP request
 * @returns ESP_OK if successful
 */
static esp_err_t post_api_status_handler(httpd_req_t *req)
{
  return post_handler(req, EventTypeWiFI, EventSubtypeWiFI_Status, 0, true);
}

// ╔════════════════════════════════════════════╗
// ║                    HTTP                    ║
// ╚════════════════════════════════════════════╝

/**
 * @brief Processing HTTP requests to work with WiFi
 * @param req HTTP request
 * @returns ESP_OK if successful
 */
static esp_err_t post_wifi_handler(httpd_req_t *req)
{
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
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

static const httpd_uri_t http_server_post_wifi_request = {
    .uri = "/api/wifi/*",
    .method = HTTP_POST,
    .handler = post_wifi_handler};

#pragma GCC diagnostic pop

/**
 * @brief Register HTTP endpoint work with WiFi
 * @param handle HTTP server handle
 * @returns ESP_OK if successful
 */
static esp_err_t register_wifi_command_handler(httpd_handle_t handle)
{
  return httpd_register_uri_handler(handle, &http_server_post_wifi_request);
}