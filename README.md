### Readme for GCC toolchain and Nordic 9.0 SDK

This repository contains the parts needed to build and flash the NRF51822 V9.0 SDK using the gcc toolchain.

It also includes fixes to the 9.0 SDK to get it to build 'out of the box'.

The interesting pieces are the gcc toolchain itself; the NRF51822 files are included as a reference; as well as the use of the S130 soft device flashing using gcc.

In an effort to 'flatten' some of the files, this repository puts all the nrf_drivers in the same directory;  this is a stark difference to the typical version of the v9.0 SDK layout.  This is going to be updated with the default layout when there's time. (Todo: Make Time).  

You should be able to download this repository and edit `main.c` to include your files and have it work.  This contains most (about 95%) of the 9.0SDK, but assumes SPI and TWI_HW_MASTER are being used.  

One of the major reasons to use this SDK vs. the stock SDK download is that this download of SDK fixes a lot of the known issues with Nordic's V9.0 SDK, and it builds out of the box, including the nrf_drv_gpiote conflict mentioned here:

https://devzone.nordicsemi.com/question/40670/sdk81-and-sdk-90-app_gpiote-and-nrf_drv_gpiote-conflict/


### LICENSE

Different parts of this code have different licenses.

Any .sh files and .Makefile files are licensed using the MIT License. A complete text of this license can be found in the LICENSE file.

nrf51_sdk and any example code in main.c is Copyright 2014 Nordic and their license can be found in the NORDIC_LICENSE file.
