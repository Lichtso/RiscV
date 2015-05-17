#pragma once
#include "RAM.hpp"

template<UInt8 XLEN = 64>
class CPU {
    public:
    typedef typename std::conditional<XLEN == 64, Int64, Int32>::type IntType;
    typedef typename std::conditional<XLEN == 64, UInt64, UInt32>::type UIntType;
    typedef typename std::conditional<XLEN == 64, Float64, Float32>::type FloatType;

    enum CSR {
        fflags = 0x001,
        frm = 0x002,
        fcsr = 0x003,
        cycle = 0xC00,
        time = 0xC01,
        instret = 0xC02,
        cycleh = 0xC80,
        timeh = 0xC81,
        instreth = 0xC82,
        sstatus = 0x100,
        stvec = 0x101,
        sie = 0x104,
        stimecmp = 0x121,
        stime = 0xD01,
        stimeh = 0xD81,
        sscratch = 0x140,
        sepc = 0x141,
        scause = 0x142,
        sbadaddr = 0x143,
        sip = 0x144,
        sdst = 0x180,
        sasid = 0x181,
        cyclew = 0x900,
        timew = 0x901,
        instretw = 0x902,
        cyclehw = 0x980,
        timehw = 0x981,
        instrethw = 0x982,
        hstatus = 0x200,
        htvec = 0x201,
        htdeleg = 0x202,
        htimecmp = 0x221,
        htime = 0xE01,
        htimeh = 0xE81,
        hscratch = 0x240,
        hepc = 0x241,
        hcause = 0x242,
        hbadaddr = 0x243,
        tbd = 0x280,
        stimew = 0xA01,
        stimehw = 0xA81,
        mcpuid = 0xF00,
        mimpid = 0xF01,
        mhartid = 0xF10,
        mstatus = 0x300,
        mtvec = 0x301,
        mtdeleg = 0x302,
        mie = 0x304,
        mtimecmp = 0x321,
        mtime = 0x701,
        mtimeh = 0x741,
        mscratch = 0x340,
        mepc = 0x341,
        mcause = 0x342,
        mbadaddr = 0x343,
        mip = 0x344,
        mbase = 0x380,
        mbound = 0x381,
        mibase = 0x382,
        mibound = 0x383,
        mdbase = 0x384,
        mdbound = 0x385,
        htimew = 0xB01,
        htimehw = 0xB81,
        mtohost = 0x780,
        mfromhost = 0x781
    };

    enum MemoryAccessType {
        FetchInstruction = 0,
        LoadData = 4,
        StoreData = 6
    };

    UIntType regI[32]; // pc = i[0]
    FloatType regF[32];
    UIntType csr[4096];

    CPU() {
        memset(regI, 0, sizeof(regI));
        memset(regF, 0, sizeof(regF));
        memset(csr, 0, sizeof(csr));
    }

    void checkAlignment(MemoryAccessType mat, UIntType address, UInt8 alignment) {
        if(address%alignment != 0)
            throw Exception(mat);
    }

    template<typename PteType, UInt8 MaxLen, UInt8 MinLen, UInt8 MaxLevel>
    UInt64 translatePaged(MemoryAccessType mat, UIntType src) {
        UInt8 type, i = MaxLevel, offsetLen = 12;
        UInt64 dst = csr[sdst];
        PteType pte;

        bool userMode = true; // TODO

        while(true) {
            dst += ((src>>(i*MinLen+offsetLen))&TrailingBitMask(MinLen))*sizeof(PteType);
            // TODO: Check dst

            pte = ram.getAlignedUInt32(dst);
            if((*pte&0x01) == 0) // Invalid
                throw Exception(mat+1);

            dst = pte>>10;
            type = (pte>>1)&TrailingBitMask(5);
            if(type >= 2) // Leaf
                break;

            if(i == 0) // To many nesting levels
                throw Exception(mat+1);
            --i;

            dst = (dst&TrailingBitMask(MaxLen+MinLen*MaxLevel))<<offsetLen;
        }

        bool storePte = false;
        switch(mat) {
            case FetchInstruction:
                if(userMode) {
                    if(type >= 8 || (type&2) == 0)
                        throw Exception(mat+1);
                }else{
                    if(type < 6 || (type&2) == 0)
                        throw Exception(mat+1);
                }
            break;
            case LoadData:
                if(userMode && type >= 8)
                    throw Exception(mat+1);
            break;
            case StoreData:
                if((type&1) == 0)
                    throw Exception(mat+1);
                if(userMode && type >= 8)
                    throw Exception(mat+1);
                if((pte&(1<<6)) == 0) { // Dirty Bit
                    pte |= 1<<6;
                    storePte = true;
                }
            break;
        }
        if((pte&(1<<5)) == 0) { // Referenced Bit
            pte |= 1<<5;
            storePte = true;
        }

        offsetLen += MinLen*i;
        dst = (dst&TrailingBitMask(MaxLen+MinLen*(MaxLevel-i)))<<offsetLen;
        return dst|(src&TrailingBitMask(offsetLen));
    }

    UInt64 translate(MemoryAccessType mat, UIntType src) {
        UInt64 dst;
        switch((csr[mstatus]>>17)&TrailingBitMask(5)) {
            case 0: // Mbare
                dst = src;
            break;
            case 1: // Mbb
                if(src >= csr[mbound])
                    throw Exception(mat+1);
                dst = src+csr[mbase];
            break;
            case 2: // Mbbid
                if(mat == FetchInstruction) {
                    if(src >= csr[mibound])
                        throw Exception(mat+1);
                    dst = src+csr[mibase];
                }else{
                    if(src >= csr[mdbound])
                        throw Exception(mat+1);
                    dst = src+csr[mdbase];
                }
            break;
            case 8: // Sv32
                dst = translatePaged<UInt32, 12, 10, 1>(mat, src);
            break;
            case 9: // Sv39
                dst = translatePaged<UInt64, 20, 9, 2>(mat, src);
            break;
            case 10: // Sv48
                dst = translatePaged<UInt64, 11, 9, 3>(mat, src);
            break;
            case 11: // Sv57
                dst = translatePaged<UInt64, 16, 9, 4>(mat, src);
            break;
            case 12: // Sv64
                dst = translatePaged<UInt64, 15, 13, 5>(mat, src);
            break;
        }
        return dst;
    }

    void fetchAndExecute() {
        UIntType address = regI[0];
        checkAlignment(FetchInstruction, address, 4);
        address = translate(FetchInstruction, address);
        Instruction next(ram.getAlignedUInt32(address));

    }
};
