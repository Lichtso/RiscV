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

    //ram.setSize(16);
    return 0;
}
