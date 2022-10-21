#pragma once

#include "Define.h"

class Module
{
public:
	Module(uint64 iBase, uint64 iSize) : m_iBase(iBase), m_iSize(iSize) {}
	uint64 GetImage() const	{ return m_iBase; }
	uint64 GetSize() const { return m_iSize; }
private:
	uint64 m_iBase;
	uint64 m_iSize;
};