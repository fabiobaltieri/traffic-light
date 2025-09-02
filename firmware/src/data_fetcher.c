#include <zephyr/logging/log.h>
#include <zephyr/net/http/client.h>
#include <zephyr/net/socket.h>

LOG_MODULE_REGISTER(data_fetcher, LOG_LEVEL_INF);

#define HTTP_HOST "merge-list.zephyrproject.io"
#define HTTP_PORT "443"
#define HTTP_PATH "/ci.json"

static uint8_t recv_buf[256];
static uint8_t *recv_data;
static int recv_data_size;
static int recv_data_len;

static int response_cb(struct http_response *rsp,
		       enum http_final_call final_data,
		       void *user_data)
{
	LOG_INF("response: final_data=%d data_len=%d status=%s",
		final_data,
		rsp->data_len,
		rsp->http_status);

	if (recv_data_len + rsp->data_len < recv_data_size - 1) {
		memcpy(&recv_data[recv_data_len], recv_buf, rsp->data_len);
		recv_data_len += rsp->data_len;
	}

	recv_data[recv_data_len] = '\0';

	return 0;
}

int fetch_data(uint8_t *data, int data_size, int *data_len)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int sock;
	int ret;

	recv_data = data;
	recv_data_size = data_size;
	recv_data_len = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	ret = getaddrinfo(HTTP_HOST, HTTP_PORT, &hints, &res);
	if (ret != 0) {
		LOG_ERR("unable to resolve address, quitting");
		return ret;
	}

	struct sockaddr_in *in = (struct sockaddr_in *)res->ai_addr;
	LOG_INF("getaddrinfo %s: %d.%d.%d.%d", HTTP_HOST,
		in->sin_addr.s4_addr[0],
		in->sin_addr.s4_addr[1],
		in->sin_addr.s4_addr[2],
		in->sin_addr.s4_addr[3]);

	sock = socket(res->ai_family, res->ai_socktype, IPPROTO_TLS_1_3);
	if (sock < 0) {
		LOG_ERR("socket error: %d", sock);
		return sock;
	}

	int verify = TLS_PEER_VERIFY_NONE;
	ret = setsockopt(sock, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));
	if (ret < 0) {
		LOG_ERR("TLS_PEER_VERIFY failed: %d", ret);
		goto close_and_return;
	}

	LOG_INF("connecting");

	ret = connect(sock, res->ai_addr, res->ai_addrlen);
	if (ret < 0) {
		LOG_ERR("connect failed: %d", ret);
		goto close_and_return;
	}

	LOG_INF("connected");

	struct http_request req;
	memset(&req, 0, sizeof(req));

	req.method = HTTP_GET;
	req.url = HTTP_PATH;
	req.host = HTTP_HOST;
	req.protocol = "HTTP/1.1";
	req.response = response_cb;
	req.recv_buf = recv_buf;
	req.recv_buf_len = sizeof(recv_buf);

	int32_t timeout = 5 * MSEC_PER_SEC;
	ret = http_client_req(sock, &req, timeout, NULL);
	if (ret < 0) {
		LOG_ERR("http_client_req failed: %d", ret);
		goto close_and_return;
	}

	*data_len = recv_data_len;

close_and_return:
	LOG_INF("close");

	close(sock);
	freeaddrinfo(res);

	return ret;
}
