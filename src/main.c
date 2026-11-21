#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/usb/usb_device.h>
#include <zboss_api.h>
#include <zigbee/zigbee_app_utils.h>

int main(void)
{
    int ret = usb_enable(NULL);
    if (ret != 0 && ret != -EALREADY) {
        return 0;
    }

    /* Запуск стека Zigbee (в режиме NCP при включенном CONFIG_ZIGBEE_LIBRARY_NCP_DEV) */
    zigbee_enable();

    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
