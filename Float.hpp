#pragma once
#include "Base.hpp"

enum FloatRoundingMode {
	RoundNearest = 0,
	RoundMinMagnitude = 1,
	RoundDown = 2,
	RoundUp = 3,
	RoundMaxMagnitude = 4,
	RoundDynamic = 7
};

enum FloatStatusFlag {
	Inexact = 1U<<0,
	Underflow = 1U<<1,
	Overflow = 1U<<2,
	DivideByZero = 1U<<3,
	InvalidOperation = 1U<<4
};

enum FloatClass {
	NegativeInfinity = 1U<<0,
	NegativeNormal = 1U<<1,
	NegativeSubnormal = 1U<<2,
	NegativeZero = 1U<<3,
	PositiveZero = 1U<<4,
	PositiveSubnormal = 1U<<5,
	PositiveNormal = 1U<<6,
	PositiveInfinity = 1U<<7,
	SignalingNaN = 1U<<8,
	QuietNaN = 1U<<9
};

enum FloatComparison {
	Greater = 0,
	Less = 1,
	Equal = 2,
	Unordered = 3
};

template<UInt8 exponentBits, UInt8 fieldBits>
class Float {
	public:
	const static UInt8 totalBits = 1+exponentBits+fieldBits;
	typedef typename Integer<totalBits>::unsigned_type RawType;
	typedef typename Integer<exponentBits>::unsigned_type ExponentType;
	typedef typename Integer<fieldBits>::unsigned_type FieldType;
	const static ExponentType ExponentMax = TrailingBitMask<ExponentType>(exponentBits);
	const static ExponentType ExponentOffset = ExponentMax>>1;
	const static FieldType FieldMax = TrailingBitMask<FieldType>(fieldBits);

	RawType raw;

	//private:
	static FloatRoundingMode directRoundingMode(FloatRoundingMode round, bool sign) {
		switch(round) {
			case RoundDown:
				return (sign) ? RoundMaxMagnitude : RoundMinMagnitude;
			case RoundUp:
				return (sign) ? RoundMinMagnitude : RoundMaxMagnitude;
			default:
				return round;
		}
	}

	template<typename UIntType>
	void setInteger(UInt8& status, FloatRoundingMode round, UIntType value) {
		if(value == 0) {
			setZero();
			return;
		}

		const UInt8 bits = sizeof(UIntType)*8;
		auto leadingZeros = clz<UIntType>(value);
		auto len = bits-leadingZeros-1;
		auto exp = ExponentOffset+len;

		FieldType field;
		if(len < fieldBits)
			field = value<<(fieldBits-len);
		else{
			len -= fieldBits;
			field = value>>len;
			if(len) {
				UIntType rest = value&TrailingBitMask<UIntType>(len);
				if(rest) {
					status |= Inexact;
					UIntType treshold;
					switch(round) {
						case RoundNearest:
							treshold = (1<<(len-1))-(field&1);
						break;
						case RoundMaxMagnitude:
							treshold = 0;
						break;
						case RoundMinMagnitude:
						default:
							treshold = 1<<len;
						break;
					}
					if(rest > treshold) {
						field &= TrailingBitMask<FieldType>(fieldBits);
						if(field == FieldMax) {
							field = 0;
							++exp;
						}else
							++field;
					}
				}
			}
		}

		if(exp >= ExponentMax) {
			status |= Overflow;
			setInfinite();
		}else{
			setExponent(exp);
			setField(field);
		}
	}

	template<typename UIntType>
	UIntType getInteger(UInt8& status) {
		const UInt8 bits = sizeof(UIntType)*8;
		ExponentType exp = getExponent();
		FieldType field = getField();

		if(exp < ExponentOffset)
			return 0;

		if(exp == ExponentMax) {
			if(field != 0)
				status |= InvalidOperation;
			status |= Overflow;
			return 0;
		}

		UIntType value;
		exp -= ExponentOffset;
		if(exp < fieldBits)
			value = field>>(fieldBits-exp);
		else if(exp < bits)
			value = static_cast<UIntType>(field)<<(exp-fieldBits);
		else{
			status |= Overflow;
			return 0;
		}

		return (1ULL<<exp)|value;
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
	/*bool isNaN() {
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
	}*/

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
		setField(TrailingBitMask<FieldType>(fieldBits));
	}

	void setSignalingNaN() {
		setExponent(ExponentMax);
		setField(TrailingBitMask<FieldType>(fieldBits-1));
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
		setSign(false);
		round = directRoundingMode(round, false);
		setInteger<UIntType>(status, round, false, value);
	}

	template<typename IntType>
	void setInt(UInt8& status, FloatRoundingMode round, IntType value) {
		if(value < 0) {
			setSign(true);
			round = directRoundingMode(round, true);
			setInteger<IntType>(status, round, -value);
		}else{
			setSign(false);
			round = directRoundingMode(round, false);
			setInteger<IntType>(status, round, value);
		}
	}

	template<typename UIntType>
	UIntType getUInt(UInt8& status) {
		UIntType value = getInteger<UIntType>(status);
		if(getSign()) {
			if(value > 0)
				status |= Overflow;
			return 0;
		}else
			return value;
	}

	template<typename IntType>
	IntType getInt(UInt8& status) {
		IntType value = getInteger<IntType>(status);
		if(value < 0)
			status |= Overflow;
		return (getSign()) ? -value : value;
	}

	template<bool signaling>
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
				return FloatComparison::Unordered;
			case QuietNaN:
				if(signaling)
					status |= InvalidOperation;
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
				return FloatComparison::Unordered;
			case QuietNaN:
				if(signaling)
					status |= InvalidOperation;
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

	template<FloatComparison type>
	void setExtremum(UInt8& status, Float a, Float b) {
		FloatClass classA = a.getClass(), classB = b.getClass();
		if(classA == FloatClass::SignalingNaN || classB == FloatClass::SignalingNaN) {
			setQuietNaN();
			return;
		}

		if(classA == FloatClass::QuietNaN) {
			if(classB == FloatClass::QuietNaN)
				setQuietNaN();
			else
				*this = b;
		}else{
			if(classB == FloatClass::QuietNaN)
				*this = a;
			else{
				auto cmp = compare<false>(status, a, b);
				if(cmp == type)
					*this = a;
				else
					*this = b;
			}
		}
	}

	/*static Float add(UInt8& status, Float a, Float b) {
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

typedef Float<5, 10> Float16;
typedef Float<8, 23> Float32;
typedef Float<11, 52> Float64;
typedef Float<15, 112> Float128;
