# nrf54l15_die_temp
A simple program to read the [TEMP](https://docs.nordicsemi.com/bundle/ps_nrf54L15/page/temp.html) of the nRF54L15 SoC on an nRF54L15-DK to a console.

# BLE branch
> [!IMPORTANT]
>The MSPL will reserve access to the TEMP peripheral if you are enabling Bluetooth in application. TEMP is used for clock calibration when the internal RC oscillator is selected as the clock source.
> In this case, you will use the nrfxlib API for MPSL to get the die temp via [mpsl_temperature_get()](https://docs.nordicsemi.com/bundle/nrfxlib-apis-latest/page/group_mpsl_temp_ga0be40956c96a226af1083a476fe57148.html#ga0be40956c96a226af1083a476fe57148)
> One thing you will note is that *"This function must be executed in the same execution priority as mpsl_low_priority_process."*
> So we will borrow the QDEC IRQ and trigger a SW interrupt at the correct priority to ensure safety around reentrancy and around then mpsl/softdevice will check NRF_TEMP.

From the manual page:
> Operation
> TEMP is started by triggering the START task.
>
> When the temperature measurement is finished, a DATARDY event will be generated and the measurement result can be read from the TEMP register.
>
> When the temperature measurement is finished, TEMP analog electronics power-down to save power.
>
> TEMP only supports one-shot operation, meaning that every TEMP measurement has to be explicitly started using the START task.

# Building and Running
Typical `west build -b nrf54l15dk/nrf54l15/cpuapp -p` and `west flash --recover`.

Connect to the DK's `VCOM1/ttyACM1` to see the logging output.

# Example Output
```
*** Booting nRF Connect SDK v3.1.0-6c6e5b32496e ***
*** Using Zephyr OS v4.1.99-1612683d4010 ***
[00:00:00.000,329] <inf> dietemp_ble: Sample to measure die temp periodically on NRF54L15-DK
[00:00:00.000,386] <inf> bt_sdc_hci_driver: SoftDevice Controller build revision: 
                                            fc de 41 eb a2 d1 42 24  00 b5 f8 57 9f ac 9d 9e |..A...B$ ...W....
                                            aa c9 b4 34                                      |...4             
[00:00:00.001,644] <inf> bt_hci_core: HW Platform: Nordic Semiconductor (0x0002)
[00:00:00.001,657] <inf> bt_hci_core: HW Variant: nRF54Lx (0x0005)
[00:00:00.001,669] <inf> bt_hci_core: Firmware: Standard Bluetooth controller (0x00) Version 252.16862 Build 1121034987
[00:00:00.002,106] <inf> bt_hci_core: HCI transport: SDC
[00:00:00.002,166] <inf> bt_hci_core: Identity: CA:9A:0D:B5:2A:CC (random)
[00:00:00.002,180] <inf> bt_hci_core: HCI: version 6.1 (0x0f) revision 0x3069, manufacturer 0x0059
[00:00:00.002,193] <inf> bt_hci_core: LMP: version 6.1 (0x0f) subver 0x3069
[00:00:00.002,198] <inf> dietemp_ble: Bluetooth initialized

[00:00:00.002,794] <inf> dietemp_ble: Advertising successfully started
[00:00:00.002,820] <inf> dietemp_ble: dietemp thread spawned from main priority 4
[00:00:00.002,908] <inf> dietemp_ble: NRF_TEMP->TEMP=CONVERSION: 28.25 deg C
[00:00:05.003,255] <inf> dietemp_ble: NRF_TEMP->TEMP=CONVERSION: 27.50 deg C
...
```

If you scan for `nrf54l15_die_temp`, you'll see the manufacturer data will have little endian representation of the retval of the mpsl temp getter.

<img width="201" height="116" alt="image" src="https://github.com/user-attachments/assets/2df04b4d-ee0c-4d6a-8791-22bc72502ff6" />

