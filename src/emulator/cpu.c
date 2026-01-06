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
            //union {
                uint8_t F;
                /*struct {
                    uint8_t f_unused : 4;
                    uint8_t C_flag : 1; //carry flag
                    uint8_t H_flag : 1; //half carry flag
                    uint8_t N_flag : 1; //Subtraction flag
                    uint8_t Z_flag : 1; //zero flag
                };*/
            //};
            
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

// Access flags via helper macros
#define Z_FLAG ((reg.F & 0x80)>>7)
#define N_FLAG ((reg.F & 0x40)>>6)
#define H_FLAG ((reg.F & 0x20)>>5)
#define C_FLAG ((reg.F & 0x10)>>4)

#define SET_Z(x) reg.F = (reg.F & ~0x80) | ((x) ? 0x80 : 0)
#define SET_N(x) reg.F = (reg.F & ~0x40) | ((x) ? 0x40 : 0)
#define SET_H(x) reg.F = (reg.F & ~0x20) | ((x) ? 0x20 : 0)
#define SET_C(x) reg.F = (reg.F & ~0x10) | ((x) ? 0x10 : 0)


bool ei_flag = false; //flag to set interrupts after next instruction
bool halt_flag = false; //flag to set CPU to halted state



void cpu_init() {
    reg.PC = 0x0000;
    reg.SP = 0xFFFE;
    reg.AF = 0x01B0;
    reg.BC = 0x0013;
    reg.DE = 0x00D8;
    reg.HL = 0x014D;
    reg.IR = 0;
    reg.IME = 0;
}

uint8_t m_cycles = 0;

uint8_t opcode = 0;

//Fetches the next opcode and increments pc
uint8_t fetchOp() {
    opcode = memory_get(reg.PC++);
    return opcode;
    
}

void handle_interrupt() {
    uint8_t interrupts = memory_get(IE) & memory_get(IF); // IE & IF
    if(!interrupts) return;

    m_cycles = 5;
    reg.IME = false;
    halt_flag = false;

    //loop through interrupts in priority order
    for(int i = 0; i< 5; i++) {
        if(interrupts & (1<<i)) {
            memory_set(IF, memory_get(IF) & ~(1<<i)); //clear interrupt flag

            //push PC to stack
            reg.SP--;
            memory_set(reg.SP, (reg.PC >> 8) & 0xFF);
            reg.SP--;
            memory_set(reg.SP, reg.PC & 0xFF);
            
            

            reg.PC = 0x40 + (i * 8); //set PC to interrupt vector address
            break;
        }

    }
}



//Returns machine cycles instruction takes
void cpu_update() {

    
    if(m_cycles > 0) {
        m_cycles--;
        return;
    }
    
    //check if interrupts need enabled
    bool enable_interrupts = ei_flag;
    ei_flag = false;

    
    //Check interrupts, even if CPU is halted
    if(reg.IME && (memory_get(IE) & memory_get(IF))) {
        handle_interrupt();
        return;
    }
    

    if(halt_flag) {
        if(memory_get(IE) & memory_get(IF)) {
            halt_flag = false;
        } else {
            return;
        }
    }
    

    uint8_t op = fetchOp();
    //run operation
    //printf("PC=0x%04X opcode=0x%02X", reg.PC, opcode);
    instructions[op]();
    //printf(" AF=0x%04X BC=0x%04X DE=0x%04X HL=0x%04X SP=0x%04X\n", reg.AF, reg.BC, reg.DE, reg.HL, reg.SP);
    //debugLog();
    #ifdef DEBUG_MODE
    debug_output->PC = reg.PC;
    debug_output->AF = reg.AF;
    debug_output->BC = reg.BC;
    debug_output->DE = reg.DE;
    debug_output->HL = reg.HL;
    debug_output->SP = reg.SP;
    debug_output->MEM_PC = ((uint32_t)memory_get(reg.PC+3)) | ((uint32_t)memory_get(reg.PC+2) << 8) | ((uint32_t)memory_get(reg.PC+1) << 16) | ((uint32_t)memory_get(reg.PC) << 24);
    #endif // !DEBUG_MODE


    //ei instruction only enables interrupts after the next opcode finishes, so thats what this does
    if(enable_interrupts) reg.IME = true;
    
    return;
}




