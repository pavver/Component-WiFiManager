#pragma once

// ╔════════════════════════════════════════════╗
// ║                    WiFi                    ║
// ╚════════════════════════════════════════════╝

/// @brief EventType WiFi
#define EventTypeWiFI (uint8_t)1

// ----------------------------------------------

/// @brief EventSubtype Команда підєднатись до WiFi
#define EventSubtypeWiFI_Connect (uint8_t)1

/// @brief EventSubtype Команда дізнатись список доступних WiFi мереж
#define EventSubtypeWiFI_GetAPs (uint8_t)2

/// @brief EventSubtype Команда дізнатись статус роботи WiFi
#define EventSubtypeWiFI_Status (uint8_t)3

/// @brief EventSubtype Команда змінити налаштування точки доступу WiFi
#define EventSubtypeWiFI_ApSettings (uint8_t)4

/// @brief EventSubtype Команда вимкнути точку доступу WiFi
#define EventSubtypeWiFI_ApOff (uint8_t)5