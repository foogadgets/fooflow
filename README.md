# fooflow
Webinterface to PID controlled clothing iron used for PCB reflowing.

Inspired by the project, https://github.com/mc-hamster/eFlow

### You will need
* Lolin D1 mini
* SSR
* MAX31850K thermocouple amplifier module
* Thermocoupler type-K
* An old clothing iron or an oven

### Electrical connections
MAX318550 connects to Lolin GPIO0 (D3), GND and 3V3.  
SSR connets to Lolin GPIO5 (D1) and GND.  
Clothing iron/Oven connects to SSR.  

### Install
Copy src/template_credentials.h to src/credentials.h and add your SSID and Password.  
Compile source in Microsoft Visual Studio Code with PlatformIO and upload to Lolin.  
Generate filesystem and upload to Lolin.

### Use
Browse to http://fooflow.local to reflow your awesome PCB.

Check src/reflowProfiles.h for solder paste profiles.
