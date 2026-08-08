# Zonary Interpreter - v2.0 - TasKin

import sys
import re
import time
from typing import Dict, List, Tuple, Optional


class ZonaryInterpreter:
    def __init__(self, code_str: str):
        self.raw_code = code_str
        self.bit_width = 8
        self.max_value = 255
        self.char_zero = '0'
        self.char_one = '1'
        self.registers: Dict[int, int] = {}
        self.instructions: List[Tuple] = []
        self.labels: Dict[int, int] = {}
        self.pc = 0
        self.running = True
        self.had_error = False
        self.reserved_chars = {'<', '>', '{', '}', '0', '1'}

    def _parse_preprocessor(self, directive: str) -> bool:
        """Parse a preprocessor directive. Returns True if successful."""
        content = directive.strip()

        if len(content) < 2:
            print(f'Warning: invalid preprocessor directive: {directive}')
            return False

        op = content[:2]
        rest = content[2:].strip()

        if op == '00':  # Custom binary characters: 18 bits total
            parts = rest.split()
            if len(parts) != 2:
                print(f'Warning: {op} directive requires 2 parameters, got {len(parts)}')
                return False

            try:
                if len(parts[0]) != 8 or len(parts[1]) != 8:
                    print(f'Warning: {op} parameters must be 8-bit binary')
                    return False

                char0_code = int(parts[0], 2)
                char1_code = int(parts[1], 2)

                char0 = chr(char0_code)
                char1 = chr(char1_code)

                if char0 in self.reserved_chars or char1 in self.reserved_chars:
                    print(f'Warning: cannot use reserved characters < > {{ }} 0 1 as custom chars')
                    return False

                if char0 == char1:
                    print(f'Warning: custom characters must be different')
                    return False

                self.char_zero = char0
                self.char_one = char1

            except ValueError:
                print(f'Warning: invalid binary value in {op} directive')
                return False

        elif op == '01':  # Set bit-width: 10 bits total
            parts = rest.split()
            if len(parts) != 1:
                print(f'Warning: {op} directive requires 1 parameter, got {len(parts)}')
                return False

            try:
                if len(parts[0]) != 8:
                    print(f'Warning: {op} parameter must be 8-bit binary')
                    return False

                new_width = int(parts[0], 2)
                if new_width < 1:
                    print(f'Warning: bit-width must be at least 1, got {new_width}')
                    return False
                if new_width > 1024:
                    print(f'Warning: bit-width too large (max 1024), got {new_width}')
                    return False

                self.bit_width = new_width
                self.max_value = (1 << self.bit_width) - 1

            except ValueError:
                print(f'Warning: invalid binary value in {op} directive')
                return False

        else:
            print(f'Warning: unknown preprocessor opcode: {op}')
            return False

        return True

    def parse(self) -> bool:
        """Parse the source code. Returns True if successful."""
        code = self.raw_code

        # Extract and parse preprocessor directives
        dir_pattern = r'\{([^}]*)\}'
        directives = re.findall(dir_pattern, code, re.DOTALL)

        for d in directives:
            self._parse_preprocessor(d.strip())

        # Remove preprocessor blocks
        code = re.sub(r'\{[^}]*\}', '', code, flags=re.DOTALL)

        # Remove comments < ... >
        code = re.sub(r'<.*?>', '', code, flags=re.DOTALL)

        # Remove whitespace
        code = ''.join(code.split())

        # Convert custom chars to 0/1, ignore everything else
        binary_parts = []
        for c in code:
            if c == self.char_zero:
                binary_parts.append('0')
            elif c == self.char_one:
                binary_parts.append('1')
        binary = ''.join(binary_parts)

        if not binary:
            self.instructions = []
            return True

        # Parse binary into instructions
        n = len(binary)
        i = 0
        ins = []
        bw = self.bit_width

        while i < n:
            if i + 3 > n:
                if i < n:
                    print(f'Warning: {n - i} leftover bits ignored')
                break

            op = binary[i:i+3]
            i += 3

            if op == '000':  # SET: 3 + (bw + bw + 1) = 4 + bw*2
                need = bw + bw + 1  # register + parameter + mode
                if i + need > n:
                    print(f'Warning: incomplete SET instruction at bit {i-3}, ignored')
                    break
                reg = int(binary[i:i+bw], 2)
                param = int(binary[i+bw:i+bw*2], 2)
                mode = int(binary[i+bw*2])
                if reg >= (1 << bw) or param >= (1 << bw):
                    print(f'Error: register or parameter out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('SET', reg, param, mode))
                i += need

            elif op == '001':  # ADD: 3 + (bw + bw + 1) = 4 + bw*2
                need = bw + bw + 1
                if i + need > n:
                    print(f'Warning: incomplete ADD instruction at bit {i-3}, ignored')
                    break
                reg = int(binary[i:i+bw], 2)
                param = int(binary[i+bw:i+bw*2], 2)
                mode = int(binary[i+bw*2])
                if reg >= (1 << bw) or param >= (1 << bw):
                    print(f'Error: register or parameter out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('ADD', reg, param, mode))
                i += need

            elif op == '010':  # SUB: 3 + (bw + bw + 1) = 4 + bw*2
                need = bw + bw + 1
                if i + need > n:
                    print(f'Warning: incomplete SUB instruction at bit {i-3}, ignored')
                    break
                reg = int(binary[i:i+bw], 2)
                param = int(binary[i+bw:i+bw*2], 2)
                mode = int(binary[i+bw*2])
                if reg >= (1 << bw) or param >= (1 << bw):
                    print(f'Error: register or parameter out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('SUB', reg, param, mode))
                i += need

            elif op == '011':  # JMP: 3 + (bw + 1 + 1) = 5 + bw
                need = bw + 1 + 1
                if i + need > n:
                    print(f'Warning: incomplete JMP instruction at bit {i-3}, ignored')
                    break
                param = int(binary[i:i+bw], 2)
                mode1 = int(binary[i+bw])
                mode2 = int(binary[i+bw+1])
                if param >= (1 << bw):
                    print(f'Error: parameter out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('JMP', param, mode1, mode2))
                i += need

            elif op == '100':  # IFZ: 3 + (bw + bw + 1) = 4 + bw*2
                need = bw + bw + 1
                if i + need > n:
                    print(f'Warning: incomplete IFZ instruction at bit {i-3}, ignored')
                    break
                reg = int(binary[i:i+bw], 2)
                param = int(binary[i+bw:i+bw*2], 2)
                mode = int(binary[i+bw*2])
                if reg >= (1 << bw) or param >= (1 << bw):
                    print(f'Error: register or parameter out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('IFZ', reg, param, mode))
                i += need

            elif op == '101':  # OUT: 3 + (bw + 2 + 1) = 6 + bw
                need = bw + 2 + 1
                if i + need > n:
                    print(f'Warning: incomplete OUT instruction at bit {i-3}, ignored')
                    break
                param = int(binary[i:i+bw], 2)
                mode1 = int(binary[i+bw:i+bw+2], 2)
                mode2 = int(binary[i+bw+2])
                if param >= (1 << bw):
                    print(f'Error: parameter out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('OUT', param, mode1, mode2))
                i += need

            elif op == '110':  # INP: 3 + (bw + 2) = 5 + bw
                need = bw + 2
                if i + need > n:
                    print(f'Warning: incomplete INP instruction at bit {i-3}, ignored')
                    break
                reg = int(binary[i:i+bw], 2)
                mode = int(binary[i+bw:i+bw+2], 2)
                if reg >= (1 << bw):
                    print(f'Error: register out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('INP', reg, mode))
                i += need

            elif op == '111':  # SYS: 3 + (bw + 1) = 4 + bw
                need = bw + 1
                if i + need > n:
                    print(f'Warning: incomplete SYS instruction at bit {i-3}, ignored')
                    break
                param = int(binary[i:i+bw], 2)
                mode = int(binary[i+bw])
                if param >= (1 << bw):
                    print(f'Error: parameter out of range for bit-width {bw}')
                    self.had_error = True
                    return False
                ins.append(('SYS', param, mode))
                i += need

            else:
                print(f'Error: unknown opcode {op} at bit {i-3}')
                self.had_error = True
                return False

        self.instructions = ins
        return True

    def _get_reg(self, reg_num: int) -> int:
        """Get register value, returns 0 if not in memory."""
        return self.registers.get(reg_num, 0)

    def _set_reg(self, reg_num: int, value: int) -> None:
        """Set register value, clears from memory if value is 0."""
        value &= self.max_value
        if value == 0:
            self.registers.pop(reg_num, None)
        else:
            self.registers[reg_num] = value

    def _resolve_labels(self) -> bool:
        """Resolve all label definitions. Returns True if successful."""
        self.labels.clear()
        for idx, instr in enumerate(self.instructions):
            if instr[0] == 'JMP':
                _, param, mode1, mode2 = instr
                if mode1 == 1:  # Define label
                    if mode2 == 0:
                        label_num = param
                    else:
                        label_num = self._get_reg(param)

                    if label_num in self.labels:
                        print(f'Error: label {label_num} already defined at instruction {self.labels[label_num]}')
                        self.had_error = True
                        return False
                    self.labels[label_num] = idx + 1
        return True

    def _get_label_target(self, label_num: int) -> Optional[int]:
        """Get instruction index for a label, or None if not found."""
        if label_num in self.labels:
            return self.labels[label_num]
        print(f'Warning: label {label_num} not found')
        return None

    def _format_value(self, val: int, mode: int) -> str:
        """Format value for output based on mode and bit_width."""
        val &= self.max_value

        if mode == 0:  # Binary
            return f'{val:0{self.bit_width}b}'
        elif mode == 1:  # Decimal
            return str(val)
        elif mode == 2:  # Hexadecimal
            hex_digits = max(1, (self.bit_width + 3) // 4)
            return f'{val:0{hex_digits}X}'
        else:  # ASCII
            if 32 <= val <= 126:
                return chr(val)
            else:
                return f'\\x{val:02X}'

    def _parse_input(self, s: str, mode: int) -> int:
        """Parse input string based on mode. Raises ValueError on error."""
        s = s.strip()
        if not s:
            raise ValueError('Empty input')

        if mode == 0:  # Binary
            if len(s) != self.bit_width:
                raise ValueError(f'Expected {self.bit_width} bits, got {len(s)}')
            val = int(s, 2)
        elif mode == 1:  # Decimal
            val = int(s)
        elif mode == 2:  # Hexadecimal
            expected_digits = max(1, (self.bit_width + 3) // 4)
            s_clean = s.lstrip('0')
            if len(s_clean) > expected_digits:
                raise ValueError(f'Value too large for {self.bit_width}-bit')
            val = int(s, 16)
        else:  # ASCII character
            val = ord(s[0])

        if val < 0 or val > self.max_value:
            raise ValueError(f'Value {val} out of range for {self.bit_width}-bit')
        return val

    def execute(self) -> int:
        """Execute the program. Returns return code (0-255)."""
        if self.had_error:
            return 0

        if not self.instructions:
            return 0

        if not self._resolve_labels():
            return 0

        self.pc = 0
        self.running = True
        return_code = 0

        while self.running:
            if self.pc >= len(self.instructions):
                self.running = False
                print(f'\n--- Return code: {0:0{self.bit_width}b}')
                break

            instr = self.instructions[self.pc]
            op = instr[0]

            if op == 'SET':
                _, reg, param, mode = instr
                if mode == 0:
                    self._set_reg(reg, param)
                else:
                    self._set_reg(reg, self._get_reg(param))

            elif op == 'ADD':
                _, reg, param, mode = instr
                if mode == 0:
                    self._set_reg(reg, self._get_reg(reg) + param)
                else:
                    self._set_reg(reg, self._get_reg(reg) + self._get_reg(param))

            elif op == 'SUB':
                _, reg, param, mode = instr
                if mode == 0:
                    self._set_reg(reg, self._get_reg(reg) - param)
                else:
                    self._set_reg(reg, self._get_reg(reg) - self._get_reg(param))

            elif op == 'JMP':
                _, param, mode1, mode2 = instr
                if mode1 == 0:  # Jump to label
                    label_num = param if mode2 == 0 else self._get_reg(param)
                    target = self._get_label_target(label_num)
                    if target is not None:
                        self.pc = target
                        continue

            elif op == 'IFZ':
                _, reg, param, mode = instr
                if self._get_reg(reg) == 0:
                    label_num = param if mode == 0 else self._get_reg(param)
                    target = self._get_label_target(label_num)
                    if target is not None:
                        self.pc = target
                        continue

            elif op == 'OUT':
                _, param, mode1, mode2 = instr
                val = param if mode2 == 0 else self._get_reg(param)
                print(self._format_value(val, mode1), end='')
                sys.stdout.flush()

            elif op == 'INP':
                _, reg, mode = instr
                try:
                    s = input()
                    val = self._parse_input(s, mode)
                    self._set_reg(reg, val)
                except (ValueError, IndexError, EOFError):
                    self._set_reg(reg, 0)

            elif op == 'SYS':
                _, param, mode = instr
                if mode == 0:  # Exit
                    self.running = False
                    return_code = param
                    print(f'\n--- Return code: {param:0{self.bit_width}b}')
                else:  # Delay
                    time.sleep(param / 1000.0)

            self.pc += 1

        return return_code


def main() -> None:
    if len(sys.argv) < 2:
        print('Zonary - v2.0 - TasKin')
        print('Github: github.com/TasKin-tk/Zonary')
        sys.exit(1)

    try:
        with open(sys.argv[1], 'r', encoding='utf-8') as f:
            code = f.read()
    except FileNotFoundError:
        print(f'Error: file not found: {sys.argv[1]}')
        sys.exit(1)
    except Exception as e:
        print(f'Error reading file: {e}')
        sys.exit(1)

    print('---')

    interpreter = ZonaryInterpreter(code)
    if not interpreter.parse():
        sys.exit(1)
    interpreter.execute()


if __name__ == '__main__':
    main()

# TasKin Made
