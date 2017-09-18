#!/usr/bin/expect -f
set ip [lindex $argv 0 ]
spawn ssh pi@$ip
expect "*password:"
set timeout 2
send "raspberry\n"
expect "*$"
# start ser2net service
send "sudo service ser2net restart\n"

# kill the grpc server if the process is alive
send "sudo kill -9 \$(pidof python)\n"

# start testbed grpc server
send "python testbed/raspi/testbed_server.py &\n"

# exit the terminal
send "exit\n"
expect eof