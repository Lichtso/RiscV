#include "Instruction.hpp"

UInt32 TrailingBitMask(UInt8 bits) {
	return 0xFFFFFFFF>>(32-bits);
}

UInt32 LeadingBitMask(UInt8 bits) {
	return 0xFFFFFFFF<<(32-bits);
}

UInt32 readBitsFrom(UInt32& data, UInt8 bits) {
	UInt32 result = data&TrailingBitMask(bits);
	data >>= bits;
	return result;
}

void writeBitsTo(UInt32& data, UInt8 bits, UInt32 content) {
	data <<= bits;
	data |= content;
}

UInt32 movedBitsFromTo(UInt32 data, UInt8 bits, UInt8 from, UInt8 to) {
	data = (from > to)
		? data>>(from-to)
		: data<<(to-from);
	return data&(TrailingBitMask(bits)<<to);
}

void writeTruncatedBitsTo(UInt32& data, UInt8 bits, UInt32 content) {
	data <<= bits;
	data |= content&TrailingBitMask(bits);
}

void decodeTypeR(Instruction& self, UInt32 data) {
	self.rd = readBitsFrom(data, 5);
	self.funct3 = readBitsFrom(data, 3);
	self.rs1 = readBitsFrom(data, 5);
	self.rs2 = readBitsFrom(data, 5);
	self.funct7 = readBitsFrom(data, 7);
}

void decodeTypeR4(Instruction& self, UInt32 data) {
	self.rd = readBitsFrom(data, 5);
	self.funct3 = readBitsFrom(data, 3);
	self.rs1 = readBitsFrom(data, 5);
	self.rs2 = readBitsFrom(data, 5);
	self.funct2 = readBitsFrom(data, 2);
	self.rs3 = readBitsFrom(data, 5);
}

void decodeTypeI(Instruction& self, UInt32 data) {
	self.rd = readBitsFrom(data, 5);
	self.funct3 = readBitsFrom(data, 3);
	self.rs1 = readBitsFrom(data, 5);
	self.imm = readBitsFrom(data, 12);
	self.imm |= LeadingBitMask(20)*data;
}

void decodeTypeS(Instruction& self, UInt32 data) {
	self.imm = readBitsFrom(data, 5);
	self.funct3 = readBitsFrom(data, 3);
	self.rs1 = readBitsFrom(data, 5);
	self.rs2 = readBitsFrom(data, 5);
	self.imm |= readBitsFrom(data, 7)<<5;
	self.imm |= LeadingBitMask(20)*data;
}

void decodeTypeSB(Instruction& self, UInt32 data) {
 	decodeTypeS(self, data);
	self.imm |= movedBitsFromTo(self.imm, 1, 0, 11);
	self.imm &= ~(TrailingBitMask(1));
}

void decodeTypeU(Instruction& self, UInt32 data) {
	self.rd = readBitsFrom(data, 5);
	self.imm = readBitsFrom(data, 20)<<12;
}

void decodeTypeUJ(Instruction& self, UInt32 data) {
	decodeTypeU(self, data);
	self.imm |= movedBitsFromTo(self.imm, 1, 20, 11);
	self.imm |= movedBitsFromTo(self.imm, 10, 21, 1);
	self.imm = (self.imm&TrailingBitMask(20))|(LeadingBitMask(20)*(self.imm>>31));
}

void decodeTypeUndefined(Instruction& self, UInt32 data) {

}

UInt32 encodeTypeR(const Instruction& self) {
	UInt32 data = 0;
	writeBitsTo(data, 7, self.funct7);
	writeBitsTo(data, 5, self.rs2);
	writeBitsTo(data, 5, self.rs1);
	writeBitsTo(data, 3, self.funct3);
	writeBitsTo(data, 5, self.rd);
	return data;
}

UInt32 encodeTypeR4(const Instruction& self) {
	UInt32 data = 0;
	writeBitsTo(data, 5, self.rs3);
	writeBitsTo(data, 2, self.funct2);
	writeBitsTo(data, 5, self.rs2);
	writeBitsTo(data, 5, self.rs1);
	writeBitsTo(data, 3, self.funct3);
	writeBitsTo(data, 5, self.rd);
	return data;
}

UInt32 encodeTypeI(const Instruction& self) {
	UInt32 data = 0;
	writeTruncatedBitsTo(data, 12, self.imm);
	writeBitsTo(data, 5, self.rs1);
	writeBitsTo(data, 3, self.funct3);
	writeBitsTo(data, 5, self.rd);
	return data;
}

UInt32 encodeTypeS(const Instruction& self) {
	UInt32 data = 0;
	writeTruncatedBitsTo(data, 7, self.imm>>5);
	writeBitsTo(data, 5, self.rs2);
	writeBitsTo(data, 5, self.rs1);
	writeBitsTo(data, 3, self.funct3);
	writeTruncatedBitsTo(data, 5, self.imm);
	return data;
}

UInt32 encodeTypeSB(const Instruction& self) {
	UInt32 data = 0, imm = self.imm;
	imm |= imm>>11;
	writeTruncatedBitsTo(data, 7, imm>>5);
	writeBitsTo(data, 5, self.rs2);
	writeBitsTo(data, 5, self.rs1);
	writeBitsTo(data, 3, self.funct3);
	writeTruncatedBitsTo(data, 5, imm);
	return data;
}

UInt32 encodeTypeU(const Instruction& self) {
	UInt32 data = 0;
	writeBitsTo(data, 20, self.imm);
	writeBitsTo(data, 5, self.rd);
	return data;
}

UInt32 encodeTypeUJ(const Instruction& self) {
	UInt32 data = 0, imm = self.imm;
	imm &= ~(TrailingBitMask(11)<<20);
	imm |= movedBitsFromTo(imm, 1, 11, 20);
	imm |= movedBitsFromTo(imm, 10, 1, 21);
	imm &= ~(TrailingBitMask(12));
	writeBitsTo(data, 20, imm);
	writeBitsTo(data, 5, self.rd);
	return data;
}

UInt32 encodeTypeUndefined(const Instruction& self) {
	return 0;
}

static std::function<void(Instruction&, UInt32)> decodeType[] = {
	decodeTypeR,
	decodeTypeR4,
	decodeTypeI,
	decodeTypeS,
	decodeTypeSB,
	decodeTypeU,
	decodeTypeUJ,
	decodeTypeUndefined
};

static std::function<UInt32(const Instruction&)> encodeType[] = {
	encodeTypeR,
	encodeTypeR4,
	encodeTypeI,
	encodeTypeS,
	encodeTypeSB,
	encodeTypeU,
	encodeTypeUJ,
	encodeTypeUndefined
};

Instruction::Instruction(UInt32 data) {
	opcode = readBitsFrom(data, 7);
	decodeType[getType()](*this, data);
}

UInt32 Instruction::encode() const {
	UInt32 data = encodeType[getType()](*this);
	writeBitsTo(data, 7, opcode);
	return data;
}
