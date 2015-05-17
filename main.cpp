#include "CPU.hpp"

RAM ram;
CPU<64> cpu;

int main(int argc, char** argv) {
    ram.setSize(10);

    return 0;
}
