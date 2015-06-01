#pragma once
#include "CSR.hpp"

enum ISAExtensions {
    A_AtomicOperations = 1U<<0,
    B_BitManipulation = 1U<<1, // Maybe some day
    C_CompressedInstructions = 1U<<2,
    D_DoubleFloat = 1U<<3,
    E_UNKNOWN = 1U<<4, // TODO Find out what RV32E is
    F_Float = 1U<<5,
    G_ScalarISA = 1U<<6, // IMAFD
    H_HypervisorMode = 1U<<7,
    I_BaseISA = 1U<<8,
    J_UNUSED = 1U<<9,
    K_UNUSED = 1U<<10,
    L_DecimalFloat = 1U<<11, // Maybe some day
    M_MultiplyAndDivide = 1U<<12,
    N_UNUSED = 1U<<13,
    O_UNUSED = 1U<<14,
    P_PackedSIMD = 1U<<15, // Maybe some day
    Q_QuadFloat = 1U<<16, // Maybe some day
    R_UNUSED = 1U<<17,
    S_SupervisorMode = 1U<<18,
    T_TransactionalMemory = 1U<<19, // Maybe some day
    U_UserMode = 1U<<20,
    V_UNUSED = 1U<<21,
    W_UNUSED = 1U<<22,
    X_NonStandardExtensions = 1U<<23,
    Y_UNUSED = 1U<<24,
    Z_UNUSED = 1U<<25
};

template<UInt8 XLEN = 64, ISAExtensions EXT = I_BaseISA>
class CPU {
    public:
    typedef typename std::conditional<XLEN == 32, Int32, typename std::conditional<XLEN == 64, Int64, Int128>::type>::type IntType;
    typedef typename std::conditional<XLEN == 32, UInt32, typename std::conditional<XLEN == 64, UInt64, UInt128>::type>::type UIntType;
    typedef typename std::conditional<EXT&D_DoubleFloat, Float64, Float32>::type FloatType;

    enum MemoryAccessType {
        FetchInstruction = 0,
        LoadData = 4,
        StoreData = 6
    };

    enum PrivilegeMode {
        User = 0,
        Supervisor = 1,
        Hypervisor = 2,
        Machine = 3
    };

    UIntType pc;
    union {
        IntType I;
        UIntType U;
        FloatType F;
    } regX[32];
    union {
        Float32 F32;
        Float64 F64;
        FloatType F;
    } regF[32];
    std::map<UInt16, UIntType> csr;

    void reset() {
        memset(regX, 0, sizeof(regX));
        memset(regF, 0, sizeof(regF));

        csr[csr_fflags] = 0; // TODO
        csr[csr_frm] = 0; // TODO
        csr[csr_fcsr] = 0; // TODO
        csr[csr_cycle] = 0; // TODO
        csr[csr_time] = 0; // TODO
        csr[csr_instret] = 0; // TODO
        csr[csr_cycleh] = 0; // TODO
        csr[csr_timeh] = 0; // TODO
        csr[csr_instreth] = 0; // TODO
        csr[csr_sstatus] = 0; // TODO
        csr[csr_stvec] = 0; // TODO
        csr[csr_sie] = 0; // TODO
        csr[csr_stimecmp] = 0; // TODO
        csr[csr_stime] = 0; // TODO
        csr[csr_stimeh] = 0; // TODO
        csr[csr_sscratch] = 0; // TODO
        csr[csr_sepc] = 0; // TODO
        csr[csr_scause] = 0; // TODO
        csr[csr_sbadaddr] = 0; // TODO
        csr[csr_sip] = 0; // TODO
        csr[csr_sptbr] = 0; // TODO
        csr[csr_sasid] = 0; // TODO
        csr[csr_cyclew] = 0; // TODO
        csr[csr_timew] = 0; // TODO
        csr[csr_instretw] = 0; // TODO
        csr[csr_cyclehw] = 0; // TODO
        csr[csr_timehw] = 0; // TODO
        csr[csr_instrethw] = 0; // TODO
        csr[csr_hstatus] = 0; // TODO
        csr[csr_htvec] = 0; // TODO
        csr[csr_htdeleg] = 0; // TODO
        csr[csr_htimecmp] = 0; // TODO
        csr[csr_htime] = 0; // TODO
        csr[csr_htimeh] = 0; // TODO
        csr[csr_hscratch] = 0; // TODO
        csr[csr_hepc] = 0; // TODO
        csr[csr_hcause] = 0; // TODO
        csr[csr_hbadaddr] = 0; // TODO
        csr[csr_tbd] = 0; // TODO
        csr[csr_stimew] = 0; // TODO
        csr[csr_stimehw] = 0; // TODO

        {
            UIntType value;
            switch(XLEN) {
                case 32:
                    value = 0; // TODO Find out what RV32E is
                break;
                case 64:
                    value = 2;
                break;
                case 128:
                    value = 3;
                break;
            }
            value <<= (XLEN-2);
            value |= setBitsAt(XLEN, 26, XLEN-28);
            value |= setBitsAt(EXT, 0, 26);
            csr[csr_mcpuid] = value;
        }

        csr[csr_mimpid] = 0; // TODO
        csr[csr_mhartid] = 0; // TODO
        csr[csr_mstatus] = 0; // TODO
        csr[csr_mtvec] = 0; // TODO
        csr[csr_mtdeleg] = 0; // TODO
        csr[csr_mie] = 0; // TODO
        csr[csr_mtimecmp] = 0; // TODO
        csr[csr_mtime] = 0; // TODO
        csr[csr_mtimeh] = 0; // TODO
        csr[csr_mscratch] = 0; // TODO
        csr[csr_mepc] = 0; // TODO
        csr[csr_mcause] = 0; // TODO
        csr[csr_mbadaddr] = 0; // TODO
        csr[csr_mip] = 0; // TODO
        csr[csr_mbase] = 0; // TODO
        csr[csr_mbound] = 0; // TODO
        csr[csr_mibase] = 0; // TODO
        csr[csr_mibound] = 0; // TODO
        csr[csr_mdbase] = 0; // TODO
        csr[csr_mdbound] = 0; // TODO
        csr[csr_htimew] = 0; // TODO
        csr[csr_htimehw] = 0; // TODO
        csr[csr_mtohost] = 0; // TODO
        csr[csr_mfromhost] = 0; // TODO
    }

