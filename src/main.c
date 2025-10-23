/*
 * nrf54l15_die_temp.c
 * main.c
 * Simple program to show spawning a task to read chip die temp
 * auth: ddhd
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dietemp, LOG_LEVEL_INF);

#define DIETEMP_THREAD_STACK_SIZE 500
#define DIETEMP_THREAD_PRIORITY 5
K_THREAD_STACK_DEFINE(dietemp_stack_area, DIETEMP_THREAD_STACK_SIZE);
struct k_thread dietemp_thread_data;

void dietemp_thread(void *p1, void *p2, void *p3)
{
    // manpage: https://docs.nordicsemi.com/bundle/ps_nrf54L15/page/temp.html
    for (;;)
    {
        NRF_TEMP->TASKS_START = 1;
        while (NRF_TEMP->EVENTS_DATARDY == 0)
        {
            k_yield();
        }

        NRF_TEMP->EVENTS_DATARDY = 0;

        int32_t temp_raw = NRF_TEMP->TEMP;
        float temperature_c = temp_raw * 0.25f; // 0.25DEGC steps, 2's complement

        NRF_TEMP->TASKS_STOP = 1;

        LOG_INF("NRF_TEMP->TEMP=0x%x | CONVERSION: %.2f deg C", temp_raw, (double)temperature_c);

        k_sleep(K_MSEC(5000));
    }
}

int main(void)
{
    LOG_INF("Sample to measure die temp periodically on NRF54L15-DK");

    k_tid_t dietemp_tid =
        k_thread_create(&dietemp_thread_data, dietemp_stack_area, K_THREAD_STACK_SIZEOF(dietemp_stack_area),
                        dietemp_thread, NULL, NULL, NULL, DIETEMP_THREAD_PRIORITY, 0, K_NO_WAIT);

    
    LOG_INF("dietemp thread spawned from main priority %d",k_thread_priority_get(dietemp_tid));

    return 0;
}
