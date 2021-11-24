#pragma once

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;


typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;

typedef uint32_t size_t;

typedef unsigned long uintptr_t;
typedef unsigned long ptr_t;

template<typename T1, typename T2>
constexpr bool matchEnum(T1 enumVal, T2 num) { return static_cast<T2>(enumVal) == num; }

template<typename R, typename T>
constexpr R orEnum(T a, T b) { return static_cast<R>(a) | static_cast<R>(b); }

template<typename R, typename T>
constexpr R andEnum(T a, T b) { return static_cast<R>(a) & static_cast<R>(b); }
