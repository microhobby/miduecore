/**
 * Castellino Duo Core API
 * Author: Matheus Castello
 * E-mail: matheus@castello.eng.br
 * Date: 19/04/2019
 **/

#ifndef CASTELLINO_H_
#define CASTELLINO_H_

#include "defconfig.h"
#include <Arduino.h>
#include <Wire.h>
#include "map.h"

#define coreCallBack		int (*)(int)

class Castellino
{
private:
	/* methods */
	void coreExecute(char* fun, int arg);
	void registerUtilsTasks(void);
	void checkEepromAndSetIRQ(void (* callBack)(void));

	/* properties */
	bool core1 = false;
	bool core2 = false;
	uint8_t pinIRQValue = 0x0;
	void (*coreIRQReceiveCallback)(void);
	void (*coreI2CReceiveCallback)(int);
	void (*coreI2CRequestCallback)(void);
	int arg0 = 0;
	int countCommands = 0;
	int (*coreExecNow)(int) = NULL;
	int taskPtr = 0;
	int taskCount = 0;
	int (*coreTasks[API_MAX_TASKS])(int);
	int coreTasksArgs[API_MAX_TASKS];
	HashType<const char*, coreCallBack> commandsType[API_MAX_CALLBACKS];
	HashType<coreCallBack, const char*> executionType[API_MAX_CALLBACKS];
	HashMap<const char*, coreCallBack> commands = 
		HashMap<const char*, coreCallBack>(commandsType,
			API_MAX_CALLBACKS);
	HashMap<coreCallBack, const char*> executions = 
		HashMap<coreCallBack, const char*>(executionType,
			API_MAX_CALLBACKS);

public:
	/* properties */
	volatile int core2Return = 0;
	void (*onCore2Return)(void) = nullptr;
	
	/* static methods */
	static void eventCoreReceived(int size, Castellino* obj);
	static void eventCoreRequest(Castellino* obj);
	static void coreRequestWriteReturn(Castellino* obj);
	static void eventCoreFree(int res, Castellino* obj,
		void (* callBack)(int));
	static int setPinModeOut(int pin);
	static int setPinModeInp(int pin);
	
	/* constructors */
	Castellino(void);
	Castellino(void (* callBackIRQ)(void), void (* callBackI2C)(int),
		void (* callBackI2CRequest)(void));
	~Castellino();

	/* methods */
	void init(void);
	void connectCores();
	void switchMasterSlave(void (*callBack)(int));
	int setPinMode(uint8_t pin, uint8_t mode);
	void addCommand(const char* name, int (*command)(int));
	void addExecTask(int (*command)(int), uint8_t core);
	void addExecTask(int (*command)(int), uint8_t core, int arg);
	void execOn(int (*command)(int), uint8_t core);
	void execOn(int (*command)(int), uint8_t core, int arg);
	void execOn(char* commands, uint8_t core);
	void execOn(char* commands, uint8_t core, int arg);
	void exec(void);
};

#endif
