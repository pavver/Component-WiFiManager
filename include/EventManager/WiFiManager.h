#pragma once

/**
 * @brief EventType WiFi
 */
#define EventTypeWiFI (uint8_t)1

// ----------------------------------------------

/** 
 * @brief EventSubtype request event to connect to a WiFi network
 */
#define EventSubtypeWiFI_Connect (uint8_t)1

/**
 * @brief EventSubtype request event to connect to a WiFi network
 */
#define EventSubtypeWiFI_GetAPs (uint8_t)2

/**
 * @brief EventSubtype request event to get WiFi status
 */
#define EventSubtypeWiFI_Status (uint8_t)3

/**
 * @brief EventSubtype request event to set WiFi AP settings
 * 
 */
#define EventSubtypeWiFI_ApSettings (uint8_t)4

/**
 * @brief EventSubtype request event to turn off WiFi AP
 */
#define EventSubtypeWiFI_ApOff (uint8_t)5
