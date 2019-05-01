#include "castellino.h"
#include <EEPROM.h>

Castellino::Castellino(void (* callBack)(void))
{
	registerUtilsTasks();
	coreIRQReceiveCallback = callBack;
}

Castellino::Castellino(void (* callBackIRQ)(void), void (* callBackI2C)(int))
{
	registerUtilsTasks();
	coreIRQReceiveCallback = callBackIRQ;
	coreI2CReceiveCallback = callBackI2C;
}

void Castellino::init()
{
	checkEepromAndSetIRQ(coreIRQReceiveCallback);
	connectCores(coreI2CReceiveCallback);
}

void Castellino::registerUtilsTasks()
{
	/* add utils commands */
	addCommand("setPinModeOut", Castellino::setPinModeOut);
	addCommand("setPinModeInp", Castellino::setPinModeInp);
}

void Castellino::checkEepromAndSetIRQ(void (* callBack)(void))
{
	int ret;

	/* check if we are core1 or core2 */
	ret = EEPROM.read(0);
	if (ret == 23) {
		core1 = true;
		
		attachInterrupt(
			digitalPinToInterrupt(CORE_IRQ_RECEIVE),
			callBack, 
			RISING
		);
	} else {
		core2 = true;
		pinMode(CORE_IRQ, OUTPUT);
	}
}

void Castellino::connectCores(void (* callBack)(int))
{
	if (core1) {
		Wire.begin();
		delay(500);
	} else {
		Wire.begin(8);
		Wire.onReceive(coreI2CReceiveCallback);
		delay(200);
	}
}

void Castellino::switchMasterSlave(void (*callBack)(int))
{
	if (core2) {
		Wire.begin();
		delay(500);
	} else {
		Wire.begin(8);
		Wire.onReceive(callBack);
		delay(200);
	}
}

void Castellino::eventCoreReceived(int size, Castellino* obj)
{
	char c;
	char command[200];
	int ix = 0;

	while (1 < Wire.available()) {
		c = Wire.read();
		command[ix] = c;
		ix++;
	}
	command[ix] = '\0';

	/* read the arg */
	int arg = Wire.read();
	obj->coreExecute(command, arg);
}

void Castellino::eventCoreFree(int size, Castellino* obj, 
	void (* callBack)(int))
{
	char c;
	char command[100];
	int ix = 0;
	int res = 0;

	while (1 < Wire.available()) {
		c = Wire.read();
		command[ix] = c;
		ix++;
	}
	command[ix] = '\0';

	res = atoi(command);
	obj->core2Return = res;
	/* back core1 to master */
	obj->connectCores(callBack);
}

int Castellino::setPinModeOut(int pin)
{
	pinMode(pin, OUTPUT);
	return -1;
}

int Castellino::setPinModeInp(int pin)
{
	pinMode(pin, INPUT);
	return -1;
}

void Castellino::coreExecute(char* fun, int arg)
{
	if (commands.getIndexStr(fun) != -1) {
		coreExecNow = commands.getValueOfStr(fun);
		arg0 = arg;
	}
}

int Castellino::setPinMode(uint8_t pin, uint8_t mode)
{
	switch (pin)
	{
		/* execute on core2 */
		case 1:
			if (mode == OUTPUT)
				addExecTask(Castellino::setPinModeOut,
					CORE2, 9);
			else
				addExecTask(Castellino::setPinModeInp,
					CORE2, 9);
			return 9;
			break;
		case 4:
			if (mode == OUTPUT)
				addExecTask(Castellino::setPinModeOut,
					CORE2, 12);
			else
				addExecTask(Castellino::setPinModeInp,
					CORE2, 12);
			return 12;
			break;
		case 5:
			if (mode == OUTPUT)
				addExecTask(Castellino::setPinModeOut,
					CORE2, 13);
			else
				addExecTask(Castellino::setPinModeInp,
					CORE2, 13);
			return 13;
			break;
		/* execute on core1 */
		case 14:
			if (mode == OUTPUT)
				addExecTask(Castellino::setPinModeOut,
					CORE1, 13);
			else
				addExecTask(Castellino::setPinModeInp,
					CORE1, 13);
			return 13;
			break;
		default:
			return -1;
			break;
	}
}

void Castellino::addCommand(const char* name, int (*command)(int))
{
	commands[countCommands](name, command);
	executions[countCommands](command, name);
	countCommands++;
}

void Castellino::execOn(char* command, uint8_t core)
{
	execOn(command, core, arg0);
}

void Castellino::execOn(char* command, uint8_t core, int arg)
{
	if (commands.getIndexOf(command) != -1 && core1) {
		if (core == CORE1) {
			coreExecNow = commands.getValueOf(command);
			arg0 = arg;
		} else {
			Wire.beginTransmission(8);
			Wire.write(command);
			Wire.write(arg);
			Wire.endTransmission();
		}
	}
}

void Castellino::execOn(int (*command)(int), uint8_t core)
{
	execOn(command, core, arg0);
}

void Castellino::execOn(int (*command)(int), uint8_t core, int arg)
{
	if (executions.getIndexOf(command) != -1 && core1) {
		if (core == CORE1) {
			coreExecNow = command;
			arg0 = arg;
		} else {
			Wire.beginTransmission(8);
			Wire.write(executions.getValueOf(command));
			Wire.write(arg);
			Wire.endTransmission();
		}
	}
}

void Castellino::addExecTask(int (*command)(int), uint8_t core) 
{
	addExecTask(command, core, arg0);
}

void Castellino::addExecTask(int (*command)(int), uint8_t core, int arg) 
{
	if (CORE1 == core && core1) {
		coreTasks[taskCount] = command;
		coreTasksArgs[taskCount] = arg;
		taskCount++;
	}
	
	if (CORE2 == core && core2) {
		coreTasks[taskCount] = command;
		coreTasksArgs[taskCount] = arg;
		taskCount++;
	}
}

void Castellino::exec()
{
	int ret = -1;
	char retStr[100];

	if (coreExecNow) {
		ret = coreExecNow(arg0);

		if (core2) {
			/* send return */
			if (ret != -1) {
				/* convert return to string */
				itoa(ret, retStr, 10);

				/* actual function end set irq */
				digitalWrite(CORE_IRQ, HIGH);
				delay(100);
				digitalWrite(CORE_IRQ, LOW);

				/* core2 is master now */
				switchMasterSlave(NULL);

				/* transmit return */
				Wire.beginTransmission(8);
				Wire.write(retStr);
				Wire.write(ret);
				Wire.endTransmission();
				delay(100);

				/* back to slave */
				connectCores(coreI2CReceiveCallback);
			}
		}
	} else if (taskPtr < taskCount) {
		coreExecNow = coreTasks[taskPtr];
		arg0 = coreTasksArgs[taskPtr];
		taskPtr++;
		return;
	}

	coreExecNow = NULL;
}

Castellino::~Castellino()
{
}