#define R8 (0b00000111 & opcode)
uint8_t *r8_p[] = {&reg.B, &reg.C, &reg.D, &reg.E, &reg.H, &reg.L, 0, &reg.A};
#define R8_S (*r8_p[R8])

#define R16 ((0b00110000 & opcode) >> 4)
uint16_t *r16_p[] = {&reg.BC, &reg.DE, &reg.HL, &reg.SP};
#define R16_S (*r16_p[R16])

#define R16stk ((0b00110000 & opcode) >> 4)
uint16_t *r16stk_p[] = {&reg.BC, &reg.DE, &reg.HL, &reg.AF};
#define R16stk_S (*r16stk_p[R16stk])

#define R16mem ((0b00110000 & opcode) >> 4)
uint16_t *r16mem_p[] = {&reg.BC, &reg.DE, &reg.HL, &reg.HL};
#define R16mem_S (*r16mem_p[R16mem])

#define COND ((0b00011000 & opcode) >> 3)

bool check_conditional(uint8_t cond) {
    switch(cond) {
        case 0: return !(Z_FLAG); // NZ
        case 1: return (Z_FLAG);  // Z
        case 2: return !(C_FLAG); // NC   
        case 3: return (C_FLAG);  // C
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

    SET_H(((reg.A & 0x0f) + (imm8 & 0x0f) + C_FLAG) > 0x0f);

    uint16_t out = reg.A + imm8 + C_FLAG;
    reg.A = (uint8_t)out;
    SET_Z(reg.A == 0);
    
    SET_C(out >> 8);
    SET_N(0);

}

/*
ADC A,r8
Add the value in r8 plus the carry flag to A.
*/
void adc_a_r8() {
    
    uint8_t r8_value;
    if(R8 == 6) {
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }
    
    SET_H(((reg.A & 0x0f) + (r8_value & 0x0f) + C_FLAG) > 0x0f);

    uint16_t out = reg.A + r8_value + C_FLAG;
    reg.A = (uint8_t)out;
    SET_Z(reg.A == 0);
    
    SET_C(out >> 8);
    SET_N(0);
    
}

/*
ADD A,imm8
Add the value imm8 to A
*/
void add_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);

    SET_H(((reg.A & 0x0f) + (imm8 & 0x0f)) > 0x0f);

    uint16_t out = reg.A + imm8;
    reg.A = (uint8_t)out;
    SET_Z(reg.A == 0);
    
    SET_C(out >> 8);
    SET_N(0);

}

/*
ADD A,r8
Add the value in r8 to A
*/
void add_a_r8() {
    uint8_t r8_value;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }
    
    SET_H(((reg.A & 0x0f) + (r8_value & 0x0f)) > 0x0f);

    uint16_t out = reg.A + r8_value;
    reg.A = (uint8_t)out;
    SET_Z(reg.A == 0);
    SET_C(out >> 8);
    SET_N(0);
}

/*
ADD HL,r16
Add the value in r16 to HL
*/
void add_hl_r16() {
    m_cycles = 2;
    uint32_t out = R16_S + reg.HL;
    
    SET_H(((reg.HL & 0x0fff) + (R16_S & 0x0fff)) > 0x0fff);

    reg.HL = (uint16_t)out;
    SET_C(out >> 16);
    SET_N(0);
}

/*
ADD SP,e8
Add the signed value e8 to SP
*/
void add_sp_imm8() {
    m_cycles = 4;

    int8_t e8 = memory_get(reg.PC++);

    uint16_t out = (uint16_t)((int32_t)reg.SP + e8);

    //TODO Confirm the following:
    SET_H(((reg.SP & 0x0F) + ((uint8_t)e8 & 0x0F)) > 0x0F);
    SET_C(((reg.SP & 0xFF) + ((uint8_t)e8 & 0xFF)) > 0xFF);
    SET_Z(0);
    SET_N(0);

    reg.SP = out;
}

/*
And A,imm8
Set A to bitwise AND between the value imm8 and A
*/
void and_a_imm8() {
    m_cycles = 2;

    uint8_t imm8 = memory_get(reg.PC++);

    reg.A = reg.A & imm8;    
    
    SET_Z( reg.A == 0 );
    SET_H(1);
    SET_C(0);
    SET_N(0);
}

/*
AND A,r8
Set A to the bitwise AND between the value in r8 and A
*/
void and_a_r8() {
    uint8_t r8_value;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }

    reg.A = reg.A & r8_value;    
    
    SET_Z( reg.A == 0 );
    SET_H(1);
    SET_C(0);
    SET_N(0);


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
    SET_N(0);
    SET_H(0);
    SET_C(!C_FLAG);
}

