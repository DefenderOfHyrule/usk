# μsk (Picofly firmware)

μsk is an open source firmware for Switch voltage glitching-based modchips. μsk runs on dev (and custom) boards containing the Raspberry Pi RP2040 microcontroller.
The name for this open source modchip is Picofly, and any modchip containing the RP2040 microcontroller is therefore a Picofly modchip.

## Features

μsk has the following main features:

* Bypassing the `sdloader` module by holding volume up + volume down on boot to boot OFW,
* Detection for when `BCT` is overwritten (this is usually done by major Switch firmware updates),
* Detection for a post-OFW update sequence (updating your Switch firmware version via OFW),
* The ability to store and compare glitching- and training data,
* The ability to allow for booting of Android/Linux on the Switch,
* The ability to update the firmware via picofly_toolbox,
* Debug LED error codes (for fault and status indication),
* Fast boot times (sub 0.2 seconds on average since firmware version `2.80`) compared to hwfly and instinctnx modchips.

## Debug/Fault indication LED colors

Since Picofly firmware version `2.80`, the LED colors indicate the following modchip status:

* `Blue` indicates glitching,
* `Light blue` indicates training,
* `Beige` indicates comparison to stored training data,
* `Yellow/Orange` indicates writing `sdloader` to `BOOT0` and/or success,
* `Red` indicates an issue with your modchip installation (otherwise known as a fault or error code).

### Fault description/Error codes

**Known error codes (`=` is a long pulse, `*` is a short pulse):**

`=` USB flashing done.

`**` RST (`B`) is not connected.

`*=` CMD (`A`)is not connected.

`=*` D0 (`C`/`DAT0`) is not connected.

`==` CLK (`D`) is not connected.

`***` No eMMC CMD1 response. (Bad eMMC?)

`**=` No eMMC block 1 read. (Should not happen.)

`*==` Bad wiring/cabling, typically has to do with the top ribbon cable that connects `3.3v`, `A`, `B`, `C`, `D` and `GND` pads. (Which is why I don't recommend using that ribbon cable.) Alternatively, it can also mean that the modchip is defective.

`*=*` No eMMC block 0 read. (eMMC init failure?)

`=**` eMMC initialization failure during glitching process.

`=*=` CPU never reach BCT check, should not happen. Typically caused by the SoC ribbon cable not being seated properly.

`==*` CPU always reach BCT check (no glitch reaction, check MOSFET/SoC ribbon cable).

`===` Glitch attempt limit reached, cannot glitch

`=***` eMMC initialization failure

`=**=` eMMC write failure - comparison failed

`==` eMMC write failure - write failed

`=*==` eMMC test failure - read failed

`==**` eMMC read failed during firmware update

`==*=` BCT copy failed - write failure

`===*` BCT copy failed - comparison failure

`====` BCT copy failed - read failure

* **For further troubleshooting, please see this page:** https://guide.nx-modchip.info/troubleshooting/error_codes/

## Building/Compiling μsk

I've written two scripts that make compiling μsk easier, these scripts can be found in the root of the repository.
A short explanation about what they do:

* `picofly_build_local.sh` is a script that is ran from within the *root* of the cloned repository. You clone the repository, `cd` into the repository and run the script with `bash picofly_build_local.sh`. The script has a preconfigured workspace (`$(pwd)/Picofly-build-local`) that clones the busk and pico-sdk repositories to the `Picofly-build-local`folder that's created in the root of the cloned μsk repository. This script is used to test and/or compile local changes made to μsk's code.
* `picofly_build_remote.sh` is a script that can be ran from whichever directory you'd like. This script (despite the possibly confusing name) does something similar to the previous script except that it clones the μsk, busk and pico-sdk repositories to `$HOME/Picofly-build-remote` (can be changed to whichever directory you'd like) automatically. This script can be ran with `bash picofly_build_remote.sh` from any location on your PC and can be used in the situation where you just want to have a quick way of cloning all relevant repositories and build the μsk firmware for yourself.

The resulting `firmware.uf2` and `update.bin` will be stored in `WORKSPACE/build/usk` for each script.

These scripts are now (as of 30-01-2026) compatible with all "mainstream" linux distributions :). If you face any issues, please report them in the issues tab.

## Modchip installation guide

I've also created and written an installation guide for Picofly modchips, this guide can be found here: https://guide.nx-modchip.info/

* **Note:** This guide is still not yet finished, and I am not entirely sure as to when i'll get time to rework the photos and instructions provided in the guide. (As they were admittedly pretty rushed :P.)

## Credits

Since this repository lacked credits previously, I do want to thank abal1000x and rehius for the effort they've put into this project.
