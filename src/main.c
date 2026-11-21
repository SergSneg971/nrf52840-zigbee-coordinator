#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zboss_api.h>
#include <zb_osif.h>
#include <zb_nrf_platform.h>

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zigbee_uart));

static serial_send_data_cb_t send_cb = NULL;
static serial_recv_data_cb_t recv_cb = NULL;

static uint8_t rx_byte;

static void uart_irq_handler(const struct device *dev, void *user_data)
{
    ARG_UNUSED(user_data);

    while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
        if (uart_irq_rx_ready(dev)) {
            int len = uart_fifo_read(dev, &rx_byte, 1);
            if (len > 0 && recv_cb != NULL) {
                recv_cb(&rx_byte, (zb_ushort_t)len);
            }
        }
    }
}

void zb_osif_serial_set_cb_send_data(serial_send_data_cb_t cb)
{
    send_cb = cb;
}

void zb_osif_serial_set_cb_recv_data(serial_recv_data_cb_t cb)
{
    recv_cb = cb;
    if (device_is_ready(uart_dev)) {
        uart_irq_callback_user_data_set(uart_dev, uart_irq_handler, NULL);
        uart_irq_rx_enable(uart_dev);
    }
}

void zb_osif_serial_send_data(zb_uint8_t *buf, zb_ushort_t len)
{
    if (!device_is_ready(uart_dev)) {
        return;
    }

    for (zb_ushort_t i = 0; i < len; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }

    if (send_cb != NULL) {
        send_cb(buf, len);
    }
}

int main(void)
{
    int ret = usb_enable(NULL);
    if (ret != 0 && ret != -EALREADY) {
        return 0;
    }

    /* Даем USB CDC ACM время на инициализацию перед стартом ZBOSS NCP */
    k_sleep(K_MSEC(500));

    zigbee_enable();

    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
