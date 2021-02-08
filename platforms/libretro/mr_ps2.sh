#!/bin/bash

make platform=ps2 -j4 && cp gearboy_libretro_ps2.a ../../../RetroArch/libretro_ps2.a

cd ../../../RetroArch

make -f Makefile.ps2 prepare

mips64r5900el-ps2-elf-g++ -T/home/isanchez/software/ps2dev/ps2sdk/ee/startup/linkfile -o retroarchps2-debug.elf ps2/irx/sio2man_irx.o ps2/irx/iomanX_irx.o ps2/irx/fileXio_irx.o  ps2/irx/mcman_irx.o ps2/irx/mcserv_irx.o ps2/irx/usbd_irx.o ps2/irx/usbhdfsd_irx.o ps2/irx/libsd_irx.o ps2/irx/audsrv_irx.o ps2/irx/cdfs_irx.o ps2/irx/padman_irx.o ps2/compat_files/ps2_devices.o  griffin/griffin.o -L/home/isanchez/software/ps2dev/ps2sdk/ee/lib -s -L/home/isanchez/software/ps2dev/gsKit/lib -L/home/isanchez/software/ps2dev/ps2sdk/ports/lib -L. -lretro_ps2 -lpatches -lgskit -ldmakit -laudsrv -lpadx -lmtap -lz -lcdvd -lelf-loader

make -f Makefile.ps2 run

