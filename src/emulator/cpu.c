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
            uint8_t L; //Low
            uint8_t H; //High
        };
        uint16_t HL;
    };

    //Instruction register
    uint8_t IR;

    //Interrupt enable
    uint8_t IME;


} reg;


void cpu_init() {
    reg.PC = 0x100;
    reg.SP = 0xFFFE;
    reg.AF = 0;
    reg.BC = 0;
    reg.DE = 0;
    reg.HL = 0;
    reg.IR = 0;
    reg.IME = 0;



}

uint8_t opcode = 0;

//Fetches the next opcode and increments pc
uint8_t fetchOp() {
    opcode = memory_get(reg.PC++);
    return opcode;
    
}


uint8_t m_cycles = 0;
//Returns machine cycles instruction takes
uint8_t cpu_update() {
    m_cycles = 0;
    uint8_t op = fetchOp();
    

    return m_cycles;
}




#define R8 (0b00000111 & opcode)
uint8_t *r8_p[] = {&reg.B, &reg.C, &reg.D, &reg.E, &reg.H, &reg.L, 0, &reg.A};
#define R8_S (*r8_p[R8])

#define R16 ((0b0011000000 & opcode) >> 6)
uint16_t *r16_p[] = {&reg.BC, &reg.DE, &reg.HL, &reg.SP};
#define R16_S (*r16_p[R16])

#define R16stk ((0b0011000000 & opcode) >> 6)
uint16_t *r16stk_p[] = {&reg.BC, &reg.DE, &reg.HL, &reg.AF};
#define R16stk_S (*r16_p[R16stk])

#define COND ((0b00011000 & opcode) >> 3)

bool check_conditional(uint8_t cond) {
    switch(cond) {
        case 0: return !(reg.Z_flag); // NZ
        case 1: return (reg.Z_flag);  // Z
        case 2: return !(reg.C_flag); // NC   
        case 3: return (reg.C_flag);  // C
        default: return false;
    }
}



/*
Default instruction, just prints out the opcode that called it
Most of these will just lock the cpu on an actual gameboy
*/
void Default() {
    m_cycles = 1;
    printf("Default opcode, should not have been called\n");
    printf("Opcode = 0x%02X\n", opcode);
    
}

/*
ADC A,imm8
Add the value imm8 plus the carry flag to A
*/
void adc_a_imm8() {
    m_cycles = 2;

    uint8_t imm8 = memory_get(reg.PC++);

    reg.H_flag = ((reg.A & 0x0f) + (imm8 & 0x0f) + reg.C_flag) > 0x0f;

    uint16_t out = reg.A + imm8 + reg.C_flag;
    reg.A = (uint8_t)out;
    reg.Z_flag = (reg.A == 0);
    
    reg.C_flag = out >> 8;
    reg.N_flag = 0;

}

/*
ADC A,r8
Add the value in r8 plus the carry flag to A.
*/
void adc_a_r8() {
    
    uint8_t r8_value = 0;
    if(R8 == 6) {
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }
    
    reg.H_flag = ((reg.A & 0x0f) + (r8_value & 0x0f) + reg.C_flag) > 0x0f;

    uint16_t out = reg.A + r8_value + reg.C_flag;
    reg.A = (uint8_t)out;
    reg.Z_flag = (reg.A == 0);
    
    reg.C_flag = out >> 8;
    reg.N_flag = 0;
    
}

/*
ADD A,imm8
Add the value imm8 to A
*/
void add_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);

    reg.H_flag = ((reg.A & 0x0f) + (imm8 & 0x0f) + reg.C_flag) > 0x0f;

    uint16_t out = reg.A + imm8 + reg.C_flag;
    reg.A = (uint8_t)out;
    reg.Z_flag = (reg.A == 0);
    
    reg.C_flag = out >> 8;
    reg.N_flag = 0;

}

/*
ADD A,r8
Add the value in r8 to A
*/
void add_a_r8() {
    uint8_t r8_value = 0;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }
    
    reg.H_flag = ((reg.A & 0x0f) + (r8_value & 0x0f)) > 0x0f;

    uint16_t out = reg.A + r8_value;
    reg.A = (uint8_t)out;
    reg.Z_flag = (reg.A == 0);
    
    reg.C_flag = out >> 8;
    reg.N_flag = 0;
}

/*
ADD HL,r16
Add the value in r16 to HL
*/
void add_hl_r16() {
    m_cycles = 2;
    uint32_t out = R16_S + reg.HL;
    
    reg.H_flag = ((reg.HL & 0xff) + (R16_S & 0xff)) > 0x00ff;

    reg.HL = (uint16_t)out;
    reg.Z_flag = (reg.HL == 0);
    reg.C_flag = out >> 16;
    reg.N_flag = 0;
}

