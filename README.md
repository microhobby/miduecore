# Miduecore - Dual Core Arduino Prototype

This is my Arduino Dual Core prototype:

<p align="center">
	<img src="./fritzing/miduecore_bb.png"/>
</p>

Two ATMega328P using the same power source and clock.
For intercom uses I2C and interrupts to notify the master core
that there is data to be shared.

## Development - Compilation

For the development of the project source code I am using Visual Studio
Code with the extension of PlatformIO.

## Flash

Use an USBasp programmer.

For the correct functioning of the application we have to define one of the
microcontrollers as the Master, core1. It will have the power to
request new tasks to be performed on core2.

To flash a microcontroller as master use the command:

```bash
avrdude -v \
	-patmega328p \
	-cusbasp \
	-Pusb \
	-Uflash:w:.pioenvs/uno/firmware.hex:i \
	-Ueeprom:w:scripts/eeprom.hex:i
```

To flash a microcontroller as slave use the command:

```bash
avrdude -v \
	-patmega328p \
	-cusbasp \
	-Pusb \
	-Uflash:w:.pioenvs/uno/firmware.hex:i
```
Remembering that the two microcontrollers will have the same code, the only thing
that differs them is a magic number inserted into the EEPROM of the core that will work
as Master.

## SDK

For the transparent programming of the microcontrollers an SDK was developed.
So leaving the programmer with the feeling of developing for a single
device.

* **CORE.init()**
	* In this method the EEPROM is checked and the firmware discovers
	if it is running on core1 or core2. It also initialize interrupts and
	I2C connections between cores.

* **CORE.setPinMode(pin, mode)**
	* We can use pins of the two cores. So instead of using the standard pinMode function of the Arduino library, this method was implemented. Under the table it converts the "dual core" numbering to the actual core pin numbering. Another important point is that the pinMode must be executed in the actual core where the pin is, so this method also includes the pinMode to the pin core task queue.

* **CORE.addCommand("function", function)**
	* This method registers the function pointer and the string of its name to a hashmap inside the SDK. So, core1 can for example send the name of a function, in string, via I2C to be executed by core2, which will fetch the function pointer in its hashmap by name. It is at this point that there is the advantage of having the same firmware on both microcontrollers. The programming of the cores becomes transparent because the two microcontrollers have the same functions. The SDK user gets the feeling of programming for a single device.

* **CORE.addExecTask(function, core, argument)**
	* This method adds the pointer of a function to the task queue of a given core passed as parameter: *CORE1* or *CORE2*. The method expects us to pass a function with the signature *void name(int argName)* so we can still pass a last argument, of integer type, to this method that will be the argument passed when the task exits the queue and executes. Important: use this method only inside *setup()* because it does not intercommunicate between cores. Inside *setup()* it is useful, so with that we can already include tasks in the execution queues of the cores during "boot".

* **CORE.onCore2Return**
	* This property expects to receive a function pointer, or a lambda expression, which will be executed when core2 has returned, triggered the interruption of core1, from some task/function with return value other than -1.

* **CORE.execOn(function, core)**
	* This method adds a function to the task queue of a given core, passed as a parameter: *CORE1* or *CORE2*. The difference here, related to *CORE.addExecTask*, is that this method should be used outside the scope of *setup()*.

* **CORE.exec()**
	* This method runs the task queue of each core on which it will be running. Always leave this method inside the *loop()* because when the core is free, that is, it has performed all its tasks, it is always checking for new tasks sent to the queue.

## Exemplo

Nesse repositório temos um exemplo da utilização do SDK para o Arduino Dual Core.

Cada função é uma tarefa e o valor retornado da função, se for diferente de -1,
será compartilhado entre os núcleos.

Each function is a task and the value returned from the function, if it is different from -1,
will be shared between the cores.

<p align="center">
	<img src="./res/blink14.png"/>
</p>

<p align="center">
	<img src="./res/blink5.png"/>
</p>

<p align="center">
	<img src="./res/blink4.png"/>
</p>

With the functions/tasks defined we can use the SDK to mount
execution queues, we can also tell in which core each task will be executed:

<p align="center">
	<img src="./res/miduecore_sections.png"/>
</p>

### Example in action - Link Youtube video

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/cl0sgdi3blo/0.jpg)](https://www.youtube.com/watch?v=cl0sgdi3blo)
