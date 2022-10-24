#pragma execution_character_set( "utf-8" )

#include <Windows.h>
#include "Process.h"

int main()
{
	Process p;

	if (p.Attach("WowClassic.exe"))
	{
		p.ProcessMessages();
	}
	return 0;
}