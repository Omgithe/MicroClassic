#include <Windows.h>
#include "Process.h"

int main()
{
	Process p;
	if (p.Attach("World of Warcraft"))
	{
		std::cout << "attached by windows name" << std::endl;
	}

	p.Detach();

	if (p.Attach("WowClassic.exe"))
	{
		std::cout << "attached by .exe name" << std::endl;
	}

	p.Detach();

	return 0;
}