#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "driver/uart.h"

LOCAL os_timer_t wifi_timeout_timer;

void ICACHE_FLASH_ATTR user_esp_platform_dns_found(const	char	*name,	ip_addr_t	*ip,	void *arg)	{
  os_printf("DNS Callback\n");
  if(ip == NULL) {
    os_printf("nslookup failed\n");
  } else {
    os_printf("QUERY: %s\tDST: %d.%d.%d.%d",
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
      os_printf("got ip\n");
      os_timer_disarm(&wifi_timeout_timer);

      struct espconn *pespconn;
      static ip_addr_t ip;

      espconn_gethostbyname(pespconn, "www.greglangford.co.uk", &ip, user_esp_platform_dns_found);

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

void ICACHE_FLASH_ATTR scan_cb(void *arg, STATUS status) {
  uint8 bssid[6];
  uint8 ssid[33];
  sint8 rssi;
  uint8 best_bssid[6];
  uint8 best_ssid[33];
  sint8 best_rssi;

  if (status == OK) {
    rssi = 0;
    struct bss_info *bss_link = (struct bss_info *) arg;

    while (bss_link != NULL) {
      os_memset(ssid, 0, 33);
      if (os_strlen(bss_link->ssid) <= 32) {
        os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
      } else {
        os_memcpy(ssid, bss_link->ssid, 32);
      }

      if (bss_link->authmode == AUTH_OPEN) {
        os_printf("%s,"MACSTR",%d\n", ssid, MAC2STR(bss_link->bssid), bss_link->rssi);

        if (rssi == 0) {
          rssi = bss_link->rssi;
        } else {
          if (bss_link->rssi > rssi) {
            rssi = bss_link->rssi;

            // set best_ssid
            os_memset(best_ssid, 0, 33);
            if (os_strlen(bss_link->ssid) <= 32) {
              os_memcpy(best_ssid, bss_link->ssid, os_strlen(bss_link->ssid));
            } else {
              os_memcpy(best_ssid, bss_link->ssid, 32);
            }

            os_memcpy(best_bssid, bss_link->bssid, os_strlen(bss_link->bssid));
          }
        }
      }
      bss_link = bss_link->next.stqe_next;
    }

    /*
    set stationConf.bssid_set = 1
    enable connecting to specific AP based on BSSID
    */
    struct station_config stationConf;

    os_memcpy(&stationConf.bssid, best_bssid, 6);
    os_memcpy(&stationConf.ssid, best_ssid, 33);
    stationConf.bssid_set = 1;

    os_printf("Connecting to AP: %s\t BSSID: "MACSTR"\n", best_ssid, MAC2STR(best_bssid));

    /*
    set timer to detect wifi timeout
    */
    os_timer_disarm(&wifi_timeout_timer);
    os_timer_setfn(&wifi_timeout_timer, (os_timer_func_t *)wifi_timeout, NULL);
    os_timer_arm(&wifi_timeout_timer, 15000, 0);

    /* disconnect from AP if connected */
    if(wifi_station_get_connect_status() != STATION_IDLE) {
      wifi_station_disconnect();
    }

    /* connect to AP */
    wifi_station_set_config(&stationConf);
    wifi_station_connect();

  } else {
    system_restart();
  }
}

void ICACHE_FLASH_ATTR user_scan() {
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

  // Finished init
  system_init_done_cb(user_scan);
}
