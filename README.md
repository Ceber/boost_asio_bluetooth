Forked from https://github.com/datengx/boost_asio_bluetooth

# boost_asio_bluetooth
This project extend the Boost ASIO functionality by adding bluetooth connection. More specifically, a `bluetooth` protocol class was added along with a bluetooth `endpoint` implementation. For more information regarding adding other types of protocol to the Boost ASIO library, check [this](http://www.boost.org/doc/libs/1_64_0/doc/html/boost_asio/overview/networking/other_protocols.html) link.

A simple, extensible echo server to demonstrate the Bluetooth communication is also included in the project.

### Prepare
To run the server/client pair, you need to prepare two Linux computers with Bluetooth function.

The echo server/client pair is based on the examples from

Boost.Asio C++ Network Programming - Second Edition by Wisnu Anggoro, John Torjo

To install the dependencies do the following:
##### Boost Library
```
sudo apt-get install libboost-all-dev
```
##### BlueZ
```
sudo apt-get install bluez
```
If you have older version of BlueZ installed, checkout [this](https://askubuntu.com/questions/662347/bluez-5-x-and-ubuntu-14-0x/662349) link.

### Build
```
make
```
### Run
Prepare two computers. Install this project on both of the computers. On one of the computers, in the project root directory, do:
```
./bin/echoserver
```

Then on the other computer, do
```
./bin/echoclient
```
in the project root directory.

You will see "Hello World!" sent to the server and echoed back to the client.



# Bluetooth Config - Dev
## Get Bluetooth MAC address
```
hcitool dev
```
## Check SPP (Serial Port Profile) - Add if missing
```
sdptool browse local
```
We should see something like:

```
Service Name: Serial Port
Service Description: COM Port
Service Provider: BlueZ
Service RecHandle: 0x10012
Service Class ID List:
  "Serial Port" (0x1101)
Protocol Descriptor List:
  "L2CAP" (0x0100)
  "RFCOMM" (0x0003)
    Channel: 12
Language Base Attr List:
  code_ISO639: 0x656e
  encoding:    0x6a
  base_offset: 0x100
Profile Descriptor List:
  "Serial Port" (0x1101)
    Version: 0x0100
```

If not, add Serial Port profile:
```
sdptool add --channel=12 SP
```


# Troubleshooting
If you have the following error:
```
Failed to connect to SDP server on FF:FF:FF:00:00:00: No such file or directory
```

It could mean that the 'bluetoothd' has issues...


You need to run the bluetooth daemon in compatibility mode to provide deprecated command line interfaces. You're running Bluez5 and you need some Bluez4 functions. You can do this by editing this file

/etc/systemd/system/dbus-org.bluez.service and changing this line
```
ExecStart=/usr/lib/bluetooth/bluetoothd
```
to this
```
ExecStart=/usr/lib/bluetooth/bluetoothd --compat
```

and then restarting bluetooth like this
```
sudo systemctl daemon-reload
sudo systemctl restart bluetooth
```
and you'll also have to change permissions on /var/run/sdp
```
sudo chmod 777 /var/run/sdp
```

    
