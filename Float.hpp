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
	typedef Int64 LengthType;
	const static ExponentType ExponentMax = TrailingBitMask<ExponentType>(exponentBits);
	const static ExponentType ExponentOffset = ExponentMax>>1;
	const static FieldType FieldMax = TrailingBitMask<FieldType>(fieldBits);

	RawType raw;

	bool getSign() {
		return getBitsFrom(raw, totalBits-1, 1);
	}

	ExponentType getExponent() {
		return getBitsFrom(raw, fieldBits, exponentBits);
	}

	FieldType getField() {
		return getBitsFrom(raw, 0, fieldBits);
	}

	void negate() {
		setSign(!getSign());
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

	void setNaN(bool signaling) {
		setExponent(ExponentMax);
		setField(TrailingBitMask<FieldType>(fieldBits-signaling));
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

	template<typename FactorType>
	void getNormalized(FactorType& factor, LengthType& exp) {
		static_assert(sizeof(factor) >= sizeof(FieldType), "FactorType to big to fit in FieldType");

		exp = getExponent();
		factor = getField();
		if(exp == 0)
			exp = -(fieldBits-1);
		else{
			factor |= 1<<fieldBits;
			LengthType shift = ctz<FactorType>(factor);
			factor >>= shift;
			exp += shift-fieldBits;
		}
	}

	template<typename FactorType>
	void setNormalized(UInt8& status, FloatRoundingMode round, FactorType factor, LengthType exp = ExponentOffset) {
		if(factor == 0) {
			setZero();
			return;
		}

		FieldType field;
		LengthType shift;
		FloatStatusFlag onRound = Inexact;

		if(exp <= 0) {
			shift = 1-fieldBits-exp;
			if(shift <= 0) {
				field = factor<<-shift;
				if(field < FieldMax) {
					setExponent(0);
					setField(field);
					return;
				}
			}else{
				onRound = static_cast<FloatStatusFlag>(onRound|Underflow);
				field = factor>>shift;
				if(field < FieldMax) {
					exp = 0;
					goto roundRest;
				}else
					factor = field;
			}
		}

		shift = sizeof(FactorType)*8-clz<FactorType>(factor)-1;
		exp += shift;
		shift -= fieldBits;

		if(shift <= 0)
			field = factor<<-shift;
		else{
			field = factor>>shift;

			roundRest:
			FactorType rest = factor&TrailingBitMask<FactorType>(shift);
			if(rest == 0)
				goto end;
			status |= onRound;
			FactorType treshold;
			switch(round) {
				case RoundNearest:
					treshold = (1<<(shift-1))-(field&1);
				break;
				case RoundMaxMagnitude:
					treshold = 0;
				break;
				case RoundMinMagnitude:
				default:
					goto end;
			}
			if(rest <= treshold)
				goto end;
			if((field&TrailingBitMask<FieldType>(fieldBits)) == FieldMax)
				++exp;
			++field;
		}

		end:
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

	template<typename FloatType>
	void setFloat(FloatType value) {
		raw = *reinterpret_cast<UInt32*>(&value);
	}

	template<typename FloatType>
	FloatType getFloat() {
		return *reinterpret_cast<FloatType*>(&raw);
	}

	FloatClass getClass() {
		return classify(getSign(), getExponent(), getField());
	}

	template<typename FloatType>
	void setFloat(UInt8& status, FloatRoundingMode round, FloatType other) {
		FieldType factor;
		LengthType exp;
		other.getNormalized(factor, exp);
		if(other.getSign()) {
			setSign(true);
			setNormalized<FieldType>(status, directRoundingMode(round, true), factor, exp);
		}else{
			setSign(false);
			setNormalized<FieldType>(status, directRoundingMode(round, false), factor, exp);
		}
	}

	template<typename UIntType>
	void setUInt(UInt8& status, FloatRoundingMode round, UIntType value) {
		setSign(false);
		setNormalized<UIntType>(status, directRoundingMode(round, false), value);
	}

	template<typename IntType>
	void setInt(UInt8& status, FloatRoundingMode round, IntType value) {
		if(value < 0) {
			setSign(true);
			setNormalized<IntType>(status, directRoundingMode(round, true), -value);
		}else{
			setSign(false);
			setNormalized<IntType>(status, directRoundingMode(round, false), value);
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
			setNaN(false);
			return;
		}

		if(classA == FloatClass::QuietNaN) {
			if(classB == FloatClass::QuietNaN)
				setNaN(false);
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

	template<bool invertSign>
	void sum(UInt8& status, FloatRoundingMode round, Float a, Float b) {
		FieldType factorA, factorB;
		LengthType expA, expB, exp;
		a.getNormalized(factorA, expA);
		b.getNormalized(factorB, expB);

		if(expA > expB) {
			exp = expA;
			factorB >>= std::min(static_cast<LengthType>(fieldBits), expA-expB); // TODO rounding
		}else{
			exp = expB;
			factorA >>= std::min(static_cast<LengthType>(fieldBits), expB-expA); // TODO rounding
		}

		bool signA = a.getSign(), signB = b.getSign()^invertSign;
		if(signA == signB) { // Add
			// TODO
			typename Integer<fieldBits+1>::unsigned_type factor = factorA+factorB;
			setNormalized(status, round, factor, exp);
			setSign(signA);
		}else{ // Subtract
			// TODO
			if(signA) {

			}else{

			}
		}
	}

	void product(UInt8& status, FloatRoundingMode round, Float a, Float b) {
		typename Integer<fieldBits*2>::unsigned_type factorA, factorB;
		LengthType expA, expB;
		a.getNormalized(factorA, expA);
		b.getNormalized(factorB, expB);
		setSign(a.getSign()^b.getSign());

		setNormalized(status, round, factorA*factorB, expA+expB-ExponentOffset);
	}

	void quotient(UInt8& status, FloatRoundingMode round, Float a, Float b) {
		typename Integer<fieldBits*2>::unsigned_type factorA, factorB;
		LengthType expA, expB;
		a.getNormalized(factorA, expA);
		b.getNormalized(factorB, expB);
		setSign(a.getSign()^b.getSign());

		if(factorB == 0) {
			status |= DivideByZero;
			if(factorA == 0)
				setNaN(false);
			else
				setInfinite();
			return;
		}

		LengthType shift = clz<decltype(factorA)>(factorA);
		factorA <<= shift;
		expA -= shift; // TODO : Prevent under/over flow
		setNormalized(status, round, factorA/factorB, expA-expB+ExponentOffset);
	}

	void sqrt(UInt8& status, FloatRoundingMode round, Float radicand) {
		// TODO
	}
};

typedef Float<5, 10> Float16;
typedef Float<8, 23> Float32;
typedef Float<11, 52> Float64;
//typedef Float<15, 112> Float128;
