#pragma once

#include "Module.h"

class Process
{
public:
	Process();
	~Process();

	bool Attach(const std::string& szExeName);
	void Detach();
	bool Read(uint64 dwAddress, pvoid lpBuffer, uint64 dwSize);
	bool Write(uint64 dwAddress, pcvoid lpBuffer, uint64 dwSize);

	template<typename T>
	inline T Read(uint64 dwAddress, const T& tDefault = T())
	{
		T tRet;
		if (!Read(dwAddress, &tRet, sizeof(T)))
			return tDefault;
		return tRet;
	}

	template<typename T>
	inline bool Write(uint64 dwAddress, const T& tValue) { return Write(dwAddress, &tValue, sizeof(T)); }

	template<typename T>
	inline bool WriteProtected(uint64 dwAddress, const T& tValue)
	{
		uint64 oldProtect;
		VirtualProtectEx(m_hProcess, (pvoid)dwAddress, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);
		Write(dwAddress, &tValue, sizeof(T));
		VirtualProtectEx(m_hProcess, (pvoid)dwAddress, sizeof(T), oldProtect, NULL);
		return true;
	}

	const Module* GetModule(const std::string& szModName);
private:
	uint32 GetProcessIdByName(const std::string& szExeName);
	bool DumpModules();

	uint32 m_dwProcessId = NULL;
	HANDLE m_hProcess = NULL;
	std::unordered_map<std::string, Module*> m_mapModDump;
};