/*
CP A,imm8
ComPare the value in A with the value imm8.
This subtracts the value n8 from A and sets flags accordingly, but discards the result.
*/
void cp_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);

    SET_Z( reg.A == imm8 );
    SET_N(1);
    SET_H( (imm8 & 0x0F) > (reg.A & 0x0F) );
    SET_C( imm8 > reg.A );

}

/*
CP A,r8
ComPare the value in A with the value in r8
This subtracts the value in r8 from A and sets flags accordingly,
but discards the result
*/
void cp_a_r8() {
    uint8_t r8_value;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }

    SET_Z(reg.A == r8_value);
    SET_N(1);
    SET_H((r8_value & 0x0F) > (reg.A & 0x0F));
    SET_C(r8_value > reg.A);
}

/*
CPL
ComPLement accumulator (A = ~A); Bitwise NOT
*/
void cpl() {
    m_cycles = 1;
    reg.A = ~reg.A;
    SET_H(1);
    SET_N(1);
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
    if(N_FLAG) {
        if(H_FLAG) adjustment += 0x6;
        if(C_FLAG) adjustment += 0x60;
        reg.A -= adjustment;
    } else {
        if(H_FLAG || (reg.A & 0xF) > 0x9) adjustment += 0x6; 
        if(C_FLAG || reg.A > 0x99) {
            adjustment += 0x60;
            SET_C(1);
        }
        reg.A += adjustment;
    }
    SET_Z( reg.A == 0 );
    SET_H(0);
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
    uint8_t r8 = (0b00111000 & opcode) >> 3; // r8 is in a non-standard location
    uint8_t r8_value;
    if(r8 == 6) { //[HL]
        m_cycles = 3;
        r8_value = memory_get(reg.HL);
        memory_set(reg.HL, r8_value - 1);
    } else {
        m_cycles = 1;
        r8_value = *r8_p[r8];
        *r8_p[r8] = r8_value - 1;      
    }
    SET_Z( (r8_value-1) == 0 );
    SET_N(1);
    SET_H( (r8_value & 0x0F) == 0 );

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
    ei_flag = true;
}

/*
HALT
Enter CPU low-power consumption mode until an interrupt occurs
*/
void halt() {
    m_cycles = 1;
    halt_flag = true;
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
    uint8_t r8 = (0b00111000 & opcode) >> 3; // r8 is in a non-standard location
    uint8_t r8_value;
    if(r8 == 6) { //[HL]
        m_cycles = 3;
        r8_value = memory_get(reg.HL);
        memory_set(reg.HL, r8_value + 1);
    } else {
        m_cycles = 1;
        r8_value = *r8_p[r8];
        *r8_p[r8] = r8_value + 1;   
    }
    SET_Z( (uint8_t)(r8_value+1) == 0 );
    SET_N(0);
    SET_H( ((r8_value & 0x0F) + 1) > 0x0F ); //TODO: This could be wrong allegedly
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
        reg.PC++;
        return;
    }
    m_cycles = 3;
    int8_t imm8 = (int8_t)memory_get(reg.PC++);
    reg.PC = (uint16_t)(reg.PC + imm8);
    //printf("PC=0x%04X opcode=0x%02X Z=%d N=%d H=%d C=%d\n",
    //   reg.PC, opcode, reg.Z_flag, reg.N_flag, reg.H_flag, reg.C_flag);
    //printf("JR Cond taken to 0x%04X\n", reg.PC); //test
}

/*
JR imm8
Relative Jump to address imm8
*/
void jr_imm8() {
    m_cycles = 3;
    int8_t imm8 = (int8_t)memory_get(reg.PC++);
    reg.PC += imm8;
    //printf("PC=0x%04X opcode=0x%02X Z=%d N=%d H=%d C=%d\n",
    //   reg.PC, opcode, reg.Z_flag, reg.N_flag, reg.H_flag, reg.C_flag);
    //printf("JR taken to 0x%04X\n", reg.PC); //test
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
    reg.A = memory_get(R16mem_S);
    if(R16mem == 2) { //HL+
        reg.HL++; 
    } else if (R16mem == 3) { //HL-
        reg.HL--; 
    }
}

