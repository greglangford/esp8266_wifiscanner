#include "user_interface.h"
#include "osapi.h"
#include "string.h"

/* convert input ascii string to hexencoded string */

ICACHE_FLASH_ATTR
void string_strtohex(char str[], char *hexstr[]) {
  char buf[255];
  int pos = 0;

  /* clear buf contents */
  os_memset(buf, 0, sizeof(buf));

  while(*str) {
    char c[2];
    os_sprintf(c, "%02X", *str++);

    int i;
    for(i = 0; i < sizeof(c); i++) {
      buf[pos] = c[i];
      pos++;
    }
  }

  // copy buf string to hexstr pointer
  os_sprintf(*hexstr, "%s", buf);
}
