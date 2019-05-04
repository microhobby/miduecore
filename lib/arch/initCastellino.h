/**
 * Castellino Duo Core API
 * Author: Matheus Castello
 * E-mail: matheus@castello.eng.br
 * Date: 19/04/2019
 **/

#ifndef INIT_CASTELLINO_H_
#define INIT_CASTELLINO_H_

#include <castellino.h>

void castellino_onCore2IRQ();
void castellino_onReceiveCommand(int size);
void castellino_onCore2Free();

/* new Castellino Core */
Castellino CORE(castellino_onCore2IRQ, castellino_onReceiveCommand,
			castellino_onCore2Free);

/* help callbacks have to be set on main */
/* the event listeners have to be defined on main */

/* core2 irq end of a function with return value */
void castellino_onCore2IRQ()
{
	Castellino::eventCoreRequest(&CORE);
}

/* I2C irq to receive tasks from core1 */
void castellino_onReceiveCommand(int size)
{
	Castellino::eventCoreReceived(size, &CORE);
}

/* core2 I2C irq to send data from task end to core1 */
void castellino_onCore2Free()
{
	Castellino::coreRequestWriteReturn(&CORE);
}

#endif
