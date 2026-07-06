# TasKin Made
# Zonary Interpreter - v1.3.1

import sys, re, time

def print_banner():  #Thank for using!!!
    print("  ███████╗ ██████╗ ███╗   ██╗ █████╗ ██████╗ ██╗   ██╗")
    print("  ╚══███╔╝██╔═══██╗████╗  ██║██╔══██╗██╔══██╗╚██╗ ██╔╝")
    print("    ███╔╝ ██║   ██║██╔██╗ ██║███████║██████╔╝ ╚████╔╝ ")
    print("   ███╔╝  ██║   ██║██║╚██╗██║██╔══██║██╔══██╗  ╚██╔╝  ")
    print("  ███████╗╚██████╔╝██║ ╚████║██║  ██║██║  ██║   ██║   ")
    print("  ╚══════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ")
    print("  Zonary v1.3.1 - TasKin Made")

print('---')

# Read file
try:
    with open(sys.argv[1], 'r', encoding='utf-8') as f:
        code = f.read()
except:
    print('File not found or invalid argument')
    sys.exit(1)

# 1. Remove comments: /. ... ./
code = re.sub(r'/\.(.*?)\./', '', code, flags=re.DOTALL)

# 2. Remove whitespace and newlines
code = ''.join(code.split())

# 3. Keep only 0 and 1
code = ''.join(c for c in code if c in '01')

# 4. Parse instructions
ins = []
i = 0
n = len(code)

while i < n:
    if i + 3 > n:
        print(f'Warning: incomplete opcode at bit {i}, ignoring remaining bits')
        break
    op = code[i:i+3]
    i += 3

    if op == '000':  # SET: 3 + 8 + 8 = 19
        if i + 16 > n:
            print(f'Warning: incomplete SET instruction at bit {i-3}')
            break
        reg = int(code[i:i+8], 2)
        val = int(code[i+8:i+16], 2)
        ins.append(('SET', reg, val))
        i += 16

    elif op == '001':  # ADD: 3 + 8 + 8 = 19
        if i + 16 > n:
            print(f'Warning: incomplete ADD instruction at bit {i-3}')
            break
        reg_a = int(code[i:i+8], 2)
        reg_b = int(code[i+8:i+16], 2)
        ins.append(('ADD', reg_a, reg_b))
        i += 16

    elif op == '010':  # SUB: 3 + 8 + 8 = 19
        if i + 16 > n:
            print(f'Warning: incomplete SUB instruction at bit {i-3}')
            break
        reg_a = int(code[i:i+8], 2)
        reg_b = int(code[i+8:i+16], 2)
        ins.append(('SUB', reg_a, reg_b))
        i += 16

    elif op == '011':  # JMP: 3 + 8 + 1 = 12
        if i + 9 > n:
            print(f'Warning: incomplete JMP instruction at bit {i-3}')
            break
        param = int(code[i:i+8], 2)
        mode = int(code[i+8])
        ins.append(('JMP', param, mode))
        i += 9

    elif op == '100':  # IFZ: 3 + 8 + 8 + 1 = 20
        if i + 17 > n:
            print(f'Warning: incomplete IFZ instruction at bit {i-3}')
            break
        reg = int(code[i:i+8], 2)
        param = int(code[i+8:i+16], 2)
        mode = int(code[i+16])
        ins.append(('IFZ', reg, param, mode))
        i += 17

    elif op == '101':  # OUT: 3 + 8 + 2 + 1 = 14
        if i + 11 > n:
            print(f'Warning: incomplete OUT instruction at bit {i-3}')
            break
        param = int(code[i:i+8], 2)
        mode1 = int(code[i+8:i+10], 2)
        mode2 = int(code[i+10])
        ins.append(('OUT', param, mode1, mode2))
        i += 11

    elif op == '110':  # INP: 3 + 8 + 2 = 13
        if i + 10 > n:
            print(f'Warning: incomplete INP instruction at bit {i-3}')
            break
        reg = int(code[i:i+8], 2)
        mode = int(code[i+8:i+10], 2)
        ins.append(('INP', reg, mode))
        i += 10

    elif op == '111':  # SYS: 3 + 8 + 1 = 12
        if i + 9 > n:
            print(f'Warning: incomplete SYS instruction at bit {i-3}')
            break
        param = int(code[i:i+8], 2)
        mode = int(code[i+8])
        ins.append(('SYS', param, mode))
        i += 9

    else:
        print(f'Unknown opcode: {op} at bit {i-3}')
        break

if not ins:
    print('No instructions to execute')
    sys.exit(0)

# 5. Execute
reg = [0] * 256
pc = 0
running = True

def first_set(r):
    """Find first assignment to register r in the entire program"""
    for idx, instr in enumerate(ins):
        op = instr[0]
        if op in ('SET', 'ADD', 'SUB') and instr[1] == r:
            return idx
    return None

while running:
    # If no more instructions, program ends with default return code 00000000
    if pc >= len(ins):
        running = False
        print(f'\n--- Return code: 00000000')
        break

    instr = ins[pc]
    op = instr[0]

    if op == 'SET':
        reg[instr[1]] = instr[2]

    elif op == 'ADD':
        reg[instr[1]] = (reg[instr[1]] + reg[instr[2]]) & 0xFF

    elif op == 'SUB':
        reg[instr[1]] = (reg[instr[1]] - reg[instr[2]]) & 0xFF

    elif op == 'JMP':
        if instr[2] == 0:
            if 0 <= instr[1] < len(ins):
                pc = instr[1]
                continue
            else:
                print(f'Warning: JMP to invalid instruction {instr[1]}')
        else:
            target = first_set(instr[1])
            if target is not None:
                pc = target + 1
                continue
            else:
                print(f'Warning: JMP mode 1: no assignment found for register {instr[1]}')

    elif op == 'IFZ':
        if reg[instr[1]] == 0:
            if instr[3] == 0:
                if 0 <= instr[2] < len(ins):
                    pc = instr[2]
                    continue
                else:
                    print(f'Warning: IFZ to invalid instruction {instr[2]}')
            else:
                target = first_set(instr[2])
                if target is not None:
                    pc = target + 1
                    continue
                else:
                    print(f'Warning: IFZ mode 1: no assignment found for register {instr[2]}')

    elif op == 'OUT':
        val = instr[1] if instr[3] == 0 else reg[instr[1]]
        val &= 0xFF
        if instr[2] == 0:
            print(f'{val:08b}', end='')
        elif instr[2] == 1:
            print(val, end='')
        elif instr[2] == 2:
            print(f'{val:02X}', end='')
        else:
            print(chr(val) if 32 <= val <= 126 else f'\\x{val:02X}', end='')
        sys.stdout.flush()

    elif op == 'INP':
        try:
            s = input().strip()
            mode = instr[2]
            if not s:
                raise ValueError('Empty input')
            if mode == 0:
                val = int(s, 2)
            elif mode == 1:
                val = int(s)
            elif mode == 2:
                val = int(s, 16)
            else:
                val = ord(s[0])
            reg[instr[1]] = val & 0xFF
        except (ValueError, IndexError):
            reg[instr[1]] = 0

    elif op == 'SYS':
        if instr[2] == 0:
            running = False
            print(f'\n--- Return code: {instr[1]:08b}')
        else:
            time.sleep(instr[1] / 1000.0)

    pc += 1

# TasKin Made
