#include "Instruction.hpp"

UInt64 TrailingBitMask(UInt8 len) {
	return 0xFFFFFFFFFFFFFFFFULL>>(64-len);
	// return ~(0xFFFFFFFFFFFFFFFFULL<<len);
}

UInt64 getBitsFrom(UInt64 data, UInt8 at, UInt8 len) {
	return (data>>at)&TrailingBitMask(len);
}

InstructionType readBitsFrom(InstructionType& data, UInt8 bits) {
	InstructionType result = data&TrailingBitMask(bits);
	data >>= bits;
	return result;
}

void writeBitsTo(InstructionType& data, UInt8 bits, InstructionType content) {
	data <<= bits;
	data |= content;
}

InstructionType movedBitsFromTo(InstructionType data, UInt8 bits, UInt8 from, UInt8 to) {
	data = (from > to)
		? data>>(from-to)
		: data<<(to-from);
	return data&(TrailingBitMask(bits)<<to);
}

void writeTruncatedBitsTo(InstructionType& data, UInt8 bits, InstructionType content) {
	data <<= bits;
	data |= content&TrailingBitMask(bits);
}

void decodeTypeR(Instruction& self, InstructionType data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.reg[2] = readBitsFrom(data, 5);
	self.funct[1] = readBitsFrom(data, 7);
}

void decodeTypeR4(Instruction& self, InstructionType data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.reg[2] = readBitsFrom(data, 5);
	self.funct[1] = readBitsFrom(data, 2);
	self.reg[3] = readBitsFrom(data, 5);
}

void decodeTypeI(Instruction& self, InstructionType data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.imm = readBitsFrom(data, 12);
	self.imm |= data*~TrailingBitMask(20);
}

void decodeTypeS(Instruction& self, InstructionType data) {
	self.imm = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.reg[2] = readBitsFrom(data, 5);
	self.imm |= readBitsFrom(data, 7)<<5;
	self.imm |= data*~TrailingBitMask(20);
}

void decodeTypeSB(Instruction& self, InstructionType data) {
 	decodeTypeS(self, data);
	self.imm |= movedBitsFromTo(self.imm, 1, 0, 11);
	self.imm &= ~(TrailingBitMask(1));
}

void decodeTypeU(Instruction& self, InstructionType data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.imm = readBitsFrom(data, 20)<<12;
}

void decodeTypeUJ(Instruction& self, InstructionType data) {
	decodeTypeU(self, data);
	self.imm |= movedBitsFromTo(self.imm, 1, 20, 11);
	self.imm |= movedBitsFromTo(self.imm, 10, 21, 1);
	self.imm = (self.imm&TrailingBitMask(20))|((self.imm>>31)*~TrailingBitMask(20));
}

void decodeTypeUndefined(Instruction& self, InstructionType data) {

}

InstructionType encodeTypeR(const Instruction& self) {
	InstructionType data = 0;
	writeBitsTo(data, 7, self.funct[1]);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

InstructionType encodeTypeR4(const Instruction& self) {
	InstructionType data = 0;
	writeBitsTo(data, 5, self.reg[3]);
	writeBitsTo(data, 2, self.funct[1]);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

InstructionType encodeTypeI(const Instruction& self) {
	InstructionType data = 0;
	writeTruncatedBitsTo(data, 12, self.imm);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

InstructionType encodeTypeS(const Instruction& self) {
	InstructionType data = 0;
	writeTruncatedBitsTo(data, 7, self.imm>>5);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeTruncatedBitsTo(data, 5, self.imm);
	return data;
}

InstructionType encodeTypeSB(const Instruction& self) {
	InstructionType data = 0, imm = self.imm;
	imm |= imm>>11;
	writeTruncatedBitsTo(data, 7, imm>>5);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeTruncatedBitsTo(data, 5, imm);
	return data;
}

InstructionType encodeTypeU(const Instruction& self) {
	InstructionType data = 0;
	writeBitsTo(data, 20, self.imm>>12);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

InstructionType encodeTypeUJ(const Instruction& self) {
	InstructionType data = 0, imm = self.imm;
	imm &= ~(TrailingBitMask(11)<<20);
	imm |= movedBitsFromTo(imm, 1, 11, 20);
	imm |= movedBitsFromTo(imm, 10, 1, 21);
	//imm &= ~(TrailingBitMask(12));
	writeBitsTo(data, 20, imm>>12);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

InstructionType encodeTypeUndefined(const Instruction& self) {
	return 0;
}

static std::function<void(Instruction&, InstructionType)> decodeType[] = {
	decodeTypeR,
	decodeTypeR4,
	decodeTypeI,
	decodeTypeS,
	decodeTypeSB,
	decodeTypeU,
	decodeTypeUJ,
	decodeTypeUndefined
};

static std::function<InstructionType(const Instruction&)> encodeType[] = {
	encodeTypeR,
	encodeTypeR4,
	encodeTypeI,
	encodeTypeS,
	encodeTypeSB,
	encodeTypeU,
	encodeTypeUJ,
	encodeTypeUndefined
};

Instruction::Instruction(InstructionType data) {
	opcode = readBitsFrom(data, 7);
	decodeType[getType()](*this, data);
}

InstructionType Instruction::encode() const {
	InstructionType data = encodeType[getType()](*this);
	writeBitsTo(data, 7, opcode);
	return data;
}

Instruction::Type Instruction::getType() const {
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
	throw Exception(Exception::Code::IllegalInstruction);
}
