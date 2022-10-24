#pragma once

#include <Windows.h>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

typedef void* pvoid;
typedef const void* pcvoid;

typedef long long int64;
typedef long int32;
typedef short int16;
typedef signed char int8;

typedef unsigned long long uint64;
typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef float vec_t;

template <typename T>
inline void safeRelease(T& ptr)
{
    if (ptr)
    {
        ptr->Release();
        ptr = NULL;
    }
}

#define SAFE_RELEASE safeRelease

template <typename T>
inline void safeDelete(T& ptr)
{
    if (ptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

#define SAFE_DELETE safeDelete