    CPU() {
        reset();
    }

    void dump(std::ostream& out) {
        out << std::setfill('0') << std::hex;
        for(UInt8 i = 0; i < 32; ++i)
            out << std::setw(2) << static_cast<UInt16>(i) << " : " << std::setw(16) << regX[i].U << std::endl;
        for(UInt8 i = 0; i < 32; ++i)
            out << std::setw(2) << static_cast<UInt16>(i) << " : " << regF[i].F << std::endl;
        out << std::endl;
    }

    UIntType readRegXU(UInt8 index) {
        return (index) ? regX[index].U : 0;
    }

    IntType readRegXI(UInt8 index) {
        return (index) ? regX[index].I : 0;
    }

    FloatType readRegXF(UInt8 index) {
        return (index) ? regX[index].F : 0;
    }

    Float32 readRegF32(UInt8 index) {
        return (index) ? regF[index].F32 : 0;
    }

    Float64 readRegF64(UInt8 index) {
        return (index) ? regF[index].F64 : 0;
    }

    FloatType readRegF(UInt8 index) {
        return (index) ? regF[index].F : 0;
    }

    UIntType readCSR(UInt16 index) {
        return csr[index];
    }

    PrivilegeMode readPM(UInt8 index = 0) {
        return (PrivilegeMode)getBitsFrom(readCSR(csr_mstatus), index*3+1, 2);
    }

    void writeRegXU(UInt8 index, UIntType value) {
        regX[index].U = value;
    }

    void writeRegXI(UInt8 index, IntType value) {
        regX[index].I = value;
    }

    void writeRegXF(UInt8 index, FloatType value) {
        regX[index].F = value;
    }

    void writeRegF32(UInt8 index, Float32 value) {
        regF[index].F32 = value;
    }

    void writeRegF64(UInt8 index, Float64 value) {
        regF[index].F64 = value;
    }

    void writeRegF(UInt8 index, FloatType value) {
        regF[index].F = value;
    }

    void writeCSR(UInt16 index, UIntType value) {
        csr[index] = value;
    }

    template<typename type, bool store, bool aligned>
    void memoryAccess(MemoryAccessType mat, UIntType address, type* value) {
        if(aligned && address%sizeof(type) != 0)
            throw MemoryAccessException((Exception::Code)mat, address);

        // TODO: Cache

        if(store)
            ram.set<type, aligned>(address, value);
        else
            ram.get<type, aligned>(address, value);
    }

