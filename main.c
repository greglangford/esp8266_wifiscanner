#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "driver/uart.h"

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


    os_printf("\nchosen ssid\n");
    os_printf("%s,%d\n", best_ssid, rssi);
    show(&ssid);
  }
}

void ICACHE_FLASH_ATTR user_scan(void) {
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

  uart_init(BIT_RATE_115200, BIT_RATE_115200);

  // Finished init
  system_init_done_cb(user_scan);
}
