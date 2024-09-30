idf_component_register(SRCS 
                            "WiFiConfig.cpp"
                            "WiFiManager.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_wifi"
                    PRIV_REQUIRES "EventManager BaseTask HttpServer Nvs json Core nvs_flash"
                    )
