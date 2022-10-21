#include <Windows.h>
#include "Process.h"

int main()
{
	Process p;
	if (p.Attach("World of Warcraft"))
	{
		auto mptr = p.GetModule("WowClassic.exe");
		std::cout << "attached by windows name " << mptr->GetImage() << std::endl;
	}

	return 0;
}