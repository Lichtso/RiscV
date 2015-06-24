#ifndef CPU
#define CPU

#include "CSR.hpp"

enum ISAExtensions {
    A_AtomicOperations = 1U<<0,
    B_BitManipulation = 1U<<1, // Maybe some day
    C_CompressedInstructions = 1U<<2,
    D_DoubleFloat = 1U<<3,
    E_UNKNOWN = 1U<<4, // TODO [Find out what RV32E is]
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
class Cpu {
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
    } regX[32];
    union {
        Float32 F32;
        Float64 F64;
        FloatType F;
    } regF[32];
    struct {
        UInt8 fflags;
        UIntType frm;
        UIntType fcsr;
        UIntType stvec;
        UIntType sie;
        UIntType stimecmp;
        UIntType sscratch;
        UIntType sepc;
        UIntType scause;
        UIntType sbadaddr;
        UIntType sip;
        UIntType sptbr;
        UIntType sasid;
        UInt64 cycle;
        UInt64 time;
        UInt64 instret;
        UIntType htvec;
        UIntType htdeleg;
        UIntType htimecmp;
        UIntType hscratch;
        UIntType hepc;
        UIntType hcause;
        UIntType hbadaddr;
        UInt64 stime;
        UIntType mcpuid;
        UIntType mimpid;
        UIntType mhartid;
        UIntType status;
        UIntType mtvec;
        UIntType mtdeleg;
        UIntType mie;
        UIntType mtimecmp;
        UInt64 mtime;
        UIntType mscratch;
        UIntType mepc;
        UIntType mcause;
        UIntType mbadaddr;
        UIntType mip;
        UIntType mbase;
        UIntType mbound;
        UIntType mibase;
        UIntType mibound;
        UIntType mdbase;
        UIntType mdbound;
        UInt64 htime;
        UIntType mtohost;
        UIntType mfromhost;
    } csr;
    std::set<std::pair<AddressType, UInt8>> seals;

    constexpr UIntType getStatusCSRMask(PrivilegeMode mode) {
        UIntType mask;
        switch(mode) {
            case Supervisor:
                mask = 0x1F019;
            break;
            default:
                return 0;
        }
        setBitsIn(mask, 1ULL, XLEN-1, 1);
        return mask;
    }

    constexpr UInt8 getLevels() {
        if(EXT&H_HypervisorMode) return 4;
        if(EXT&S_SupervisorMode) return 3;
        if(EXT&U_UserMode) return 2;
        return 1;
    }

    void reset() {
        memset(regX, 0, sizeof(regX));
        memset(regF, 0, sizeof(regF));

        // TODO : multi core clock
        // TODO : timer interrupts

        csr.fflags = 0;
        csr.frm = 0;
        csr.fcsr = 0;
        csr.stvec = 0;
        csr.sie = 0;
        csr.stimecmp = 0;
        csr.sscratch = 0;
        csr.sepc = 0;
        csr.scause = 0;
        csr.sbadaddr = 0;
        csr.sip = 0;
        csr.sptbr = 0;
        csr.sasid = 0;
        csr.htvec = 0;
        csr.htdeleg = 0;
        csr.htimecmp = 0;
        csr.hscratch = 0;
        csr.hepc = 0;
        csr.hcause = 0;
        csr.hbadaddr = 0;

        {
            csr.status = 6;
            for(UInt8 l = 1; l < getLevels(); ++l)
                csr.status |= 1<<(l*3);
            // TODO XS, VM, SD
        }
        csr.mtvec = 0x100;
        pc = csr.mtvec+0x100;

        csr.mtdeleg = 0;
        csr.mie = 0;
        csr.mtimecmp = 0;
        csr.mtime = 0;
        csr.mscratch = 0;
        csr.mepc = 0;
        csr.mcause = 0;
        csr.mbadaddr = 0;
        csr.mip = 0;
        csr.mbase = 0;
        csr.mbound = 0;
        csr.mibase = 0;
        csr.mibound = 0;
        csr.mdbase = 0;
        csr.mdbound = 0;
        csr.mtohost = 0;
        csr.mfromhost = 0;
    }

    Cpu(UIntType index = 0) {
        reset();

        UIntType mcpuid;
        switch(XLEN) {
            case 32:
                mcpuid = 0; // TODO Find out what RV32E is
            break;
            case 64:
                mcpuid = 2;
            break;
            case 128:
                mcpuid = 3;
            break;
        }
        mcpuid <<= (XLEN-2);
        setBitsIn(mcpuid, static_cast<UIntType>(XLEN), 26, XLEN-28);
        setBitsIn(mcpuid, static_cast<UIntType>(EXT), 0, 26);
        csr.mcpuid = mcpuid;
        csr.mimpid = 0; // TODO
        csr.mhartid = index;
    }

    void dump(std::ostream& out) {
        out << std::setfill('0') << std::hex;
        out << "pc : " << std::setw(16) << pc << std::endl;
        for(UInt8 i = 1; i < 32; ++i)
            out << std::setw(2) << static_cast<UInt16>(i) << " : " << std::setw(16) << regX[i].U << std::endl;
        for(UInt8 i = 0; i < 32; ++i)
            out << std::setw(2) << static_cast<UInt16>(i) << " : " << regF[i].F64.template getFloat<double>() << std::endl;
        out << std::endl;
    }

    UIntType readRegXU(UInt8 index) {
        return (index) ? regX[index].U : 0;
    }

    IntType readRegXI(UInt8 index) {
        return (index) ? regX[index].I : 0;
    }

