#include "Process.h"

#include <TlHelp32.h>
#include <Psapi.h>

Process::Process()
{
	Detach();
}

Process::~Process()
{
	Detach();
}

bool Process::Attach(const std::string& szName)
{
	Detach();

	if (szName.empty())
	{
		return false;
	}

	m_iProcessId = GetProcessIdByName(szName);

	if (!m_iProcessId)
	{
		m_hWindow = FindWindowA(NULL, szName.c_str());

		if (m_hWindow == NULL)
			return false;

		GetWindowThreadProcessId(m_hWindow, &m_iProcessId);

		if (!m_iProcessId)
			return false;
	}
	else
	{
		HWND hWindow = NULL;
		do
		{
			hWindow = FindWindowEx(NULL, hWindow, NULL, NULL);
			uint32 iProcID = 0;
			GetWindowThreadProcessId(hWindow, &iProcID);
			if (iProcID == m_iProcessId)
			{
				if (GetWindow(hWindow, GW_OWNER) == (HWND)0 && IsWindowVisible(hWindow))
				{
					m_hWindow = hWindow;
					break;
				}
			}
		} while (hWindow != NULL);

		if (m_hWindow == NULL)
		{
			return false;
		}
	}
	
	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_iProcessId);

	if (!m_hProcess)
		return false;


	if (!DumpModules())
		return false;

	m_pWndContainer = new WindowContainer();

	if (!m_pWndContainer->Init(m_hWindow))
	{
		SAFE_DELETE(m_pWndContainer);
		return false;
	}

	return true;
}

void Process::Detach()
{
	SAFE_DELETE(m_pWndContainer);

	if (m_hProcess)
	{
		CloseHandle(m_hProcess);
	}

	if (!m_mapModules.empty())
	{
		for (auto it : m_mapModules)
		{
			if (it.second != nullptr)
			{
				delete it.second;
				it.second = nullptr;
			}
		}
		m_mapModules.clear();
	}

	m_hProcess = NULL;
	m_hWindow = NULL;
	m_iProcessId = NULL;
}

bool Process::Read(uint64 iAddress, pvoid pBuffer, uint64 iSize)
{
	SIZE_T Out = NULL;
	return (ReadProcessMemory(m_hProcess, (pcvoid)(iAddress), pBuffer, iSize, &Out) == TRUE);
}

bool Process::Write(uint64 iAddress, pcvoid pBuffer, uint64 iSize)
{
	SIZE_T Out = NULL;
	return (WriteProcessMemory(m_hProcess, (pvoid)(iAddress), pBuffer, iSize, &Out) == TRUE);
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

	CloseHandle(hSnapshot);

	return NULL;
}

bool Process::DumpModules()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_iProcessId);

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
		m_mapModules.insert({ Entry.szModule, pMod });
	} while (Module32Next(hSnapshot, &Entry));

	CloseHandle(hSnapshot);

	return !m_mapModules.empty();
}

const Module* Process::GetModule(const std::string& szModName)
{
	if (m_mapModules.contains(szModName))
	{
		return m_mapModules[szModName];
	}
	return nullptr;
}

void Process::ProcessMessages()
{

	while (m_pWndContainer->ProcessMessages())
	{
		if (!IsWindow(m_hWindow))
			break;

		m_pWndContainer->GetOverlay()->TakeTargetShape(m_hWindow);

		m_pWndContainer->GetGraphics()->BeginScense();

		m_pWndContainer->GetGraphics()->EndScense();
	}
}