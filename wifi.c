#include "user_interface.h"
#include "osapi.h"
#include "driver/gpio16.h"
#include "scan.h"
#include "payload.h"

struct station_config stationConf;
os_timer_t wifi_connect_network_timeout_timer;

void ICACHE_FLASH_ATTR wifi_network_connect_timeout() {
  os_timer_disarm(&wifi_connect_network_timeout_timer);
  os_printf("wifi timeout occured\n");

  scan_init();
}

/* wifi event callback handler */
void ICACHE_FLASH_ATTR wifi_handle_event_cb(System_Event_t	*evt) {
  switch (evt->event)	{
    case EVENT_STAMODE_GOT_IP:

      /* disarm timer to prevent timeout */
      os_timer_disarm(&wifi_connect_network_timeout_timer);

      /* turn led on */
      gpio16_output_set(0);

      /* start payload */
      os_printf("ip address received\n");
      payload_main();
      break;

    default:
      break;
  }
}

void ICACHE_FLASH_ATTR wifi_connect_network(struct station_config *ptrstationConf) {
  stationConf = *ptrstationConf;

  os_printf("\nattempting network connection\n");

  /* turn led off */
  gpio16_output_set(1);

  /* disconnect */
  wifi_station_disconnect();

  /* configure and connect */
  wifi_station_set_config(&stationConf);
  wifi_station_connect();
}
