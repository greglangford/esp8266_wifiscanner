#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "driver/uart.h"
#include "driver/gpio16.h"
#include "string.h"

LOCAL os_timer_t wifi_timeout_timer;
char pquery[255];
uint8 best_bssid[6];
uint8 best_ssid[33];
sint8 best_rssi;
struct station_config *stationConf;

void ICACHE_FLASH_ATTR user_esp_platform_dns_found(const	char	*name,	ip_addr_t	*ip,	void *arg)	{
  if(ip == NULL) {
    os_printf("nslookup failed\n");
  } else {
    os_printf("DNS ANSWER: %s\tDST: %d.%d.%d.%d\n",
    name,
    *((uint8 *)&ip->addr), *((uint8 *)&ip->addr + 1),
    *((uint8 *)&ip->addr + 2), *((uint8 *)&ip->addr + 3));
  }
  user_scan();
}


/* wifi event callback handler */
void ICACHE_FLASH_ATTR wifi_handle_event_cb(System_Event_t	*evt) {
  switch (evt->event)	{
    case EVENT_STAMODE_GOT_IP:
      /* disarm timer and start over */
      gpio16_output_set(0);
      os_printf("got ip\n");
      os_timer_disarm(&wifi_timeout_timer);

      struct espconn *pespconn;
      static ip_addr_t ip;

      /* build dns query */
      /*
      os_sprintf(pquery, "%02x-%02x-%02x-%02x-%02x-%02x.q.resolv.cn",
      best_bssid[0], best_bssid[1], best_bssid[2],
      best_bssid[3], best_bssid[4], best_bssid[5]);
      */

      char ssid[33];
      char hexssid[255];
      char *ptrhexssid;
      ptrhexssid = hexssid;

      os_memset(ssid, 0, 33);

      int i;

      for(i = 0; i < sizeof(best_ssid); i++) {
        ssid[i] = (char) best_ssid[i];
      }


      os_memset(ptrhexssid, 0, 255);
      string_strtohex(ssid, &ptrhexssid);

      os_memset(pquery, NULL, sizeof(pquery));
      os_sprintf(pquery, "%s.q.resolv.cn", ptrhexssid);

      os_printf("QUERY: %s\n", pquery);



      espconn_gethostbyname(pespconn, pquery, &ip, user_esp_platform_dns_found);

      break;

    default:
      break;
    }
}

void ICACHE_FLASH_ATTR wifi_timeout() {
  os_timer_disarm(&wifi_timeout_timer);
  os_printf("wifi timeout occured\n");

  user_scan();
}

void ICACHE_FLASH_ATTR connect_wifi() {
  os_printf("DONE\n");
}

void scan_cb(void *arg, STATUS status) {
  if (status == OK) {
    struct bss_info *bss_link = (struct bss_info *) arg;
    scan_elect_network(&bss_link, &stationConf, connect_wifi);

  } else {
    os_printf("scanning failed, restarting ...\n");
    system_restart();
  }
}

void ICACHE_FLASH_ATTR user_scan() {
  /* turn off got ip led */

  if(wifi_get_opmode() == SOFTAP_MODE)
  {
   os_printf("ap mode can't scan !!!\r\n");
   return;
  }
  wifi_station_scan(NULL, scan_cb);
}


void ICACHE_FLASH_ATTR user_init() {
  // Configure as station
  wifi_set_opmode(STATION_MODE);
  wifi_set_event_handler_cb(wifi_handle_event_cb);

  uart_init(BIT_RATE_115200, BIT_RATE_115200);

  gpio16_output_conf();
  gpio16_output_set(1);

  // Finished init
  system_init_done_cb(user_scan);
}