    template<typename PteType, UInt8 MaxLen, UInt8 MinLen, UInt8 MaxLevel>
    AddressType translatePaged(PrivilegeMode mode, MemoryAccessType mat, UIntType src) {
        UInt8 type, i = MaxLevel, offsetLen = 12;
        AddressType dst = readCSR(csr_sptbr);
        PteType pte;

        // TODO: csr_sasid

        while(true) {
            dst += getBitsFrom(src, i*MinLen+offsetLen, MinLen)*sizeof(PteType);
            // TODO: Check dst

            memoryAccess<PteType, false, true>(mat, dst, &pte);
            if((pte&0x01) == 0) // Invalid
                throw MemoryAccessException((Exception::Code)(mat+1), src);

            type = getBitsFrom(pte, 1, 5);
            if(type >= 2) // Leaf
                break;

            if(i == 0) // To many nesting levels
                throw MemoryAccessException((Exception::Code)(mat+1), src);
            --i;

            dst = getBitsFrom(dst, 10, MaxLen+MinLen*MaxLevel)<<offsetLen;
        }

        bool storePte = false;
        switch(mat) {
            case FetchInstruction:
                if(mode == User) {
                    if(type >= 8 || (type&2) == 0)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                }else{
                    if(type < 6 || (type&2) == 0)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                }
            break;
            case LoadData:
                if(mode == User && type >= 8)
                    throw MemoryAccessException((Exception::Code)(mat+1), src);
            break;
            case StoreData:
                if((type&1) == 0)
                    throw MemoryAccessException((Exception::Code)(mat+1), src);
                if(mode == User && type >= 8)
                    throw MemoryAccessException((Exception::Code)(mat+1), src);
                if((pte&(1ULL<<6)) == 0) { // Dirty Bit
                    pte |= 1ULL<<6;
                    storePte = true;
                }
            break;
        }
        if((pte&(1ULL<<5)) == 0) { // Referenced Bit
            pte |= 1ULL<<5;
            storePte = true;
        }
        if(storePte)
            memoryAccess<PteType, true, true>(mat, dst, &pte);

        offsetLen += MinLen*i;
        dst = getBitsFrom(dst, 10, MaxLen+MinLen*(MaxLevel-i))<<offsetLen;
        return dst|getBitsFrom(src, 0, offsetLen);
    }

    AddressType translate(MemoryAccessType mat, UIntType src) {
        UIntType mstatus = readCSR(csr_mstatus);
        UInt8 mPrv = getBitsFrom(mstatus, 16, 1) & (mat!=FetchInstruction);
        PrivilegeMode mode = readPM(mPrv);
        if(mode == Supervisor) {
            UIntType status = readCSR(csr_sstatus);
            if(getBitsFrom(status, 16, 1) & (mat!=FetchInstruction))
                mode = (PrivilegeMode)getBitsFrom(status, 4, 1);
        }

        AddressType dst;
        switch(getBitsFrom(mstatus, 17, 5)) {
            case 0: // Mbare
                dst = src;
            break;
            case 1: // Mbb
                if(src >= readCSR(csr_mbound))
                    throw MemoryAccessException((Exception::Code)(mat+1), src);
                dst = src+readCSR(csr_mbase);
            break;
            case 2: { // Mbbid
                UIntType halfVAS = 1ULL<<(XLEN-1);
                if(mat == FetchInstruction) {
                    if(src < halfVAS)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                    src -= halfVAS;
                    if(src >= readCSR(csr_mibound))
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                    dst = src+readCSR(csr_mibase);
                }else{
                    if(src >= halfVAS || src >= readCSR(csr_mdbound))
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                    dst = src+readCSR(csr_mdbase);
                }
            } break;
            case 8: // Sv32
                dst = translatePaged<UInt32, 12, 10, 1>(mode, mat, src);
            break;
            case 9: // Sv39
                dst = translatePaged<UInt64, 20, 9, 2>(mode, mat, src);
            break;
            case 10: // Sv48
                dst = translatePaged<UInt64, 11, 9, 3>(mode, mat, src);
            break;
            case 11: // Sv57
                dst = translatePaged<UInt64, 16, 9, 4>(mode, mat, src);
            break;
            case 12: // Sv64
                dst = translatePaged<UInt64, 15, 13, 5>(mode, mat, src);
            break;
        }
        return dst;
    }

    void executeOpcode03(Instruction& instruction) {
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        switch(instruction.funct[0]) {
            case 0: { // LB rd,rs1,imm
                Int8 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegXI(instruction.reg[0], data);
            } break;
            case 1: { // LH rd,rs1,imm
                Int16 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegXI(instruction.reg[0], data);
            } break;
            case 2: { // LW rd,rs1,imm
                Int32 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegXI(instruction.reg[0], data);
            } break;
            case 3: { // LD rd,rs1,imm (64)
                if(XLEN < 64)
                    throw Exception(Exception::Code::IllegalInstruction);
                Int64 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegXI(instruction.reg[0], data);
            } break;
            case 4: { // LBU rd,rs1,imm
                UInt8 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegXU(instruction.reg[0], data);
            } break;
            case 5: { // LHU rd,rs1,imm
                UInt16 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegXU(instruction.reg[0], data);
            } break;
            case 6: { // LWU rd,rs1,imm (64)
                if(XLEN < 64)
                    throw Exception(Exception::Code::IllegalInstruction);
                UInt32 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegXU(instruction.reg[0], data);
            } break;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }
    }

