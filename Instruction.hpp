#pragma once
#include "Exception.hpp"

class Instruction {
	public:
	UInt8 opcode, reg[4], funct[2];
	UInt32 imm;

	enum Type {
		R,
		R4,
		I,
		S,
		SB,
		U,
		UJ,
		Undefined
	};

	void decode32(UInt32 data);
	UInt32 encode32() const;
	Type getType() const;
};

UInt64 TrailingBitMask(UInt8 len);
UInt64 getBitsFrom(UInt64 data, UInt8 at, UInt8 len);
