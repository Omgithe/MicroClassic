#include "Defines.h"

#include "DrawManager.h"

int main()
{
	DrawManager dm;

	if (dm.Initialize())
	{
		while (dm.ProcessMessages())
		{
			dm.OnPresent();
		}
	}
}