/*
LD HL,SP+e8
Add the signed value e8 to SP and copy the result to HL
*/
void ld_hl_sp_imm8() {
    m_cycles = 4;

    int8_t e8 = memory_get(reg.PC++);

    reg.HL = (uint16_t)((int32_t)reg.SP + e8);
    //TODO confirm the following:
    SET_H(((reg.SP & 0x0F) + ((uint8_t)e8 & 0x0F)) > 0x0F);
    SET_C(((reg.SP & 0xFF) + ((uint8_t)e8 & 0x0FF)) > 0xFF);
    SET_Z(0);
    SET_N(0);

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
LD [imm16],SP
Copy SP & $FF at address imm16 and SP >> 8 at address n16+1
*/
void ld_imm16_sp() {
    m_cycles = 5;
    uint8_t lo = memory_get(reg.PC++);
    uint8_t hi = memory_get(reg.PC++);
    uint16_t imm16 = (hi << 8) | (lo);
    memory_set(imm16, reg.SP & 0xFF);
    memory_set(imm16 + 1, reg.SP >> 8);
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
    //TODO check if there is special behavior for SP
    R16_S = imm16;
}

/*
LD [r16],A
Copy the value in register A into the byte pointed to by r16.
*/
void ld_r16mem_a() {
    m_cycles = 2;
    memory_set(R16mem_S, reg.A);
    if(R16mem == 2) { //HL+
        reg.HL++; 
    } else if (R16mem == 3) { //HL-
        reg.HL--; 
    }
}

/*
LD r8,imm8
Copy the value imm8 into register r8
*/
void ld_r8_imm8() {
    uint8_t imm8 = memory_get(reg.PC++);
    uint8_t r8 = (0b00111000 & opcode) >> 3; // r8 is in a non-standard location
    if(r8 == 6) {
        m_cycles = 2;
        memory_set(reg.HL, imm8);
        return;
    }
    m_cycles = 3;
    *r8_p[r8] = imm8;
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
        *r8_p[dest] = *r8_p[src]; 
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
LDH A,[imm16]
Copy the byte at address n16 into register A, provided the address is between $FF00 and $FFFF.
*/
void ldh_a_imm8() {
    m_cycles = 3;
    uint8_t lo = memory_get(reg.PC++);
    uint16_t imm16 = 0xFF00 | (lo);
    reg.A = memory_get(imm16);
    
}

/*
LDH [C],A
Copy the value in register A into the byte at address $FF00+C.
*/
void ldh_c_a() {
    m_cycles = 2;
    memory_set(reg.C + 0xFF00, reg.A);
}

/*
LDH [n16],A
Copy the value in register A into the byte at address n16, provided
the address is between $FF00 and $FFFF.
*/
void ldh_imm8_a() {
    m_cycles = 3;
    uint8_t lo = memory_get(reg.PC++);
    uint16_t imm16 = 0xFF00 | (lo);
    memory_set(imm16, reg.A);
}

/*
NOP
No Operation, 1 cycle
*/
void nop() {
    m_cycles = 1;
}

/*
OR A,imm8
Set A to the bitwise OR between the value imm8 and A.
*/
void or_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);
    reg.A = reg.A | imm8;

    SET_Z( reg.A == 0 );
    SET_N(0);
    SET_H(0);
    SET_C(0);
}

/*
OR A,r8
Set A to the bitwise OR between the value in r8 and A.
*/
void or_a_r8() {
    
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        reg.A = reg.A | memory_get(reg.HL);
    } else {
        m_cycles = 1;
        reg.A = reg.A | R8_S;
    }

    SET_Z( reg.A == 0 );
    SET_N(0);
    SET_H(0);
    SET_C(0);
}

/*
POP AF
Pop register AF from the stack. This is roughly equivalent to the following imaginary instructions:

    LD F, [SP]
    INC SP
    LD A, [SP]
    INC SP

OR

POP r16
Pop register r16 from the stack. This is roughly equivalent to the following imaginary instructions:

    LD LOW(r16), [SP]   ; C, E or L
    INC SP
    LD HIGH(r16), [SP]  ; B, D or H
    INC SP


*/
void pop_r16stk() {
    m_cycles = 3;
    if(R16stk == 3) {
        reg.F = memory_get(reg.SP++) & 0xF0;
        reg.A = memory_get(reg.SP++);
        SET_Z( (reg.F & 0b10000000) >> 7 );
        SET_N( (reg.F & 0b01000000) >> 6 );
        SET_H( (reg.F & 0b00100000) >> 5 );
        SET_C( (reg.F & 0b00010000) >> 4 );

    } else {
        uint8_t lo = memory_get(reg.SP++);
        uint8_t hi = memory_get(reg.SP++);
        R16stk_S = (hi << 8) | (lo);
    }
}

