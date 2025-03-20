Project contains three types of nodes: Mirror node (mirrorNode.c), sender Node (toMirrorSenderNode.c) and relaying nodes (relayingNode.c). Sender node sends packets, mirror returns received packets to the sender. Sender measures network throughput.    

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

4. There are three types of network nodes in this example. Make sure to copy chosen node into main/main.c:
```
	cp main/nodeMains/mirrorNode.c main/main.c
```
	or
```
	cp main/nodeMains/relayingNode.c main/main.c
```
	or
```
	cp main/nodeMains/toMirrorSenderNode.c main/main.c
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
