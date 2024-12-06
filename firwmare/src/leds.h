enum event {
	EV_BOOT,
	EV_CONNECTED,
	EV_DISCONNECTED,
	EV_RUNNING,
	EV_PASS,
	EV_FAIL,
	EV_UNKNOWN,
};

void led_event(enum event);
