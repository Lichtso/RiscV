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

typedef UInt64 AddressType; // TODO

template<UInt8 bits>
class Integer {
	public:
	typedef typename std::conditional<(bits > 32),
		typename std::conditional<(bits > 64), UInt128, UInt64>::type,
		typename std::conditional<(bits > 16), UInt32,
		typename std::conditional<(bits > 8), UInt16, UInt8>::type>::type
	>::type unsigned_type;

	typedef typename std::conditional<(bits > 32),
		typename std::conditional<(bits > 64), Int128, Int64>::type,
		typename std::conditional<(bits > 16), Int32,
		typename std::conditional<(bits > 8), Int16, Int8>::type>::type
	>::type signed_type;
};

template<typename unsigned_type>
constexpr unsigned_type TrailingBitMask(UInt8 len) {
	return (len == sizeof(unsigned_type)*8) ? -1 : (1<<len)-1;
}

template<typename unsigned_type>
unsigned_type getBitsFrom(unsigned_type data, UInt8 at, UInt8 len) {
	return (data>>at)&TrailingBitMask<unsigned_type>(len);
}

template<typename unsigned_type>
void setBitsIn(unsigned_type& in, unsigned_type data, UInt8 at, UInt8 len) {
	in &= (~TrailingBitMask<unsigned_type>(len))<<at;
	in |= (data&TrailingBitMask<unsigned_type>(len))<<at;
}

template<typename unsigned_type>
UInt8 clz(unsigned_type value) {
	// if(value == 0) return bits;
	if(sizeof(unsigned_type) <= 2)
		return __builtin_clzs(value);
	else if(sizeof(unsigned_type) <= 4)
		return __builtin_clz(value);
	else if(sizeof(unsigned_type) <= 8)
		return __builtin_clzll(value);
	else if(sizeof(unsigned_type) <= 16) {
		auto upper = static_cast<UInt128>(value)>>64;
		return (upper) ? __builtin_clzll(upper) : 64+__builtin_clzll(value&TrailingBitMask<unsigned_type>(64));
	}
}

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
