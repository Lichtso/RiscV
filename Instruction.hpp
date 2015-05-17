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

	Type getType() const {
		switch(opcode) {
			case 0x2F:
			case 0x33:
			case 0x3B:
			case 0x53:
			return R;
			case 0x43:
			case 0x47:
			case 0x4B:
			case 0x4F:
			return R4;
			case 0x03:
			case 0x07:
			case 0x0F:
			case 0x13:
			case 0x1B:
			case 0x67:
			case 0x73:
			return I;
			case 0x23:
			case 0x27:
			return S;
			case 0x63:
			return SB;
			case 0x17:
			case 0x37:
			return U;
			case 0x6F:
			return UJ;
		}
		throw Exception(Exception::Type::IllegalInstruction);
	}

	Instruction(UInt32 data);
	UInt32 encode() const;
};

UInt32 TrailingBitMask(UInt8 bits);
UInt32 LeadingBitMask(UInt8 bits);
