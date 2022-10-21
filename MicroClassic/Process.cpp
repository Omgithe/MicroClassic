#include "Process.h"

#include <TlHelp32.h>
#include <Psapi.h>

Process::Process()
{

}

Process::~Process()
{
	Detach();
}

bool Process::Attach(const std::string& szExeName)
{
	Detach();

	if (szExeName.empty())
	{
		return false;
	}

	m_dwProcessId = GetProcessIdByName(szExeName);

	if (!m_dwProcessId)
	{
		HWND hWnd = FindWindowA(NULL, szExeName.c_str());

		if (hWnd == NULL)
			return false;

		GetWindowThreadProcessId(hWnd, &m_dwProcessId);

		if (!m_dwProcessId)
			return false;
	}

	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwProcessId);

	if (!m_hProcess)
	{
		return false;
	}

	return DumpModules();
}

void Process::Detach()
{
	if (m_hProcess)
	{
		CloseHandle(m_hProcess);
	}

	if (!m_mapModDump.empty())
	{
		for (auto it : m_mapModDump)
		{
			if (it.second != nullptr)
			{
				delete it.second;
				it.second = nullptr;
			}
		}
		m_mapModDump.clear();
	}

	m_hProcess = NULL;
	m_dwProcessId = NULL;
}

bool Process::Read(uint64 dwAddress, pvoid lpBuffer, uint64 dwSize)
{
	SIZE_T Out = NULL;
	return (ReadProcessMemory(m_hProcess, (pcvoid)(dwAddress), lpBuffer, dwSize, &Out) == TRUE);
}

bool Process::Write(uint64 dwAddress, pcvoid lpBuffer, uint64 dwSize)
{
	SIZE_T Out = NULL;
	return (WriteProcessMemory(m_hProcess, (pvoid)(dwAddress), lpBuffer, dwSize, &Out) == TRUE);
}

DWORD Process::GetProcessIdByName(const std::string& szExeName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	PROCESSENTRY32 Entry = { NULL };

	Entry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &Entry))
	{
		CloseHandle(hSnapshot);
		return NULL;
	}

	do
	{
		if (szExeName.compare(Entry.szExeFile) == 0)
		{
			CloseHandle(hSnapshot);
			return Entry.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &Entry));

	return NULL;
}

bool Process::DumpModules()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_dwProcessId);

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	MODULEENTRY32 Entry = { NULL };

	Entry.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(hSnapshot, &Entry))
	{
		CloseHandle(hSnapshot);
		return false;
	}

	Module* pMod = nullptr;
	do
	{
		char szPath[MAX_PATH] = { NULL };
		GetModuleFileNameExA(m_hProcess, Entry.hModule, szPath, MAX_PATH);


		pMod = new Module((uint64)Entry.hModule, (uint64)Entry.modBaseSize);
		m_mapModDump.insert({ Entry.szModule, pMod });
	} while (Module32Next(hSnapshot, &Entry));

	CloseHandle(hSnapshot);

	return !m_mapModDump.empty();
}

const Module* Process::GetModule(const std::string& szModName)
{
	if (m_mapModDump.contains(szModName))
	{
		return m_mapModDump[szModName];
	}
	return nullptr;
}