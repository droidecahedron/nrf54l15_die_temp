/*
 * nrf54l15_die_temp.c
 * main.c
 * Simple program to show spawning a task to read chip die temp
 * auth: ddhd
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <mpsl_temp.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define COMPANY_ID_CODE 0x0059
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define DIETEMP_THREAD_STACK_SIZE 500
#define DIETEMP_THREAD_PRIORITY 4
K_THREAD_STACK_DEFINE(dietemp_stack_area, DIETEMP_THREAD_STACK_SIZE);
struct k_thread dietemp_thread_data;

static struct k_work dietemp_work;
struct k_work_q dietemp_work_q;
static K_THREAD_STACK_DEFINE(dietemp_work_stack, 256);
#define DIETEMP_WORK_PRIO K_PRIO_COOP(CONFIG_MPSL_THREAD_COOP_PRIO)

volatile int32_t g_die_temp;

LOG_MODULE_REGISTER(dietemp_ble, LOG_LEVEL_INF);

// WorkQ item.
static void die_temp_work_fn(struct k_work *work)
{
    g_die_temp = mpsl_temperature_get();
}

typedef struct adv_mfg_data
{
    uint16_t company_code; /* Company Identifier Code. */
    uint16_t die_temp;
} adv_mfg_data_type;

static const struct bt_le_adv_param *adv_param =
    BT_LE_ADV_PARAM(BT_LE_ADV_OPT_NONE, /* No options specified */
                    800,                /* Min Advertising Interval 500ms (800*0.625ms) */
                    801,                /* Max Advertising Interval 500.625ms (801*0.625ms) */
                    NULL);              /* Set to NULL for undirected advertising */

static adv_mfg_data_type adv_mfg_data = {COMPANY_ID_CODE, 0x00};

static unsigned char url_data[] = {0x17, '/', '/', 'n', 'o', 'r', 'd', 'i', 'c',
                                   's',  'e', 'm', 'i', '.', 'c', 'o', 'm'};

static struct k_work adv_work;
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    /* STEP 3 - Include the Manufacturer Specific Data in the advertising packet. */
    BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_URI, url_data, sizeof(url_data)),
};

static void adv_work_handler(struct k_work *work)
{
    int err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err)
    {
        LOG_INF("Advertising failed to start (err %d)", err);
        return;
    }

    LOG_INF("Advertising successfully started");
}

static void advertising_start(void)
{
    k_work_submit(&adv_work);
}

void dietemp_thread(void *p1, void *p2, void *p3)
{
    k_work_queue_start(&dietemp_work_q, dietemp_work_stack, K_THREAD_STACK_SIZEOF(dietemp_work_stack),
                       DIETEMP_WORK_PRIO, NULL);
    k_thread_name_set(&dietemp_work_q.thread, "dietemp work");
    k_work_init(&dietemp_work, die_temp_work_fn);

    // manpage: https://docs.nordicsemi.com/bundle/ps_nrf54L15/page/temp.html
    // note: If you use BLE, SD uses TEMP. So you need to ask for the temperature.
    // mpsl_temperature_get() returns die temp in 0.25degC
    for (;;)
    {

        // Note:  mpsl_temperature_get function must be executed in the same execution priority as
        // mpsl_low_priority_process.
        // If you call the function from a thread and then the SDC signals that it needs to do some low prio processing
        // Issues can arise if you try to get the temp at the same time that we check for LFOSC calibration.
        // see mpsl_init.c, but I've mapped out the exec order here
        /*
            -> mpsl_low_priority_process() -> gets called in mpsl_low_prio_work_handler
            -> which has work submitted in mpsl_low_prio_irq_handler
            -> set up via IRQ_CONNECT(CONFIG_MPSL_LOW_PRIO_IRQN, MPSL_LOW_PRIO, mpsl_low_prio_irq_handler, NULL,0);
            -> mpsl_low_prio_work_handler is submitted in an IRQ, but the workQ itself is
                K_PRIO_COOP(CONFIG_MPSL_THREAD_COOP_PRIO).
                Check the readme or the mpsl kconfing init for a breakdown of what that prio is.
        */
        k_work_submit_to_queue(&dietemp_work_q , &dietemp_work);

        LOG_INF("NRF_TEMP->TEMP=CONVERSION: %.2f deg C", (double)(g_die_temp / 4.0f));
        adv_mfg_data.die_temp = g_die_temp / 4;
        bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

        k_sleep(K_MSEC(5000));
    }
}

int main(void)
{
    int err;
    LOG_INF("Sample to measure die temp periodically on NRF54L15-DK");

    err = bt_enable(NULL);
    if (err)
    {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }

    LOG_INF("Bluetooth initialized\n");
    k_work_init(&adv_work, adv_work_handler);
    advertising_start();

    k_tid_t dietemp_tid =
        k_thread_create(&dietemp_thread_data, dietemp_stack_area, K_THREAD_STACK_SIZEOF(dietemp_stack_area),
                        dietemp_thread, NULL, NULL, NULL, DIETEMP_THREAD_PRIORITY, 0, K_NO_WAIT);

    LOG_INF("dietemp thread spawned from main priority %d", k_thread_priority_get(dietemp_tid));

    return 0;
}
