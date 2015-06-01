#pragma once
#include "Exception.hpp"

class Instruction {
	public:
	UInt8 opcode, reg[4], funct[2];
	UInt32 imm;

	enum Type {
		R, R4, I, S, SB, U, UJ,
		CR, CI, CSS, CIW, CL, CS, CB, CJ,
		CDS, CSD, CRI, CR3
	};

	void decode16(UInt16 data);
	UInt16 encode16() const;
	void decode32(UInt32 data);
	UInt32 encode32() const;
	Type getType() const;
};

UInt64 TrailingBitMask(UInt8 len);
UInt64 getBitsFrom(UInt64 data, UInt8 at, UInt8 len);
UInt64 setBitsAt(UInt64 data, UInt8 at, UInt8 len);
