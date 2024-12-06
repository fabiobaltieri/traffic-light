#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "leds.h"

LOG_MODULE_REGISTER(leds, LOG_LEVEL_INF);

static const struct led_dt_spec led_red = LED_DT_SPEC_GET(DT_NODELABEL(led_red));
static const struct led_dt_spec led_yellow = LED_DT_SPEC_GET(DT_NODELABEL(led_yellow));
static const struct led_dt_spec led_green = LED_DT_SPEC_GET(DT_NODELABEL(led_green));

#define LED_ON_R 15
#define LED_ON_Y 15
#define LED_ON_G 2

void led_event(enum event event)
{
	switch (event) {
	case EV_BOOT:
		led_off_dt(&led_red);
		led_off_dt(&led_yellow);
		led_off_dt(&led_green);

		k_sleep(K_MSEC(200));

		led_set_brightness_dt(&led_red, LED_ON_R);
		k_sleep(K_MSEC(200));
		led_set_brightness_dt(&led_yellow, LED_ON_Y);
		k_sleep(K_MSEC(200));
		led_set_brightness_dt(&led_green, LED_ON_G);

		k_sleep(K_MSEC(200));

		led_off_dt(&led_red);
		k_sleep(K_MSEC(200));
		led_off_dt(&led_yellow);
		k_sleep(K_MSEC(200));
		led_off_dt(&led_green);
		return;
	case EV_CONNECTED:
		led_off_dt(&led_red);
		led_off_dt(&led_yellow);
		led_off_dt(&led_green);

		k_sleep(K_MSEC(200));

		led_set_brightness_dt(&led_green, LED_ON_G);
		k_sleep(K_MSEC(100));
		led_off_dt(&led_green);
		k_sleep(K_MSEC(200));
		led_set_brightness_dt(&led_green, LED_ON_G);
		k_sleep(K_MSEC(100));
		led_off_dt(&led_green);
		return;
	case EV_DISCONNECTED:
		led_off_dt(&led_red);
		led_off_dt(&led_yellow);
		led_off_dt(&led_green);

		k_sleep(K_MSEC(200));

		led_set_brightness_dt(&led_red, LED_ON_R);
		k_sleep(K_MSEC(500));
		led_off_dt(&led_red);
		return;
	case EV_UNKNOWN:
		led_off_dt(&led_red);
		led_off_dt(&led_yellow);
		led_off_dt(&led_green);

		k_sleep(K_MSEC(200));

		led_set_brightness_dt(&led_yellow, LED_ON_Y);
		k_sleep(K_MSEC(500));
		led_off_dt(&led_yellow);
		return;
	case EV_FAIL:
		led_set_brightness_dt(&led_red, LED_ON_R);
		led_off_dt(&led_yellow);
		led_off_dt(&led_green);
		return;
	case EV_RUNNING:
		led_off_dt(&led_red);
		led_set_brightness_dt(&led_yellow, LED_ON_Y);
		led_off_dt(&led_green);
		return;
	case EV_PASS:
		led_off_dt(&led_red);
		led_off_dt(&led_yellow);
		led_set_brightness_dt(&led_green, LED_ON_G);
		return;
	}
}
