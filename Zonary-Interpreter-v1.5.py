# TasKin Made
# Zonary Interpreter - v1.5

import sys
import re
import time


class ZonaryInterpreter:
    def __init__(self, code_str):
        self.raw_code = code_str
        self.registers = [0] * 256
        self.instructions = []
        self.labels = {}  # label_number -> instruction_index
        self.pc = 0
        self.running = True
        self.return_code = 0
        
    def preprocess(self):
        """Remove comments and whitespace, keep only 0s and 1s"""
        code = self.raw_code
        
        # Remove comments: /. ... ./
        code = re.sub(r'/\.(.*?)\./', '', code, flags=re.DOTALL)
        
        # Remove all whitespace
        code = ''.join(code.split())
        
        # Keep only 0 and 1
        code = ''.join(c for c in code if c in '01')
        
        return code
    
    def parse(self):
        """Parse binary code into instruction list"""
        code = self.preprocess()
        n = len(code)
        i = 0
        ins = []
        
        while i < n:
            if i + 3 > n:
                print(f'Warning: incomplete opcode at bit {i}, ignoring remaining bits')
                break
            
            op = code[i:i+3]
            i += 3
            
            if op == '000':  # SET: 3 + 8 + 8 + 1 = 20
                if i + 17 > n:
                    print(f'Warning: incomplete SET instruction at bit {i-3}')
                    break
                reg = int(code[i:i+8], 2)
                param = int(code[i+8:i+16], 2)
                mode = int(code[i+16])
                ins.append(('SET', reg, param, mode))
                i += 17
                
            elif op == '001':  # ADD: 3 + 8 + 8 + 1 = 20
                if i + 17 > n:
                    print(f'Warning: incomplete ADD instruction at bit {i-3}')
                    break
                reg_a = int(code[i:i+8], 2)
                param = int(code[i+8:i+16], 2)
                mode = int(code[i+16])
                ins.append(('ADD', reg_a, param, mode))
                i += 17
                
            elif op == '010':  # SUB: 3 + 8 + 8 + 1 = 20
                if i + 17 > n:
                    print(f'Warning: incomplete SUB instruction at bit {i-3}')
                    break
                reg_a = int(code[i:i+8], 2)
                param = int(code[i+8:i+16], 2)
                mode = int(code[i+16])
                ins.append(('SUB', reg_a, param, mode))
                i += 17
                
            elif op == '011':  # JMP: 3 + 8 + 1 + 1 = 13
                if i + 9 > n:
                    print(f'Warning: incomplete JMP instruction at bit {i-3}')
                    break
                param = int(code[i:i+8], 2)
                mode1 = int(code[i+8])
                mode2 = int(code[i+9])
                ins.append(('JMP', param, mode1, mode2))
                i += 10
                
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
        
        self.instructions = ins
        return ins
    
    def resolve_labels(self):
        """First pass: collect all label definitions (JMP mode1=1)"""
        for idx, instr in enumerate(self.instructions):
            if instr[0] == 'JMP':
                _, param, mode1, mode2 = instr
                if mode1 == 1:  # Define label
                    label_num = param if mode2 == 0 else self.registers[param]
                    if label_num in self.labels:
                        print(f'Error: label {label_num} already defined at instruction {self.labels[label_num]}')
                        sys.exit(1)
                    # Label points to the NEXT instruction
                    self.labels[label_num] = idx + 1
    
    def get_label_target(self, label_num):
        """Get instruction index for a label, or None if not found"""
        if label_num in self.labels:
            return self.labels[label_num]
        else:
            print(f'Warning: label {label_num} not found')
            return None
    
    def get_param_value(self, param, mode):
        """Get parameter value: either immediate or from register"""
        if mode == 0:
            return param
        else:
            return self.registers[param]
    
    def execute(self):
        """Execute the parsed instructions"""
        if not self.instructions:
            print('No instructions to execute')
            return
        
        # First pass: collect label definitions
        self.resolve_labels()
        
        self.pc = 0
        self.running = True
        
        while self.running:
            if self.pc >= len(self.instructions):
                # Program ended normally
                self.running = False
                print(f'\n--- Return code: 00000000')
                break
            
            instr = self.instructions[self.pc]
            op = instr[0]
            
            if op == 'SET':
                _, reg, param, mode = instr
                if mode == 0:  # Immediate value
                    self.registers[reg] = param
                else:  # Copy from register
                    self.registers[reg] = self.registers[param]
                    
            elif op == 'ADD':
                _, reg_a, param, mode = instr
                if mode == 0:  # Add immediate
                    self.registers[reg_a] = (self.registers[reg_a] + param) & 0xFF
                else:  # Add register
                    self.registers[reg_a] = (self.registers[reg_a] + self.registers[param]) & 0xFF
                    
            elif op == 'SUB':
                _, reg_a, param, mode = instr
                if mode == 0:  # Subtract immediate
                    self.registers[reg_a] = (self.registers[reg_a] - param) & 0xFF
                else:  # Subtract register
                    self.registers[reg_a] = (self.registers[reg_a] - self.registers[param]) & 0xFF
                    
            elif op == 'JMP':
                _, param, mode1, mode2 = instr
                
                if mode1 == 1:  # Define label
                    # Already handled in resolve_labels(), skip during execution
                    pass
                else:  # Jump to label
                    label_num = param if mode2 == 0 else self.registers[param]
                    target = self.get_label_target(label_num)
                    if target is not None:
                        self.pc = target
                        continue
                    # If label not found, fall through to next instruction
                    
            elif op == 'IFZ':
                _, reg, param, mode = instr
                
                if self.registers[reg] == 0:
                    label_num = param if mode == 0 else self.registers[param]
                    target = self.get_label_target(label_num)
                    if target is not None:
                        self.pc = target
                        continue
                # If not zero or label not found, fall through
                
            elif op == 'OUT':
                _, param, mode1, mode2 = instr
                val = param if mode2 == 0 else self.registers[param]
                val &= 0xFF
                
                if mode1 == 0:  # Binary
                    print(f'{val:08b}', end='')
                elif mode1 == 1:  # Decimal
                    print(val, end='')
                elif mode1 == 2:  # Hexadecimal
                    print(f'{val:02X}', end='')
                else:  # ASCII
                    if 32 <= val <= 126:
                        print(chr(val), end='')
                    else:
                        print(f'\\x{val:02X}', end='')
                sys.stdout.flush()
                
            elif op == 'INP':
                _, reg, mode = instr
                try:
                    s = input().strip()
                    if not s:
                        raise ValueError('Empty input')
                    
                    if mode == 0:  # Binary
                        val = int(s, 2)
                    elif mode == 1:  # Decimal
                        val = int(s)
                    elif mode == 2:  # Hexadecimal
                        val = int(s, 16)
                    else:  # ASCII character
                        val = ord(s[0])
                    
                    self.registers[reg] = val & 0xFF
                except (ValueError, IndexError):
                    self.registers[reg] = 0
                    
            elif op == 'SYS':
                _, param, mode = instr
                if mode == 0:  # Exit
                    self.running = False
                    self.return_code = param
                    print(f'\n--- Return code: {param:08b}')
                else:  # Delay
                    time.sleep(param / 1000.0)
            
            self.pc += 1


def main():
    if len(sys.argv) < 2:
        print('Usage: python zonary.py <file.zonary>')
        sys.exit(1)
    
    try:
        with open(sys.argv[1], 'r', encoding='utf-8') as f:
            code = f.read()
    except Exception as e:
        print(f'Error reading file: {e}')
        sys.exit(1)
    
    print('---')
    
    interpreter = ZonaryInterpreter(code)
    interpreter.parse()
    interpreter.execute()


if __name__ == '__main__':
    main()

# TasKin Made
