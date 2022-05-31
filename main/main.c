#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"

#include "driver/gpio.h"
#include "sensors.h"
#include "http.h"
float temperature_degC;


void app_main(void)
{
	ESP_ERROR_CHECK(i2c_master_init());
	init_stts751();

	    esp_err_t ret = nvs_flash_init();
	    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	    {
	      ESP_ERROR_CHECK(nvs_flash_erase());
	      ret = nvs_flash_init();
	    }
	    ESP_ERROR_CHECK(ret);

	    if(init_wifi() == ESP_OK)
	    {
	        if(scan_wifi())
	        {
	            connect_wifi();
	        }
	    }

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;
    xTaskCreate(get_temp_task, "task", 10000, NULL, 1, NULL);
    while (true) {
        gpio_set_level(GPIO_NUM_4, level);
        level = !level;
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

