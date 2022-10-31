#pragma execution_character_set( "utf-8" )

#include "Process.h"
#include "Overlay.h"

int main()
{
	SetConsoleOutputCP(65001);
	try
	{
		Process p;
		Overlay o;

		if (!p.Attach("notepad.exe"))
			return 0;

		o.Attach(&p);

		while (o.ProcessMessages())
		{
			o.Tick();
		}
	}
	catch (std::exception e)
	{
		printf("%s\n", e.what());
	}
	return 0;
}