/*
	WARNING:

	The assembler / disassembler was made for testing purpose only.
	They are highly instable and might produce wrong outputs.
	Do not use them in production or any other projects!
*/

#include "Disassembler.hpp"
#include <elfio/elfio.hpp>

const std::map<UInt8, std::string> disassembler_03 = {
	{0, "LB"}, {1, "LH"}, {2, "LW"}, {3, "LD"},
	{4, "LBU"}, {5, "LHU"}, {6, "LWU"}
};

const std::map<UInt8, std::string> disassembler_07 = {
	{2, "FLW"}, {3, "FLD"}
};

const std::map<UInt8, std::string> disassembler_0F = {
	{0, "W"}, {1, "R"}, {2, "O"}, {3, "I"}
};

const std::map<UInt8, std::string> disassembler_23 = {
	{0, "SB"}, {1, "SH"}, {2, "SW"}, {3, "SD"}
};

const std::map<UInt8, std::string> disassembler_27 = {
	{2, "FSW"}, {3, "FSD"}
};

const std::map<UInt8, std::string> disassembler_2F = {
	{2, "W"}, {3, "D"}
};

const std::map<UInt8, std::string> disassembler_2F_2 = {
	{0, "RL"}, {1, "AQ"}
};

const std::map<UInt8, std::string> disassembler_33 = {
	{0, "MUL"}, {1, "MULH"}, {2, "MULHSU"}, {3, "MULHU"},
	{4, "DIV"}, {5, "DIVU"}, {6, "REM"}, {7, "REMU"}
};

const std::map<UInt8, std::string> disassembler_3X_0 = {
	{0, "ADD"}, {32, "SUB"}
};

const std::map<UInt8, std::string> disassembler_3X_5 = {
	{0, "SRL"}, {32, "SRA"}
};

const std::map<UInt8, std::string> disassembler_4X = {
	{0, "FMADD"}, {1, "FMSUB"}, {2, "FNMSUB"}, {3, "FNMADD"}
};

const std::map<UInt8, std::string> disassembler_Float = {
	{0, "S"}, {1, "D"}
};

const std::map<UInt8, std::string> disassembler_FloatStatusFlags = {
	{0, "NX"}, {1, "UF"}, {2, "OF"}, {3, "DZ"}, {4, "NV"}
};

const std::map<UInt8, std::string> disassembler_FloatRoundingModes = {
	{0, "RNE"}, {1, "RTZ"}, {2, "RDN"}, {3, "RUP"}, {4, "RMM"}
};

const std::map<UInt8, std::string> disassembler_53_10 = {
	{0, "FSGNJ"}, {1, "FSGNJN"}, {2, "FSGNJX"}
};

const std::map<UInt8, std::string> disassembler_53_10_2 = {
	{0, "FMV"}, {1, "FNEG"}, {2, "FABS"}
};

const std::map<UInt8, std::string> disassembler_53_14 = {
	{0, "FMIN"}, {1, "FMAX"}
};

const std::map<UInt8, std::string> disassembler_53_50 = {
	{0, "FLE"}, {1, "FLT"}, {2, "FEQ"}
};

const std::map<UInt8, std::string> disassembler_53_CVT = {
	{0, "W"}, {1, "WU"}, {2, "L"}, {3, "LU"}
};

const std::map<UInt8, std::string> disassembler_53_70 = {
	{0, "FMV"}, {1, "FCLASS"}
};

const std::map<UInt8, std::string> disassembler_63 = {
	{0, "BEQ"}, {1, "BNE"},
	{4, "BLT"}, {5, "BGE"}, {6, "BLTU"}, {7, "BGEU"}
};

const std::map<UInt8, std::string> disassembler_73 = {
	{1, "CSRRW"}, {2, "CSRRS"}, {3, "CSRRC"},
	{5, "CSRRWI"}, {6, "CSRRSI"}, {7, "CSRRCI"}
};

const std::map<UInt8, std::string> disassembler_73_2 = {
	{1, "FSFLAGS"}, {2, "FSRM"}, {3, "FSCSR"}
};

const std::map<UInt8, std::string> disassembler_IntRegABINames = {
	{0, "zero"}, {1, "ra"}, {2, "fp"}, {3, "s1"}, {4, "s2"}, {5, "s3"}, {6, "s4"}, {7, "s5"},
	{8, "s6"}, {9, "s7"}, {10, "s8"}, {11, "s9"}, {12, "s10"}, {13, "s11"}, {14, "sp"}, {15, "tp"},
	{16, "v0"}, {17, "v1"}, {18, "a0"}, {19, "a1"}, {20, "a2"}, {21, "a3"}, {22, "a4"}, {23, "a5"},
	{24, "a6"}, {25, "a7"}, {26, "t0"}, {27, "t1"}, {28, "t2"}, {29, "t3"}, {30, "t4"}, {31, "gp"}
};

const std::map<UInt8, std::string> disassembler_FloatRegABINames = {
	{0, "fs0"}, {1, "fs1"}, {2, "fs2"}, {3, "fs3"}, {4, "fs4"}, {5, "fs5"}, {6, "fs6"}, {7, "fs7"},
	{8, "fs8"}, {9, "fs9"}, {10, "fs10"}, {11, "fs11"}, {12, "fs12"}, {13, "fs13"}, {14, "fs14"}, {15, "fs15"},
	{16, "fv0"}, {17, "fv1"}, {18, "fa0"}, {19, "fa1"}, {20, "fa2"}, {21, "fa3"}, {22, "fa4"}, {23, "fa5"},
	{24, "fa6"}, {25, "fa7"}, {26, "ft0"}, {27, "ft1"}, {28, "ft2"}, {29, "ft3"}, {30, "ft4"}, {31, "ft5"}
};