/*
$CB prefix
Use next byte to call another instruction
*/
void prefix() {
    fetchOp();
    cb_instructions[opcode]();
    //printf("CB PREFIX: PC=0x%04X opcode=0x%02X\n", reg.PC, opcode);
    
}

/*
PUSH r16
Push register r16 into the stack.  This is roughly equivalent to the following imaginary instructions
    DEC SP
    LD [SP], HIGH(r16)
    DEC SP
    LD [SP], LOW(r16)
*/
void push_r16stk() {
    m_cycles = 4;
    if(R16stk == 3) { //AF
        reg.SP--;
        memory_set(reg.SP, reg.A);
        reg.SP--;
        uint8_t f = (Z_FLAG << 7) | (N_FLAG << 6) | (H_FLAG << 5) | (C_FLAG << 4);
        memory_set(reg.SP, f);
        return;   
    }
    //TODO check order of high vs low
    reg.SP--;
    memory_set(reg.SP, (R16stk_S >> 8) & 0xFF);
    reg.SP--;
    memory_set(reg.SP, R16stk_S & 0xFF);
}

/*
RET
Return from subroutine.  This is basically a POP PC. See POPr16
*/
void ret() {
    m_cycles = 4;
    uint8_t lo = memory_get(reg.SP++);
    uint8_t hi = memory_get(reg.SP++);
    reg.PC = (hi << 8) | (lo);
}

/*
RET cc
Return from subroutine if condition cc is met
*/
void ret_cond() {
    if(!check_conditional(COND)) {
        m_cycles = 2;
        return;
    }
    m_cycles = 5;
    uint8_t lo = memory_get(reg.SP++);
    uint8_t hi = memory_get(reg.SP++);
    reg.PC = (hi << 8) | (lo);
}

/*
RETI
Return from subroutine and enable interrupts
*/
void reti() {
    m_cycles = 4;
    uint8_t lo = memory_get(reg.SP++);
    uint8_t hi = memory_get(reg.SP++);
    reg.PC = (hi << 8) | (lo);
    reg.IME = 1;
}

/*
RLA
Rotate register A left, through the carry flag
*/
void rla() {
    m_cycles = 1;
    uint8_t shifted_a = (reg.A << 1) | C_FLAG;
    SET_C( (reg.A >> 7) & 0b1 ); //masking not entirely necessary
    reg.A = shifted_a;
    SET_Z(0);
    SET_N(0);
    SET_H(0);
}

/*
RLCA
Rotate Register A left
*/
void rlca() {
    m_cycles = 2;
    uint8_t shifted_r8 = (reg.A << 1) | (reg.A >> 7);
    SET_C( (reg.A >> 7) & 0b1 ); //masking not entirely necessary
    reg.A = shifted_r8;

    SET_Z(0);
    SET_N(0);
    SET_H(0);
}

/*
RRA
Rotate register A right, through the carry flag
*/
void rra() {
    m_cycles = 1;
    uint8_t shifted_a = (reg.A >> 1) | (C_FLAG << 7);
    SET_C( reg.A & 0b1 );
    reg.A = shifted_a;
    
    SET_Z(0);
    SET_N(0);
    SET_H(0);
}

/*
RRCA
Rotate register A right
*/
void rrca() {
    m_cycles = 1;
    SET_C( reg.A & 0b1 );
    reg.A = (reg.A >> 1) | (reg.A << 7);

    SET_Z(0);
    SET_N(0);
    SET_H(0);
}

/*
RST tgt3
Call address tgt3.  This is a shorter and faster equivalent to CALL for suitable values of tgt3
*/
void rst_tgt3() {
    m_cycles = 4;
    uint8_t tgt3 = (0b00111000 & opcode) >> 3;
    uint16_t addr = tgt3<<3; // multiplication by 8


    reg.SP--;
    memory_set(reg.SP, (reg.PC >> 8) & 0xFF);
    reg.SP--;
    memory_set(reg.SP, reg.PC & 0xFF);

    reg.PC = addr;

}

