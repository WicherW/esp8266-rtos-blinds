set(COMPONENT_SRCS
    "sntp_config.c"
    "scheduler_config.c"
    "http_post_handlers.c"
    "server_config.c"
    "spiffs_config.c"
    "wifi_config.c"
    "http_get_handlers.c"
    "stepper_motor_config.c"
    "nvs_config.c"
    "project_config.c"
    "main.c"
)

set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
