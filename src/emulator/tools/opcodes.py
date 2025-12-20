#This script is meant to generate the lookup table for function pointers to the specified opcode

class Opcode:
    code = ""
    function = ""
    def __init__(self, code, function):
        self.code = code
        self.function = function

ocList = []

#Block 0
ocList.append(Opcode("00000000", "nop"))
ocList.append(Opcode("00XX0001", "ld_r16_imm16"))
ocList.append(Opcode("00XX0010", "ld_r16mem_a"))
ocList.append(Opcode("00001010", "ld_a_r16mem"))
ocList.append(Opcode("00001000", "ld_imm16_sp"))

ocList.append(Opcode("00XX0011", "inc_r16"))
ocList.append(Opcode("00XX1011", "dec_r16"))
ocList.append(Opcode("00XX1001", "add_hl_r16"))

ocList.append(Opcode("00XXX100", "inc_r8"))
ocList.append(Opcode("00XXX101", "dec_r8"))

ocList.append(Opcode("00XXX110", "ld_r8_imm8"))

ocList.append(Opcode("00000111", "rlca"))
ocList.append(Opcode("00001111", "rrca"))
ocList.append(Opcode("00010111", "rla"))
ocList.append(Opcode("00011111", "rra"))
ocList.append(Opcode("00100111", "daa"))
ocList.append(Opcode("00101111", "cpl"))
ocList.append(Opcode("00110111", "scf"))
ocList.append(Opcode("00000111", "ccf"))

ocList.append(Opcode("00011000", "jr_imm8"))
ocList.append(Opcode("001XX000", "jr_cond_imm8"))
ocList.append(Opcode("00010000", "stop"))


#Block 1
ocList.append(Opcode("01XXXXXX", "ld_r8_r8"))

ocList.append(Opcode("01110110", "halt"))

#Block 2
ocList.append(Opcode("10000XXX", "add_a_r8"))
ocList.append(Opcode("10001XXX", "adc_a_r8"))
ocList.append(Opcode("10010XXX", "sub_a_r8"))
ocList.append(Opcode("10011XXX", "sbc_a_r8"))
ocList.append(Opcode("10100XXX", "and_a_r8"))
ocList.append(Opcode("10101XXX", "xor_a_r8"))
ocList.append(Opcode("10110XXX", "or_a_r8"))
ocList.append(Opcode("10111XXX", "cp_a_r8"))

#Block 3
ocList.append(Opcode("11000110", "add_a_imm8"))
ocList.append(Opcode("11001110", "adc_a_imm8"))
ocList.append(Opcode("11010110", "sub_a_imm8"))
ocList.append(Opcode("11011110", "sbc_a_imm8"))
ocList.append(Opcode("11100110", "and_a_imm8"))
ocList.append(Opcode("11101110", "xor_a_imm8"))
ocList.append(Opcode("11110110", "or_a_imm8"))
ocList.append(Opcode("11111110", "cp_a_imm8"))

ocList.append(Opcode("110XX000", "ret_cond"))
ocList.append(Opcode("11001001", "ret"))
ocList.append(Opcode("11011001", "reti"))
ocList.append(Opcode("110XX010", "jp_cond_imm16"))
ocList.append(Opcode("11000011", "jp_imm16"))
ocList.append(Opcode("11101001", "jp_hl"))
ocList.append(Opcode("110XX100", "call_cond_imm16"))
ocList.append(Opcode("11001101", "call_imm16"))
ocList.append(Opcode("11XXX111", "rst_tgt3"))

ocList.append(Opcode("11XX0001", "pop_r16stk"))
ocList.append(Opcode("11XX0101", "push_r16stk"))

#0xCB Prefix
ocList.append(Opcode("11001011", "prefix"))

ocList.append(Opcode("11100010", "ldh_c_a"))
ocList.append(Opcode("11100000", "ldh_imm8_a"))
ocList.append(Opcode("11101010", "ld_imm16_a"))
ocList.append(Opcode("11110010", "ldh_a_c"))
ocList.append(Opcode("11110000", "ldh_a_imm8"))
ocList.append(Opcode("11111010", "ld_a_imm16"))

ocList.append(Opcode("11101000", "add_sp_imm8"))
ocList.append(Opcode("11111000", "ld_hl_sp_imm8"))
ocList.append(Opcode("11111001", "ld_sp_hl"))

ocList.append(Opcode("11110011", "di"))
ocList.append(Opcode("11111011", "ei"))



default_value = "Default" #0xD3 is one of the values that will just hard lock cpu

sortedList = [default_value] * 256



for oc in ocList:
    num_X = oc.code.count('X')
    if(num_X != 0):
        combinations = 2**num_X
        for i in range(combinations):
            binary = '{:0>9b}'.format(i)
            code = oc.code
            for j in range(num_X):
                code = code.replace('X', binary[len(binary) - 1 - j], 1)
            sortedList[int(code, 2)] = oc.function
    else:
        sortedList[int(oc.code, 2)] = oc.function

print(sortedList[0])

arrayString = "{"

for i in range(len(sortedList)):
    
    arrayString += "&" + sortedList[i]
    if(i < len(sortedList) - 1):
        arrayString += ', '

arrayString += "}"

print(arrayString)

print("\n")

#remove duplicate strings
dupRemoved = list(set(sortedList))

dupRemoved.sort()

for i in range(len(dupRemoved)):
    print("/*\n\n*/\nvoid " + dupRemoved[i] + "() {\n    m_cycles = 1;\n}\n")

print(len(dupRemoved))
 



#CB Prefix instructions
cbList = []

cbList.append(Opcode("00000XXX", "rlc_r8"))
cbList.append(Opcode("00001XXX", "rrc_r8"))
cbList.append(Opcode("00010XXX", "rl_r8"))
cbList.append(Opcode("00011XXX", "rr_r8"))
cbList.append(Opcode("00100XXX", "sla_r8"))
cbList.append(Opcode("00101XXX", "sra_r8"))
cbList.append(Opcode("00110XXX", "swap_r8"))
cbList.append(Opcode("00111XXX", "srl_r8"))

cbList.append(Opcode("01XXXXXX", "bit_b3_r8"))
cbList.append(Opcode("10XXXXXX", "res_b3_r8"))
cbList.append(Opcode("11XXXXXX", "set_b3_r8"))

cbSortedList = [default_value] * 256

print("\n\nCB Prefix instructions \n")


for oc in cbList:
    num_X = oc.code.count('X')
    if(num_X != 0):
        combinations = 2**num_X
        for i in range(combinations):
            binary = '{:0>9b}'.format(i)
            code = oc.code
            for j in range(num_X):
                code = code.replace('X', binary[len(binary) - 1 - j], 1)
            cbSortedList[int(code, 2)] = oc.function
    else:
        cbSortedList[int(oc.code, 2)] = oc.function

print(cbSortedList[0])

arrayString = "{"

for i in range(len(cbSortedList)):
    
    arrayString += "&" + cbSortedList[i]
    if(i < len(cbSortedList) - 1):
        arrayString += ', '

arrayString += "}"

print(arrayString)

print("\n")

#remove duplicate strings
dupRemoved = list(set(cbSortedList))

dupRemoved.sort()

for i in range(len(dupRemoved)):
    print("/*\n\n*/\nvoid " + dupRemoved[i] + "() {\n    m_cycles = 1;\n}\n")

print(len(dupRemoved))