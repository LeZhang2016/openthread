#!/usr/bin/expect -f
set ip [lindex $argv 0 ]
set source_file [lindex $argv 1 ]
set dest_file [lindex $argv 2 ]
set dir [lindex $argv 3 ]

#arm-none-eabi-objcopy -O ihex /home/lezhan/program/$dir/output/nrf52840/bin/ot-cli-ftd /home/lezhan/program/$dir/output/nrf52840/bin/arm-none-eabi-ot-cli-ftd.hex
spawn scp $source_file pi@$ip:$dest_file
#spawn scp /home/lezhan/program/$dir/output/nrf52840/bin/arm-none-eabi-ot-cli-ftd.hex pi@$ip:/home/pi/
expect "*password:"
set timeout 1
send "raspberry\n"
expect eof

