#include <stdio.h>
#include "nvs_flash.h" 
#include "esp_netif.h"           
#include "esp_event.h"           
#include "esp_log.h"            
#include "esp_wifi.h"   

#include "project_config.h"
#include "sntp_config.h"
#include "spiffs_config.h"
#include "stepper_motor_config.h"
#include "server_config.h"
#include "wifi_config.h"
#include "nvs_config.h"
#include "scheduler_config.h"


static httpd_handle_t server = NULL;

void app_main() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NOT_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  stepper_motor_config();
  scheduleSemaphore = xSemaphoreCreateMutex();
  init_wifi();
  init_nvs();
  init_spiffs();
  init_start_values();

  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

  server = start_server();

  sntp_initialize();

  scheduleTimer = xTimerCreate("ScheduleTimer", pdMS_TO_TICKS(60000), pdTRUE, (void *)0, schedule_timer_callback);
  if (scheduleTimer == NULL) {
      ESP_LOGE("app_main", "Failed to create schedule timer!");
  } else {
      if (xTimerStart(scheduleTimer, 0) != pdPASS) {
          ESP_LOGE("app_main", "Failed to start schedule timer!");
      } else {
          ESP_LOGI("app_main", "SCHEDULE TIMER STARTED SUCCESSFULLY");
      }
  }
}