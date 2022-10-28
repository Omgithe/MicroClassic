#pragma once

#include "Module.h"

class Process
{
public:
	Process();
	~Process();

	bool Attach(const std::string& szName);
	void Detach();

	bool Read(uint64 iAddress, pvoid pBuffer, uint64 iSize);
	bool Write(uint64 iAddress, pcvoid pBuffer, uint64 iSize);

	template<typename T>
	inline T Read(uint64 iAddress, const T& tDefault = T())
	{
		T tRet;
		if (!Read(iAddress, &tRet, sizeof(T)))
			return tDefault;
		return tRet;
	}

	template<typename T>
	inline bool Write(uint64 iAddress, const T& tValue) { return Write(iAddress, &tValue, sizeof(T)); }

	template<typename T>
	inline bool WriteProtected(uint64 iAddress, const T& tValue)
	{
		uint64 oldProtect;
		VirtualProtectEx(m_hProcess, (pvoid)iAddress, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);
		Write(iAddress, &tValue, sizeof(T));
		VirtualProtectEx(m_hProcess, (pvoid)iAddress, sizeof(T), oldProtect, NULL);
		return true;
	}

	const Module* GetModule(const std::string& szModName);
	inline const HWND GetWindowHandle() { return m_hWindow; }
	void ProcessMessages();
private:
	uint32 GetProcessIdByName(const std::string& szExeName);
	bool DumpModules();

	uint32 m_iProcessId = NULL;
	HANDLE m_hProcess = NULL;
	HWND m_hWindow = NULL;

	std::unordered_map<std::string, Module*> m_mapModules;

	uint64 m_iCurrFrame = 0;
	uint64 m_iRPMCalls = 0;
	uint64 m_iWPMCalls = 0;
};