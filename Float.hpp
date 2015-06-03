#pragma once
#include "Base.hpp"

enum FloatRoundingMode {
	RoundNearest = 0,
	RoundMinMagnitude = 1,
	RoundDown = 2,
	RoundUp = 3,
	RoundMaxMagnitude = 4
};

enum {
	Inexact = 1U<<0,
	Underflow = 1U<<1,
	Overflow = 1U<<2,
	DivideByZero = 1U<<3,
	InvalidOperation = 1U<<4
};

enum FloatClass {
	NegativeInfinity = 0,
	NegativeNormal = 1,
	NegativeSubnormal = 2,
	NegativeZero = 3,
	PositiveZero = 4,
	PositiveSubnormal = 5,
	PositiveNormal = 6,
	PositiveInfinity = 7,
	SignalingNaN = 8,
	QuietNaN = 9
};

enum FloatComparison {
	Greater = 0,
	Less = 1,
	Equal = 2,
	Unordered = 3
};

// template<UInt8 exponentBits, UInt8 fieldBits>
class Float {
	public:
	const static UInt8 exponentBits = 8, fieldBits = 23, totalBits = 1+exponentBits+fieldBits;
	typedef typename Integer<totalBits>::unsigned_type RawType;
	typedef typename Integer<exponentBits>::unsigned_type ExponentType;
	typedef typename Integer<fieldBits>::unsigned_type FieldType;
	const static ExponentType ExponentMax = TrailingBitMask<ExponentType>(exponentBits);
	const static ExponentType ExponentOffset = ExponentMax>>1;

	RawType raw;

	//private:
	template<typename UIntType>
	void setInteger(UInt8& status, FloatRoundingMode round, UIntType value) {
		const UInt8 bits = sizeof(UIntType)*8;
		if(value == 0)
			setZero();
		else{
			auto leadingZeros = clz<UIntType>(value);
			auto len = bits-leadingZeros-1;
			if(ExponentOffset+len > ExponentMax) {
				status |= Overflow;
				setExponent(ExponentMax);
			}else
				setExponent(ExponentOffset+len);
			if(len < fieldBits)
				setField(value<<(fieldBits-len));
			else{
				len -= fieldBits;
				if(len) {
					if(value&TrailingBitMask<UIntType>(len))
						status |= Inexact;
					bool rest = (value>>(len-1))&1;
					value >>= len;
					value += rest&value&1;
				}
				// TODO : Debug Rounding and add other modi
				setField(value);
			}
		}
	}

	template<typename UIntType>
	UIntType getInteger(UInt8& status, FloatRoundingMode round) {
		const UInt8 bits = sizeof(UIntType)*8;
		bool sign = getSign();
		switch(round) {
			case RoundDown:
				round = (sign) ? RoundMaxMagnitude : RoundMinMagnitude;
			break;
			case RoundUp:
				round = (sign) ? RoundMinMagnitude : RoundMaxMagnitude;
			break;
			default:;
		}
		bool toInfinity = false;
		ExponentType exp = getExponent();
		FieldType field = getField();
		if(exp == ExponentMax) {
			if(getBitsFrom(field, fieldBits-1, 1) == 0)
				status |= InvalidOperation;
			else
				status |= Overflow;
			return TrailingBitMask<UIntType>(bits-1);
		}else if(exp < ExponentOffset) {
			if(round == RoundMinMagnitude)
				return 0;
			if(round == RoundMaxMagnitude)
				return (exp == 0 && field == 0) ? 0 : 1;
			return (exp == ExponentOffset-1) ? 1 : 0;
		}
		exp -= ExponentOffset;
		UIntType value = field>>(fieldBits-exp-1);
		if(round != RoundMinMagnitude) value += value&TrailingBitMask<UIntType>(1); // TODO : Check rounding
		return (1U<<exp)|(value>>1);
	}

	static FloatClass classify(bool sign, ExponentType exp, FieldType field) {
		if(exp == ExponentMax) {
			if(field == 0)
				return (sign) ? FloatClass::NegativeInfinity : FloatClass::PositiveInfinity;
			else
				return (getBitsFrom(field, fieldBits-1, 1)) ? FloatClass::QuietNaN : FloatClass::SignalingNaN;
		}else if(exp == 0) {
			if(field == 0)
				return (sign) ? FloatClass::NegativeZero : FloatClass::PositiveZero;
			else
				return (sign) ? FloatClass::NegativeSubnormal : FloatClass::PositiveSubnormal;
		}else
			return (sign) ? FloatClass::NegativeNormal : FloatClass::PositiveNormal;
	}

	public:
	bool isNaN() {
		return getExponent() == ExponentMax && getField() != 0;
	}

	bool isInfinite() {
		return getExponent() == ExponentMax && getField() == 0;
	}

	bool isSubnormal() {
		return getExponent() == 0 && getField() != 0;
	}

	bool isZero() {
		return getExponent() == 0 && getField() == 0;
	}

