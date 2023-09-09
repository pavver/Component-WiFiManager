idf_component_register(SRCS 
                            "WiFiConfig.cpp"
                            "WiFiManager.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "esp_wifi EventManager BaseTask HttpServer Nvs json Core"
                    )
