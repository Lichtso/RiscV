#pragma once
#include <typeinfo>
#include <type_traits>
#include <functional>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <inttypes.h>

typedef __uint8_t UInt8;
typedef __int8_t Int8;

typedef __uint16_t UInt16;
typedef __int16_t Int16;

typedef __uint32_t UInt32;
typedef __int32_t Int32;
typedef float Float32;

typedef __uint64_t UInt64;
typedef __int64_t Int64;
typedef double Float64;

typedef __uint128_t UInt128;
typedef __int128_t Int128;

namespace std {
	template<bool condition, typename TrueType, TrueType trueValue, typename FalseType, FalseType falseValue>
	struct conditional_value : std::conditional<condition,
	std::integral_constant<TrueType, trueValue>,
	std::integral_constant<FalseType, falseValue>>::type
	{ };

	template<class string>
	static inline string& trimFromBegin(string& s) {
    	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
    	return s;
	}

	template<class string>
	static inline string& trimFromEnd(string& s) {
    	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
    	return s;
	}

	template<class string>
	static inline string& trim(string& s) {
    	return trimFromBegin(trimFromEnd(s));
	}
}

typedef UInt64 AddressType;
