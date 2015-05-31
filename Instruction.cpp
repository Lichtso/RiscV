#include "Instruction.hpp"

UInt64 TrailingBitMask(UInt8 len) {
	return 0xFFFFFFFFFFFFFFFFULL>>(64-len);
}

UInt64 getBitsFrom(UInt64 data, UInt8 at, UInt8 len) {
	return (data>>at)&TrailingBitMask(len);
}

template<typename type>
type readBitsFrom(type& data, UInt8 bits) {
	type result = data&TrailingBitMask(bits);
	data >>= bits;
	return result;
}

template<typename type>
void writeBitsTo(type& data, UInt8 bits, UInt32 content) {
	data <<= bits;
	data |= content;
}

template<typename type>
type moveBitsFromTo(type data, UInt8 bits, UInt8 from, UInt8 to) {
	data = (from > to)
		? data>>(from-to)
		: data<<(to-from);
	return data&(TrailingBitMask(bits)<<to);
}

template<typename type>
void writeTruncatedBitsTo(type& data, UInt8 bits, UInt32 content) {
	data <<= bits;
	data |= content&TrailingBitMask(bits);
}



void decodeTypeCR(Instruction& self, UInt16 data) {
	self.reg[2] = readBitsFrom(data, 5);
	self.reg[0] = self.reg[1] = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 4);
}

void decodeTypeCI(Instruction& self, UInt16 data) {
	self.imm = readBitsFrom(data, 5);
	self.reg[0] = self.reg[1] = readBitsFrom(data, 5);
	self.imm |= readBitsFrom(data, 1)<<5;
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCSS(Instruction& self, UInt16 data) {
	self.reg[2] = readBitsFrom(data, 5);
	self.imm = readBitsFrom(data, 6);
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCIW(Instruction& self, UInt16 data) {
	self.reg[0] = readBitsFrom(data, 3)+8;
	self.imm = readBitsFrom(data, 8);
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCL(Instruction& self, UInt16 data) {
	self.reg[0] = readBitsFrom(data, 3)+8;
	self.imm = readBitsFrom(data, 2);
	self.reg[1] = readBitsFrom(data, 3)+8;
	self.imm |= readBitsFrom(data, 3)<<2;
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCS(Instruction& self, UInt16 data) {
	self.reg[2] = readBitsFrom(data, 3)+8;
	self.imm = readBitsFrom(data, 2);
	self.reg[1] = readBitsFrom(data, 3)+8;
	self.imm |= readBitsFrom(data, 3)<<2;
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCB(Instruction& self, UInt16 data) {
	self.imm = readBitsFrom(data, 5);
	self.reg[1] = readBitsFrom(data, 3)+8;
	self.imm |= readBitsFrom(data, 3)<<5;
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCJ(Instruction& self, UInt16 data) {
	self.imm = readBitsFrom(data, 11);
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCDS(Instruction& self, UInt16 data) {
	self.reg[2] = readBitsFrom(data, 3)+8;
	self.funct[1] = readBitsFrom(data, 2);
	self.reg[0] = readBitsFrom(data, 3)+8;
	self.funct[0] = readBitsFrom(data, 6);
}

void decodeTypeCSD(Instruction& self, UInt16 data) {
	self.reg[0] = readBitsFrom(data, 3)+8;
	self.funct[1] = readBitsFrom(data, 2);
	self.reg[1] = readBitsFrom(data, 3)+8;
	self.funct[0] = readBitsFrom(data, 6);
}

void decodeTypeCRI(Instruction& self, UInt16 data) {
	self.reg[0] = readBitsFrom(data, 3)+8;
	self.funct[1] = readBitsFrom(data, 2);
	self.reg[1] = readBitsFrom(data, 3)+8;
	self.imm = readBitsFrom(data, 3);
	self.funct[0] = readBitsFrom(data, 3);
}

void decodeTypeCR3(Instruction& self, UInt16 data) {
	self.reg[2] = readBitsFrom(data, 3)+8;
	self.funct[1] = readBitsFrom(data, 2);
	self.reg[1] = readBitsFrom(data, 3)+8;
	self.reg[0] = readBitsFrom(data, 3)+8;
	self.funct[0] = readBitsFrom(data, 3);
}

UInt16 encodeTypeCR(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 4, self.funct[0]);
	writeBitsTo(data, 5, self.reg[0]);
	writeBitsTo(data, 5, self.reg[2]);
	return data;
}

UInt16 encodeTypeCI(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 1, self.imm>>5);
	writeBitsTo(data, 5, self.reg[0]);
	writeTruncatedBitsTo(data, 5, self.imm);
	return data;
}

UInt16 encodeTypeCSS(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 6, self.imm);
	writeBitsTo(data, 5, self.reg[2]);
	return data;
}

UInt16 encodeTypeCIW(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 8, self.imm);
	writeTruncatedBitsTo(data, 3, self.reg[0]);
	return data;
}

UInt16 encodeTypeCL(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 3, self.imm>>2);
	writeTruncatedBitsTo(data, 3, self.reg[1]);
	writeTruncatedBitsTo(data, 2, self.imm);
	writeTruncatedBitsTo(data, 3, self.reg[0]);
	return data;
}

UInt16 encodeTypeCS(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 3, self.imm>>2);
	writeTruncatedBitsTo(data, 3, self.reg[1]);
	writeTruncatedBitsTo(data, 2, self.imm);
	writeTruncatedBitsTo(data, 3, self.reg[2]);
	return data;
}

UInt32 encodeTypeCB(const Instruction& self) {
	UInt32 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 3, self.imm>>5);
	writeTruncatedBitsTo(data, 3, self.reg[1]);
	writeTruncatedBitsTo(data, 5, self.imm);
	return data;
}

