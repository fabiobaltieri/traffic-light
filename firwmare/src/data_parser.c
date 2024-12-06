#include <zephyr/data/json.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "leds.h"

LOG_MODULE_REGISTER(data_parser, LOG_LEVEL_INF);

struct run {
	const char *name;
	const char *status;
};

struct json_obj_descr run_status_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct run, name, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct run, status, JSON_TOK_STRING),
};

struct runs {
	struct run runs[10];
	size_t run_len;
};

struct json_obj_descr runs_descr[] = {
	JSON_OBJ_DESCR_OBJ_ARRAY(struct runs, runs, 10,
				 run_len,
				 run_status_descr, ARRAY_SIZE(run_status_descr)),
};

#define STALE_TIMEOUT_S (60 * 5)

static void stale_worker(struct k_work *work) {
	led_event(EV_UNKNOWN);
}

static K_WORK_DELAYABLE_DEFINE(stale_work, stale_worker);

static void run_status_process(struct runs *runs)
{
	int failed = 0;
	int pass = 0;
	int in_progress = 0;

	LOG_INF("run_len=%d", runs->run_len);

	for (uint8_t i = 0; i < runs->run_len; i++) {
		struct run *run = &runs->runs[i];
		LOG_INF("run name=%s status=%s", run->name, run->status);

		if (strcmp(run->status, "fail") == 0 ||
		    strcmp(run->status, "cancelled") == 0) {
			failed++;
		} else if (strcmp(run->status, "running") == 0) {
			in_progress++;
		} else if (strcmp(run->status, "pass") == 0) {
			pass++;
		} else {
			LOG_WRN("invalid status: %s", run->status);
		}
	}

	LOG_INF("failed=%d pass=%d in_progress=%d", failed, pass, in_progress);
	if (failed > 0) {
		led_event(EV_FAIL);
	} else if (in_progress > 0) {
		led_event(EV_RUNNING);
	} else {
		led_event(EV_PASS);
	}

	k_work_reschedule(&stale_work, K_SECONDS(STALE_TIMEOUT_S));
}

int ci_json_process(uint8_t *data, int data_len)
{
	int ret;
	struct runs runs;

	char *data_start = strstr(data, "\r\n\r\n");
	if (data_start == NULL) {
		LOG_ERR("cannot find the response data");
		return -EINVAL;
	}
	data_start += 4;

	LOG_INF("data_start=%p", (void *)data_start);

	int payload_len = strlen(data_start);

	LOG_INF("payload_len=%d", payload_len);

	ret = json_obj_parse(data_start, payload_len, runs_descr, ARRAY_SIZE(runs_descr), &runs);
	if (ret != 1) {
		LOG_ERR("json_obj_parse error: %d", ret);
		return ret;
	}

	run_status_process(&runs);

	return 0;
}
