# nrf54l15_die_temp
A simple program to read the [TEMP](https://docs.nordicsemi.com/bundle/ps_nrf54L15/page/temp.html) of the nRF54L15 SoC on an nRF54L15-DK to a console in NCS `v3.1.x`.

From the manual page:
> Operation
> TEMP is started by triggering the START task.
>
> When the temperature measurement is finished, a DATARDY event will be generated and the measurement result can be read from the TEMP register.
>
> When the temperature measurement is finished, TEMP analog electronics power-down to save power.
>
> TEMP only supports one-shot operation, meaning that every TEMP measurement has to be explicitly started using the START task.

> [!IMPORTANT]
>The MSPL will reserve access to the TEMP peripheral if you are enabling Bluetooth in application. TEMP is used for clock calibration when the internal RC oscillator is selected as the clock source.
> In this case, you will use the nrfxlib API for MPSL to get the die temp via [mpsl_temperature_get()](https://docs.nordicsemi.com/bundle/nrfxlib-apis-latest/page/group_mpsl_temp_ga0be40956c96a226af1083a476fe57148.html#ga0be40956c96a226af1083a476fe57148)
> See the [prio_fix](https://github.com/droidecahedron/nrf54l15_die_temp/tree/prio_fix) branch for an example application utilizing this.

# Building and Running
Typical `west build -b nrf54l15dk/nrf54l15/cpuapp -p` and `west flash --recover`.

Connect to the DK's `VCOM1/ttyACM1` to see the logging output.

# Example Output
```
*** Booting nRF Connect SDK v3.1.0-6c6e5b32496e ***
*** Using Zephyr OS v4.1.99-1612683d4010 ***
[00:00:00.009,407] <inf> dietemp: Sample to measure die temp periodically on NRF54L15-DK
[00:00:00.009,418] <inf> dietemp: dietemp thread spawned from main priority 5
[00:00:00.009,485] <inf> dietemp: NRF_TEMP->TEMP=0x69 | CONVERSION: 26.25 deg C
[00:00:05.009,583] <inf> dietemp: NRF_TEMP->TEMP=0x67 | CONVERSION: 25.75 deg C
[00:00:10.009,710] <inf> dietemp: NRF_TEMP->TEMP=0x67 | CONVERSION: 25.75 deg C
[00:00:15.009,806] <inf> dietemp: NRF_TEMP->TEMP=0x67 | CONVERSION: 25.75 deg C
```
