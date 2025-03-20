
# Universal mesh library

Directory contains implementation of universal mesh library dedicated for memory constrained microcontrollers and sensor networks. Library uses directed diffusion protocol as routing protocol. Nodes running library detect neighbor nodes automaticaly and there is no node naming requirement since protocol is data-centric.

You can find API functions in MeshAPI/include/meshAPI.h file.
If you want to play with library configuration open InterfaceFunctions/include/meshSystemConfig.h file and modify it.

Library is designed to be universal so it can be easily implemented for new environments. To compile mesh system for new environment
you have to provide implementation of few functions grouped in three header files (located in InterfaceFunctions/include subdirectory): packetTransmission.h, systemDependentHelpFunctions.h and universal_semaphore.h. Project already contains tested implementations for three platforms: Esp8266 using Esp-Now library for one-to-one communication, esp8266 using nrf24L01+ chip and for linux simulation enviroment. You can find these implementations in InterfaceFunctions/ESP8266-ESPNOWFiles, InterfaceFunctions/Esp8266-nRF24l01Files, and InterfaceFunctions/linux-simulFiles subdirectories. You can also use these subdirectories as examples when it comes to new platform implementation. To set up library for chosen environment you only have to place whole content of corresponding subdirectory into one directory lower, for example:

```
cp MeshDirectedDiffusion/InterfaceFunctions/ESP8266-ESPNOWFiles/include/* MeshDirectedDiffusion/InterfaceFunctions/include/
```
```
find MeshDirectedDiffusion/InterfaceFunctions/ESP8266-ESPNOWFiles/ -maxdepth 1 -type f -exec cp {} MeshDirectedDiffusion/InterfaceFunctions/ \;
