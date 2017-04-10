#include "user_interface.h"
#include "osapi.h"
#include "wifi.h"

void ICACHE_FLASH_ATTR scan_elect_network(struct bss_info *bss_link) {
  struct station_config stationConf;
  struct station_config *ptrstationConf;

  uint8 bssid[6], elected_bssid[6];
  uint8 ssid[33], elected_ssid[33];
  sint8 rssi;
  uint8 i = 0;

  /* make sure previous elected_ssid and elected_bssid are unset */
  os_memset(elected_ssid, NULL, sizeof(elected_ssid));
  os_memset(elected_bssid, NULL, sizeof(elected_bssid));

  os_printf("scanning for networks\n");

  while (bss_link != NULL) {
    /* clear variables at start of each loop iteration */
    os_memset(bssid, NULL, sizeof(bssid));
    os_memset(ssid, NULL, sizeof(ssid));

    /* set ssid */
    if (os_strlen(bss_link->ssid) <= 32) {
      os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
    } else {
      os_memcpy(ssid, bss_link->ssid, 32);
    }

    /* set bssid */
    os_memcpy(bssid, bss_link->bssid, 6);

    if (bss_link->authmode == AUTH_OPEN) {
      /* configured elected_ssid and elected_bssid to be first result */
      /* this is being done so there is something to compare against  */
      if(i == 0) {
        if (os_strlen(bss_link->ssid) <= 32) {
          os_memcpy(elected_ssid, bss_link->ssid, os_strlen(bss_link->ssid));
        } else {
          os_memcpy(elected_ssid, bss_link->ssid, 32);
        }

        os_memcpy(elected_bssid, bss_link->bssid, 6);
        rssi = bss_link->rssi;

        /* set to 1 ensures that this block does not run again */
        i++;
      }

      /* show open network scan results */
      os_printf("%s,"MACSTR",%d\n", ssid, MAC2STR(bssid), bss_link->rssi);

      /* find the best quality signal based on RSSI */
      if(bss_link->rssi >= rssi) {

        /* clear previous values */
        os_memset(elected_bssid, NULL, sizeof(elected_bssid));
        os_memset(elected_ssid, NULL, sizeof(elected_ssid));

        /* update rssi */
        rssi = bss_link->rssi;

        /* update elected_ssid */
        if (os_strlen(ssid) <= 32) {
          os_memcpy(elected_ssid, bss_link->ssid, os_strlen(bss_link->ssid));
        } else {
          os_memcpy(elected_ssid, bss_link->ssid, 32);
        }

        /* update elected_bssid */
        os_memcpy(elected_bssid, bss_link->bssid, 6);
      }
    }

    /* get next scan result */
    bss_link = bss_link->next.stqe_next;
  }

  os_memcpy(&stationConf.bssid, elected_bssid, sizeof(elected_bssid));
  os_memcpy(&stationConf.ssid, elected_ssid, sizeof(elected_ssid));

  os_printf("\nelected ssid: %s\n", elected_ssid);
  os_printf("elected bssid: "MACSTR"\n", MAC2STR(elected_bssid));

  /* connect to network */
  wifi_connect_network(&stationConf);
}

void ICACHE_FLASH_ATTR scan_callback(void *arg, STATUS status) {
  if (status == OK) {
    struct bss_info *bss_link = (struct bss_info *) arg;

    scan_elect_network(bss_link);

  } else {
    os_printf("scan callback failed ...\n");
    scan_init();
  }
}

void ICACHE_FLASH_ATTR scan_init() {
  /* set timeout timer */
  os_timer_disarm(&wifi_connect_network_timeout_timer);
  os_timer_setfn(&wifi_connect_network_timeout_timer, (os_timer_func_t *)wifi_network_connect_timeout, NULL);
  os_timer_arm(&wifi_connect_network_timeout_timer, 15000, 0);
  wifi_station_scan(NULL, scan_callback);
}
