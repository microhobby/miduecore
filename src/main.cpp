#include <Arduino.h>
#include <initCastellino.h>

/* pins */
int led5;
int led4;
int led14;

/* user functions */
/* blink5 will be exec on Core2 */
int blink5(int arg)
{
	for (int i = 0; i < 4; i++) {
		digitalWrite(led5, HIGH);
		delay(1000);
		digitalWrite(led5, LOW);
		delay(1000);		
	}

	return 1000;
}

/* blink4 will be exec on Core2 */
int blink4(int arg)
{
	for (int i = 0; i < 10; i++) {
		digitalWrite(led4, HIGH);
		delay(arg);
		digitalWrite(led4, LOW);
		delay(arg);		
	}

	return 4;
}

/* blink14 will be exec on Core1 */
int blink14(int arg)
{
	CORE.core2Return = 100;

	while(1) {
		digitalWrite(led14, HIGH);
		delay(CORE.core2Return);
		digitalWrite(led14, LOW);
		delay(CORE.core2Return);
	}

	return -1;
}

void setup()
{
	/* core init */
	CORE.init();

	/* config pins */
	led5 = CORE.setPinMode(5, OUTPUT);
	led14 = CORE.setPinMode(14, OUTPUT);
	led4 = CORE.setPinMode(4, OUTPUT);
	
	/* register the functions */
	CORE.addCommand("blink5", blink5);
	CORE.addCommand("blink14", blink14);
	CORE.addCommand("blink4", blink4);

	/* Core1 queue tasks */
	CORE.addExecTask(blink14, CORE1);

	/* Core2 queue tasks */
	CORE.addExecTask(blink5, CORE2);
	CORE.addExecTask(blink4, CORE2, 200);

	/* and after the tasks on core2 are fineshed
		start blink5 again */
	CORE.onCore2Return = []() {
		if (CORE.core2Return == 4) {
			CORE.core2Return = 500;
			CORE.execOn(blink5, CORE2);
		}
	};
}

void loop()
{
	/* run tasks */
	CORE.exec();
}
