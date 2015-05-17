#pragma once
#include <type_traits>
#include <functional>
#include <typeinfo>
#include <memory>
#include <map>

typedef unsigned char UInt8; typedef char Int8;
typedef unsigned short UInt16; typedef short Int16;
typedef unsigned UInt32; typedef int Int32; typedef float  Float32;
typedef unsigned long UInt64; typedef long int Int64; typedef double Float64;

namespace std {
	template <bool condition, typename TrueType, TrueType trueValue, typename FalseType, FalseType falseValue>
	struct conditional_value : std::conditional<condition,
	std::integral_constant<TrueType, trueValue>,
	std::integral_constant<FalseType, falseValue>>::type
	{ };
};

typedef UInt64 AddressType;
typedef UInt32 InstructionType;
