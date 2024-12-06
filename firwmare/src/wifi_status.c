#include <zephyr/drivers/led.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/wifi_mgmt.h>

#include "leds.h"

LOG_MODULE_REGISTER(wifi_status, LOG_LEVEL_INF);

static struct net_mgmt_event_callback mgmt_cb;

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
				    uint64_t mgmt_event, struct net_if *iface)
{
	const struct wifi_status *status = (const struct wifi_status *)cb->info;

	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
		LOG_DBG("wifi on %d", status->status);
		led_event(EV_CONNECTED);
		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		LOG_DBG("wifi off %d", status->status);
		led_event(EV_DISCONNECTED);
		break;
	default:
		break;
	}
}

static int wifi_status(void)
{
	net_mgmt_init_event_callback(&mgmt_cb,
				     wifi_mgmt_event_handler,
				     NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);

	net_mgmt_add_event_callback(&mgmt_cb);

	return 0;
}
SYS_INIT(wifi_status, APPLICATION, 0);