	FloatClass getClass() {
		return classify(getSign(), getExponent(), getField());
	}

	bool getSign() {
		return getBitsFrom(raw, totalBits-1, 1);
	}

	ExponentType getExponent() {
		return getBitsFrom(raw, fieldBits, exponentBits);
	}

	FieldType getField() {
		return getBitsFrom(raw, 0, fieldBits);
	}

	void setSign(bool sign) {
		setBitsIn(raw, static_cast<RawType>(sign), totalBits-1, 1);
	}

	void setExponent(ExponentType exponent) {
		setBitsIn(raw, static_cast<RawType>(exponent), fieldBits, exponentBits);
	}

	void setField(FieldType field) {
		setBitsIn(raw, static_cast<RawType>(field), 0, fieldBits);
	}

	void setQuietNaN() {
		setExponent(ExponentMax);
		setField(1U<<(fieldBits-1));
	}

	void setSignalingNaN() {
		setExponent(ExponentMax);
		setField(1);
	}

	void setInfinite() {
		setExponent(ExponentMax);
		setField(0);
	}

	void setZero() {
		setExponent(0);
		setField(0);
	}

	void setOne() {
		setExponent(ExponentOffset);
		setField(0);
	}

	template<typename UIntType>
	void setUInt(UInt8& status, FloatRoundingMode round, UIntType value) {
		setSign(0);
		setInteger<UIntType>(status, round, value);
	}

	template<typename IntType>
	void setInt(UInt8& status, FloatRoundingMode round, IntType value) {
		if(value < 0) {
			setSign(1);
			setInteger<IntType>(status, round, -value);
		}else{
			setSign(0);
			setInteger<IntType>(status, round, value);
		}
	}

	template<typename UIntType>
	UIntType getUInt(UInt8& status, FloatRoundingMode round) {
		UIntType value = getInteger<UIntType>(status, round);
		if(getSign()) {
			if(value > 0)
				status |= Underflow;
			return 0;
		}else
			return value;
	}

	template<typename IntType>
	IntType getInt(UInt8& status, FloatRoundingMode round) {
		IntType value = getInteger<IntType>(status, round);
		if(value < 0)
			status |= Overflow;
		return (getSign()) ? -value : value;
	}

	static FloatComparison compare(UInt8& status, Float a, Float b) {
		bool signA = a.getSign(), signB = b.getSign();
		ExponentType expA = a.getExponent(), expB = b.getExponent();
		FieldType fieldA = a.getField(), fieldB = b.getField();
		FloatClass classA = classify(signA, expA, fieldA), classB = classify(signB, expB, fieldB);

		switch(classA) {
			case NegativeInfinity:
				if(classB == NegativeInfinity)
					return FloatComparison::Equal;
				return FloatComparison::Less;
			case NegativeZero:
			case PositiveZero:
				if(classB == NegativeZero || classB == PositiveZero)
					return FloatComparison::Equal;
			case NegativeSubnormal:
			case PositiveSubnormal:
			case NegativeNormal:
			case PositiveNormal:
				break;
			case PositiveInfinity:
				if(classB == PositiveInfinity)
					return FloatComparison::Equal;
				return FloatComparison::Greater;
			case SignalingNaN:
				status |= InvalidOperation;
			case QuietNaN:
				return FloatComparison::Unordered;
		}

		switch(classB) {
			case NegativeInfinity:
				if(classA == NegativeInfinity)
					return FloatComparison::Equal;
				return FloatComparison::Greater;
			case NegativeZero:
			case PositiveZero:
				return static_cast<FloatComparison>(signA);
			case NegativeSubnormal:
			case PositiveSubnormal:
			case NegativeNormal:
			case PositiveNormal:
				break;
			case PositiveInfinity:
				if(classA == PositiveInfinity)
					return FloatComparison::Equal;
				return FloatComparison::Less;
			case SignalingNaN:
				status |= InvalidOperation;
			case QuietNaN:
				return FloatComparison::Unordered;
		}

		if(a.raw == b.raw)
			return FloatComparison::Equal;

		if(signA != signB)
			return static_cast<FloatComparison>(signA);

		if(expA == expB)
			return static_cast<FloatComparison>((fieldA<fieldB)^signA);

		return static_cast<FloatComparison>((expA<expB)^signA);
	}

	/*static Float add(Float a, Float b) {
		bool signA = a.getSign(), signB = b.getSign();
		ExponentType expA = a.getExponent(), expB = b.getExponent();
		FieldType fieldA = a.getField(), fieldB = b.getField();

		Integer<exponentBits>::signed_type expDiff = expA-expB;

		Float result;
		if(signA == signB) { // Add

			result.setSign(signA);
		}else{ // Substract

		}

		return result;
	}*/
};

/*Float<5, 10> Float16;
Float<8, 23> Float32;
Float<11, 52> Float64;
Float<15, 112> Float128;*/
