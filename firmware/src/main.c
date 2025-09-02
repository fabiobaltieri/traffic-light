#include <zephyr/drivers/hwinfo.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>

#include "data_fetcher.h"
#include "data_parser.h"
#include "leds.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static uint8_t data_buf[2048];
static int data_len;

void update_hostname(void)
{
	char hostname[32];
	uint8_t dev_id[16];
	ssize_t length;

	length = hwinfo_get_device_id(dev_id, sizeof(dev_id));

	if (length != 6) {
		LOG_ERR("unexpected hwinfo length: %d", length);
		return;
	}

	snprintf(hostname, sizeof(hostname), "%s-%02x%02x%02x",
		 CONFIG_NET_HOSTNAME,
		 dev_id[3], dev_id[4], dev_id[5]);

	LOG_INF("hostname=%s", hostname);

	net_hostname_set(hostname, strlen(hostname));
}

int main(void)
{
	int ret;

	LOG_INF("started");

	update_hostname();

#if CONFIG_NET_DHCPV4
	struct net_if *iface = net_if_get_first_wifi();

	net_dhcpv4_start(iface);
#endif

	led_event(EV_BOOT);

	k_sleep(K_SECONDS(12));

	while (true) {
		memset(data_buf, 0xaa, sizeof(data_buf));

		LOG_INF("fetch data_buf=%p", (void *)data_buf);

		ret = fetch_data(data_buf, sizeof(data_buf), &data_len);
		if (ret < 0) {
			LOG_ERR("fetch_data failed: %d", ret);
			goto out;
		}

		LOG_INF("data_len=%d", data_len);

		ret = ci_json_process(data_buf, data_len);
		if (ret < 0) {
			LOG_ERR("ci_json_process failed: %d", ret);
			goto out;
		}
out:
		k_sleep(K_SECONDS(60));
	}

	k_sleep(K_FOREVER);
}
