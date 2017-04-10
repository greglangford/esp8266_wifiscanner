#include "user_interface.h"
#include "osapi.h"
#include "driver/uart.h"
#include "scan.h"
#include "wifi.h"

void ICACHE_FLASH_ATTR user_scan() {
  os_printf("INIT IS COMPLETE\n");
}

void ICACHE_FLASH_ATTR user_init() {
  // Configure as station
  wifi_set_opmode(STATION_MODE);
  wifi_set_event_handler_cb(wifi_handle_event_cb);

  uart_init(BIT_RATE_115200, BIT_RATE_115200);

  gpio16_output_conf();
  gpio16_output_set(1);

  // Finished init
  system_init_done_cb(scan_init);
}
