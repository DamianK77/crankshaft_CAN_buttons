#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "esp_log.h"
#include "esp_timer.h"

//pin definitions for seeed studio s3
#define SEED_D0 GPIO_NUM_1
#define SEED_D1 GPIO_NUM_2
#define SEED_D2 GPIO_NUM_3
#define SEED_D3 GPIO_NUM_4
#define SEED_D4 GPIO_NUM_5
#define SEED_D5 GPIO_NUM_6
#define SEED_D6 GPIO_NUM_43
#define SEED_D7 GPIO_NUM_44
#define SEED_D8 GPIO_NUM_7
#define SEED_D9 GPIO_NUM_8
#define SEED_D10 GPIO_NUM_9

#define TAG "MAIN"
 
 
#define TWAI_TIMING_CONFIG_33_3KBITS()    {.clk_src = TWAI_CLK_SRC_DEFAULT, .quanta_resolution_hz = 400000, .brp = 0, .tseg_1 = 8, .tseg_2 = 3, .sjw = 3, .triple_sampling = false}

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

/**
 * @brief HID report descriptor
 *
 * In this example we implement Keyboard + Mouse HID device,
 * so we must define both report descriptors
 */
const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD) ),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE) )
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and 1 HID interface
 */
static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

/******* Functions *******/

void send_keycode(uint16_t input_keycode) {
    ESP_LOGI(TAG, "Sending Keyboard report");
    uint8_t keycode[6] = {input_keycode};
    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, keycode);
    vTaskDelay(pdMS_TO_TICKS(50));
    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, NULL);

}


void app_main()
{
    //Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(SEED_D10, SEED_D9, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_33_3KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        //printf("Driver installed\n");
    } else {
        //printf("Failed to install driver\n");
        return;
    }

    /***** Config USB *****/
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");


    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }

    // while (1) {
    //     send_keycode(HID_KEY_A);
    //     vTaskDelay(1000/portTICK_PERIOD_MS);
    // }


    while (1) {
        //Wait for message to be received
        twai_message_t message;
        twai_clear_receive_queue();
        twai_receive(&message, pdMS_TO_TICKS(10000));
        bool pressed = false;

        if (message.identifier == 0x175) { //message is from steering wheel
            if (message.data[5] == 0x5 && pressed == false) { //button down, prev song
                send_keycode(HID_KEY_V);
                vTaskDelay(800/portTICK_PERIOD_MS);
                pressed = true;
            }
            if (message.data[5] == 0x4 && pressed == false) { //button up, next song
                send_keycode(HID_KEY_N);
                vTaskDelay(800/portTICK_PERIOD_MS);
                pressed = true;
            }
            if (message.data[5] == 0x0) { //clear, no buttons pressed
                pressed = false;
            }
        }
    }
}