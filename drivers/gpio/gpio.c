#include <esp32s3_reg.h>

void gpio_init(void) {
}

void gpio_set_direction(int gpio, int mode) {
    (void)gpio; (void)mode;
}

void gpio_set_level(int gpio, int level) {
    (void)gpio; (void)level;
}

int gpio_get_level(int gpio) {
    (void)gpio;
    return 0;
}

void gpio_pullup_en(int gpio) {
    (void)gpio;
}

void gpio_pulldown_en(int gpio) {
    (void)gpio;
}

void gpio_pullup_dis(int gpio) {
    (void)gpio;
}

void gpio_pulldown_dis(int gpio) {
    (void)gpio;
}

void gpio_set_intr_type(int gpio, int type) {
    (void)gpio; (void)type;
}

void gpio_intr_enable(int gpio) {
    (void)gpio;
}

void gpio_intr_disable(int gpio) {
    (void)gpio;
}