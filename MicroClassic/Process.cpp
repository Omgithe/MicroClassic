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

	return GetExeInfo();
}

void Process::Detach()
{
	if (m_hProcess)
	{
		CloseHandle(m_hProcess);
	}

	m_hProcess = NULL;
	m_dwProcessId = NULL;
	m_dwBase = NULL;
	m_dwSize = NULL;
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

bool Process::GetExeInfo()
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

	do
	{
		char szPath[MAX_PATH] = { NULL };
		GetModuleFileNameExA(m_hProcess, Entry.hModule, szPath, MAX_PATH);

		size_t len = strlen(Entry.szModule);
		if (Entry.szModule[len - 4] == '.' && Entry.szModule[len - 3] == 'e' && Entry.szModule[len - 2] == 'x' && Entry.szModule[len - 1] == 'e')
		{
			m_dwBase = (uint64)Entry.hModule;
			m_dwSize = (uint64)Entry.modBaseSize;
			CloseHandle(hSnapshot);
			return true;
		}

	} while (Module32Next(hSnapshot, &Entry));

	CloseHandle(hSnapshot);

	return false;
}