/*
ADD SP,e8
Add the signed value e8 to SP
*/
void add_sp_imm8() {
    m_cycles = 4;

    int8_t e8 = memory_get(reg.PC++);

    uint16_t out = (uint16_t)((int32_t)reg.SP + e8);

    reg.H_flag = ((reg.SP & 0x0F) + (e8 & 0x0F)) > 0x0F;

    reg.SP = out;

    reg.C_flag = ((reg.SP & 0xFF) + (e8 & 0xFF)) > 0xFF;
    reg.Z_flag = 0;
    reg.N_flag = 0;
}

/*
And A,imm8
Set A to bitwise AND between the value imm8 and A
*/
void and_a_imm8() {
    m_cycles = 2;

    uint8_t imm8 = memory_get(reg.PC++);

    reg.A = reg.A & imm8;    
    
    reg.Z_flag = (reg.A == 0);
    reg.H_flag = 1;
    reg.C_flag = 0;
    reg.N_flag = 0;
}

/*
AND A,r8
Set A to the bitwise AND between the value in r8 and A
*/
void and_a_r8() {
    uint8_t r8_value = 0;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }

    reg.A = reg.A & r8_value;    
    
    reg.Z_flag = (reg.A == 0);
    reg.H_flag = 1;
    reg.C_flag = 0;
    reg.N_flag = 0;

}

/*
CALL cc,n16
Call address n16 if condition cc is met.
*/
void call_cond_imm16() {
    
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    

    if(!check_conditional(COND)) {
        m_cycles = 3;
        return;
    }
    uint16_t addr = (hi << 8) | (lo);

    reg.SP--;
    memory_set(reg.SP, (reg.PC >> 8) & 0xFF);
    reg.SP--;
    memory_set(reg.SP, reg.PC & 0xFF);

    reg.PC = addr;
    m_cycles = 6;
}

/*
Call imm16
Call address imm16
This pushes the address of the instruction after the CALL on 
the stack, such that RET can pop it later; then, it executes 
an implicit JP n16.
*/
void call_imm16() {
    m_cycles = 6;
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    uint16_t addr = (hi << 8) | (lo);

    reg.SP--;
    memory_set(reg.SP, (reg.PC >> 8) & 0xFF);
    reg.SP--;
    memory_set(reg.SP, reg.PC & 0xFF);

    reg.PC = addr;

}

/*
CCF
Complement Carry Flag
*/
void ccf() {
    m_cycles = 1;
    reg.N_flag = 0;
    reg.H_flag = 0;
    reg.C_flag = !reg.C_flag;;
}

/*
CP A,imm8
ComPare the value in A with the value imm8.
This subtracts the value n8 from A and sets flags accordingly, but discards the result.
*/
void cp_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);

    reg.Z_flag = (reg.A == imm8);
    reg.N_flag = 1;
    reg.H_flag = ((imm8 & 0x0F) > (reg.A & 0x0F));
    reg.C_flag = (imm8 > reg.A);

}

/*
CP A,r8
ComPare the value in A with the value in r8
This subtracts the value in r8 from A and sets flags accordingly,
but discards the result
*/
void cp_a_r8() {
    uint8_t r8_value = 0;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }

    reg.Z_flag = (reg.A == r8_value);
    reg.N_flag = 1;
    reg.H_flag = ((r8_value & 0x0F) > (reg.A & 0x0F));
    reg.C_flag = (r8_value > reg.A); 
}

/*
CPL
ComPLement accumulator (A = ~A); Bitwise NOT
*/
void cpl() {
    m_cycles = 1;
    reg.A = ~reg.A;
    reg.N_flag = 1;
    reg.H_flag = 1;
}

/*
DAA
Decimal Adjust Accumulator
Designed to be used after performing an arithmetic instruction (ADD, ADC, SUB, SBC) whose inputs 
were in Binary-Coded Decimal (BCD), adjusting the result to likewise be in BCD.
*/
void daa()  {
    m_cycles = 1;

    uint8_t adjustment = 0;
    if(reg.N_flag) {
        if(reg.H_flag) adjustment += 0x6;
        if(reg.C_flag) adjustment += 0x60;
        reg.A -= adjustment;
    } else {
        if(reg.H_flag || (reg.A & 0xF) > 0x9) adjustment += 0x6; 
        if(reg.C_flag) {
            adjustment += 0x60;
            reg.C_flag = 1;
        }
        reg.A += adjustment;
    }
    if(reg.A == 0) reg.Z_flag = 0;
    reg.H_flag = 0;
}

