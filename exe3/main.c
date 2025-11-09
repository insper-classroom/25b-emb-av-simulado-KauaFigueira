/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

const int LED_PIN_B = 8;
const int LED_PIN_Y = 13;

typedef struct input {
    int num_led1;
    int num_led2;
} input_t;

QueueHandle_t xQueueInput;
QueueHandle_t xQueueLed1;
QueueHandle_t xQueueLed2;

SemaphoreHandle_t xSemaphoreLed2;
/**
 * NÃO MEXER!
 */
void input_task(void* p) {
    input_t test_case;

    test_case.num_led1 = 3;
    test_case.num_led2 = 4;
    xQueueSend(xQueueInput, &test_case, 0);

    test_case.num_led1 = 3;
    test_case.num_led2 = 2;
    xQueueSend(xQueueInput, &test_case, 0);

    while (true) {
    }
}

void main_task(void* p){
    input_t data;

    while (1){
        if (xQueueReceive(xQueueInput, &data, pdMS_TO_TICKS(500))) {
            xQueueSend(xQueueLed1, &(data.num_led1), 0);
            xQueueSend(xQueueLed2, &(data.num_led2), 0);
        }   
    }
}

void led_1_task(void* p){
    int data;
    int recebeu = 0;
    int cont = 0;

    gpio_init(LED_PIN_B);
	gpio_set_dir(LED_PIN_B, GPIO_OUT);

    while (1){
        if (!recebeu && xQueueReceive(xQueueLed1, &data, pdMS_TO_TICKS(500))) {
            printf("rodou\n");
            recebeu = 1;
        }
        

        if (recebeu && cont < data) {
            gpio_put(LED_PIN_B, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_put(LED_PIN_B, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
            cont ++;
        } else if (cont >= data){
            cont = 0;
            recebeu = 0;
            xSemaphoreGive(xSemaphoreLed2);
        }
        
    }
}

void led_2_task(void* p){
    int data;
    int recebeu = 0;
    int cont = 0;

    gpio_init(LED_PIN_Y);
	gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    while (1){
        if (xSemaphoreTake(xSemaphoreLed2, pdMS_TO_TICKS(500)) == pdTRUE && !recebeu && xQueueReceive(xQueueLed2, &data, pdMS_TO_TICKS(500))) {
            printf("rodou\n");
            recebeu = 1;
        }
        
        if (recebeu && cont < data) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
            cont ++;
        } else if (cont >= data){
            cont = 0;
            recebeu = 0;
        }
        
    }
}





int main() {
    stdio_init_all();

    /**
     * manter essas duas linhas!
     */

    xSemaphoreLed2 = xSemaphoreCreateBinary();

    xQueueInput = xQueueCreate(32, sizeof(input_t));
    xQueueLed1 = xQueueCreate(32, sizeof(int));
    xQueueLed2 = xQueueCreate(32, sizeof(int));
    
    xTaskCreate(input_task, "Input", 256, NULL, 1, NULL);
    xTaskCreate(main_task, "Main", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "Led_1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "Led_2", 256, NULL, 1, NULL);

    /**
     * Seu código vem aqui!
     */

    vTaskStartScheduler();

    while (1) {}

    return 0;
}
