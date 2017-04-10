/* variables */
extern struct station_config stationConf;
extern os_timer_t wifi_connect_network_timeout_timer;

void wifi_handle_event_cb(System_Event_t	*evt);
void wifi_connect_network(struct station_config *stationConf);
void wifi_network_connect_timeout();
