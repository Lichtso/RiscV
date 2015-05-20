#pragma once
#include "CSR.hpp"

template<UInt8 XLEN = 64>
class CPU {
    public:
    typedef typename std::conditional<XLEN == 64, Int64, Int32>::type IntType;
    typedef typename std::conditional<XLEN == 64, UInt64, UInt32>::type UIntType;
    typedef typename std::conditional<XLEN == 64, Float64, Float32>::type FloatType;

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

    UIntType regI[32]; // pc = i[0]
    FloatType regF[32];
    std::map<UInt16, UIntType> csr;

    CPU() {
        memset(regI, 0, sizeof(regI));
        memset(regF, 0, sizeof(regF));
    }

    template<typename type, bool store, bool aligned>
    void memoryAccess(MemoryAccessType mat, UIntType address, type* value) {
        if(aligned && address%sizeof(type) != 0)
            throw MemoryAccessException(mat, address);

        // TODO: TLB

        if(store)
            ram.set<type, aligned>(address, value);
        else
            ram.get<type, aligned>(address, value);
    }

    template<typename PteType, UInt8 MaxLen, UInt8 MinLen, UInt8 MaxLevel>
    AddressType translatePaged(PrivilegeMode mode, MemoryAccessType mat, UIntType src) {
        UInt8 type, i = MaxLevel, offsetLen = 12;
        AddressType dst = csr[csr_sptbr];
        PteType pte;

        // TODO: csr_sasid

        while(true) {
            dst += getBitsFrom(src, i*MinLen+offsetLen, MinLen)*sizeof(PteType);
            // TODO: Check dst

            memoryAccess<PteType, false, true>(mat, dst, &pte);
            if((*pte&0x01) == 0) // Invalid
                throw MemoryAccessException(mat+1, src);

            type = getBitsFrom(pte, 1, 5);
            if(type >= 2) // Leaf
                break;

            if(i == 0) // To many nesting levels
                throw MemoryAccessException(mat+1, src);
            --i;

            dst = getBitsFrom(dst, 10, MaxLen+MinLen*MaxLevel)<<offsetLen;
        }

        bool storePte = false;
        switch(mat) {
            case FetchInstruction:
                if(mode == User) {
                    if(type >= 8 || (type&2) == 0)
                        throw MemoryAccessException(mat+1, src);
                }else{
                    if(type < 6 || (type&2) == 0)
                        throw MemoryAccessException(mat+1, src);
                }
            break;
            case LoadData:
                if(mode == User && type >= 8)
                    throw MemoryAccessException(mat+1, src);
            break;
            case StoreData:
                if((type&1) == 0)
                    throw MemoryAccessException(mat+1, src);
                if(mode == User && type >= 8)
                    throw MemoryAccessException(mat+1, src);
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
        if(storePte)
            memoryAccess<PteType, true, true>(mat, dst, &pte);

        offsetLen += MinLen*i;
        dst = getBitsFrom(dst, 10, MaxLen+MinLen*(MaxLevel-i))<<offsetLen;
        return dst|getBitsFrom(src, 0, offsetLen);
    }

    AddressType translate(MemoryAccessType mat, UIntType src) {
        UIntType mstatus = csr[csr_mstatus];
        UInt8 mPrv = getBitsFrom(mstatus, 16, 1) & (mat!=FetchInstruction);
        PrivilegeMode mode = getBitsFrom(mstatus, mPrv*3+1, 2);
        if(mode == Supervisor) {
            UIntType status = csr[csr_sstatus];
            if(getBitsFrom(status, 16, 1) & (mat!=FetchInstruction))
                mode = getBitsFrom(status, 4, 1);
        }

        AddressType dst;
        switch(getBitsFrom(mstatus, 17, 5)) {
            case 0: // Mbare
                dst = src;
            break;
            case 1: // Mbb
                if(src >= csr[csr_mbound])
                    throw MemoryAccessException(mat+1, src);
                dst = src+csr[csr_mbase];
            break;
            case 2: { // Mbbid
                UIntType halfVAS = 1<<(sizeof(UIntType)*8-1);
                if(mat == FetchInstruction) {
                    if(src < halfVAS)
                        throw MemoryAccessException(mat+1, src);
                    src -= halfVAS;
                    if(src >= csr[csr_mibound])
                        throw MemoryAccessException(mat+1, src);
                    dst = src+csr[csr_mibase];
                }else{
                    if(src >= halfVAS || src >= csr[csr_mdbound])
                        throw MemoryAccessException(mat+1, src);
                    dst = src+csr[csr_mdbase];
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

    void executeOpcode03(const Instruction& instruction) {
        /* I-Type
        LB rd,rs1,imm
        LH rd,rs1,imm
        LW rd,rs1,imm
        LBU rd,rs1,imm
        LHU rd,rs1,imm
        LWU rd,rs1,imm
        LD rd,rs1,imm
        */
    }

    void executeOpcode07(const Instruction& instruction) {
        /* I-Type
        FLW rd,rs1,imm
        FLD rd,rs1,imm
        */
    }

    void executeOpcode0F(const Instruction& instruction) {
        /* I-Type
        FENCE
        FENCE.I
        */
    }

    void executeOpcode13(const Instruction& instruction) {
        /* I-Type
        ADDI rd,rs1,imm
        SLTI rd,rs1,imm
        SLTIU rd,rs1,imm
        XORI rd,rs1,imm
        ORI rd,rs1,imm
        ANDI rd,rs1,imm
        SLLI rd,rs1,shamt
        SRLI rd,rs1,shamt
        SRAI rd,rs1,shamt
        */
    }

    void executeOpcode17(const Instruction& instruction) {
        /* U-Type
        AUIPC rd,imm
        */
    }

    void executeOpcode1B(const Instruction& instruction) {
        /* I-Type
        ADDIW rd,rs1,imm
        SLLIW rd,rs1,shamt
        SRLIW rd,rs1,shamt
        SRAIW rd,rs1,shamt
        */
    }

    void executeOpcode23(const Instruction& instruction) {
        /* S-Type
        SB rs1,rs2,imm
        SH rs1,rs2,imm
        SW rs1,rs2,imm
        SD rs1,rs2,imm
        */
    }

    void executeOpcode27(const Instruction& instruction) {
        /* S-Type
        FSW rs1,rs2,imm
        FSD rs1,rs2,imm
        */
    }

    void executeOpcode2F(const Instruction& instruction) {
        /* U-Type
        LR.W rd,rs1
        SC.W rd,rs1,rs2
        AMOSWAP.W rd,rs1,rs2
        AMOADD.W rd,rs1,rs2
        AMOXOR.W rd,rs1,rs2
        AMOAND.W rd,rs1,rs2
        AMOOR.W rd,rs1,rs2
        AMOMIN.W rd,rs1,rs2
        AMOMAX.W rd,rs1,rs2
        AMOMINU.W rd,rs1,rs2
        AMOMAXU.W rd,rs1,rs2
        LR.D rd,rs1
        SC.D rd,rs1,rs2
        AMOSWAP.D rd,rs1,rs2
        AMOADD.D rd,rs1,rs2
        AMOXOR.D rd,rs1,rs2
        AMOAND.D rd,rs1,rs2
        AMOOR.D rd,rs1,rs2
        AMOMIN.D rd,rs1,rs2
        AMOMAX.D rd,rs1,rs2
        AMOMINU.D rd,rs1,rs2
        AMOMAXU.D rd,rs1,rs2
        */
    }

    void executeOpcode33(const Instruction& instruction) {
        /* R-Type
        ADD rd,rs1,rs2
        SUB rd,rs1,rs2
        SLL rd,rs1,rs2
        SLT rd,rs1,rs2
        SLTU rd,rs1,rs2
        XOR rd,rs1,rs2
        SRL rd,rs1,rs2
        SRA rd,rs1,rs2
        OR rd,rs1,rs2
        AND rd,rs1,rs2
        MUL rd,rs1,rs2
        MULH rd,rs1,rs2
        MULHSU rd,rs1,rs2
        MULHU rd,rs1,rs2
        DIV rd,rs1,rs2
        DIVU rd,rs1,rs2
        REM rd,rs1,rs2
        REMU rd,rs1,rs2
        */
    }

    void executeOpcode37(const Instruction& instruction) {
        /* U-Type
        LUI rd,imm
        */
    }

    void executeOpcode3B(const Instruction& instruction) {
        /* R-Type
        ADDW rd,rs1,rs2
        SUBW rd,rs1,rs2
        SLLW rd,rs1,rs2
        SRLW rd,rs1,rs2
        SRAW rd,rs1,rs2
        MULW rd,rs1,rs2
        DIVW rd,rs1,rs2
        DIVUW rd,rs1,rs2
        REMW rd,rs1,rs2
        REMUW rd,rs1,rs2
        */
    }

    void executeOpcode43(const Instruction& instruction) {
        /* R4-Type
        FMADD.S rd,rs1,rs2,rs3
        FMADD.D rd,rs1,rs2,rs3
        */
    }

    void executeOpcode47(const Instruction& instruction) {
        /* R4-Type
        FMSUB.S rd,rs1,rs2,rs3
        FMSUB.D rd,rs1,rs2,rs3
        */
    }

    void executeOpcode4B(const Instruction& instruction) {
        /* R4-Type
        FNMSUB.S rd,rs1,rs2,rs3
        FNMSUB.D rd,rs1,rs2,rs3
        */
    }

    void executeOpcode4F(const Instruction& instruction) {
        /* R4-Type
        FNMADD.S rd,rs1,rs2,rs3
        FNMADD.D rd,rs1,rs2,rs3
        */
    }

    void executeOpcode53(const Instruction& instruction) {
        /* R-Type
        FADD.S rd,rs1,rs2
        FSUB.S rd,rs1,rs2
        FMUL.S rd,rs1,rs2
        FDIV.S rd,rs1,rs2
        FSQRT.S rd,rs1
        FSGNJ.S rd,rs1,rs2
        FSGNJN.S rd,rs1,rs2
        FSGNJX.S rd,rs1,rs2
        FMIN.S rd,rs1,rs2
        FMAX.S rd,rs1,rs2
        FCVT.W.S rd,rs1
        FCVT.WU.S rd,rs1
        FMV.X.S rd,rs1
        FEQ.S rd,rs1,rs2
        FLT.S rd,rs1,rs2
        FLE.S rd,rs1,rs2
        FCLASS.S rd,rs1
        FCVT.S.W rd,rs1
        FCVT.S.WU rd,rs1
        FMV.S.X rd,rs1
        FRCSR rd
        FRRM rd
        FRFLAGS rd
        FSCSR rd,rs1
        FSRM rd,rs1
        FSFLAGS rd,rs1
        FSRMI rd,imm
        FSFLAGSI rd,imm
        FCVT.L.S rd,rs1
        FCVT.LU.S rd,rs1
        FCVT.S.L rd,rs1
        FCVT.S.LU rd,rs1
        FADD.D rd,rs1,rs2
        FSUB.D rd,rs1,rs2
        FMUL.D rd,rs1,rs2
        FDIV.D rd,rs1,rs2
        FSQRT.D rd,rs1
        FSGNJ.D rd,rs1,rs
        FSGNJN.D rd,rs1,r
        FSGNJX.D rd,rs1,r
        FMIN.D rd,rs1,rs2
        FMAX.D rd,rs1,rs2
        FCVT.S.D rd,rs1
        FCVT.D.S rd,rs1
        FEQ.D rd,rs1,rs2
        FLT.D rd,rs1,rs2
        FLE.D rd,rs1,rs2
        FCLASS.D rd,rs1
        FCVT.W.D rd,rs1
        FCVT.WU.D rd,rs1
        FCVT.D.W rd,rs1
        FCVT.D.WU rd,rs1
        FCVT.L.D rd,rs1
        FCVT.LU.D rd,rs1
        FMV.X.D rd,rs1
        FCVT.D.L rd,rs1
        FCVT.D.LU rd,rs1
        FMV.D.X rd,rs1
        */
    }

    void executeOpcode63(const Instruction& instruction) {
        /* B-Type
        BEQ rs1,rs2,imm
        BNE rs1,rs2,imm
        BLT rs1,rs2,imm
        BGE rs1,rs2,imm
        BLTU rs1,rs2,imm
        BGEU rs1,rs2,imm
        */
    }

    void executeOpcode67(const Instruction& instruction) {
        /* I-Type
        JALR rd,rs1,imm
        */
    }

    void executeOpcode6F(const Instruction& instruction) {
        /* J-Type
        JAL rd,imm
        */
    }

    void executeOpcode73(const Instruction& instruction) {
        /* I-Type
        SCALL
        SBREAK
        RDCYCLE rd
        RDCYCLEH rd
        RDTIME rd
        RDTIMEH rd
        RDINSTRET rd
        RDINSTRETH rd
        CSRRW rd,csr,rs1
        CSRRS rd,csr,rs1
        CSRRC rd,csr,rs1
        CSRRWI rd,csr,imm
        CSRRSI rd,csr,imm
        CSRRCI rd,csr,imm
        ECALL
        EBREAK
        ERET
        MRTS
        MRTH
        HRTS
        WFI
        SFENCE.VM rs1
        */
    }

    void fetchAndExecute() {
        UIntType pc = regI[0];
        PrivilegeMode cpm = getBitsFrom(csr[csr_mstatus], 1, 2);

        try {
            AddressType mappedPC = translate(FetchInstruction, pc);
            InstructionType rawInstruction;
            memoryAccess<InstructionType, false, true>(FetchInstruction, mappedPC, &rawInstruction);
            Instruction instruction;
            instruction.decode32(rawInstruction);
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
                break;
        		case 0x67:
                executeOpcode67(instruction);
                break;
                case 0x6F:
                executeOpcode6F(instruction);
                break;
        		case 0x73:
                executeOpcode73(instruction);
                break;
            }
            return;
        } catch(MemoryAccessException e) {
            csr[csr_mbadaddr] = e.address;
            csr[csr_mcause] = e.cause;
        } catch(Exception e) {
            csr[csr_mcause] = e.cause;
        }

        csr[csr_mepc] = pc;
        // TODO Exception Trap

        /* TODO mtdeleg, htdeleg
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