UInt16 encodeTypeCJ(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 11, self.imm);
	return data;
}

UInt16 encodeTypeCDS(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 6, self.funct[0]);
	writeTruncatedBitsTo(data, 3, self.reg[0]);
	writeBitsTo(data, 2, self.funct[1]);
	writeTruncatedBitsTo(data, 3, self.reg[2]);
	return data;
}

UInt16 encodeTypeCSD(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 6, self.funct[0]);
	writeTruncatedBitsTo(data, 3, self.reg[1]);
	writeBitsTo(data, 2, self.funct[1]);
	writeTruncatedBitsTo(data, 3, self.reg[0]);
	return data;
}

UInt16 encodeTypeCRI(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeTruncatedBitsTo(data, 3, self.imm);
	writeTruncatedBitsTo(data, 3, self.reg[1]);
	writeBitsTo(data, 2, self.funct[1]);
	writeTruncatedBitsTo(data, 3, self.reg[0]);
	return data;
}

UInt16 encodeTypeCR3(const Instruction& self) {
	UInt16 data = 0;
	writeBitsTo(data, 3, self.funct[0]);
	writeTruncatedBitsTo(data, 3, self.reg[0]);
	writeTruncatedBitsTo(data, 3, self.reg[1]);
	writeBitsTo(data, 2, self.funct[1]);
	writeTruncatedBitsTo(data, 3, self.reg[2]);
	return data;
}

static std::function<void(Instruction&, UInt16)> decode16Type[] = {
	decodeTypeCR,
	decodeTypeCI,
	decodeTypeCSS,
	decodeTypeCIW,
	decodeTypeCL,
	decodeTypeCS,
	decodeTypeCB,
	decodeTypeCJ,
	decodeTypeCDS,
	decodeTypeCSD,
	decodeTypeCRI,
	decodeTypeCR3
};

static std::function<UInt16(const Instruction&)> encode16Type[] = {
	encodeTypeCR,
	encodeTypeCI,
	encodeTypeCSS,
	encodeTypeCIW,
	encodeTypeCL,
	encodeTypeCS,
	encodeTypeCB,
	encodeTypeCJ,
	encodeTypeCDS,
	encodeTypeCSD,
	encodeTypeCRI,
	encodeTypeCR3
};

void Instruction::decode16(UInt16 data) {
	// TODO: Waiting for next riscv-compressed-spec
	opcode = readBitsFrom(data, 2);
	decode16Type[getType()](*this, data);
}

UInt16 Instruction::encode16() const {
	// TODO: Waiting for next riscv-compressed-spec
	UInt32 data = encode16Type[getType()](*this);
	writeBitsTo(data, 2, opcode);
	return data;
}



void decodeTypeR(Instruction& self, UInt32 data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.funct[1] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.reg[2] = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 7);
}

void decodeTypeR4(Instruction& self, UInt32 data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.funct[1] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.reg[2] = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 2);
	self.reg[3] = readBitsFrom(data, 5);
}

