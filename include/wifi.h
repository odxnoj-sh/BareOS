#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <net.h>

#define WIFI_MAX_FRAME_LEN 1536
#define WIFI_STATE_DOWN 0
#define WIFI_STATE_INIT 1
#define WIFI_STATE_UP 2
#define WIFI_STATE_ERROR 3

int wifi_init(void);
int wifi_start(void);
int wifi_stop(void);
int wifi_transmit(struct net_iface *iface, struct pbuf *p);
int wifi_set_ip(uint32_t ip, uint32_t mask, uint32_t gw);
int wifi_get_state(void);
const uint8_t *wifi_get_mac(void);
int wifi_assoc(const char *ssid, const char *password);

#endif