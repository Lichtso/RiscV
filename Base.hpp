#pragma once
#include <type_traits>
#include <functional>
#include <iostream>
#include <typeinfo>
#include <memory>
#include <vector>
#include <set>
#include <map>

typedef unsigned char UInt8; typedef char Int8;
typedef unsigned short int UInt16; typedef short int Int16;
typedef unsigned long int UInt32; typedef long int Int32; typedef float Float32;
typedef unsigned long long int UInt64; typedef long long int Int64; typedef double Float64;

namespace std {
	template <bool condition, typename TrueType, TrueType trueValue, typename FalseType, FalseType falseValue>
	struct conditional_value : std::conditional<condition,
	std::integral_constant<TrueType, trueValue>,
	std::integral_constant<FalseType, falseValue>>::type
	{ };
};

typedef UInt64 AddressType;
typedef UInt32 InstructionType;