/*
DEC r16
Decrement the value in register r16 by 1
*/
void dec_r16() {
    m_cycles = 2;
    R16_S -= 1;
}

/*
DEC r8
Decrement the value in register r8 by 1
*/
void dec_r8() {
    m_cycles = 1;
    uint8_t r8_value = 0;
    if(R8 == 6) { //[HL]
        m_cycles = 3;
        r8_value = memory_get(reg.HL);
        memory_set(reg.HL, r8_value - 1);
    } else {
        m_cycles = 1;
        r8_value = R8_S;
        R8_S = r8_value - 1;      
    }
    reg.Z_flag = (r8_value-1 == 0);
    reg.N_flag = 1;
    reg.H_flag = ((r8_value & 0x0F) == 0);

}

/*
DI
Disable Interrupts by clearing the IME flag
*/
void di() {
    m_cycles = 1;
    reg.IME = 0;
}

/*
EI
Enable Interrupts by seting the IME flag
*/
void ei() {
    m_cycles = 1;
    reg.IME = 1;
}

/*
HALT
Enter CPU low-power consumption mode until an interrupt occurs
*/
void halt() {
    m_cycles = 0;
    //TODO
}

/*
INC r16
Increment the value in register r16 by 1
*/
void inc_r16() {
    m_cycles = 2;
    R16_S += 1;
}

/*
INC r8
Increment the value in register r8 by 1
*/
void inc_r8() {
    m_cycles = 1;
    uint8_t r8_value = 0;
    if(R8 == 6) { //[HL]
        m_cycles = 3;
        r8_value = memory_get(reg.HL);
        memory_set(reg.HL, r8_value + 1);
    } else {
        m_cycles = 1;
        r8_value = R8_S;
        R8_S = r8_value + 1;   
    }
    reg.Z_flag = (r8_value+1 == 0);
    reg.N_flag = 0;
    reg.H_flag = ((r8_value & 0x0F) + 1 > 0x0F);
}

/*
JP cc,imm16
Jump to address imm16 if condition cc is met
*/
void jp_cond_imm16() {
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    

    if(!check_conditional(COND)) {
        m_cycles = 3;
        return;
    }
    uint16_t addr = (hi << 8) | (lo);

    m_cycles = 4;
    reg.PC = addr;

}

/*
JP HL
Jump to address in HL
*/
void jp_hl() {
    m_cycles = 1;
    reg.PC = reg.HL;

}

/*
JP imm8
Jump to address imm16
*/
void jp_imm16() {
    m_cycles = 4;
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    uint16_t addr = (hi << 8) | (lo);

    reg.PC = addr;
}

/*
JR cc,imm8
Relative Jump to 
*/
void jr_cond_imm8() {
    if(!check_conditional(COND)) {
        m_cycles = 2;
        return;
    }
    m_cycles = 3;
    int8_t imm8 = (int8_t)memory_get(reg.PC++);
    reg.PC += imm8;
}

/*
JR imm8
Relative Jump to address imm8
*/
void jr_imm8() {
    m_cycles = 3;
    int8_t imm8 = (int8_t)memory_get(reg.PC++);
    reg.PC += imm8;
}

/*
LD A,[imm16]
Copy the byte at address n16 into register A.
*/
void ld_a_imm16() {
    m_cycles = 4;
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    uint16_t imm16 = (hi << 8) | (lo);
    reg.A = memory_get(imm16);
}

/*
LD A,[r16]
Copy the byte pointed to by r16 into register
*/
void ld_a_r16mem() {
    m_cycles = 2;
    reg.A = memory_get(R16_S);
}

/*

*/
void ld_hl_sp_imm8() {
    m_cycles = 1;
}

/*
LD [imm16],A
Copy the value in register A into the byte at address n16.
*/
void ld_imm16_a() {
    m_cycles = 4;
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    uint16_t imm16 = (hi << 8) | (lo);
    memory_set(imm16, reg.A);
}

/*

*/
void ld_imm16_sp() {
    m_cycles = 1;
}

/*
LD r16,imm16
Copy the value imm16 into register r16
*/
void ld_r16_imm16() {
    m_cycles = 3;
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    uint16_t imm16 = (hi << 8) | (lo);
    R16_S = imm16;
}

/*
LD [r16],A
Copy the value in register A into the byte pointed to by r16.
*/
void ld_r16mem_a() {
    m_cycles = 2;
    memory_set(R16_S, reg.A);
}

/*
LD r8,imm8
Copy the value imm8 into register r8
*/
void ld_r8_imm8() {
    uint8_t imm8 = memory_get(reg.PC++);
    if(R8 == 6) {
        m_cycles = 2;
        memory_set(reg.HL, imm8);
        return;
    }
    m_cycles = 3;
    R8_S = imm8;
}

