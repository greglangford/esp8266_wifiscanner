#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "driver/uart.h"
#include "driver/gpio16.h"

LOCAL os_timer_t wifi_timeout_timer;
char pquery[255];
uint8 bssid[6];
uint8 ssid[33];
sint8 rssi;
uint8 best_bssid[6];
uint8 best_ssid[33];
sint8 best_rssi;

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
      gpio16_output_set(0);
      os_printf("got ip\n");
      os_timer_disarm(&wifi_timeout_timer);

      struct espconn *pespconn;
      static ip_addr_t ip;

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

void scan_cb(void *arg, STATUS status) {
  int i = 0;

  if (status == OK) {
    struct bss_info *bss_link = (struct bss_info *) arg;

    if(bss_link == NULL) {
      os_printf("Empty Result\n");
    }

    while (bss_link != NULL) {
      os_memset(ssid, 0, 33);
      if (os_strlen(bss_link->ssid) <= 32) {
        os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
      } else {
        os_memcpy(ssid, bss_link->ssid, 32);
      }

      os_memset(bssid, 0, 6);
      os_memcpy(bssid, bss_link->bssid, 6);

      if (bss_link->authmode == AUTH_OPEN) {

        /* set best_ssid on first AP */
        if(i == 0) {
          os_memset(best_ssid, 0, 33);
          if (os_strlen(bss_link->ssid) <= 32) {
            os_memcpy(best_ssid, bss_link->ssid, os_strlen(bss_link->ssid));
          } else {
            os_memcpy(best_ssid, bss_link->ssid, 32);
          }

          i = 1;
        }

        os_printf("%s,"MACSTR",%d\n\n", ssid, MAC2STR(bssid), rssi);

        if(rssi > best_rssi) {
          best_rssi = bss_link->rssi;

          if (os_strlen(ssid) <= 32) {
            os_memcpy(best_ssid, bss_link->ssid, os_strlen(bss_link->ssid));
          } else {
            os_memcpy(best_ssid, bss_link->ssid, 32);
          }
        }
      }
      bss_link = bss_link->next.stqe_next;
    }

    os_printf("\n\nbest network\n");
    os_printf("%s, %d\n\n\n", best_ssid, best_rssi);

  } else {
    os_printf("Scan failed\n");
    user_scan();
  }

  user_scan();
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

  gpio16_output_conf();
  gpio16_output_set(1);

  // Finished init
  system_init_done_cb(user_scan);
}