/*
SBC A,imm8
*/
void sbc_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);

    uint8_t result = reg.A - imm8 - C_FLAG;

    SET_Z(result == 0 );
    SET_N(1);
    SET_H( ((imm8 & 0x0F) + C_FLAG) > (reg.A & 0x0F) );
    SET_C( reg.A < imm8 + C_FLAG );

    reg.A = result;
}

/*
SBC A,r8
Subtract the value in r8 and the carry flag from A
*/
void sbc_a_r8() {
    m_cycles = 1;
    uint8_t r8_value;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }
    uint8_t result = reg.A - r8_value - C_FLAG;

    SET_Z(result == 0 );
    SET_N(1);
    SET_H( ((r8_value & 0x0F) + C_FLAG) > (reg.A & 0x0F) );
    SET_C( reg.A < r8_value + C_FLAG );

    reg.A = result;
}

/*
Set Carry Flag
*/
void scf() {
    m_cycles = 1;
    SET_N(0);
    SET_H(0);
    SET_C(1);
}

/*
STOP
Enter CPU very low power mode. Also used to switch between GBC double speed and normal speed CPU modes.
This is not used in any licensed rom.  On GDC it switches speed modes.
TODO: implement the Stop function shenanigans, referencing https://gbdev.io/pandocs/Reducing_Power_Consumption.html
*/
void stop() {
    m_cycles = 1;
    reg.PC++; //ignore the next byte
}

/*
SUB A,imm8
Subtract the value imm8 from A.
*/
void sub_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);

    uint8_t result = reg.A - imm8;
    SET_Z(result == 0 );
    SET_N(1);
    SET_H( (imm8 & 0x0F) > (reg.A & 0x0F) );
    SET_C( reg.A < imm8 );

    reg.A = result;
}

/*
SUB A,r8
*/
void sub_a_r8() {
    m_cycles = 1;
    uint8_t r8_value;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }
    uint8_t result = reg.A - r8_value;
    
    SET_Z(result == 0 );
    SET_N(1);
    SET_H( (r8_value & 0x0F) > (reg.A & 0x0F) );
    SET_C( reg.A < r8_value );
    
    reg.A = result;
}

/*
XOR A,imm8
Set A to the bitwise XOR between the value imm8 and A.
*/
void xor_a_imm8() {
    m_cycles = 2;
    uint8_t imm8 = memory_get(reg.PC++);
    reg.A = imm8 ^ reg.A;

    SET_Z( reg.A == 0 );
    SET_N(0);
    SET_H(0);
    SET_C(0);
}

/*
XOR A,r8
Set A to the bitwise XOR between the value in r8 and A.
*/
void xor_a_r8() {
    m_cycles = 1;
    uint8_t r8_value;
    if(R8 == 6) { //[HL]
        m_cycles = 2;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 1;
        r8_value = R8_S;        
    }
    reg.A = r8_value ^ reg.A;

    SET_Z( reg.A == 0 );
    SET_N(0);
    SET_H(0);
    SET_C(0);
}




void (*instructions[256])() = {&nop, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rlca, &ld_imm16_sp, &add_hl_r16, &ld_a_r16mem, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rrca, &stop, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rla, &jr_imm8, &add_hl_r16, &ld_a_r16mem, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &rra, &jr_cond_imm8, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &daa, &jr_cond_imm8, &add_hl_r16, &ld_a_r16mem, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &cpl, &jr_cond_imm8, &ld_r16_imm16, &ld_r16mem_a, &inc_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &scf, &jr_cond_imm8, &add_hl_r16, &ld_a_r16mem, &dec_r16, &inc_r8, &dec_r8, &ld_r8_imm8, &ccf, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &halt, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &ld_r8_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &add_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &adc_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sub_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &sbc_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &and_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &xor_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &or_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &cp_a_r8, &ret_cond, &pop_r16stk, &jp_cond_imm16, &jp_imm16, &call_cond_imm16, &push_r16stk, &add_a_imm8, &rst_tgt3, &ret_cond, &ret, &jp_cond_imm16, &prefix, &call_cond_imm16, &call_imm16, &adc_a_imm8, &rst_tgt3, &ret_cond, &pop_r16stk, &jp_cond_imm16, &Default, &call_cond_imm16, &push_r16stk, &sub_a_imm8, &rst_tgt3, &ret_cond, &reti, &jp_cond_imm16, &Default, &call_cond_imm16, &Default, &sbc_a_imm8, &rst_tgt3, &ldh_imm8_a, &pop_r16stk, &ldh_c_a, &Default, &Default, &push_r16stk, &and_a_imm8, &rst_tgt3, &add_sp_imm8, &jp_hl, &ld_imm16_a, &Default, &Default, &Default, &xor_a_imm8, &rst_tgt3, &ldh_a_imm8, &pop_r16stk, &ldh_a_c, &di, &Default, &push_r16stk, &or_a_imm8, &rst_tgt3, &ld_hl_sp_imm8, &ld_sp_hl, &ld_a_imm16, &ei, &Default, &Default, &cp_a_imm8, &rst_tgt3};




