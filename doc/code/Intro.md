# Introduction to the code

This page is meant to guide you through the source code, so you can find the relevant files for what you're working on.

## FreeRTOS

Infinitime is based on FreeRTOS, a real-time operating system.
FreeRTOS provides several quality of life abstractions (for example easy software timers)
and most importantly supports multiple tasks.
If you want to read up on real-time operating systems, you can look [here](https://www.freertos.org/implementation/a00002.html) and [here](https://www.freertos.org/features.html).
The main "process" creates at least one task and then starts the FreeRTOS task scheduler.
This main "process" is the standard main() function inside [main.cpp](/src/main.cpp).
The task scheduler is responsible for giving every task enough cpu time.
As there is only one core on the SoC of the PineTime, real concurrency is impossible and the scheduler has to swap tasks in and out to emulate it.

### Tasks

Tasks are created by calling `xTaskCreate` and passing a function with the signature `void functionName(void*)`.
For more info on task creation see the [FreeRTOS Documentation](https://www.freertos.org/a00125.html).
In our case, main calls `systemTask.Start()`, which creates the **"MAIN" task**.
The function running inside that task is `SystemTask::Work()`.
You may also see this task being referred to as the **work task**.
Both functions are located inside [systemtask/SystemTask.cpp](/src/systemtask/SystemTask.cpp). `SystemTask::Work()` initializes all the driver and controller objects.
It also starts the **task "displayapp"**, which is responsible for launching and running apps, controlling the screen and handling touch events (or forwarding them to the active app).
You can find the "displayapp" task inside [displayapp/DisplayApp.cpp](/src/displayapp/DisplayApp.cpp).
There are also other tasks that are responsible for Bluetooth ("ll" and "ble" inside [libs/mynewt-nimble/porting/npl/freertos/src/nimble_port_freertos.c](/src/libs/mynewt-nimble/porting/npl/freertos/src/nimble_port_freertos.c))
and periodic tasks like heartrate measurements ([heartratetask/HeartRateTask.cpp](/src/heartratetask/HeartRateTask.cpp)).

While it is possible for you to create your own task when you need it, it is recommended to just add functionality to `SystemTask::Work()` if possible.
If you absolutely need to create another task, try to estimate how much [stack space](https://www.freertos.org/FAQMem.html#StackSize) (in words/4-byte packets)
it will need instead of just typing in a large-ish number.
You can use `configMINIMAL_STACK_SIZE` which is currently set to 120 words.

## Controllers

Controllers in InfiniTime are singleton objects that can provide access to certain resources to apps.
Some of them interface with drivers, others are the driver for the resource.
The resources provided don't have to be hardware-based.
They are declared in main.cpp and initialized in [systemtask/SystemTask.cpp](/src/systemtask/SystemTask.cpp).
Some controllers can be passed by reference to apps that need access to the resource (for example vibration motor).
They reside in [components/](/src/components/) inside their own subfolder.

## Apps

For more detail see the [Apps page](./Apps.md)

## Bluetooth

Header files with short documentation for the functions are inside [libs/mynewt-nimble/nimble/host/include/host/](/src/libs/mynewt-nimble/nimble/host/include/host/).

## Hardware abstraction and device drivers

Until version 1.12, InfiniTime did not provide any kind of hardware abstraction : the drivers were written specifically for the PineTime, and there was no easy way to provide any alternative implementation to support variants of the PineTime or for the integration in [InfiniSim](https://github.com/InfiniTimeOrg/InfiniSim).

In [InfiniTime 1.12](https://github.com/InfiniTimeOrg/InfiniTime/releases/tag/1.12.0), we implemented a new design that allows to easily choose **at build time** a specific implementation for multiple device drivers (Display, heart rate sensor, motion sensor, SPI, flash memory, touch panel, TWI and Watchdog).

This new design makes the code much cleaner in InfiniSim and allows the port of InfiniTime on other hardware (ex : many PineTime variants like the Colmi P8) more easily.

This hardware abstraction is based on C++ `concepts` and a proxy object that enables 'static polymorphism'. It means that the actual implementations of the drivers are known at **build time** and that there's no `virtual` calls at runtime. 

Here's an overview of the design :

```c++
namespace Pinetime {
  namespace Drivers  {
    template <typename DeviceImpl>
    concept IsDevice = { /* ... */ };
    
    namespace Interface {
      template <class T>
        requires IsDevice<T>
      class Device {
      public:
         explicit Device(T& impl) : impl {impl} {}
      /* ... */
      private:
        T& impl;
      };
    }

    using Device = Interface::Device<Pinetime::Drivers::SomeDevice::Device>;
  }
}

int main() {
  /* ... */
  
  Pinetime::Drivers::Category::Device deviceImpl { /* ctor arguments specific to this implementation of Device */ };
  Pinetime::Drivers::Device device {deviceImpl};
  
  /* ... */
}

```

The concept `Pinetime::Drivers::IsDevice` describes the interface a class that implements a `Device` must expose.

The template class `Pinetime::Drivers::Interface::Device` is the "proxy" objects that allows the build time polymorphism.

`Pinetime::Drivers::Device` is aliased to `Pinetime::Drivers::Interface::Device`. This allows to remove the template argument so that the rest of the application does not need to handle it. Those aliases are defined in header files located in `port/`. This is the only place where `#ifdef`s are needed.

The actual drivers are implemented in specific namespaces (`Pinetime::Drivers::MCU::Device` or `Pinetime::Drivers::Category::Device`).

To declare a new driver in the code, you'll first need to instantiate an actual implementation of the driver and then give the reference to this instance to the corresponding proxy object. Here is an example with the display driver:

```c++
// Actual implementation of the SPI bus for the NRF52 MCU
Pinetime::Drivers::Nrf52::SpiMaster spiImpl {Pinetime::Drivers::Nrf52::SpiMaster::SpiModule::SPI0,
                                  {Pinetime::Drivers::Nrf52::SpiMaster::BitOrder::Msb_Lsb,
                                   Pinetime::Drivers::Nrf52::SpiMaster::Modes::Mode3,
                                   Pinetime::Drivers::Nrf52::SpiMaster::Frequencies::Freq8Mhz,
                                   Pinetime::PinMap::SpiSck,
                                   Pinetime::PinMap::SpiMosi,
                                   Pinetime::PinMap::SpiMiso}};

// Proxy object 
Pinetime::Drivers::SpiMaster spi {spiImpl};

// Actual implementation of the SpiMaster drivers (it handles the chip select pin)
Pinetime::Drivers::Nrf52::Spi lcdSpiIpmpl {spiImpl, Pinetime::PinMap::SpiLcdCsn};

// Proxy object
Pinetime::Drivers::Spi lcdSpi {lcdSpiIpmpl};

// Actual implementation of the display driver (ST7789 display controller)
Pinetime::Drivers::Displays::St7789 lcdImpl {lcdSpi, Pinetime::PinMap::LcdDataCommand};

// Proxy object
Pinetime::Drivers::Display lcd{lcdImpl};

Pinetime::System::SystemTask systemTask(spi, lcd /* ... */);
```

Once the initialization is done, the application does not need to know the actual implementation of the drivers, all calls to the drivers will go through the proxy objects. 
