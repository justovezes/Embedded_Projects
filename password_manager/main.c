#include <stdbool.h>
#include "hal/gpio_types.h"
#include <driver/gpio.h>
#include "esp_rom_sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef short button_input_t;
typedef char pswd;

#define BUZZER_PIN (gpio_num_t) 23
#define GREEN_LED_PIN (gpio_num_t) 22
#define YELLOW_LED_PIN (gpio_num_t) 21
#define RED_LED_PIN (gpio_num_t) 19
#define RED_BUTTON_PIN (gpio_num_t) 18
#define BLUE_BUTTON_PIN (gpio_num_t) 33

#define NIL_INPUT (button_input_t) -1
#define RED_PRESSED (button_input_t) 0
#define BLUE_PRESSED (button_input_t) 1

static button_input_t get_button_input(void);
static bool enter_passwd(pswd passwd);
static void reset_passwd(pswd *passwd);
static void denied_blink(void);
static void success_blink(void);
static void wrong_blink(void);
static void beep(gpio_num_t pin, uint32_t duration);

void app_main(void)
{
    gpio_reset_pin(BUZZER_PIN);
    gpio_reset_pin(GREEN_LED_PIN);
    gpio_reset_pin(YELLOW_LED_PIN);
    gpio_reset_pin(RED_LED_PIN);
    gpio_reset_pin(RED_BUTTON_PIN);
    gpio_reset_pin(BLUE_BUTTON_PIN);

    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREEN_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(YELLOW_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RED_LED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_direction(RED_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BLUE_BUTTON_PIN, GPIO_MODE_INPUT);
    
    gpio_pullup_en(RED_BUTTON_PIN);
    gpio_pulldown_dis(RED_BUTTON_PIN);
    gpio_pullup_en(BLUE_BUTTON_PIN);
    gpio_pulldown_dis(BLUE_BUTTON_PIN);

    bool has_passwd = false;
    pswd passwd;

    while (true) {
        {
        if (!get_button_input()) { // RED PRESSED
            reset_passwd(&passwd);
            has_passwd = true;
            continue;
        } 
        if (!has_passwd) { // BLUE PRESSED (NO PASSWD)
            denied_blink();
            continue;
        }
        if (enter_passwd(passwd)) // BLUE PRESSED (HAS PASSWD)
            success_blink(); // SUCCESS
        else
            wrong_blink();   // WRONG
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

button_input_t get_button_input(void)
{
    button_input_t inpt = -1;
    while (inpt == NIL_INPUT) {
        if (!gpio_get_level(RED_BUTTON_PIN)) {
            printf("RED BUTTON PRESSED\n");
            while (!gpio_get_level(RED_BUTTON_PIN))
                vTaskDelay(pdMS_TO_TICKS(10));
            inpt = RED_PRESSED;
        } else if (!gpio_get_level(BLUE_BUTTON_PIN)) {
            printf("BLUE BUTTON PRESSED\n");
            while(!gpio_get_level(BLUE_BUTTON_PIN))
                vTaskDelay(pdMS_TO_TICKS(10));
            inpt = BLUE_PRESSED;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    beep(BUZZER_PIN, 250);
    return inpt;
}

static bool enter_passwd(pswd passwd)
{
    printf("BEGIN PASSWD ENTER:\n");
    gpio_set_level(YELLOW_LED_PIN, 1);
    pswd key = 0;
    for (int i = 0; i < (int) sizeof(pswd) * 8; i++) {
        key <<= 1;
        key += (int) get_button_input();
    }
    gpio_set_level(YELLOW_LED_PIN, 0);
    if (key == passwd)
        printf("SUCCESS!\n");
    else
        printf("PASSWD FAILURE!\n");
    return key == passwd;
}

static void reset_passwd(pswd *passwd)
{
    gpio_set_level(RED_LED_PIN, 1);
    *passwd = 0;
    for (int i = 0; i < (int) sizeof(pswd) * 8; i++) {
        *passwd <<= 1;
        *passwd += (int) get_button_input();
    }
    gpio_set_level(RED_LED_PIN, 0);
}

static void denied_blink(void)
{
    for (int i = 0; i < 3; i++) {
        gpio_set_level(RED_LED_PIN, 1);
        gpio_set_level(YELLOW_LED_PIN, 1);
        beep(BUZZER_PIN, 250);
        gpio_set_level(RED_LED_PIN, 0);
        gpio_set_level(YELLOW_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void success_blink(void)
{
    for (int i = 0; i < 3; i++) {
        gpio_set_level(GREEN_LED_PIN, 1);
        beep(BUZZER_PIN, 250);
        gpio_set_level(GREEN_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
static void wrong_blink(void)
{
    for (int i = 0; i < 3; i++) {
        gpio_set_level(RED_LED_PIN, 1);
        beep(BUZZER_PIN, 250);
        gpio_set_level(RED_LED_PIN, 0);
        gpio_set_level(BUZZER_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void beep(gpio_num_t pin, uint32_t duration)
{
    uint32_t cycles = (duration * 1000) / 500; 
    
    for (uint32_t i = 0; i < cycles; i++) {
        gpio_set_level(pin, 1);
        esp_rom_delay_us(250);
        gpio_set_level(pin, 0);
        esp_rom_delay_us(250);
    }
}