/*
LD r8,r8
Copy (aka Load) the value in register on the right into the register on the left.
*/
void ld_r8_r8() {
    
    uint8_t dest = (0b00111000 & opcode) >> 3;
    uint8_t src  = (0b00000111 & opcode);

    if(dest == src) { // NOP if both are the same
        m_cycles = 1;
        return;
    } else if (dest == 6) { //[HL]
        m_cycles = 2;
        memory_set(reg.HL, *r8_p[src]);
    } else if (src == 6) { //[HL]
        m_cycles = 2;
        *r8_p[dest] = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_p[dest] = r8_p[src]; 
    }
}

/*
LD SP,HL
Copy register HL into register SP.
*/
void ld_sp_hl() {
    m_cycles = 2;
    reg.SP = reg.HL;
}

/*
LD A,[$FF00+C]
Copy the byte at address $FF00+C into register A.
*/
void ldh_a_c() {
    m_cycles = 2;
    reg.A = memory_get(reg.C + 0xFF00);
}

/*

*/
void ldh_a_imm8() {
    m_cycles = 1;
}

/*

*/
void ldh_c_a() {
    m_cycles = 2;
    memory_set(reg.C + 0xFF00, reg.A);
}

/*

*/
void ldh_imm8_a() {
    m_cycles = 1;
}

/*

*/
void nop() {
    m_cycles = 1;
}

/*

*/
void or_a_imm8() {
    m_cycles = 1;
}

/*

*/
void or_a_r8() {
    m_cycles = 1;
}

/*

*/
void pop_r16stk() {
    m_cycles = 1;
}

/*

*/
void prefix() {
    m_cycles = 1;
}

/*

*/
void push_r16stk() {
    m_cycles = 1;
}

/*

*/
void ret() {
    m_cycles = 1;
}

/*

*/
void ret_cond() {
    m_cycles = 1;
}

/*

*/
void reti() {
    m_cycles = 1;
}

/*

*/
void rla() {
    m_cycles = 1;
}

/*

*/
void rra() {
    m_cycles = 1;
}

/*

*/
void rrca() {
    m_cycles = 1;
}

/*

*/
void rst_tgt3() {
    m_cycles = 1;
}

/*

*/
void sbc_a_imm8() {
    m_cycles = 1;
}

/*

*/
void sbc_a_r8() {
    m_cycles = 1;
}

/*

*/
void scf() {
    m_cycles = 1;
}

/*

*/
void stop() {
    m_cycles = 1;
}

/*

*/
void sub_a_imm8() {
    m_cycles = 1;
}

/*

*/
void sub_a_r8() {
    m_cycles = 1;
}

/*

*/
void xor_a_imm8() {
    m_cycles = 1;
}

/*

*/
void xor_a_r8() {
    m_cycles = 1;
}


void *instructions[] = {&nop, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &ccf, &ld_imm16_sp, &add_hl_r16, &ld_a_r16mem, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rrca, &stop, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rla, &jr_imm8, &add_hl_r16, &Default, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rra, &jr_cond_imm8, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &daa, &jr_cond_imm8, &add_hl_r16, &Default, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &cpl, &jr_cond_imm8, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &scf, &jr_cond_imm8, &add_hl_r16, &Default, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &Default, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &halt, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &ret_cond, &pop_r16stk, &jp_cond_imm16, &jp_imm16, &call_cond_imm16, &push_r16stk, &add_a_imm8, &rst_tgt3, &ret_cond, &ret, &jp_cond_imm16, &prefix, &call_cond_imm16, &call_imm16, &adc_a_imm8, &rst_tgt3, &ret_cond, &pop_r16stk, &jp_cond_imm16, &Default, &call_cond_imm16, &push_r16stk, &sub_a_imm8, &rst_tgt3, &ret_cond, &reti, &jp_cond_imm16, &Default, &call_cond_imm16, &Default, &sbc_a_imm8, &rst_tgt3, &ldh_imm8_a, &pop_r16stk, &ldh_c_a, &Default, &Default, &push_r16stk, &and_a_imm8, &rst_tgt3, &add_sp_imm8, &jp_hl, &ld_imm16_a, &Default, &Default, &Default, &xor_a_imm8, &rst_tgt3, &ldh_a_imm8, &pop_r16stk, &ldh_a_c, &di, &Default, &push_r16stk, &or_a_imm8, &rst_tgt3, &ld_hl_sp_imm8, &ld_sp_hl, &ld_a_imm16, &ei, &Default, &Default, &cp_a_imm8, &rst_tgt3};