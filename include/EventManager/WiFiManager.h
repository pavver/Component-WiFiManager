#pragma once

// ╔════════════════════════════════════════════╗
// ║                    WiFi                    ║
// ╚════════════════════════════════════════════╝

/// @brief EventType WiFi
#define EventTypeWiFI (uint8_t)1

// ----------------------------------------------

/// @brief EventSubtype Команда підєднатись до WiFi
#define EventSubtypeTypeWiFI_Connect (uint8_t)1

/// @brief EventSubtype Команда дізнатись список доступних WiFi мереж
#define EventSubtypeTypeWiFI_GetAPs (uint8_t)2

/// @brief EventSubtype Команда дізнатись статус роботи WiFi
#define EventSubtypeTypeWiFI_Status (uint8_t)3

/// @brief EventSubtype Команда змінити налаштування точки доступу WiFi
#define EventSubtypeTypeWiFI_ApSettings (uint8_t)4

/// @brief EventSubtype Команда вимкнути точку доступу WiFi
#define EventSubtypeTypeWiFI_ApOff (uint8_t)5