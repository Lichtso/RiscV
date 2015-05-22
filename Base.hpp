#pragma once
#include <typeinfo>
#include <type_traits>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <set>
#include <map>

typedef unsigned char UInt8; typedef char Int8;
typedef unsigned short int UInt16; typedef short int Int16;
typedef unsigned int UInt32; typedef int Int32; typedef float Float32;
typedef unsigned long long int UInt64; typedef long long int Int64; typedef double Float64;

namespace std {
	template <bool condition, typename TrueType, TrueType trueValue, typename FalseType, FalseType falseValue>
	struct conditional_value : std::conditional<condition,
	std::integral_constant<TrueType, trueValue>,
	std::integral_constant<FalseType, falseValue>>::type
	{ };
};

static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

typedef UInt64 AddressType;
typedef UInt32 InstructionType;
