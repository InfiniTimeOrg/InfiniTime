#!/bin/bash

/home/jf/nrf52/openocd-code/src/openocd  -s /home/jf/nrf52/openocd-code/tcl/ -c "tcl_port disabled" -c "gdb_port 3333" -c "telnet_port 4444" -f /home/jf/nrf52/openocd-code/tcl/interface/jlink.cfg -c "transport select swd" -f /home/jf/nrf52/openocd-code/tcl/target/nrf52.cfg -f booloader_app_jlink.ocd