    void executeOpcode07(Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        switch(instruction.funct[0]) {
            case 2: { // FLW rd,rs1,imm (F)
                Float32 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegF32(instruction.reg[0], data);
            } break;
            case 3: { // FLD rd,rs1,imm (D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                Float64 data;
                memoryAccess<decltype(data), false, false>(LoadData, address, &data);
                writeRegF64(instruction.reg[0], data);
            } break;
        }
    }

    void executeOpcode0F(Instruction& instruction) {
        /* I-Type
        FENCE
        FENCE.I
        */
        // TODO : Pipeline, Cache
    }

    void executeOpcode13(Instruction& instruction) {
        switch(instruction.funct[0]) {
            case 0: //ADDI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])+instruction.imm);
            break;
            case 1: //SLLI rd,rs1,shamt
                if(XLEN < 64 && instruction.imm&(1ULL<<5))
                    throw Exception(Exception::Code::IllegalInstruction);
                instruction.imm &= (XLEN < 64) ? TrailingBitMask(5) : TrailingBitMask(6);
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])<<instruction.imm);
            break;
            case 2: //SLTI rd,rs1,imm
                writeRegXU(instruction.reg[0], (readRegXI(instruction.reg[1]) < static_cast<Int32>(instruction.imm)) ? 1 : 0);
            break;
            case 3: //SLTIU rd,rs1,imm
                writeRegXU(instruction.reg[0], (readRegXU(instruction.reg[1]) < instruction.imm) ? 1 : 0);
            break;
            case 4: //XORI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])^instruction.imm);
            break;
            case 5: {
                if(XLEN < 64 && instruction.imm&(1ULL<<5))
                    throw Exception(Exception::Code::IllegalInstruction);
                bool arithmetic = instruction.imm&(1ULL<<10);
                instruction.imm &= (XLEN < 64) ? TrailingBitMask(5) : TrailingBitMask(6);
                if(arithmetic) //SRAI rd,rs1,shamt
                    writeRegXI(instruction.reg[0], readRegXI(instruction.reg[1])>>instruction.imm);
                else //SRLI rd,rs1,shamt
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])>>instruction.imm);
            } break;
            case 6: //ORI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])|instruction.imm);
            break;
            case 7: //ANDI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])&instruction.imm);
            break;
        }
    }

    void executeOpcode17(Instruction& instruction) {
        // AUIPC rd,imm
        writeRegXU(instruction.reg[0], pc+instruction.imm);
    }

    void executeOpcode1B(Instruction& instruction) {
        if(XLEN < 64)
            throw Exception(Exception::Code::IllegalInstruction);
        switch(instruction.funct[0]) {
            case 0: //ADDIW rd,rs1,imm (64)
                writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1])+instruction.imm));
            break;
            case 1: //SLLIW rd,rs1,shamt (64)
                if(instruction.imm&(1ULL<<5))
                    throw Exception(Exception::Code::IllegalInstruction);
                instruction.imm &= TrailingBitMask(5);
                writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1])<<instruction.imm));
            break;
            case 5: {
                if(instruction.imm&(1ULL<<5))
                    throw Exception(Exception::Code::IllegalInstruction);
                bool arithmetic = instruction.imm&(1ULL<<10);
                instruction.imm &= TrailingBitMask(5);
                if(arithmetic) //SRAIW rd,rs1,shamt (64)
                    writeRegXI(instruction.reg[0], static_cast<Int32>(readRegXI(instruction.reg[1])>>instruction.imm));
                else //SRLIW rd,rs1,shamt (64)
                    writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1])>>instruction.imm));
            } break;
        }
    }

    void executeOpcode23(Instruction& instruction) {
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        switch(instruction.funct[0]) {
            case 0: { // SB rs1,rs2,imm
                UInt8 data = readRegXU(instruction.reg[2]);
                memoryAccess<decltype(data), true, false>(StoreData, address, &data);
            } break;
            case 1: { // SH rs1,rs2,imm
                UInt16 data = readRegXU(instruction.reg[2]);
                memoryAccess<decltype(data), true, false>(StoreData, address, &data);
            } break;
            case 2: { // SW rs1,rs2,imm
                UInt32 data = readRegXU(instruction.reg[2]);
                memoryAccess<decltype(data), true, false>(StoreData, address, &data);
            } break;
            case 3: { // SD rs1,rs2,imm (64)
                if(XLEN < 64)
                    throw Exception(Exception::Code::IllegalInstruction);
                UInt64 data = readRegXU(instruction.reg[2]);
                memoryAccess<decltype(data), true, false>(StoreData, address, &data);
            } break;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }
    }

    void executeOpcode27(Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        switch(instruction.funct[0]) {
            case 2: { // FSW rd,rs1,imm (F)
                Float32 data = readRegF32(instruction.reg[2]);
                memoryAccess<decltype(data), true, false>(StoreData, address, &data);
            } break;
            case 3: { // FSD rd,rs1,imm (D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                Float64 data = readRegF64(instruction.reg[2]);
                memoryAccess<decltype(data), true, false>(StoreData, address, &data);
            } break;
        }
    }

    void executeOpcode2F(Instruction& instruction) {
        if(!(EXT&A_AtomicOperations))
            throw Exception(Exception::Code::IllegalInstruction);
        /* U-Type
        LR.W rd,rs1 (A)
        SC.W rd,rs1,rs2 (A)
        AMOSWAP.W rd,rs1,rs2 (A)
        AMOADD.W rd,rs1,rs2 (A)
        AMOXOR.W rd,rs1,rs2 (A)
        AMOAND.W rd,rs1,rs2 (A)
        AMOOR.W rd,rs1,rs2 (A)
        AMOMIN.W rd,rs1,rs2 (A)
        AMOMAX.W rd,rs1,rs2 (A)
        AMOMINU.W rd,rs1,rs2 (A)
        AMOMAXU.W rd,rs1,rs2 (A)
        LR.D rd,rs1 (A, 64)
        SC.D rd,rs1,rs2 (A, 64)
        AMOSWAP.D rd,rs1,rs2 (A, 64)
        AMOADD.D rd,rs1,rs2 (A, 64)
        AMOXOR.D rd,rs1,rs2 (A, 64)
        AMOAND.D rd,rs1,rs2 (A, 64)
        AMOOR.D rd,rs1,rs2 (A, 64)
        AMOMIN.D rd,rs1,rs2 (A, 64)
        AMOMAX.D rd,rs1,rs2 (A, 64)
        AMOMINU.D rd,rs1,rs2 (A, 64)
        AMOMAXU.D rd,rs1,rs2 (A, 64)
        */
        // TODO : Multi core
    }

    void executeOpcode33(Instruction& instruction) {
        if(instruction.funct[0] == 1) {
            if(!(EXT&M_MultiplyAndDivide))
                throw Exception(Exception::Code::IllegalInstruction);
            switch(instruction.funct[1]) {
                case 0: // MUL rd,rs1,rs2 (M)
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])*readRegXU(instruction.reg[2]));
                break;
                case 1: // MULH rd,rs1,rs2 (M)
                    writeRegXI(instruction.reg[0], (
                         static_cast<Int128>(readRegXU(instruction.reg[1]))
                        *static_cast<Int128>(readRegXU(instruction.reg[2]))
                    )>>64);
                break;
                case 2: // MULHSU rd,rs1,rs2 (M)
                    writeRegXI(instruction.reg[0], (
                          static_cast<Int128>(readRegXU(instruction.reg[1]))
                        *static_cast<UInt128>(readRegXU(instruction.reg[2]))
                    )>>64);
                break;
                case 3: // MULHU rd,rs1,rs2 (M)
                    writeRegXU(instruction.reg[0], (
                         static_cast<UInt128>(readRegXU(instruction.reg[1]))
                        *static_cast<UInt128>(readRegXU(instruction.reg[2]))
                    )>>64);
                break;
                case 4: // DIV rd,rs1,rs2 (M)
                    writeRegXI(instruction.reg[0], readRegXI(instruction.reg[1])/readRegXI(instruction.reg[2]));
                break;
                case 5: // DIVU rd,rs1,rs2 (M)
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])/readRegXU(instruction.reg[2]));
                break;
                case 6: // REM rd,rs1,rs2 (M)
                    writeRegXI(instruction.reg[0], readRegXI(instruction.reg[1])%readRegXI(instruction.reg[2]));
                break;
                case 7: // REMU rd,rs1,rs2 (M)
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])%readRegXU(instruction.reg[2]));
                break;
            }
        }else{
    		switch(instruction.funct[1]) {
    			case 0:
                    if(instruction.funct[0] == 0) //ADD rd,rs1,rs2
                        writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])+readRegXU(instruction.reg[2]));
                    else if(instruction.funct[0] == 32) //SUB rd,rs1,rs2
                        writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])-readRegXU(instruction.reg[2]));
                    else
                        throw Exception(Exception::Code::IllegalInstruction);
    			break;
    			case 1: // SLL rd,rs1,rs2
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])<<readRegXU(instruction.reg[2]));
    			break;
    			case 2: // SLT rd,rs1,rs2
                    writeRegXU(instruction.reg[0], (readRegXI(instruction.reg[1]) < readRegXI(instruction.reg[2]))?1:0);
    			break;
    			case 3: // SLTU rd,rs1,rs2
                    writeRegXU(instruction.reg[0], (readRegXU(instruction.reg[1]) < readRegXU(instruction.reg[2]))?1:0);
    			break;
    			case 4: // XOR rd,rs1,rs2
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])^readRegXU(instruction.reg[2]));
    			break;
    			case 5:
                    if(instruction.funct[0] == 0) //SRL rd,rs1,rs2
                        writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])>>readRegXU(instruction.reg[2]));
                    else if(instruction.funct[0] == 32) //SRA rd,rs1,rs2
                        writeRegXI(instruction.reg[0], readRegXI(instruction.reg[1])>>readRegXI(instruction.reg[2]));
                    else
                        throw Exception(Exception::Code::IllegalInstruction);
    			break;
    			case 6: // OR rd,rs1,rs2
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])|readRegXU(instruction.reg[2]));
    			break;
    			case 7: // AND rd,rs1,rs2
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])&readRegXU(instruction.reg[2]));
    			break;
    		}
        }
    }

    void executeOpcode37(Instruction& instruction) {
        // LUI rd,imm
        writeRegXU(instruction.reg[0], instruction.imm);
    }

    void executeOpcode3B(Instruction& instruction) {
        if(XLEN < 64)
            throw Exception(Exception::Code::IllegalInstruction);
        if(instruction.funct[0] == 1) {
            if(!(EXT&M_MultiplyAndDivide))
                throw Exception(Exception::Code::IllegalInstruction);
            switch(instruction.funct[1]) {
                case 0: // MULW rd,rs1,rs2 (M, 64)
                    writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1]))*static_cast<UInt32>(readRegXU(instruction.reg[2])));
                break;
                case 4: // DIVW rd,rs1,rs2 (M, 64)
                    writeRegXI(instruction.reg[0], static_cast<Int32>(readRegXU(instruction.reg[1]))/static_cast<Int32>(readRegXU(instruction.reg[2])));
                break;
                case 5: // DIVUW rd,rs1,rs2 (M, 64)
                    writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1]))/static_cast<UInt32>(readRegXU(instruction.reg[2])));
                break;
                case 6: // REMW rd,rs1,rs2 (M, 64)
                    writeRegXI(instruction.reg[0], static_cast<Int32>(readRegXU(instruction.reg[1]))/static_cast<Int32>(readRegXU(instruction.reg[2])));
                break;
                case 7: // REMUW rd,rs1,rs2 (M, 64)
                    writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1]))%static_cast<UInt32>(readRegXU(instruction.reg[2])));
                break;
            }
        }else{
    		switch(instruction.funct[1]) {
    			case 0:
                    if(instruction.funct[0] == 0) //ADDW rd,rs1,rs2 (64)
                        writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1]))+static_cast<UInt32>(readRegXU(instruction.reg[2])));
                    else if(instruction.funct[0] == 32) //SUBW rd,rs1,rs2 (64)
                        writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1]))-static_cast<UInt32>(readRegXU(instruction.reg[2])));
                    else
                        throw Exception(Exception::Code::IllegalInstruction);
    			break;
    			case 1: // SLLW rd,rs1,rs2 (64)
                    writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1]))<<static_cast<UInt32>(readRegXU(instruction.reg[2])));
    			break;
    			case 5:
                    if(instruction.funct[0] == 0) //SRLW rd,rs1,rs2 (64)
                        writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1]))>>static_cast<UInt32>(readRegXU(instruction.reg[2])));
                    else if(instruction.funct[0] == 32) //SRAW rd,rs1,rs2 (64)
                        writeRegXI(instruction.reg[0], static_cast<Int32>(readRegXI(instruction.reg[1]))>>static_cast<Int32>(readRegXI(instruction.reg[2])));
                    else
                        throw Exception(Exception::Code::IllegalInstruction);
    			break;
    		}
        }
    }

    void executeOpcode43(Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        /* R4-Type
        FMADD.S rd,rs1,rs2,rs3 (F)
        FMADD.D rd,rs1,rs2,rs3 (D)
        */
        // TODO: Float arithmetic
    }

    void executeOpcode47(Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        /* R4-Type
        FMSUB.S rd,rs1,rs2,rs3 (F)
        FMSUB.D rd,rs1,rs2,rs3 (D)
        */
        // TODO: Float arithmetic
    }

    void executeOpcode4B(Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        /* R4-Type
        FNMSUB.S rd,rs1,rs2,rs3 (F)
        FNMSUB.D rd,rs1,rs2,rs3 (D)
        */
        // TODO: Float arithmetic
    }

    void executeOpcode4F(Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        /* R4-Type
        FNMADD.S rd,rs1,rs2,rs3 (F)
        FNMADD.D rd,rs1,rs2,rs3 (D)
        */
        // TODO: Float arithmetic
    }

    void executeOpcode53(Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        /* R-Type
        FADD.S rd,rs1,rs2 (F)
        FSUB.S rd,rs1,rs2 (F)
        FMUL.S rd,rs1,rs2 (F)
        FDIV.S rd,rs1,rs2 (F)
        FSQRT.S rd,rs1 (F)
        FSGNJ.S rd,rs1,rs2 (F)
        FSGNJN.S rd,rs1,rs2 (F)
        FSGNJX.S rd,rs1,rs2 (F)
        FMIN.S rd,rs1,rs2 (F)
        FMAX.S rd,rs1,rs2 (F)
        FCVT.W.S rd,rs1 (F)
        FCVT.WU.S rd,rs1 (F)
        FMV.X.S rd,rs1 (F)
        FEQ.S rd,rs1,rs2 (F)
        FLT.S rd,rs1,rs2 (F)
        FLE.S rd,rs1,rs2 (F)
        FCLASS.S rd,rs1 (F)
        FCVT.S.W rd,rs1 (F)
        FCVT.S.WU rd,rs1 (F)
        FMV.S.X rd,rs1 (F)
        FCVT.L.S rd,rs1 (F, 64)
        FCVT.LU.S rd,rs1 (F, 64)
        FCVT.S.L rd,rs1 (F, 64)
        FCVT.S.LU rd,rs1 (F, 64)
        FADD.D rd,rs1,rs2 (D)
        FSUB.D rd,rs1,rs2 (D)
        FMUL.D rd,rs1,rs2 (D)
        FDIV.D rd,rs1,rs2 (D)
        FSQRT.D rd,rs1 (D)
        FSGNJ.D rd,rs1,rs (D)
        FSGNJN.D rd,rs1,r (D)
        FSGNJX.D rd,rs1,r (D)
        FMIN.D rd,rs1,rs2 (D)
        FMAX.D rd,rs1,rs2 (D)
        FCVT.S.D rd,rs1 (D)
        FCVT.D.S rd,rs1 (D)
        FEQ.D rd,rs1,rs2 (D)
        FLT.D rd,rs1,rs2 (D)
        FLE.D rd,rs1,rs2 (D)
        FCLASS.D rd,rs1 (D)
        FCVT.W.D rd,rs1 (D)
        FCVT.WU.D rd,rs1 (D)
        FCVT.D.W rd,rs1 (D)
        FCVT.D.WU rd,rs1 (D)
        FCVT.L.D rd,rs1 (D, 64)
        FCVT.LU.D rd,rs1 (D, 64)
        FMV.X.D rd,rs1 (D, 64)
        FCVT.D.L rd,rs1 (D, 64)
        FCVT.D.LU rd,rs1 (D, 64)
        FMV.D.X rd,rs1 (D, 64)
        */
        // TODO: Float arithmetic
    }

    void executeOpcode63(Instruction& instruction) {
        switch(instruction.funct[0]) {
            case 0: // BEQ rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) == readRegXU(instruction.reg[2])) break;
            return;
            case 1:// BNE rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) != readRegXU(instruction.reg[2])) break;
            return;
            case 4:// BLT rs1,rs2,imm
                if(readRegXI(instruction.reg[1]) < readRegXI(instruction.reg[2])) break;
            return;
            case 5:// BGE rs1,rs2,imm
                if(readRegXI(instruction.reg[1]) > readRegXI(instruction.reg[2])) break;
            return;
            case 6:// BLTU rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) < readRegXU(instruction.reg[2])) break;
            return;
            case 7:// BGEU rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) > readRegXU(instruction.reg[2])) break;
            return;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }
        pc += instruction.imm;
    }

    void executeOpcode67(Instruction& instruction, UIntType pcNextValue) {
        // JALR rd,rs1,imm
        writeRegXU(instruction.reg[0], pcNextValue);
        pc = (readRegXU(instruction.reg[1])+instruction.imm)&~TrailingBitMask(1);
    }

    void executeOpcode6F(Instruction& instruction, UIntType pcNextValue) {
        // JAL rd,imm
        writeRegXU(instruction.reg[0], pcNextValue);
        pc += instruction.imm;
    }

    void executeOpcode73(Instruction& instruction, UIntType pcNextValue) {
        switch(instruction.funct[0]) {
            case 0:
                switch(instruction.imm) {
                    case 0x0000: // ECALL

                    break;
                    case 0x0001: // EBREAK

                    break;
                    case 0x0100: // ERET

                    break;
                    case 0x0101: // SFENCE.VM rs1
                        // TODO : Flush caches
                    return;
                    case 0x0102: // WFI
                        // TODO
                    break;
                    case 0x0205: // HRTS
                        // TODO : Redirect Trap
                    break;
                    case 0x0305: // MRTS
                        // TODO : Redirect Trap
                    break;
                    case 0x0306: // MRTH
                        // TODO : Redirect Trap
                    break;
                    default:
                        throw Exception(Exception::Code::IllegalInstruction);
                }
            break;
            case 1: { // CSRRW rd,csr,rs1
                UIntType value = readRegXU(instruction.reg[1]);
                writeRegXU(instruction.reg[0], readCSR(instruction.imm));
                writeCSR(instruction.imm, value);
            } break;
            case 2: { // CSRRS rd,csr,rs1
                UIntType value = readRegXU(instruction.reg[1]),
                         csr = readCSR(instruction.imm);
                writeRegXU(instruction.reg[0], csr);
                if(instruction.reg[1])
                    writeCSR(instruction.imm, csr|value);
            } break;
            case 3: { // CSRRC rd,csr,rs1
                UIntType value = readRegXU(instruction.reg[1]),
                         csr = readCSR(instruction.imm);
                writeRegXU(instruction.reg[0], csr);
                if(instruction.reg[1])
                    writeCSR(instruction.imm, csr&~value);
            } break;
            case 5: { // CSRRWI rd,csr,imm
                UIntType value = instruction.reg[1];
                writeRegXU(instruction.reg[0], readCSR(instruction.imm));
                writeCSR(instruction.imm, value);
            } break;
            case 6: { // CSRRSI rd,csr,imm
                UIntType value = instruction.reg[1],
                         csr = readCSR(instruction.imm);
                writeRegXU(instruction.reg[0], csr);
                if(value)
                    writeCSR(instruction.imm, csr|value);
            } break;
            case 7: { // CSRRCI rd,csr,imm
                UIntType value = instruction.reg[1],
                         csr = readCSR(instruction.imm);
                writeRegXU(instruction.reg[0], csr);
                if(value)
                    writeCSR(instruction.imm, csr&~value);
            } break;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }

        pc = pcNextValue;
    }

    void fetchAndExecute() {
        PrivilegeMode cpm = readPM();

        try {
            UIntType pcNextValue = pc, mappedPC = translate(FetchInstruction, pc);
            Instruction instruction;
            bool compressed = false; // TODO : Waiting for next riscv-compressed-spec
            if(compressed) {
                UInt16 rawInstruction;
                memoryAccess<UInt16, false, true>(FetchInstruction, mappedPC, &rawInstruction);
                instruction.decode16(rawInstruction);
                pcNextValue += 2;
            }else{
                UInt32 rawInstruction;
                memoryAccess<UInt32, false, true>(FetchInstruction, mappedPC, &rawInstruction);
                instruction.decode32(rawInstruction);
                pcNextValue += 4;
            }
            switch(instruction.opcode) {
                case 0x03:
                    executeOpcode03(instruction);
                break;
        		case 0x07:
                    executeOpcode07(instruction);
                break;
        		case 0x0F:
                    executeOpcode0F(instruction);
                break;
        		case 0x13:
                    executeOpcode13(instruction);
                break;
                case 0x17:
                    executeOpcode17(instruction);
                break;
        		case 0x1B:
                    executeOpcode1B(instruction);
                break;
                case 0x23:
                    executeOpcode23(instruction);
                break;
        		case 0x27:
                    executeOpcode27(instruction);
                break;
                case 0x2F:
                    executeOpcode2F(instruction);
                break;
        		case 0x33:
                    executeOpcode33(instruction);
                break;
                case 0x37:
                    executeOpcode37(instruction);
                break;
        		case 0x3B:
                    executeOpcode3B(instruction);
                break;
                case 0x43:
                    executeOpcode43(instruction);
                break;
        		case 0x47:
                    executeOpcode47(instruction);
                break;
        		case 0x4B:
                    executeOpcode4B(instruction);
                break;
        		case 0x4F:
                    executeOpcode4F(instruction);
                break;
        		case 0x53:
                    executeOpcode53(instruction);
                break;
                case 0x63:
                    executeOpcode63(instruction);
                return;
        		case 0x67:
                    executeOpcode67(instruction, pcNextValue);
                return;
                case 0x6F:
                    executeOpcode6F(instruction, pcNextValue);
                return;
        		case 0x73:
                    executeOpcode73(instruction, pcNextValue);
                return;
            }
            pc = pcNextValue;
            return;
        } catch(MemoryAccessException e) {
            writeCSR(csr_mbadaddr, e.address);
            writeCSR(csr_mcause, e.cause);
        } catch(Exception e) {
            writeCSR(csr_mcause, e.cause);
        }
        writeCSR(csr_mepc, pc);
        printf("TRAPED!\n");

        // TODO : Exception Trap

        /* TODO : mtdeleg, htdeleg
        switch(cpm) {
            case Supervisor:

            break;
            case Hypervisor:

            break;
            case Machine:

            break;
        }*/
    }
};