////////////////////////////////////////////////////////////////
/////////////////// $ CB Prefix Instructions ///////////////////
////////////////////////////////////////////////////////////////




/*
BIT b3,r8
Test bit b3 in register r8, set the zero flag if bit not set.
*/
void bit_b3_r8() {
    uint8_t b3 = (0b00111000 & opcode) >> 3;
    uint8_t r8_value;
    if(R8 == 6) { //[HL]
        m_cycles = 3;
        r8_value = memory_get(reg.HL);
    } else {
        m_cycles = 2;
        r8_value = R8_S;        
    }

    SET_Z( !((r8_value >> b3) & 0b1) );
    SET_N(0);
    SET_H(1);
}

/*
RES b3,r8
Set bit u3 in register r8 to 0. Bit 0 is the rightmost one, bit 7 is the leftmost one.
*/
void res_b3_r8() {
    uint8_t b3 = (0b00111000 & opcode) >> 3;
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        memory_set(reg.HL, memory_get(reg.HL) & ~(1 << b3));
    } else {
        m_cycles = 2;
        R8_S = R8_S & ~(1 << b3);        
    }
}

/*
RL r8
Rotate bits in register r8 left, through the carry flag.
*/
void rl_r8() {
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        uint8_t shifted_r8 = (imm8 << 1) | C_FLAG;
        SET_C( (imm8 >> 7) & 0b1 ); //masking not entirely necessary
        memory_set(reg.HL, shifted_r8);
        SET_Z( shifted_r8 == 0 );
    } else {
        m_cycles = 2;
        uint8_t shifted_r8 = (R8_S << 1) | C_FLAG;
        SET_C( (R8_S >> 7) & 0b1 ); //masking not entirely necessary
        R8_S = shifted_r8;
        SET_Z( R8_S == 0 );
    }
    SET_N(0);
    SET_H(0);
}

/*
RLC r8
Rotate register r8 left.
*/
void rlc_r8() {
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        uint8_t shifted_r8 = (imm8 << 1) | (imm8 >> 7);
        SET_C( (imm8 >> 7) & 0b1 ); //masking not entirely necessary
        memory_set(reg.HL, shifted_r8);
        SET_Z( shifted_r8 == 0 );
    } else {
        m_cycles = 2;
        uint8_t shifted_r8 = (R8_S << 1) | (R8_S >> 7);
        SET_C( (R8_S >> 7) & 0b1 ); //masking not entirely necessary
        R8_S = shifted_r8;
        SET_Z(R8_S == 0);
    }
    SET_N(0);
    SET_H(0);
}


/*
RR r8
Rotate register r8 right, through the carry flag.
*/
void rr_r8() {
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        uint8_t shifted_r8 = (imm8 >> 1) | (C_FLAG << 7);
        SET_C( imm8 & 0b1 ); //masking not entirely necessary
        memory_set(reg.HL, shifted_r8);
        SET_Z( shifted_r8 == 0 );
    } else {
        m_cycles = 2;
        uint8_t shifted_r8 = (R8_S >> 1) | (C_FLAG << 7);
        SET_C(R8_S & 0b1); //masking not entirely necessary
        R8_S = shifted_r8;
        SET_Z(R8_S == 0);
    }
    SET_N(0);
    SET_H(0);
}

/*
RRC r8
Rotate register r8 right.
*/
void rrc_r8() {
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        uint8_t shifted_r8 = (imm8 >> 1) | (imm8 << 7);
        SET_C( imm8 & 0b1 ); //masking not entirely necessary
        memory_set(reg.HL, shifted_r8);
        SET_Z( shifted_r8 == 0 );
    } else {
        m_cycles = 2;
        uint8_t shifted_r8 = (R8_S >> 1) | (R8_S << 7);
        SET_C(R8_S & 0b1); //masking not entirely necessary
        R8_S = shifted_r8;
        SET_Z(R8_S == 0);
    }
    SET_N(0);
    SET_H(0);
}

