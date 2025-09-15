#include "cpu.h"


struct Registers {
    //Program Counter
    uint16_t PC;

    //Stack Pointer
    uint16_t SP;

    // AF
    union {
        struct {

            //Flags
            union {
                uint8_t F;
                struct {
                    uint8_t f_unused : 4;
                    uint8_t C_flag : 1; //carry flag
                    uint8_t H_flag : 1; //half carry flag
                    uint8_t N_flag : 1; //Subtraction flag
                    uint8_t Z_flag : 1; //zero flag
                };
            };
            
            uint8_t A;// Accumulator
        };
        uint16_t AF;
    };

    // BC
    union {
        struct {
            uint8_t C;
            uint8_t B;
        };
        uint16_t BC;
    };

    // DE
    union {
        struct {
            uint8_t E;
            uint8_t D;
        };
        uint16_t DE;
    };

    // HL
    union {
        struct {
            uint8_t L; //high
            uint8_t H; //low
        };
        uint16_t HL;
    };

    //Instruction register
    uint8_t IR;

    //Interrupt enable
    uint8_t IE;


} reg;




void cpu_init() {
    reg.PC = 0;
    reg.SP = 0;
    reg.AF = 0;
    reg.BC = 0;
    reg.DE = 0;
    reg.HL = 0;
    reg.IR = 0;
    reg.IE = 0;



}

uint8_t opcode = 0;

//Fetches the next opcode and increments pc
uint8_t fetchOp() {
    opcode = memory_get(reg.PC++);
    return opcode;
    
}



//Returns machine cycles instruction takes
uint8_t cpu_update() {
    uint8_t m_cycles = 0;
    uint8_t op = fetchOp();
    

    return m_cycles;
}










void Default() {
    print("Default opcode, should not have been called");
    print(opcode);
}


void adc_a_imm8() {

}


void adc_a_r8() {

}


void add_a_imm8() {

}


void add_a_r8() {

}


void add_hl_r16() {

}


void add_sp_imm8() {

}


void and_a_imm8() {

}


void and_a_r8() {

}


void call_cond_imm16() {

}


void call_imm16() {

}


void ccf() {

}


void cp_a_imm8() {

}


void cp_a_r8() {

}


void cpl() {

}


void dec_r16() {

}


void dec_r8() {

}


void di() {

}


void ei() {

}


void halt() {

}


void inc_r16() {

}


void inc_r8() {

}


void jp_cond_imm16() {

}


void jp_hl() {

}


void jp_imm16() {

}


void jr_cond_imm8() {

}


void jr_imm8() {

}


void ld_a_imm16() {

}


void ld_a_r16mem() {

}


void ld_hl_sp_imm8() {

}


void ld_imm16_a() {

}


void ld_imm16_sp() {

}


void ld_r16_imm16() {

}


void ld_r16mem_a() {

}


void ld_r8_imm8() {

}


void ld_r8_r8() {

}


void ld_sp_hl() {

}


void ldh_a_c() {

}


void ldh_a_imm8() {

}


void ldh_c_a() {

}


void ldh_imm8_a() {

}


void nop() {

}


void or_a_imm8() {

}


void or_a_r8() {

}


void pop_r16stk() {

}


void prefix() {

}


void push_r16stk() {

}


void ret() {

}


void ret_cond() {

}


void reti() {

}


void rla() {

}


void rra() {

}


void rrca() {

}


void rst_tgt3() {

}


void sbc_a_imm8() {

}


void sbc_a_r8() {

}


void scf() {

}


void stop() {

}


void sub_a_imm8() {

}


void sub_a_r8() {

}


void xor_a_imm8() {

}


void xor_a_r8() {

}

void *instructions[] = {&nop, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &ccf, &ld_imm16_sp, &add_hl_r16, &ld_a_r16mem, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rrca, &stop, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rla, &jr_imm8, &add_hl_r16, &Default, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rra, &jr_cond_imm8, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &Default, &jr_cond_imm8, &add_hl_r16, &Default, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &cpl, &jr_cond_imm8, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &scf, &jr_cond_imm8, &add_hl_r16, &Default, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &Default, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &halt, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &ret_cond, &pop_r16stk, &jp_cond_imm16, &jp_imm16, &call_cond_imm16, &push_r16stk, &add_a_imm8, &rst_tgt3, &ret_cond, &ret, &jp_cond_imm16, &prefix, &call_cond_imm16, &call_imm16, &adc_a_imm8, &rst_tgt3, &ret_cond, &pop_r16stk, &jp_cond_imm16, &Default, &call_cond_imm16, &push_r16stk, &sub_a_imm8, &rst_tgt3, &ret_cond, &reti, &jp_cond_imm16, &Default, &call_cond_imm16, &Default, &sbc_a_imm8, &rst_tgt3, &ldh_imm8_a, &pop_r16stk, &ldh_c_a, &Default, &Default, &push_r16stk, &and_a_imm8, &rst_tgt3, &add_sp_imm8, &jp_hl, &ld_imm16_a, &Default, &Default, &Default, &xor_a_imm8, &rst_tgt3, &ldh_a_imm8, &pop_r16stk, &ldh_a_c, &di, &Default, &push_r16stk, &or_a_imm8, &rst_tgt3, &ld_hl_sp_imm8, &ld_sp_hl, &ld_a_imm16, &ei, &Default, &Default, &cp_a_imm8, &rst_tgt3};