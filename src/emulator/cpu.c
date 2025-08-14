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

//Fetches the next opcode and increments pc
uint8_t fetchOp() {
    return memory_get(reg.PC++);
    
}



//Instructions are split into blocks for readability
uint8_t block_0(uint8_t op);
uint8_t block_1(uint8_t op);
uint8_t block_2(uint8_t op);
uint8_t block_3(uint8_t op);



//Returns machine cycles instruction takes
uint8_t cpu_update() {
    uint8_t m_cycles = 0;
    uint8_t op = fetchOp();
    


    switch(op & 0b11000000) {
        case 0x00:
            m_cycles = block_0(op);
        break;

        case 0b01000000:
            m_cycles = block_1(op);
        break;

        case 0b10000000:
            m_cycles = block_2(op);
        break;

        case 0b11000000:
            m_cycles = block_3(op);
        break;

        default:
            platform_log("It appears the rules of mathematics have somehow been broken");

    }
    


    return m_cycles;
}




uint8_t block_0(uint8_t op) {
    uint8_t m_cycles = 0;
    switch(op) {
        case 0x00: // NOP
        break;

        case 0b00010000: // STOP
            //technically a two byte operation
        break;

        case 0b00001000: // ld [imm16], sp
        break;

        case 0b00000111: // rlca
        break;

        case 0b00001111: // rrca
        break;

        case 0b00010111: //rla
        break;

        case 0b00011111: //rra
        break;

        case 0b00100111: //daa
        break;

        case 0b00101111: //cpl
        break;

        case 0b00110111: //scf
        break;

        case 0b00111111: //ccf
        break;

        case 0b00011000: //jr imm8
        break;

        default:
            switch(op & 0b11000111) {
                case 0b00000100: // 00(Operand r8)100 - inc r8
                break;

                case 0b00000101: // 00(Operand r8)101 - dec r8
                break;

                case 0b00000110: // 00(Dest r8)110 - ld r8, imm8
                break;

                default:
                    if(op & 0b11100111 == 0b00100000) { // 001(Conditional cond)000 - jr cond, imm8
                        break;
                    }
                    switch(op & 0b11001111) {
                        case 0x00: // NOP
                        break;

                        case 0b00000001: // 00(Dest r16)0001 - ld r16, imm16
                        break;

                        case 0b00000010: // 00(Dest r16mem)0010 - ld [r16mem], a
                        break;

                        case 0b00001010: // 00(Source r16mem)1010 - ld a, [r16mem]
                        break;

                        case 0b00000011: // 00(Operand r16)0011 - inc r16
                        break;

                        case 0b00001011: // 00(Operand r16)1011 - dec r16
                        break;

                        case 0b00001001: // 00(Operand r16)1001 - add hl, r16
                        break;


                        default:
                            platform_log("Unknown instruction Block 0");
                    }
            }

            
    }

    return m_cycles;
}



uint8_t block_1(uint8_t op);
uint8_t block_2(uint8_t op);
uint8_t block_3(uint8_t op);
