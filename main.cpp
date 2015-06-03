#include "CPU.hpp"

RAM ram;
CPU<> cpu;

int main(int argc, char** argv) {
    if(argc == 4) {
        if(strcmp(argv[1], "--disassemble") == 0) {
            Disassembler disassembler;
            return (disassembler.readFromFile(argv[2]) && disassembler.writeToFile(argv[3])) ? 0 : 1;
        }else if(strcmp(argv[1], "--assemble") == 0) {
            Assembler assembler;
            return (assembler.readFromFile(argv[2]) && assembler.writeToFile(argv[3])) ? 0 : 1;
        }
    }

    UInt8 status = 0;
    FloatRoundingMode round = FloatRoundingMode::RoundNearest;

    for(UInt32 i = 0; i < TrailingBitMask<UInt32>(31); ++i) {
        Float test;
        test.setSign(0);
        test.setInteger<UInt32>(status, round, i);
        float mirror = i;
        UInt32 mirrored = *reinterpret_cast<UInt32*>(&mirror);
        if(test.raw == mirrored) continue;
        //if(status == 0) continue;
        printf("i: %x, status: %hhx\n", i, status);
        printf("%08x %d %02hhx %06x\n", test.raw, test.getSign(), test.getExponent(), test.getField());
        printf("%08x\n", mirrored);
        break;
        //if(test.raw != mirrored) break;
    }



    /*ram.setSize(16);
    UInt32 data;
    Instruction instruction;

    // Store 12 in x1
    instruction.opcode = 0x13;
    instruction.reg[0] = 1;
    instruction.reg[1] = 0;
    instruction.imm = 12;
    data = instruction.encode32();
    ram.set<UInt32, false>(0x200, &data);

    // If x1 == 0 then Jump +8
    instruction.opcode = 0x63;
    instruction.funct[0] = 0;
    instruction.reg[1] = 1;
    instruction.reg[2] = 0;
    instruction.imm = 12;
    data = instruction.encode32();
    ram.set<UInt32, false>(0x204, &data);

    // Substract 1 from x1
    instruction.opcode = 0x13;
    instruction.reg[0] = 1;
    instruction.reg[1] = 1;
    instruction.imm = -1;
    data = instruction.encode32();
    ram.set<UInt32, false>(0x208, &data);

    // Jump -8
    instruction.opcode = 0x6F;
    instruction.reg[0] = 0;
    instruction.imm = -8;
    data = instruction.encode32();
    ram.set<UInt32, false>(0x20C, &data);

    for(size_t i = 0; cpu.fetchAndExecute(); ++i)
        printf("%zu %016llx\n", i, cpu.pc);

    cpu.dump(std::cout);
    ram.dump(std::cout);*/

    return 0;
}
