#include "CPU.hpp"
#include <elfio/elfio.hpp>

Ram ram;
Cpu<> cpu;

int main(int argc, char** argv) {
    /*if(argc == 4) {
        if(strcmp(argv[1], "--disassemble") == 0) {
            Disassembler disassembler;
            return (disassembler.readFromFile(argv[2]) && disassembler.writeToFile(argv[3])) ? 0 : 1;
        }else if(strcmp(argv[1], "--assemble") == 0) {
            Assembler assembler;
            return (assembler.readFromFile(argv[2]) && assembler.writeToFile(argv[3])) ? 0 : 1;
        }
    }

    ram.setSize(16);*/
    UInt32 dataSection[32];
    Instruction instruction;

    // Store 12 in x1
    instruction.opcode = 0x13;
    instruction.reg[0] = 1;
    instruction.reg[1] = 0;
    instruction.imm = 12;
    dataSection[0] = instruction.encode32();
    //ram.set<UInt32, false>(0x200, &data);

    // If x1 == 0 then Jump +8
    instruction.opcode = 0x63;
    instruction.funct[0] = 0;
    instruction.reg[1] = 1;
    instruction.reg[2] = 0;
    instruction.imm = 12;
    dataSection[1] = instruction.encode32();
    //ram.set<UInt32, false>(0x204, &data);

    // Subtract 1 from x1
    instruction.opcode = 0x13;
    instruction.reg[0] = 1;
    instruction.reg[1] = 1;
    instruction.imm = -1;
    dataSection[2] = instruction.encode32();
    //ram.set<UInt32, false>(0x208, &data);

    // Jump -8
    instruction.opcode = 0x6F;
    instruction.reg[0] = 0;
    instruction.imm = -8;
    dataSection[3] = instruction.encode32();
    //ram.set<UInt32, false>(0x20C, &data);


    ELFIO::elfio writer;
    writer.create(ELFCLASS64, ELFDATA2LSB);
    writer.set_os_abi(ELFOSABI_LINUX);
    writer.set_type(ET_EXEC);
    writer.set_machine(243);
    ELFIO::section* text_sec = writer.sections.add(".text");
    text_sec->set_type(SHT_PROGBITS);
    text_sec->set_flags(SHF_ALLOC|SHF_EXECINSTR);
    text_sec->set_addr_align(4);
    text_sec->set_data(reinterpret_cast<char*>(dataSection), 4*4);

    ELFIO::segment* text_seg = writer.segments.add();
    text_seg->set_type(PT_LOAD);
    text_seg->set_virtual_address(0x2000);
    text_seg->set_physical_address(0x2000);
    text_seg->set_flags(PF_X|PF_R);
    text_seg->set_align(0x1000);
    text_seg->add_section_index(text_sec->get_index(), text_sec->get_addr_align());

    writer.set_entry(0x2000);
    writer.save("a.out");

    /*for(size_t i = 0; cpu.fetchAndExecute(); ++i)
        printf("%zu %016llx\n", i, cpu.pc);

    cpu.dump(std::cout);
    ram.dump(std::cout);*/

    return 0;
}
