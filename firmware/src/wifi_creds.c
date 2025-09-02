#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>

LOG_MODULE_REGISTER(wifi_creds, LOG_LEVEL_INF);

#define WIFI_SSID "ssid"
#define WIFI_PSK "psk"

#define WIFI_CONN_RETRY_SECS 5

static void wifi_conn_handler(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(wifi_conn_dwork, wifi_conn_handler);

static void wifi_conn_handler(struct k_work *work)
{
	struct net_if *iface = net_if_get_first_wifi();
	struct wifi_iface_status status = {0};
	static struct wifi_connect_req_params conn_params = {
		.ssid = WIFI_SSID,
		.ssid_length = sizeof(WIFI_SSID) - 1,
#ifdef WIFI_PSK
		.psk = WIFI_PSK,
		.psk_length = sizeof(WIFI_PSK) - 1,
		.security = WIFI_SECURITY_TYPE_PSK,
#else
		.security = WIFI_SECURITY_TYPE_NONE,
#endif
	};
	int ret;

	ret = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status,
		       sizeof(struct wifi_iface_status));
	if (ret != 0) {
		LOG_WRN("Status request failed: %d", ret);
		goto out;
	}

	if (status.state == WIFI_STATE_DISCONNECTED ||
	    status.state == WIFI_STATE_INACTIVE) {
		LOG_INF("Trying to connect");
		net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &conn_params, sizeof(conn_params));
	} else if (status.state >= WIFI_STATE_ASSOCIATED) {
		goto out;
	} else {
		LOG_INF("WiFi status: %s", wifi_state_txt(status.state));
	}

out:
	k_work_schedule(&wifi_conn_dwork, K_SECONDS(WIFI_CONN_RETRY_SECS));
}

static int wifi_conn(void)
{
	k_work_schedule(&wifi_conn_dwork, K_SECONDS(WIFI_CONN_RETRY_SECS));

	return 0;
}
SYS_INIT(wifi_conn, APPLICATION, 91);
