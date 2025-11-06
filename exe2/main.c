#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/timer.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

const int BTN_PIN_G = 28;

const int LED_Y = 5;
const int LED_B = 9;

volatile int flag_g = 0;
volatile int flag_y = 0;
volatile int flag_b = 0;
volatile int alarme = 0;
volatile int parou = 0;

repeating_timer_t timer_y;
repeating_timer_t timer_b;


bool timer_y_callback(repeating_timer_t *rt){
	flag_y = 1;

	return true;
}

bool timer_b_callback(repeating_timer_t *rt){
	flag_b = 1;

	return true;
}

int64_t alarm_callback(alarm_id_t id, void *user_data){
	alarme = 0;
	parou = 1;

	return 0;
}

void btn_callback(uint gpio, uint32_t events) {
	if (events == 0x4){
		flag_g = 1;
	}
}

int main() {
	stdio_init_all();

	gpio_init(BTN_PIN_G);
	gpio_set_dir(BTN_PIN_G, GPIO_IN);
	gpio_pull_up(BTN_PIN_G);

	gpio_init(LED_B);
	gpio_set_dir(LED_B, GPIO_OUT);

	gpio_init(LED_Y);
	gpio_set_dir(LED_Y, GPIO_OUT);

	gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &btn_callback);
	
	int estado_y = 0;
	int estado_b = 0;

	while (true) {
		if (flag_g){
			add_repeating_timer_ms(500, timer_y_callback, NULL, &timer_y);
			add_repeating_timer_ms(150, timer_b_callback, NULL, &timer_b);
			add_alarm_in_ms(5000, alarm_callback, NULL, false);
			flag_g = 0;
			alarme = 1;
			}
			
		if (alarme){
			if (flag_y){
				estado_y = !estado_y;
				gpio_put(LED_Y, estado_y);
				flag_y = 0;
			}
			if (flag_b){
				estado_b = !estado_b;
				gpio_put(LED_B, estado_b);
				flag_b = 0;
			}
		}

		if (parou){
			gpio_put(LED_Y, 0);
			gpio_put(LED_B, 0);
			cancel_repeating_timer(&timer_y);
			cancel_repeating_timer(&timer_b);
			parou = 0;
		}


	}
}