/*

*/
void set_b3_r8() {
    uint8_t b3 = (0b00111000 & opcode) >> 3;
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        memory_set(reg.HL, memory_get(reg.HL) | (1 << b3));
    } else {
        m_cycles = 2;
        R8_S = R8_S | (1 << b3);        
    }
}

/*
SLA r8
Shift Left Arithmetically register r8
*/
void sla_r8() {
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        uint8_t shifted_r8 = (imm8 << 1);
        SET_C((imm8 >> 7) & 0b1); //masking not entirely necessary
        memory_set(reg.HL, shifted_r8);
        SET_Z( shifted_r8 == 0 );
    } else {
        m_cycles = 2;
        uint8_t shifted_r8 = (R8_S << 1);
        SET_C((R8_S >> 7) & 0b1); //masking not entirely necessary
        R8_S = shifted_r8;
        SET_Z( R8_S == 0 );
    }
    SET_N(0);
    SET_H(0);
}

/*
SRA r8
Shift Right Arithmetically register r8
*/
void sra_r8() {
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        uint8_t shifted_r8 = (imm8 >> 1) | (imm8 & 0b10000000);
        SET_C(imm8 & 0b1); //masking not entirely necessary
        memory_set(reg.HL, shifted_r8);
        SET_Z( shifted_r8 == 0 );
    } else {
        m_cycles = 2;
        uint8_t shifted_r8 = (R8_S >> 1) | (R8_S & 0b10000000);
        SET_C(R8_S & 0b1); //masking not entirely necessary
        R8_S = shifted_r8;
        SET_Z(R8_S == 0);
    }
    SET_N(0);
    SET_H(0);
}

/*
SRL r8
Shift Right Logically register r8
*/
void srl_r8() {
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        uint8_t shifted_r8 = (imm8 >> 1);
        SET_C(imm8 & 0b1); //masking not entirely necessary
        memory_set(reg.HL, shifted_r8);
        SET_Z( shifted_r8 == 0 );
    } else {
        m_cycles = 2;
        uint8_t shifted_r8 = (R8_S >> 1);
        SET_C(R8_S & 0b1); //masking not entirely necessary
        R8_S = shifted_r8;
        SET_Z(R8_S == 0);
    }
    SET_N(0);
    SET_H(0);
}

/*
SWAP r8
Swap the upper 4 bits in register r8 and the lower 4 ones.
*/
void swap_r8() {
    uint8_t swap;
    if(R8 == 6) { //[HL]
        m_cycles = 4;
        uint8_t imm8 = memory_get(reg.HL);
        swap = (imm8 >> 4) | (imm8 << 4);
        memory_set(reg.HL, swap);
    } else {
        m_cycles = 2;
        swap = (R8_S >> 4) | (R8_S << 4);
        R8_S = swap;

    }
    SET_Z( swap == 0 );
    SET_N(0);
    SET_H(0);
    SET_C(0);
}




void (*cb_instructions[256])() = {&rlc_r8, &rlc_r8, &rlc_r8, &rlc_r8, &rlc_r8, &rlc_r8, &rlc_r8, &rlc_r8, &rrc_r8, &rrc_r8, &rrc_r8, &rrc_r8, &rrc_r8, &rrc_r8, &rrc_r8, &rrc_r8, &rl_r8, &rl_r8, &rl_r8, &rl_r8, &rl_r8, &rl_r8, &rl_r8, &rl_r8, &rr_r8, &rr_r8, &rr_r8, &rr_r8, &rr_r8, &rr_r8, &rr_r8, &rr_r8, &sla_r8, &sla_r8, &sla_r8, &sla_r8, &sla_r8, &sla_r8, &sla_r8, &sla_r8, &sra_r8, &sra_r8, &sra_r8, &sra_r8, &sra_r8, &sra_r8, &sra_r8, &sra_r8, &swap_r8, &swap_r8, &swap_r8, &swap_r8, &swap_r8, &swap_r8, &swap_r8, &swap_r8, &srl_r8, &srl_r8, &srl_r8, &srl_r8, &srl_r8, &srl_r8, &srl_r8, &srl_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &bit_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &res_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8, &set_b3_r8};

