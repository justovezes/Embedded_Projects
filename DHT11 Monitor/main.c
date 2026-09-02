#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "dht.h"

#define SDA_GPIO 21
#define SCL_GPIO 22
#define DHT_GPIO 23

const char *DHT_TAG = "DHT11";
const char *LCD_TAG = "SSD1306";

void app_main(void)
{
    SSD1306_t lcd;
    short temp, umidity;
    char temp_str[17], umid_str[17];

    // screen setup 
    i2c_master_init(&lcd, SDA_GPIO, SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&lcd, 128, 64); 
    ssd1306_clear_screen(&lcd, false);

    while (true) {

        ssd1306_display_text(&lcd, 0, "DHT11 - READINGS", 16, false);
        if (dht_read_data(DHT_TYPE_DHT11, DHT_GPIO, &umidity, &temp) != ESP_OK)
            ssd1306_display_text(&lcd, 2, "READ FAIL!!", 11, false);
        else {
            snprintf(temp_str, 17, "Temp:%10dC", temp / 10);
            snprintf(umid_str, 17, "Umidity:%7d%%", umidity / 10);
            ssd1306_display_text(&lcd, 2, temp_str, 16, false);
            ssd1306_display_text(&lcd, 3, umid_str, 16, false);
        }
        
        for (int i = 0; i < 30; i++) {
            _ssd1306_line(&lcd, 0, 63, (123 / 30) * (i + 1) + 3, 63, false);
            ssd1306_show_buffer(&lcd);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        ssd1306_clear_screen(&lcd, false);
    }
}
