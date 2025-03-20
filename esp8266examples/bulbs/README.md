Project presents basic example of MeshDirectedDiffusion library.

There are three types of network nodes in this example. Bulb nodes directly connected to the bulbs (bulb.c), nodes remotely switching bulbs state (remoteBulbControlApplication.c) and nodes which are just relaying network packets (relayingNode.c). Switching nodes search for bulbs in the network and switch their states on and off every 2 sec.

1. Make sure that you have properly installed xtensa-lx106-elf toolchain and ESP8266 free RTOS SDK in your system (https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html).

2. Place MeshDirectedDiffusion library to this directory.

3. Copy whole content of MeshDirectedDiffusion/InterfaceFunctions/Esp8266-nRF24l01Files one directory lower into MeshDirectedDiffusion/InterfaceFunctions (and merge their include directories).
If you want to compile for ESP_NOW environment use ESP8266-ESPNOWFiles directory instead.
for nRF24l01+:
```
	cp MeshDirectedDiffusion/InterfaceFunctions/Esp8266-nRF24l01Files/include/* MeshDirectedDiffusion/InterfaceFunctions/include/
	```
	```
	find MeshDirectedDiffusion/InterfaceFunctions/Esp8266-nRF24l01Files/ -maxdepth 1 -type f -exec cp {} MeshDirectedDiffusion/InterfaceFunctions/ \;
```

	or for ESP_NOW:
```
	cp MeshDirectedDiffusion/InterfaceFunctions/ESP8266-ESPNOWFiles/include/* MeshDirectedDiffusion/InterfaceFunctions/include/
```
```
	find MeshDirectedDiffusion/InterfaceFunctions/ESP8266-ESPNOWFiles/ -maxdepth 1 -type f -exec cp {} MeshDirectedDiffusion/InterfaceFunctions/ \;
```

4. There are three types of network nodes in this example. Bulb nodes directly connected to the bulbs, nodes remotely switching bulbs state and nodes which are just relaying network packets.
Their main functions are placed in main/nodeMains subdirectory. Make sure to copy chosen code into main/main.c
```
	cp main/nodeMains/bulb.c main/main.c
```
	or
```
	cp main/nodeMains/remoteBulbControlApplication.c main/main.c
```
	or
```
	cp main/nodeMains/relayingNode.c main/main.c
```

5. Make sure sdkconfig file is correctly filled, expecialy fill CONFIG_SDK_TOOLPREFIX and CONFIG_TOOLPREFIX definitions with path to xtensa toolchain prefix ("/home/your/path/xtensa-lx106-elf/bin/xtensa-lx106-elf-") on your machine. To edit sdkconfig file you can use interactive tool:
```
	make menuconfig
```

6. Plug esp8266 board into configured USB port, then compile and flash:
```
	make flash
```
cmake wasn't tested yet there and propably doesn't work.
If you encounter problems during flashing then command below might help:
```
	sudo chmod a+rw /dev/ttyUSB0
```

7. For every bulb node in the network connect pin 5 (D1 on nodeMCU) and ground pin to diode or bulb.

8. Make sure to set up unique name and switch id for each bulb.

9. Place nodes in the space.
