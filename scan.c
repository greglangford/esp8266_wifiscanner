#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"

void scan_elect_network(struct bss_info *bss_link, struct station_config *stationConf, void *cb) {
  uint8 bssid[6];
  uint8 ssid[33];
  uint8 elected_bssid;
  uint8 elected_ssid;
  int i = 0;

  os_memset(ssid, NULL, 33);
  os_memset(bssid, NULL, 6);

  while (bss_link != NULL) {
    /* clear ssid */
    os_memset(ssid, NULL, 33);

    /* set ssid */
    if (os_strlen(bss_link->ssid) <= 32) {
      os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid));
    } else {
      os_memcpy(ssid, bss_link->ssid, 32);
    }

    /* clear bssid */
    os_memset(bssid, NULL, 6);

    /* set bssid */
    os_memcpy(bssid, bss_link->bssid, 6);

    os_printf("%s,"MACSTR",%d\n", ssid, MAC2STR(bssid), bss_link->rssi);

    bss_link = bss_link->next.stqe_next;
  }

  void (*callback)(void);
  callback=(void *) cb;

  callback();
}
