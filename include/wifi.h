#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <net.h>

#define WIFI_MAX_FRAME_LEN 1536

#define WIFI_STATE_DOWN 0
#define WIFI_STATE_INIT 1
#define WIFI_STATE_SCANNING 2
#define WIFI_STATE_AUTHENTICATING 3
#define WIFI_STATE_ASSOCIATING 4
#define WIFI_STATE_ASSOCIATED 5
#define WIFI_STATE_FAILED 6

#define WIFI_SEC_OPEN 0
#define WIFI_SEC_WPA_PSK 1

int wifi_init(void);
int wifi_assoc(const char *ssid, const char *password);
int wifi_transmit(struct net_iface *iface, struct pbuf *p);
int wifi_start(void);
int wifi_stop(void);
int wifi_set_ip(uint32_t ip, uint32_t mask, uint32_t gw);
int wifi_get_state(void);
const uint8_t *wifi_get_mac(void);
void wifi_set_credentials(const char *ssid, const char *pass, int sec_mode);
const char *wifi_get_ssid(void);
int wifi_get_rssi(void);
uint8_t wifi_get_channel(void);

#endif