    UIntType readCSR(UInt16 index) {
        PrivilegeMode cpm = (PrivilegeMode)getBitsFrom(csr.status, 1, 2);

        switch(index) {
            case csr_fflags:
                return csr.fflags;
            case csr_frm:
                return csr.frm;
            case csr_fcsr:
                return csr.fcsr;
            case csr_cycle:
            case csr_cyclew:
                return getBitsFrom(csr.cycle, 0, XLEN);
            case csr_time:
                return getBitsFrom(csr.time, 0, XLEN);
            case csr_instret:
                return getBitsFrom(csr.instret, 0, XLEN);
            case csr_cycleh:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.cycle, 32, XLEN);
            case csr_timeh:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.time, 32, XLEN);
            case csr_instreth:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.instret, 32, XLEN);
            default:
                if(cpm == User)
                    throw Exception(Exception::Code::IllegalInstruction);
        }

        switch(index) {
            case csr_sstatus:
                return csr.status&getStatusCSRMask(Supervisor);
            case csr_stvec:
                return csr.stvec;
            case csr_sie:
                return csr.sie&0x22;
            case csr_stimecmp:
                return csr.stimecmp;
            case csr_stime:
                return getBitsFrom(csr.stime, 0, XLEN);
            case csr_stimeh:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.stime, 32, XLEN);
            case csr_sscratch:
                return csr.sscratch;
            case csr_sepc:
                return csr.sepc;
            case csr_scause:
                return csr.scause;
            case csr_sbadaddr:
                return csr.sbadaddr;
            case csr_sip:
                return csr.sip&0x2;
            case csr_sptbr:
                return csr.sptbr;
            case csr_sasid:
                return csr.sasid;
            case csr_timew:
                return getBitsFrom(csr.time, 0, XLEN);
            case csr_instretw:
                return getBitsFrom(csr.instret, 0, XLEN);
            case csr_cyclehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.cycle, 32, XLEN);
            case csr_timehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.time, 32, XLEN);
            case csr_instrethw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.instret, 32, XLEN);
            default:
                if(cpm == Supervisor)
                    throw Exception(Exception::Code::IllegalInstruction);
        }

        switch(index) {
            case csr_hstatus:
                return csr.status; // TODO wait for next riscv-privilege-spec
            case csr_htvec:
                return csr.htvec;
            case csr_htdeleg:
                return csr.htdeleg;
            /*case csr_hie: TODO wait for next riscv-privilege-spec
                return csr.hie&0x66;*/
            case csr_htimecmp:
                return csr.htimecmp;
            case csr_htime:
                return getBitsFrom(csr.htime, 0, XLEN);
            case csr_htimeh:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.htime, 32, XLEN);
            case csr_hscratch:
                return csr.hscratch;
            case csr_hepc:
                return csr.hepc;
            case csr_hcause:
                return csr.hcause;
            case csr_hbadaddr:
                return csr.hbadaddr;
            /*case csr_hip: TODO wait for next riscv-privilege-spec
                return csr.hip&0x6;*/
            case csr_stimew:
                return getBitsFrom(csr.stime, 0, XLEN);
            case csr_stimehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.stime, 32, XLEN);
            default:
                if(cpm == Hypervisor)
                    throw Exception(Exception::Code::IllegalInstruction);
        }

        switch(index) {
            case csr_mcpuid:
                return csr.mcpuid;
            case csr_mimpid:
                return csr.mimpid;
            case csr_mhartid:
                return csr.mhartid;
            case csr_mstatus:
                return csr.status;
            case csr_mtvec:
                return csr.mtvec;
            case csr_mtdeleg:
                return csr.mtdeleg;
            case csr_mie:
                return csr.mie;
            case csr_mtimecmp:
                return csr.mtimecmp;
            case csr_mtime:
                return getBitsFrom(csr.mtime, 0, XLEN);
            case csr_mtimeh:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.mtime, 32, XLEN);
            case csr_mscratch:
                return csr.mscratch;
            case csr_mepc:
                return csr.mepc;
            case csr_mcause:
                return csr.mcause;
            case csr_mbadaddr:
                return csr.mbadaddr;
            case csr_mip:
                return csr.mip;
            case csr_mbase:
                return csr.mbase;
            case csr_mbound:
                return csr.mbound;
            case csr_mibase:
                return csr.mibase;
            case csr_mibound:
                return csr.mibound;
            case csr_mdbase:
                return csr.mdbase;
            case csr_mdbound:
                return csr.mdbound;
            case csr_htimew:
                return getBitsFrom(csr.htime, 0, XLEN);
            case csr_htimehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                return getBitsFrom(csr.htime, 32, XLEN);
            case csr_mtohost:
                return csr.mtohost;
            case csr_mfromhost:
                return csr.mfromhost;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }
    }

    void writeRegXU(UInt8 index, UIntType value) {
        regX[index].U = value;
    }

    void writeRegXI(UInt8 index, IntType value) {
        regX[index].I = value;
    }

    void writeCSR(UInt16 index, UIntType value) {
        PrivilegeMode cpm = (PrivilegeMode)getBitsFrom(csr.status, 1, 2);

        switch(index) {
            case csr_fflags:
                csr.fflags = value;
            break;
            case csr_frm:
                csr.frm = value;
            break;
            case csr_fcsr:
                csr.fcsr = value;
            break;
            case csr_cycle:
            case csr_time:
            case csr_instret:
            case csr_cycleh:
            case csr_timeh:
            case csr_instreth:
                throw Exception(Exception::Code::IllegalInstruction);
            default:
                if(cpm == User)
                    throw Exception(Exception::Code::IllegalInstruction);
        }

        switch(index) {
            case csr_sstatus:
                setMaskedIn(csr.status, value, getStatusCSRMask(Supervisor));
            break;
            case csr_stvec:
                csr.stvec = value;
            break;
            case csr_sie:
                setMaskedIn(csr.sie, value, 0x22ULL);
            break;
            case csr_stimecmp:
                csr.stimecmp = value;
            break;
            case csr_stime:
            case csr_stimeh:
                throw Exception(Exception::Code::IllegalInstruction);
            case csr_sscratch:
                csr.sscratch = value;
            break;
            case csr_sepc:
                csr.sepc = value;
            break;
            case csr_scause:
                csr.scause = value;
            break;
            case csr_sbadaddr:
                csr.sbadaddr = value;
            break;
            case csr_sip:
                setMaskedIn(csr.sip, value, 0x2ULL);
            break;
            case csr_sptbr:
                csr.sptbr = value;
            break;
            case csr_sasid:
                csr.sasid = value;
            break;
            case csr_cyclew:
                setBitsIn(csr.cycle, value, 0, XLEN);
            break;
            case csr_timew:
                setBitsIn(csr.time, value, 0, XLEN);
            break;
            case csr_instretw:
                setBitsIn(csr.instret, value, 0, XLEN);
            break;
            case csr_cyclehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                setBitsIn(csr.cycle, value, 32, 32);
            break;
            case csr_timehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                setBitsIn(csr.time, value, 32, 32);
            break;
            case csr_instrethw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                setBitsIn(csr.instret, value, 32, 32);
            break;
            default:
                if(cpm == Supervisor)
                    throw Exception(Exception::Code::IllegalInstruction);
        }

        switch(index) {
            case csr_hstatus:
                csr.status = value; // TODO wait for next riscv-privilege-spec
            break;
            case csr_htvec:
                csr.htvec = value;
            break;
            case csr_htdeleg:
                csr.htdeleg = value;
            break;
            /*case csr_hie: TODO wait for next riscv-privilege-spec
                setMaskedIn(csr.hie, value, 0x66ULL);
            break;*/
            case csr_htimecmp:
                csr.htimecmp = value;
            break;
            case csr_htime:
            case csr_htimeh:
                throw Exception(Exception::Code::IllegalInstruction);
            case csr_hscratch:
                csr.hscratch = value;
            break;
            case csr_hepc:
                csr.hepc = value;
            break;
            case csr_hcause:
                csr.hcause = value;
            break;
            case csr_hbadaddr:
                csr.hbadaddr = value;
            break;
            /*case csr_hip: TODO wait for next riscv-privilege-spec
                setMaskedIn(csr.hip, value, 0x6ULL);
            break;*/
            case csr_stimew:
                setBitsIn(csr.stime, value, 0, XLEN);
            break;
            case csr_stimehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                setBitsIn(csr.stime, value, 32, 32);
            break;
            default:
                if(cpm == Hypervisor)
                    throw Exception(Exception::Code::IllegalInstruction);
        }

        switch(index) {
            case csr_mstatus:
                csr.status = value; // TODO
            break;
            case csr_mtvec:
                csr.mtvec = value;
            break;
            case csr_mtdeleg:
                csr.mtdeleg = value;
            break;
            case csr_mie:
                csr.mie = value;
            break;
            case csr_mtimecmp:
                csr.mtimecmp = value;
            break;
            case csr_mtime:
                setBitsIn(csr.mtime, value, 0, XLEN);
            break;
            case csr_mtimeh:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                setBitsIn(csr.mtime, value, 32, 32);
            break;
            case csr_mscratch:
                csr.mscratch = value;
            break;
            case csr_mepc:
                csr.mepc = value;
            break;
            case csr_mcause:
                csr.mcause = value;
            break;
            case csr_mbadaddr:
                csr.mbadaddr = value;
            break;
            case csr_mip:
                setMaskedIn(csr.mip, value, 0xEULL);
            break;
            case csr_mbase:
                csr.mbase = value;
            break;
            case csr_mbound:
                csr.mbound = value;
            break;
            case csr_mibase:
                csr.mibase = value;
            break;
            case csr_mibound:
                csr.mibound = value;
            break;
            case csr_mdbase:
                csr.mdbase = value;
            break;
            case csr_mdbound:
                csr.mdbound = value;
            break;
            case csr_htimew:
                setBitsIn(csr.htime, value, 0, XLEN);
            break;
            case csr_htimehw:
                if(XLEN > 32)
                    throw Exception(Exception::Code::IllegalInstruction);
                setBitsIn(csr.htime, value, 32, 32);
            break;
            case csr_mtohost:
                csr.mtohost = value;
            break;
            case csr_mfromhost:
                csr.mfromhost = value;
            break;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }
    }

    void seal(UIntType address, UInt8 length) {
        std::pair<AddressType, UInt8> entry(address, length);
        ram.seal(seals, { entry });
    }

    bool unseal(UIntType address, UInt8 length) {
        std::pair<AddressType, UInt8> entry(address, length);
        return ram.unseal(entry);
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
    AddressType translatePaged(PrivilegeMode cpm, MemoryAccessType mat, UIntType src) {
        UInt8 type, i = MaxLevel, offsetLen = 12;
        AddressType dst = csr.sptbr;
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
                if(cpm == User) {
                    if(type >= 8 || (type&2) == 0)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                }else{
                    if(type < 6 || (type&2) == 0)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                }
            break;
            case LoadData:
                if(cpm == User && type >= 8)
                    throw MemoryAccessException((Exception::Code)(mat+1), src);
            break;
            case StoreData:
                if((type&1) == 0)
                    throw MemoryAccessException((Exception::Code)(mat+1), src);
                if(cpm == User && type >= 8)
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
        bool mPrv = getBitsFrom(csr.status, 16, 1);
        PrivilegeMode cpm = (PrivilegeMode)getBitsFrom(csr.status, 1, 2);
        if(cpm != Machine || mat != FetchInstruction || mPrv)
            cpm = (PrivilegeMode)getBitsFrom(csr.status, 4, 2);

        AddressType dst;
        switch(getBitsFrom(csr.status, 17, 5)) {
            case 0: // Mbare
                dst = src;
            break;
            case 1: // Mbb
                if(src >= csr.mbound)
                    throw MemoryAccessException((Exception::Code)(mat+1), src);
                dst = src+csr.mbase;
            break;
            case 2: { // Mbbid
                UIntType halfVAS = 1ULL<<(XLEN-1);
                if(mat == FetchInstruction) {
                    if(src < halfVAS)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                    src -= halfVAS;
                    if(src >= csr.mibound)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                    dst = src+csr.mibase;
                }else{
                    if(src >= halfVAS || src >= csr.mdbound)
                        throw MemoryAccessException((Exception::Code)(mat+1), src);
                    dst = src+csr.mdbase;
                }
            } break;
            case 8: // Sv32
                dst = translatePaged<UInt32, 12, 10, 1>(cpm, mat, src);
            break;
            case 9: // Sv39
                dst = translatePaged<UInt64, 20, 9, 2>(cpm, mat, src);
            break;
            case 10: // Sv48
                dst = translatePaged<UInt64, 11, 9, 3>(cpm, mat, src);
            break;
            case 11: // Sv57
                dst = translatePaged<UInt64, 16, 9, 4>(cpm, mat, src);
            break;
            case 12: // Sv64
                dst = translatePaged<UInt64, 15, 13, 5>(cpm, mat, src);
            break;
        }
        return dst;
    }

    void executeOpcode03(const Instruction& instruction) {
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        address = translate(LoadData, address);
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

    void executeOpcode07(const Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        address = translate(LoadData, address);
        switch(instruction.funct[0]) {
            case 2: { // FLW rd,rs1,imm (F)
                memoryAccess<UInt32, false, false>(LoadData, address, &regF[instruction.reg[0]].F32.raw);
            } break;
            case 3: { // FLD rd,rs1,imm (D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                memoryAccess<UInt64, false, false>(LoadData, address, &regF[instruction.reg[0]].F64.raw);
            } break;
        }
    }

    void executeOpcode0F(const Instruction& instruction) {
        /* I-Type
        FENCE
        FENCE.I
        */
        // TODO : Pipeline, Cache
    }

    void executeOpcode13(const Instruction& instruction) {
        switch(instruction.funct[0]) {
            case 0: //ADDI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])+instruction.imm);
            break;
            case 1: { //SLLI rd,rs1,shamt
                if(XLEN < 64 && getBitsFrom(instruction.imm, 5, 1))
                    throw Exception(Exception::Code::IllegalInstruction);
                UIntType shift = instruction.imm&TrailingBitMask<UIntType>((XLEN < 64)?5:6);
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])<<shift);
            } break;
            case 2: //SLTI rd,rs1,imm
                writeRegXU(instruction.reg[0], (readRegXI(instruction.reg[1]) < instruction.imm) ? 1 : 0);
            break;
            case 3: //SLTIU rd,rs1,imm
                writeRegXU(instruction.reg[0], (readRegXU(instruction.reg[1]) < static_cast<UInt32>(instruction.imm)) ? 1 : 0);
            break;
            case 4: //XORI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])^instruction.imm);
            break;
            case 5: {
                if(XLEN < 64 && getBitsFrom(instruction.imm, 5, 1))
                    throw Exception(Exception::Code::IllegalInstruction);
                bool arithmetic = getBitsFrom(instruction.imm, 10, 1);
                UIntType shift = instruction.imm&TrailingBitMask<UIntType>((XLEN < 64)?5:6);
                if(arithmetic) //SRAI rd,rs1,shamt
                    writeRegXI(instruction.reg[0], readRegXI(instruction.reg[1])>>shift);
                else //SRLI rd,rs1,shamt
                    writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])>>shift);
            } break;
            case 6: //ORI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])|instruction.imm);
            break;
            case 7: //ANDI rd,rs1,imm
                writeRegXU(instruction.reg[0], readRegXU(instruction.reg[1])&instruction.imm);
            break;
        }
    }

    void executeOpcode17(const Instruction& instruction) {
        // AUIPC rd,imm
        writeRegXU(instruction.reg[0], pc+instruction.imm);
    }

    void executeOpcode1B(const Instruction& instruction) {
        if(XLEN < 64)
            throw Exception(Exception::Code::IllegalInstruction);
        switch(instruction.funct[0]) {
            case 0: //ADDIW rd,rs1,imm (64)
                writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1])+instruction.imm));
            break;
            case 1: { //SLLIW rd,rs1,shamt (64)
                if(instruction.imm&(1ULL<<5))
                    throw Exception(Exception::Code::IllegalInstruction);
                UIntType shift = instruction.imm&TrailingBitMask<UIntType>(5);
                writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1])<<shift));
            } break;
            case 5: {
                if(instruction.imm&(1ULL<<5))
                    throw Exception(Exception::Code::IllegalInstruction);
                bool arithmetic = instruction.imm&(1ULL<<10);
                UIntType shift = instruction.imm&TrailingBitMask<UIntType>(5);
                if(arithmetic) //SRAIW rd,rs1,shamt (64)
                    writeRegXI(instruction.reg[0], static_cast<Int32>(readRegXI(instruction.reg[1])>>shift));
                else //SRLIW rd,rs1,shamt (64)
                    writeRegXU(instruction.reg[0], static_cast<UInt32>(readRegXU(instruction.reg[1])>>shift));
            } break;
        }
    }

    void executeOpcode23(const Instruction& instruction) {
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        address = translate(StoreData, address);
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

    void executeOpcode27(const Instruction& instruction) {
        if(!(EXT&F_Float))
            throw Exception(Exception::Code::IllegalInstruction);
        UIntType address = readRegXU(instruction.reg[1])+instruction.imm;
        address = translate(StoreData, address);
        switch(instruction.funct[0]) {
            case 2: { // FSW rd,rs1,imm (F)
                memoryAccess<UInt32, true, false>(StoreData, address, &regF[instruction.reg[0]].F32.raw);
            } break;
            case 3: { // FSD rd,rs1,imm (D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                memoryAccess<UInt64, true, false>(StoreData, address, &regF[instruction.reg[0]].F64.raw);
            } break;
        }
    }

    void executeOpcode2F(const Instruction& instruction) {
        if(!(EXT&A_AtomicOperations))
            throw Exception(Exception::Code::IllegalInstruction);
        UInt8 funct = instruction.funct[0]&~TrailingBitMask<UInt8>(2);
        UIntType address = readRegXU(instruction.reg[1]);
        address = translate(StoreData, address);

        if(instruction.funct[1] == 2) {
            Int32 data;
            if(funct != 12) {
                seal(address, sizeof(data));
                memoryAccess<decltype(data), false, true>(LoadData, address, &data);
                writeRegXU(instruction.reg[0], data);
            }

            switch(funct) {
                case 0: // AMOADD.W rd,rs1,rs2 (A)
                    data += readRegXI(instruction.reg[2]);
                break;
                case 4: // AMOSWAP.W rd,rs1,rs2 (A)
                    data = readRegXI(instruction.reg[2]);
                break;
                case 8: // LR.W rd,rs1 (A)
                return;
                case 12: // SC.W rd,rs1,rs2 (A)
                    data = readRegXI(instruction.reg[2]);
                break;
                case 16: // AMOXOR.W rd,rs1,rs2 (A)
                    data ^= readRegXI(instruction.reg[2]);
                break;
                case 32: // AMOOR.W rd,rs1,rs2 (A)
                    data |= readRegXI(instruction.reg[2]);
                break;
                case 48: // AMOAND.W rd,rs1,rs2 (A)
                    data &= readRegXI(instruction.reg[2]);
                break;
                case 64: // AMOMIN.W rd,rs1,rs2 (A)
                    data = std::min(data, static_cast<Int32>(readRegXI(instruction.reg[2])));
                break;
                case 80: // AMOMAX.W rd,rs1,rs2 (A)
                    data = std::max(data, static_cast<Int32>(readRegXI(instruction.reg[2])));
                break;
                case 96: // AMOMINU.W rd,rs1,rs2 (A)
                    data = std::min(static_cast<UInt32>(data), static_cast<UInt32>(readRegXI(instruction.reg[2])));
                break;
                case 112: // AMOMAXU.W rd,rs1,rs2 (A)
                    data = std::max(static_cast<UInt32>(data), static_cast<UInt32>(readRegXI(instruction.reg[2])));
                break;
            }

            std::lock_guard<std::recursive_mutex> lock(ram.sealsMutex);
            if(unseal(address, sizeof(data))) {
                memoryAccess<decltype(data), true, true>(StoreData, address, &data);
                writeRegXU(instruction.reg[0], 0);
            }else
                writeRegXU(instruction.reg[0], 1);
        }else if(instruction.funct[1] == 3) {
            if(XLEN < 64)
                throw Exception(Exception::Code::IllegalInstruction);

            Int64 data;
            if(funct != 12) {
                seal(address, sizeof(data));
                memoryAccess<decltype(data), false, true>(LoadData, address, &data);
                writeRegXU(instruction.reg[0], data);
            }

            switch(funct) {
                case 0: // AMOADD.D rd,rs1,rs2 (A, 64)
                    data += readRegXI(instruction.reg[2]);
                break;
                case 4: // AMOSWAP.D rd,rs1,rs2 (A, 64)
                    data = readRegXI(instruction.reg[2]);
                break;
                case 8: // LR.D rd,rs1 (A, 64)
                return;
                case 12: // SC.D rd,rs1,rs2 (A, 64)
                    data = readRegXI(instruction.reg[2]);
                break;
                case 16: // AMOXOR.D rd,rs1,rs2 (A, 64)
                    data ^= readRegXI(instruction.reg[2]);
                break;
                case 32: // AMOOR.D rd,rs1,rs2 (A, 64)
                    data |= readRegXI(instruction.reg[2]);
                break;
                case 48: // AMOAND.D rd,rs1,rs2 (A, 64)
                    data &= readRegXI(instruction.reg[2]);
                break;
                case 64: // AMOMIN.D rd,rs1,rs2 (A, 64)
                    data = std::min(data, static_cast<Int64>(readRegXI(instruction.reg[2])));
                break;
                case 80: // AMOMAX.D rd,rs1,rs2 (A, 64)
                    data = std::max(data, static_cast<Int64>(readRegXI(instruction.reg[2])));
                break;
                case 96: // AMOMINU.D rd,rs1,rs2 (A, 64)
                    data = std::min(static_cast<UInt64>(data), static_cast<UInt64>(readRegXI(instruction.reg[2])));
                break;
                case 112: // AMOMAXU.D rd,rs1,rs2 (A, 64)
                    data = std::max(static_cast<UInt64>(data), static_cast<UInt64>(readRegXI(instruction.reg[2])));
                break;
            }

            std::lock_guard<std::recursive_mutex> lock(ram.sealsMutex);
            if(unseal(address, sizeof(data))) {
                memoryAccess<decltype(data), true, true>(StoreData, address, &data);
                writeRegXU(instruction.reg[0], 0);
            }else
                writeRegXU(instruction.reg[0], 1);
        }else
            throw Exception(Exception::Code::IllegalInstruction);
    }

    void executeOpcode33(const Instruction& instruction) {
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

    void executeOpcode37(const Instruction& instruction) {
        // LUI rd,imm
        writeRegXU(instruction.reg[0], instruction.imm);
    }

    void executeOpcode3B(const Instruction& instruction) {
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

    #define FloatInstructionAux \
        if(!(EXT&F_Float)) \
            throw Exception(Exception::Code::IllegalInstruction); \
        FloatRoundingMode round = static_cast<FloatRoundingMode>(instruction.funct[1]); \
        if(round == RoundDynamic) \
            round = static_cast<FloatRoundingMode>((csr.fflags>>5)&TrailingBitMask<UInt8>(3));

    void executeOpcode43(const Instruction& instruction) {
        FloatInstructionAux

        switch(instruction.funct[0]) {
            case 0: // FMADD.S rd,rs1,rs2,rs3 (F)
                regF[instruction.reg[0]].F32.product(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                regF[instruction.reg[0]].F32.template sum<false>(csr.fflags, round, regF[instruction.reg[0]].F32, regF[instruction.reg[3]].F32);
            break;
            case 1: // FMADD.D rd,rs1,rs2,rs3 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.product(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                regF[instruction.reg[0]].F64.template sum<false>(csr.fflags, round, regF[instruction.reg[0]].F64, regF[instruction.reg[3]].F64);
            break;
        }
    }

    void executeOpcode47(const Instruction& instruction) {
        FloatInstructionAux

        switch(instruction.funct[0]) {
            case 0: // FMSUB.S rd,rs1,rs2,rs3 (F)
                regF[instruction.reg[0]].F32.product(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                regF[instruction.reg[0]].F32.template sum<true>(csr.fflags, round, regF[instruction.reg[0]].F32, regF[instruction.reg[3]].F32);
            break;
            case 1: // FMSUB.D rd,rs1,rs2,rs3 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.product(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                regF[instruction.reg[0]].F64.template sum<true>(csr.fflags, round, regF[instruction.reg[0]].F64, regF[instruction.reg[3]].F64);
            break;
        }
    }

    void executeOpcode4B(const Instruction& instruction) {
        FloatInstructionAux

        switch(instruction.funct[0]) {
            case 0: // FNMSUB.S rd,rs1,rs2,rs3 (F)
                regF[instruction.reg[0]].F32.product(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                regF[instruction.reg[0]].F32.template sum<true>(csr.fflags, round, regF[instruction.reg[0]].F32, regF[instruction.reg[3]].F32);
                regF[instruction.reg[0]].F32.negate();
            break;
            case 1: // FNMSUB.D rd,rs1,rs2,rs3 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.product(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                regF[instruction.reg[0]].F64.template sum<true>(csr.fflags, round, regF[instruction.reg[0]].F64, regF[instruction.reg[3]].F64);
                regF[instruction.reg[0]].F64.negate();
            break;
        }
    }

    void executeOpcode4F(const Instruction& instruction) {
        FloatInstructionAux

        switch(instruction.funct[0]) {
            case 0: // FNMADD.S rd,rs1,rs2,rs3 (F)
                regF[instruction.reg[0]].F32.product(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                regF[instruction.reg[0]].F32.template sum<false>(csr.fflags, round, regF[instruction.reg[0]].F32, regF[instruction.reg[3]].F32);
                regF[instruction.reg[0]].F32.negate();
            break;
            case 1: // FNMADD.D rd,rs1,rs2,rs3 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.product(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                regF[instruction.reg[0]].F64.template sum<false>(csr.fflags, round, regF[instruction.reg[0]].F64, regF[instruction.reg[3]].F64);
                regF[instruction.reg[0]].F64.negate();
            break;
        }
    }

    void executeOpcode53(const Instruction& instruction) {
        FloatInstructionAux

        switch(instruction.funct[0]) {
            case 0x00: // FADD.S rd,rs1,rs2 (F)
                regF[instruction.reg[0]].F32.template sum<false>(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
            break;
            case 0x01: // FADD.D rd,rs1,rs2 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.template sum<false>(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
            break;
            case 0x04: // FSUB.S rd,rs1,rs2 (F)
                regF[instruction.reg[0]].F32.template sum<true>(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
            break;
            case 0x05: // FSUB.D rd,rs1,rs2 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.template sum<true>(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
            break;
            case 0x08: // FMUL.S rd,rs1,rs2 (F)
                regF[instruction.reg[0]].F32.product(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
            break;
            case 0x09: // FMUL.D rd,rs1,rs2 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.product(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
            break;
            case 0x0C: // FDIV.S rd,rs1,rs2 (F)
                regF[instruction.reg[0]].F32.quotient(csr.fflags, round, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
            break;
            case 0x0D: // FDIV.D rd,rs1,rs2 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.quotient(csr.fflags, round, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
            break;
            case 0x20: // FCVT.S.D rd,rs1 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F32.setFloat(regF[instruction.reg[0]].F64);
            break;
            case 0x21: // FCVT.D.S rd,rs1 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.setFloat(regF[instruction.reg[0]].F32);
            break;
            case 0x2C: // FSQRT.S rd,rs1 (F)
                regF[instruction.reg[0]].F32.sqrt(csr.fflags, round, regF[instruction.reg[1]].F32);
            break;
            case 0x2D: // FSQRT.D rd,rs1 (F, D)
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.sqrt(csr.fflags, round, regF[instruction.reg[1]].F64);
            break;
            case 0x10:
                regF[instruction.reg[0]].F32 = regF[instruction.reg[1]].F32;
                switch(instruction.funct[1]) {
                    case 0: // FSGNJ.S rd,rs1,rs2 (F)
                        regF[instruction.reg[1]].F32.setSign(regF[instruction.reg[2]].F32.getSign());
                    break;
                    case 1: // FSGNJN.S rd,rs1,rs2 (F)
                        regF[instruction.reg[1]].F32.setSign(!regF[instruction.reg[2]].F32.getSign());
                    break;
                    case 2: // FSGNJX.S rd,rs1,rs2 (F)
                        regF[instruction.reg[1]].F32.setSign(regF[instruction.reg[1]].F32.getSign()^
                        regF[instruction.reg[2]].F32.getSign());
                    break;
                }
            break;
            case 0x11:
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64 = regF[instruction.reg[1]].F64;
                switch(instruction.funct[1]) {
                    case 0: // FSGNJ.D rd,rs1,rs2 (F, D)
                        regF[instruction.reg[1]].F64.setSign(regF[instruction.reg[2]].F64.getSign());
                    break;
                    case 1: // FSGNJN.D rd,rs1,rs2 (F, D)
                        regF[instruction.reg[1]].F64.setSign(!regF[instruction.reg[2]].F64.getSign());
                    break;
                    case 2: // FSGNJX.D rd,rs1,rs2 (F, D)
                        regF[instruction.reg[1]].F64.setSign(regF[instruction.reg[1]].F64.getSign()^
                        regF[instruction.reg[2]].F64.getSign());
                    break;
                }
            break;
            case 0x14:
                switch(instruction.funct[1]) {
                    case 0: // FMIN.S rd,rs1,rs2 (F)
                        regF[instruction.reg[0]].F32.template extremum<FloatComparison::Less>(csr.fflags,
                            regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                    break;
                    case 1: // FMAX.S rd,rs1,rs2 (F)
                        regF[instruction.reg[0]].F32.template extremum<FloatComparison::Greater>(csr.fflags,
                            regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                    break;
                }
            break;
            case 0x15:
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                switch(instruction.funct[1]) {
                    case 0: // FMIN.D rd,rs1,rs2 (F, D)
                        regF[instruction.reg[0]].F64.template extremum<FloatComparison::Less>(csr.fflags,
                            regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                    break;
                    case 1: // FMAX.D rd,rs1,rs2 (F, D)
                        regF[instruction.reg[0]].F64.template extremum<FloatComparison::Greater>(csr.fflags,
                            regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                    break;
                }
            break;
            case 0x50:
                switch(instruction.funct[1]) {
                    case 0: { // FLE.S rd,rs1,rs2 (F)
                        auto cmp = Float32::compare<true>(csr.fflags, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                        writeRegXU(instruction.reg[0], (cmp == FloatComparison::Less) ? 1 : 0);
                    } break;
                    case 1: { // FLT.S rd,rs1,rs2 (F)
                        auto cmp = Float32::compare<true>(csr.fflags, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                        writeRegXU(instruction.reg[0], (cmp == FloatComparison::Less || cmp == FloatComparison::Equal) ? 1 : 0);
                    } break;
                    case 2: { // FEQ.S rd,rs1,rs2 (F)
                        auto cmp = Float32::compare<false>(csr.fflags, regF[instruction.reg[1]].F32, regF[instruction.reg[2]].F32);
                        writeRegXU(instruction.reg[0], (cmp == FloatComparison::Equal) ? 1 : 0);
                    } break;
                }
            break;
            case 0x51:
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                switch(instruction.funct[1]) {
                    case 0: { // FLE.D rd,rs1,rs2 (F, D)
                        auto cmp = Float64::compare<true>(csr.fflags, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                        writeRegXU(instruction.reg[0], (cmp == FloatComparison::Less) ? 1 : 0);
                    } break;
                    case 1: { // FLT.D rd,rs1,rs2 (F, D)
                        auto cmp = Float64::compare<true>(csr.fflags, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                        writeRegXU(instruction.reg[0], (cmp == FloatComparison::Less || cmp == FloatComparison::Equal) ? 1 : 0);
                    } break;
                    case 2: { // FEQ.D rd,rs1,rs2 (F, D)
                        auto cmp = Float64::compare<false>(csr.fflags, regF[instruction.reg[1]].F64, regF[instruction.reg[2]].F64);
                        writeRegXU(instruction.reg[0], (cmp == FloatComparison::Equal) ? 1 : 0);
                    } break;
                }
            break;
            case 0x60:
                if(instruction.reg[2] >= 2 && XLEN < 64)
                    throw Exception(Exception::Code::IllegalInstruction);
                switch(instruction.reg[2]) {
                    case 0: // FCVT.W.S rd,rs1 (F)
                        writeRegXI(instruction.reg[0], regF[instruction.reg[1]].F32.template getInt<Int32>(csr.fflags));
                    break;
                    case 1: // FCVT.WU.S rd,rs1 (F)
                        writeRegXU(instruction.reg[0], regF[instruction.reg[1]].F32.template getInt<UInt32>(csr.fflags));
                    break;
                    case 2: // FCVT.L.S rd,rs1 (F, 64)
                        writeRegXI(instruction.reg[0], regF[instruction.reg[1]].F32.template getInt<Int64>(csr.fflags));
                    break;
                    case 3: // FCVT.LU.S rd,rs1 (F, 64)
                        writeRegXU(instruction.reg[0], regF[instruction.reg[1]].F32.template getInt<UInt64>(csr.fflags));
                    break;
                }
            break;
            case 0x61:
                if(!(EXT&D_DoubleFloat) || (instruction.reg[2] >= 2 && XLEN < 64))
                    throw Exception(Exception::Code::IllegalInstruction);
                switch(instruction.reg[2]) {
                    case 0: // FCVT.W.D rd,rs1 (F, D)
                        writeRegXI(instruction.reg[0], regF[instruction.reg[1]].F64.template getInt<Int32>(csr.fflags));
                    break;
                    case 1: // FCVT.WU.D rd,rs1 (F, D)
                        writeRegXU(instruction.reg[0], regF[instruction.reg[1]].F64.template getInt<UInt32>(csr.fflags));
                    break;
                    case 2: // FCVT.L.D rd,rs1 (F, D, 64)
                        writeRegXI(instruction.reg[0], regF[instruction.reg[1]].F64.template getInt<Int64>(csr.fflags));
                    break;
                    case 3: // FCVT.LU.D rd,rs1 (F, D, 64)
                        writeRegXU(instruction.reg[0], regF[instruction.reg[1]].F64.template getInt<UInt64>(csr.fflags));
                    break;
                }
            break;
            case 0x68:
                if(instruction.reg[2] >= 2 && XLEN < 64)
                    throw Exception(Exception::Code::IllegalInstruction);
                switch(instruction.reg[2]) {
                    case 0: // FCVT.S.W rd,rs1 (F)
                        regF[instruction.reg[0]].F32.template setInt<Int32>(csr.fflags, round, readRegXI(instruction.reg[1]));
                    break;
                    case 1: // FCVT.S.WU rd,rs1 (F)
                        regF[instruction.reg[0]].F32.template setUInt<UInt32>(csr.fflags, round, readRegXU(instruction.reg[1]));
                    break;
                    case 2: // FCVT.S.L rd,rs1 (F, 64)
                        regF[instruction.reg[0]].F32.template setInt<Int64>(csr.fflags, round, readRegXI(instruction.reg[1]));
                    break;
                    case 3: // FCVT.S.LU rd,rs1 (F, 64)
                        regF[instruction.reg[0]].F32.template setUInt<UInt64>(csr.fflags, round, readRegXU(instruction.reg[1]));
                    break;
                }
            break;
            case 0x69:
                if(!(EXT&D_DoubleFloat) || (instruction.reg[2] >= 2 && XLEN < 64))
                    throw Exception(Exception::Code::IllegalInstruction);
                switch(instruction.reg[2]) {
                    case 0: // FCVT.D.W rd,rs1 (F, D)
                        regF[instruction.reg[0]].F64.template setInt<Int32>(csr.fflags, round, readRegXI(instruction.reg[1]));
                    break;
                    case 1: // FCVT.D.WU rd,rs1 (F, D)
                        regF[instruction.reg[0]].F64.template setUInt<UInt32>(csr.fflags, round, readRegXU(instruction.reg[1]));
                    break;
                    case 2: // FCVT.D.L rd,rs1 (F, D, 64)
                        regF[instruction.reg[0]].F64.template setInt<Int64>(csr.fflags, round, readRegXI(instruction.reg[1]));
                    break;
                    case 3: // FCVT.D.LU rd,rs1 (F, D, 64)
                        regF[instruction.reg[0]].F64.template setUInt<UInt64>(csr.fflags, round, readRegXU(instruction.reg[1]));
                    break;
                }
            break;
            case 0x70:
                switch(instruction.funct[1]) {
                    case 0: // FMV.X.S rd,rs1 (F)
                        writeRegXI(instruction.reg[0], static_cast<Int32>(regF[instruction.reg[1]].F32.raw));
                    break;
                    case 1: // FCLASS.S rd,rs1 (F)
                        writeRegXU(instruction.reg[0], regF[instruction.reg[1]].F32.getClass());
                    break;
                }
            break;
            case 0x71:
                if(!(EXT&D_DoubleFloat))
                    throw Exception(Exception::Code::IllegalInstruction);
                switch(instruction.funct[1]) {
                    case 0: // FMV.X.D rd,rs1 (F, D)
                        writeRegXI(instruction.reg[0], static_cast<Int64>(regF[instruction.reg[1]].F64.raw));
                    break;
                    case 1: // FCLASS.D rd,rs1 (F, D)
                        writeRegXU(instruction.reg[0], regF[instruction.reg[1]].F64.getClass());
                    break;
                }
            break;
            case 0x78: // FMV.S.X rd,rs1 (F)
                regF[instruction.reg[0]].F32.raw = readRegXU(instruction.reg[1]);
            break;
            case 0x79: // FMV.D.X rd,rs1 (F, D, 64)
                if(!(EXT&D_DoubleFloat) || XLEN < 64)
                    throw Exception(Exception::Code::IllegalInstruction);
                regF[instruction.reg[0]].F64.raw = readRegXU(instruction.reg[1]);
            break;
            default:
               throw Exception(Exception::Code::IllegalInstruction);
        }
    }

    void executeOpcode63(const Instruction& instruction, UIntType pcNextValue) {
        switch(instruction.funct[0]) {
            case 0: // BEQ rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) == readRegXU(instruction.reg[2])) break;
                pc = pcNextValue;
            return;
            case 1:// BNE rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) != readRegXU(instruction.reg[2])) break;
                pc = pcNextValue;
            return;
            case 4:// BLT rs1,rs2,imm
                if(readRegXI(instruction.reg[1]) < readRegXI(instruction.reg[2])) break;
                pc = pcNextValue;
            return;
            case 5:// BGE rs1,rs2,imm
                if(readRegXI(instruction.reg[1]) > readRegXI(instruction.reg[2])) break;
                pc = pcNextValue;
            return;
            case 6:// BLTU rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) < readRegXU(instruction.reg[2])) break;
                pc = pcNextValue;
            return;
            case 7:// BGEU rs1,rs2,imm
                if(readRegXU(instruction.reg[1]) > readRegXU(instruction.reg[2])) break;
                pc = pcNextValue;
            return;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }
        pc += instruction.imm;
    }

    void executeOpcode67(const Instruction& instruction, UIntType pcNextValue) {
        // JALR rd,rs1,imm
        writeRegXU(instruction.reg[0], pcNextValue);
        pc = (readRegXU(instruction.reg[1])+instruction.imm)&~TrailingBitMask<UIntType>(1);
    }

    void executeOpcode6F(const Instruction& instruction, UIntType pcNextValue) {
        // JAL rd,imm
        writeRegXU(instruction.reg[0], pcNextValue);
        pc += instruction.imm;
    }

    void executeOpcode73(const Instruction& instruction, UIntType pcNextValue) {
        switch(instruction.funct[0]) {
            case 0: {
                PrivilegeMode cpm = (PrivilegeMode)getBitsFrom(csr.status, 1, 2);
                switch(static_cast<UInt32>(instruction.imm)) {
                    case 0x0000: { // ECALL
                        pc = pcNextValue; // TODO : [Find out how pc behaves]
                        ++csr.instret;
                        throw Exception((Exception::Code)(Exception::Code::EnvironmentCallFromU+cpm));
                    }
                    case 0x0001: // EBREAK
                        ++csr.instret;
                        throw Exception(Exception::Code::Breakpoint);
                    case 0x0100: { // ERET
                        switch(cpm) {
                            case User:
                                throw Exception(Exception::Code::IllegalInstruction);
                            case Supervisor:
                                pc = csr.sepc;
                            break;
                            case Hypervisor:
                                pc = csr.hepc;
                            break;
                            case Machine:
                                pc = csr.mepc;
                            break;
                        }
                        setBitsIn(csr.status, static_cast<UIntType>((csr.status&TrailingBitMask<UIntType>(12))>>3), 0, 12);
                        setBitsIn(csr.status, static_cast<UIntType>((EXT&U_UserMode)?1:7), (getLevels()-1)*3, 3);
                        ++csr.instret;
                    } return;
                    case 0x0101: // SFENCE.VM rs1
                        // TODO : Flush caches
                    break;
                    case 0x0102: // WFI
                        // TODO
                    break;
                    case 0x0205: // HRTS
                        if(cpm != Hypervisor)
                            throw Exception(Exception::Code::IllegalInstruction);
                        setBitsIn(csr.status, static_cast<UIntType>(Supervisor), 1, 2);
                        csr.status = csr.status;
                        csr.sepc = csr.hepc;
                        csr.scause = csr.hcause;
                        csr.sbadaddr = csr.hbadaddr;
                        pc = csr.stvec;
                        ++csr.instret;
                    return;
                    case 0x0305: // MRTS
                        if(cpm != Machine)
                            throw Exception(Exception::Code::IllegalInstruction);
                        setBitsIn(csr.status, static_cast<UIntType>(Supervisor), 1, 2);
                        csr.status = csr.status;
                        csr.sepc = csr.mepc;
                        csr.scause = csr.mcause;
                        csr.sbadaddr = csr.mbadaddr;
                        pc = csr.stvec;
                        ++csr.instret;
                    return;
                    case 0x0306: // MRTH
                        if(cpm != Machine)
                            throw Exception(Exception::Code::IllegalInstruction);
                        setBitsIn(csr.status, static_cast<UIntType>(Hypervisor), 1, 2);
                        csr.hepc = csr.mepc;
                        csr.hcause = csr.mcause;
                        csr.hbadaddr = csr.mbadaddr;
                        pc = csr.htvec;
                        ++csr.instret;
                    return;
                    default:
                        throw Exception(Exception::Code::IllegalInstruction);
                }
            } break;
            case 1: { // CSRRW rd,csr,rs1
                UIntType value = readRegXU(instruction.reg[1]),
                         key = instruction.imm&TrailingBitMask<UIntType>(12);
                writeRegXU(instruction.reg[0], readCSR(key));
                writeCSR(key, value);
            } break;
            case 2: { // CSRRS rd,csr,rs1
                UIntType value = readRegXU(instruction.reg[1]),
                         key = instruction.imm&TrailingBitMask<UIntType>(12),
                         csr = readCSR(key);
                writeRegXU(instruction.reg[0], csr);
                if(instruction.reg[1])
                    writeCSR(key, csr|value);
            } break;
            case 3: { // CSRRC rd,csr,rs1
                UIntType value = readRegXU(instruction.reg[1]),
                         key = instruction.imm&TrailingBitMask<UIntType>(12),
                         csr = readCSR(key);
                writeRegXU(instruction.reg[0], csr);
                if(instruction.reg[1])
                    writeCSR(key, csr&~value);
            } break;
            case 5: { // CSRRWI rd,csr,imm
                UIntType value = instruction.reg[1],
                         key = instruction.imm&TrailingBitMask<UIntType>(12);
                writeRegXU(instruction.reg[0], readCSR(key));
                writeCSR(key, value);
            } break;
            case 6: { // CSRRSI rd,csr,imm
                UIntType value = instruction.reg[1],
                         key = instruction.imm&TrailingBitMask<UIntType>(12),
                         csr = readCSR(key);
                writeRegXU(instruction.reg[0], csr);
                if(value)
                    writeCSR(key, csr|value);
            } break;
            case 7: { // CSRRCI rd,csr,imm
                UIntType value = instruction.reg[1],
                         key = instruction.imm&TrailingBitMask<UIntType>(12),
                         csr = readCSR(key);
                writeRegXU(instruction.reg[0], csr);
                if(value)
                    writeCSR(key, csr&~value);
            } break;
            default:
                throw Exception(Exception::Code::IllegalInstruction);
        }
        pc = pcNextValue;
    }

    bool fetchAndExecute() {
        ++csr.cycle;
        Exception::Code cause;
        UIntType badaddr = 0, pcNextValue = pc;
        PrivilegeMode cpm = (PrivilegeMode)getBitsFrom(csr.status, 1, 2);

        try {
            UIntType mappedPC = translate(FetchInstruction, pc);
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
                    executeOpcode63(instruction, pcNextValue);
                return true;
        		case 0x67:
                    executeOpcode67(instruction, pcNextValue);
                return true;
                case 0x6F:
                    executeOpcode6F(instruction, pcNextValue);
                return true;
        		case 0x73:
                    executeOpcode73(instruction, pcNextValue);
                return true;
            }
            pc = pcNextValue;
            ++csr.instret;
            return true;
        } catch(MemoryAccessException e) {
            badaddr = e.address;
            cause = e.cause;
        } catch(Exception e) {
            cause = e.cause;
        }

        pcNextValue = cpm*0x40;
        //TODO : Non Maskable Interrupts

        if(getBitsFrom(static_cast<UIntType>(cause), XLEN-1, 1)) { // Interrupt
            if(getBitsFrom(csr.status, 0, 1)) {
                // TODO: Check Interrupt flag in csr.status
            }
            // TODO: mip, mie

            if(getBitsFrom(csr.mtdeleg, cause*4+cpm+16, 1)) {
                if(EXT&H_HypervisorMode) {
                    if(cpm <= Hypervisor)
                        cpm = (getBitsFrom(csr.htdeleg, cause*4+cpm+16, 1)) ? Supervisor : Hypervisor;
                    else
                        cpm = Machine;
                }else if(EXT&S_SupervisorMode)
                    cpm = (cpm <= Supervisor) ? Supervisor : Machine;
            }
        }else{ // Trap
            if(getBitsFrom(csr.mtdeleg, cause, 1)) {
                if(EXT&H_HypervisorMode) {
                    if(cpm <= Hypervisor)
                        cpm = (getBitsFrom(csr.htdeleg, cause, 1)) ? Supervisor : Hypervisor;
                    else
                        cpm = Machine;
                }else if(EXT&S_SupervisorMode)
                    cpm = (cpm <= Supervisor) ? Supervisor : Machine;
            }
        }

        pc &= ~TrailingBitMask<UIntType>((EXT&C_CompressedInstructions)?1:2);
        switch(cpm) {
            case Supervisor:
                csr.sbadaddr = badaddr;
                csr.scause = cause;
                csr.sepc = pc;
                pcNextValue += csr.stvec;
            break;
            case Hypervisor:
                csr.hbadaddr = badaddr;
                csr.hcause = cause;
                csr.hepc = pc;
                pcNextValue += csr.htvec;
            break;
            case User:
                cpm = Machine;
            case Machine:
                csr.mbadaddr = badaddr;
                csr.mcause = cause;
                csr.mepc = pc;
                pcNextValue += csr.mtvec;
            break;
        }
        pc = pcNextValue;
        setBitsIn(csr.status, getBitsFrom(csr.status, 3, 12), 0, 12);
        setBitsIn(csr.status, static_cast<UIntType>(cpm<<1), 0, 3);
        setBitsIn(csr.status, static_cast<UIntType>(0), 16, 1);

        // TODO : Debugging
        printf("TRAPED!\n");

        return false;
    }
};

#endif