void decodeTypeI(Instruction& self, UInt32 data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.imm = readBitsFrom(data, 12);
	self.imm |= data*~TrailingBitMask(20);
}

void decodeTypeS(Instruction& self, UInt32 data) {
	self.imm = readBitsFrom(data, 5);
	self.funct[0] = readBitsFrom(data, 3);
	self.reg[1] = readBitsFrom(data, 5);
	self.reg[2] = readBitsFrom(data, 5);
	self.imm |= readBitsFrom(data, 7)<<5;
	self.imm |= data*~TrailingBitMask(20);
}

void decodeTypeSB(Instruction& self, UInt32 data) {
 	decodeTypeS(self, data);
	self.imm |= moveBitsFromTo(self.imm, 1, 0, 11);
	self.imm &= ~(TrailingBitMask(1));
}

void decodeTypeU(Instruction& self, UInt32 data) {
	self.reg[0] = readBitsFrom(data, 5);
	self.imm = readBitsFrom(data, 20)<<12;
}

void decodeTypeUJ(Instruction& self, UInt32 data) {
	decodeTypeU(self, data);
	self.imm |= moveBitsFromTo(self.imm, 1, 20, 11);
	self.imm |= moveBitsFromTo(self.imm, 10, 21, 1);
	self.imm = (self.imm&TrailingBitMask(20))|((self.imm>>31)*~TrailingBitMask(20));
}

UInt32 encodeTypeR(const Instruction& self) {
	UInt32 data = 0;
	writeBitsTo(data, 7, self.funct[0]);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[1]);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

UInt32 encodeTypeR4(const Instruction& self) {
	UInt32 data = 0;
	writeBitsTo(data, 5, self.reg[3]);
	writeBitsTo(data, 2, self.funct[0]);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[1]);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

UInt32 encodeTypeI(const Instruction& self) {
	UInt32 data = 0;
	writeTruncatedBitsTo(data, 12, self.imm);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

UInt32 encodeTypeS(const Instruction& self) {
	UInt32 data = 0;
	writeTruncatedBitsTo(data, 7, self.imm>>5);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeTruncatedBitsTo(data, 5, self.imm);
	return data;
}

UInt32 encodeTypeSB(const Instruction& self) {
	UInt32 data = 0, imm = self.imm;
	imm |= imm>>11;
	writeTruncatedBitsTo(data, 7, imm>>5);
	writeBitsTo(data, 5, self.reg[2]);
	writeBitsTo(data, 5, self.reg[1]);
	writeBitsTo(data, 3, self.funct[0]);
	writeTruncatedBitsTo(data, 5, imm);
	return data;
}

UInt32 encodeTypeU(const Instruction& self) {
	UInt32 data = 0;
	writeBitsTo(data, 20, self.imm>>12);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

UInt32 encodeTypeUJ(const Instruction& self) {
	UInt32 data = 0, imm = self.imm;
	imm &= ~(TrailingBitMask(11)<<20);
	imm |= moveBitsFromTo(imm, 1, 11, 20);
	imm |= moveBitsFromTo(imm, 10, 1, 21);
	writeBitsTo(data, 20, imm>>12);
	writeBitsTo(data, 5, self.reg[0]);
	return data;
}

static std::function<void(Instruction&, UInt32)> decode32Type[] = {
	decodeTypeR,
	decodeTypeR4,
	decodeTypeI,
	decodeTypeS,
	decodeTypeSB,
	decodeTypeU,
	decodeTypeUJ
};

static std::function<UInt32(const Instruction&)> encode32Type[] = {
	encodeTypeR,
	encodeTypeR4,
	encodeTypeI,
	encodeTypeS,
	encodeTypeSB,
	encodeTypeU,
	encodeTypeUJ
};

void Instruction::decode32(UInt32 data) {
	opcode = readBitsFrom(data, 7);
	decode32Type[getType()](*this, data);
}

UInt32 Instruction::encode32() const {
	UInt32 data = encode32Type[getType()](*this);
	writeBitsTo(data, 7, opcode);
	return data;
}



Instruction::Type Instruction::getType() const {
	switch(opcode) {
		/*case 0x00:
		case 0x01:
		case 0x02:
		// TODO: Waiting for next riscv-compressed-spec
		return ;*/
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
