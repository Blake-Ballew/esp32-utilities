# esp32-utilities

The esp32-utilities library provides high-level software components designed to simplify and enhance the functionality of ESP32 microcontrollers. Whether you're developing IoT projects, embedded systems, or any application that leverages the power and versatility of the ESP32, this library offers a collection of utilities to streamline your development process. With comprehensive and easy-to-use modules, esp32-utilities helps you focus on building innovative solutions without getting bogged down by low-level details.

## Core Features

### Display Manager

The Display Manager handles graphical interfaces on ESP32 devices by reading inputIDs and system calls through a queue. Custom windows are created by inheriting from the parent window class, with each window supporting multiple states. These states can map inputIDs to system calls, switch to other states, or execute custom logic. Additionally, window states can output serialized data upon exiting and receive serialized data upon entering, enabling the sharing of common states, such as text or number editing, between windows. This provides a flexible and efficient way to manage display updates and interactions.

### Filesystem Manager

The Filesystem Manager is responsible for reading and writing files to and from the SPIFFS filesystem. It ensures efficient file operations, enabling your ESP32 projects to store and retrieve data seamlessly. Additionally, this module manages the settings file, providing a central place for configuration data. The settings file can be easily parsed and edited through the included Settings Window, streamlining the process of updating configurations and preferences.

### System Utils

The System Utils module oversees the device and is responsible for managing RTOS components, such as timers. It ensures efficient operation by handling critical system tasks and maintaining overall system health. The module monitors various parameters, including battery levels.

### LED Manager

The LED Manager is responsible for controlling WS2812B LEDs on the device. It enables dynamic and visually appealing LED animations by allowing other modules to register lighting functions. These functions are animated using system timers, ensuring smooth and synchronized lighting effects that enhance the overall user experience.

## Application Specific Features

### LoRa Network Manager

The LoRa Network Manager interfaces with a LoRa chip over a SPI connection, enabling devices to form a mesh network. This module facilitates the sending of Message_Base objects, which are serialized to MessagePack for efficient transmission. Upon receiving messages, the module triggers their corresponding InputID, which is then passed to the Display Manager's queue. This seamless integration allows for effective communication and coordination within the mesh network.

### Navigation Manager

The Navigation Manager interfaces with a UART GPS module and various compass modules, providing essential navigation capabilities. It accepts GPS coordinates and utilizes the device's current location and compass heading to create an LED compass with the help of the LED Manager. This LED compass visually guides users towards specified coordinates.

## Roadmap

- This code was originally written for the [Celestial Wayfinder](https://github.com/Blake-Ballew/Celestial-Wayfinder) project. These projects will become more decoupled as more work is done on each.
- Update code to use C++20 modules.
- LoRa Network Manager
  - Support for gRPC-like function calls using MessagePack.

