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
void castellino_onCore2Free(int size);

Castellino CORE(castellino_onCore2IRQ, castellino_onReceiveCommand);
//Castellino CORE(castellino_onCore2IRQ);

/* help callbacks have to be set on main */
/* the event listeners have to be defined on main */
void castellino_onReceiveCommand(int size)
{
	Castellino::eventCoreReceived(size, &CORE);
}

void castellino_onCore2Free(int size)
{
	Castellino::eventCoreFree(size, &CORE, castellino_onReceiveCommand);
}

void castellino_onCore2IRQ()
{

	CORE.switchMasterSlave(castellino_onCore2Free);
}

#endif