enum assembler_dir_type {
	assembler_dir_text,
	assembler_dir_data,
	assembler_dir_align,
	assembler_dir_skip,
	assembler_dir_byte,
	assembler_dir_half,
	assembler_dir_word,
	assembler_dir_dword
};

const std::map<std::string, assembler_dir_type> assembler_cmds = {
	{".TEXT", assembler_dir_text},
	{".DATA", assembler_dir_data},
	{".ALIGN", assembler_dir_align},
	{".SKIP", assembler_dir_skip},
	{".BYTE", assembler_dir_byte},
	{".HALF", assembler_dir_half},
	{".WORD", assembler_dir_word},
	{".DWORD", assembler_dir_dword}
};

const std::map<std::string, UInt8> assembler_instructions = {
	{"LB", 0x03},
	{"LH", 0x03},
	{"LW", 0x03},
	{"LD", 0x03},
	{"LBU", 0x03},
	{"LHU", 0x03},
	{"LWU", 0x03},

	{"FLW", 0x07},
	{"FLD", 0x07},

	{"FENCE", 0x0F},

	{"ADDI", 0x13},
	{"SLTI", 0x13},
	{"SLTIU", 0x13},
	{"XORI", 0x13},
	{"ORI", 0x13},
	{"ANDI", 0x13},
	{"SLLI", 0x13},
	{"SRLI", 0x13},
	{"SRAI", 0x13},
	{"NOP", 0x13},
	{"MV", 0x13},
	{"SEQZ", 0x13},
	{"NOT", 0x13},

	{"AUIPC", 0x17},

	{"ADDIW", 0x1B},
	{"SLLIW", 0x1B},
	{"SRLIW", 0x1B},
	{"SRAIW", 0x1B},
	{"SEXT", 0x1B},

	{"SB", 0x23},
	{"SH", 0x23},
	{"SW", 0x23},
	{"SD", 0x23},

	{"FSW", 0x27},
	{"FSD", 0x27},

	{"LR", 0x2F},
	{"SC", 0x2F},
	{"AMOSWAP", 0x2F},
	{"AMOADD", 0x2F},
	{"AMOXOR", 0x2F},
	{"AMOAND", 0x2F},
	{"AMOOR", 0x2F},
	{"AMOMIN", 0x2F},
	{"AMOMAX", 0x2F},
	{"AMOMINU", 0x2F},
	{"AMOMAXU", 0x2F},

	{"ADD", 0x33},
	{"SUB", 0x33},
	{"SLL", 0x33},
	{"SLT", 0x33},
	{"SLTU", 0x33},
	{"XOR", 0x33},
	{"SRL", 0x33},
	{"SRA", 0x33},
	{"OR", 0x33},
	{"AND", 0x33},
	{"SNEZ", 0x33},
	{"MUL", 0x33},
	{"MULH", 0x33},
	{"MULHSU", 0x33},
	{"MULHU", 0x33},
	{"DIV", 0x33},
	{"DIVU", 0x33},
	{"REM", 0x33},
	{"REMU", 0x33},

	{"LUI", 0x37},

	{"ADDW", 0x3B},
	{"SUBW", 0x3B},
	{"SLLW", 0x3B},
	{"SRLW", 0x3B},
	{"SRAW", 0x3B},
	{"MULW", 0x3B},
	{"DIVW", 0x3B},
	{"DIVUW", 0x3B},
	{"REMW", 0x3B},
	{"REMUW", 0x3B},

	{"FMADD", 0x43},
	{"FMSUB", 0x47},
	{"FNMSUB", 0x4B},
	{"FNMADD", 0x4F},

	{"FADD", 0x53},
	{"FSUB", 0x53},
	{"FMUL", 0x53},
	{"FDIV", 0x53},
	{"FSQRT", 0x53},
	{"FSGNJ", 0x53},
	{"FSGNJN", 0x53},
	{"FSGNJX", 0x53},
	{"FMIN", 0x53},
	{"FMAX", 0x53},
	{"FCVT", 0x53},
	{"FEQ", 0x53},
	{"FLT", 0x53},
	{"FLE", 0x53},
	{"FCLASS", 0x53},
	{"FMV", 0x53},
	{"FNEG", 0x53},
	{"FABS", 0x53},

	{"BEQ", 0x63},
	{"BNE", 0x63},
	{"BLT", 0x63},
	{"BGE", 0x63},
	{"BLTU", 0x63},
	{"BGEU", 0x63},

	{"JALR", 0x67},

	{"J", 0x6F},
	{"JAL", 0x6F},

	{"CSRRW", 0x73},
	{"CSRRS", 0x73},
	{"CSRRC", 0x73},
	{"CSRRWI", 0x73},
	{"CSRRSI", 0x73},
	{"CSRRCI", 0x73},
	{"FSFLAGS", 0x73},
	{"FSRM", 0x73},
	{"FSCSR", 0x73},
	{"FSFLAGSI", 0x73},
	{"FSRMI", 0x73},
	{"FRFLAGS", 0x73},
	{"FRRM", 0x73},
	{"FRCSR", 0x73},
	{"RDCYCLE", 0x73},
	{"RDCYCLEH", 0x73},
	{"RDTIME", 0x73},
	{"RDTIMEH", 0x73},
	{"RDINSTRET", 0x73},
	{"RDINSTRETH", 0x73},
	{"SFENCE", 0x73},
	{"ECALL", 0x73},
	{"EBREAK", 0x73},
	{"ERET", 0x73},
	{"SCALL", 0x73},
	{"SBREAK", 0x73},
	{"MRTS", 0x73},
	{"MRTH", 0x73},
	{"HRTS", 0x73},
	{"WFI", 0x73}
};
