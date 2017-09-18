#!/usr/bin/expect -f

set ip [lindex $argv 0 ]
set sn [lindex $argv 1 ]

spawn ssh pi@$ip
expect "*password:"
set timeout 1
send "raspberry\n"
expect "*$"

send "sudo kill -9 \$(pidof JLinkGDBServer  -if swd -device nRF52840_xxAA -select usb="$sn")\n"
# start jlink gdb server
send "JLinkGDBServer  -if swd -device nRF52840_xxAA -select usb="$sn"\n &"

# exit the terminal
send "exit\n"
expect eof