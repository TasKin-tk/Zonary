// Zonary Interpreter - v2.1 - TasKin

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


/* Type Definitions */

typedef struct {
    int bit_width;
    uint64_t max_value;
    char char_zero;
    char char_one;
    uint64_t* registers;
    int* reg_keys;
    int reg_count;
    int reg_capacity;

    char** instr_op;
    uint64_t* instr_p1;
    uint64_t* instr_p2;
    uint64_t* instr_p3;
    int instr_count;
    int instr_capacity;

    uint64_t* label_keys;
    int* label_values;
    int label_count;
    int label_capacity;

    int pc;
    int running;
    int had_error;
} ZonaryInterpreter;


/* Constants */

static const char* RESERVED_CHARS = "<>/01";


/* Helper Functions */

#ifdef _WIN32
static void sleep_ms(int ms) {
    Sleep(ms);
}
#else
static void sleep_ms(int ms) {
    usleep((useconds_t)(ms * 1000));
}
#endif

static int is_reserved_char(char c) {
    return strchr(RESERVED_CHARS, c) != NULL;
}

static int max_int(int a, int b) {
    return a > b ? a : b;
}

static int find_register(ZonaryInterpreter* interp, int key) {
    for (int i = 0; i < interp->reg_count; i++) {
        if (interp->reg_keys[i] == key) {
            return i;
        }
    }
    return -1;
}

static void ensure_reg_capacity(ZonaryInterpreter* interp) {
    if (interp->reg_count >= interp->reg_capacity) {
        interp->reg_capacity *= 2;
        interp->registers = (uint64_t*)realloc(interp->registers, interp->reg_capacity * sizeof(uint64_t));
        interp->reg_keys = (int*)realloc(interp->reg_keys, interp->reg_capacity * sizeof(int));
    }
}

static void ensure_instr_capacity(ZonaryInterpreter* interp) {
    if (interp->instr_count >= interp->instr_capacity) {
        interp->instr_capacity *= 2;
        interp->instr_op = (char**)realloc(interp->instr_op, interp->instr_capacity * sizeof(char*));
        interp->instr_p1 = (uint64_t*)realloc(interp->instr_p1, interp->instr_capacity * sizeof(uint64_t));
        interp->instr_p2 = (uint64_t*)realloc(interp->instr_p2, interp->instr_capacity * sizeof(uint64_t));
        interp->instr_p3 = (uint64_t*)realloc(interp->instr_p3, interp->instr_capacity * sizeof(uint64_t));
    }
}

static void ensure_label_capacity(ZonaryInterpreter* interp) {
    if (interp->label_count >= interp->label_capacity) {
        interp->label_capacity = interp->label_capacity == 0 ? 16 : interp->label_capacity * 2;
        interp->label_keys = (uint64_t*)realloc(interp->label_keys, interp->label_capacity * sizeof(uint64_t));
        interp->label_values = (int*)realloc(interp->label_values, interp->label_capacity * sizeof(int));
    }
}


/* Interpreter Lifecycle */

ZonaryInterpreter* interpreter_create(void) {
    ZonaryInterpreter* interp = (ZonaryInterpreter*)calloc(1, sizeof(ZonaryInterpreter));
    if (!interp) return NULL;

    interp->bit_width = 8;
    interp->max_value = 255;
    interp->char_zero = '0';
    interp->char_one = '1';
    interp->reg_capacity = 16;
    interp->registers = (uint64_t*)calloc(interp->reg_capacity, sizeof(uint64_t));
    interp->reg_keys = (int*)calloc(interp->reg_capacity, sizeof(int));
    interp->reg_count = 0;

    interp->instr_capacity = 64;
    interp->instr_op = (char**)calloc(interp->instr_capacity, sizeof(char*));
    interp->instr_p1 = (uint64_t*)calloc(interp->instr_capacity, sizeof(uint64_t));
    interp->instr_p2 = (uint64_t*)calloc(interp->instr_capacity, sizeof(uint64_t));
    interp->instr_p3 = (uint64_t*)calloc(interp->instr_capacity, sizeof(uint64_t));
    interp->instr_count = 0;

    interp->label_capacity = 16;
    interp->label_keys = (uint64_t*)calloc(interp->label_capacity, sizeof(uint64_t));
    interp->label_values = (int*)calloc(interp->label_capacity, sizeof(int));
    interp->label_count = 0;

    interp->pc = 0;
    interp->running = 1;
    interp->had_error = 0;

    return interp;
}

void interpreter_destroy(ZonaryInterpreter* interp) {
    if (!interp) return;

    free(interp->registers);
    free(interp->reg_keys);

    for (int i = 0; i < interp->instr_count; i++) {
        free(interp->instr_op[i]);
    }
    free(interp->instr_op);
    free(interp->instr_p1);
    free(interp->instr_p2);
    free(interp->instr_p3);

    free(interp->label_keys);
    free(interp->label_values);

    free(interp);
}


/* Register Operations */

static uint64_t get_reg(ZonaryInterpreter* interp, int reg_num) {
    int idx = find_register(interp, reg_num);
    if (idx < 0) return 0;
    return interp->registers[idx];
}

static void set_reg(ZonaryInterpreter* interp, int reg_num, uint64_t value) {
    value &= interp->max_value;
    int idx = find_register(interp, reg_num);

    if (value == 0) {
        if (idx >= 0) {
            for (int i = idx; i < interp->reg_count - 1; i++) {
                interp->reg_keys[i] = interp->reg_keys[i + 1];
                interp->registers[i] = interp->registers[i + 1];
            }
            interp->reg_count--;
        }
        return;
    }

    if (idx >= 0) {
        interp->registers[idx] = value;
    } else {
        ensure_reg_capacity(interp);
        interp->reg_keys[interp->reg_count] = reg_num;
        interp->registers[interp->reg_count] = value;
        interp->reg_count++;
    }
}


/* Label Operations */

static int find_label(ZonaryInterpreter* interp, uint64_t label_num) {
    for (int i = 0; i < interp->label_count; i++) {
        if (interp->label_keys[i] == label_num) {
            return interp->label_values[i];
        }
    }
    return -1;
}

static int add_label(ZonaryInterpreter* interp, uint64_t label_num, int value) {
    if (find_label(interp, label_num) >= 0) {
        return 0;
    }
    ensure_label_capacity(interp);
    interp->label_keys[interp->label_count] = label_num;
    interp->label_values[interp->label_count] = value;
    interp->label_count++;
    return 1;
}


/* Parsing Helpers */

static int parse_binary(const char* s, int len) {
    int result = 0;
    for (int i = 0; i < len; i++) {
        result = (result << 1) | (s[i] - '0');
    }
    return result;
}

static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* result = (char*)malloc(len);
    if (result) {
        memcpy(result, s, len);
    }
    return result;
}

static char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(f);
        return NULL;
    }

    size_t read_size = fread(content, 1, size, f);
    content[read_size] = '\0';
    fclose(f);
    return content;
}

static void trim_whitespace(char* s) {
    char* start = s;
    while (isspace((unsigned char)*start)) start++;
    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}


/* Preprocessor Parsing */

static int parse_preprocessor(ZonaryInterpreter* interp, const char* directive) {
    char* content = my_strdup(directive);
    if (!content) return 0;
    trim_whitespace(content);

    if (strlen(content) < 2) {
        printf("Warning: invalid preprocessor directive: %s\n", directive);
        free(content);
        return 0;
    }

    char op[3] = {content[0], content[1], '\0'};
    char* rest = content + 2;
    trim_whitespace(rest);

    if (strcmp(op, "00") == 0) {
        char* parts[2];
        int part_count = 0;
        char* token = strtok(rest, " \t");
        while (token && part_count < 2) {
            parts[part_count++] = token;
            token = strtok(NULL, " \t");
        }

        if (part_count != 2) {
            printf("Warning: %s directive requires 2 parameters, got %d\n", op, part_count);
            free(content);
            return 0;
        }

        if (strlen(parts[0]) != 8 || strlen(parts[1]) != 8) {
            printf("Warning: %s parameters must be 8-bit binary\n", op);
            free(content);
            return 0;
        }

        int char0_code = parse_binary(parts[0], 8);
        int char1_code = parse_binary(parts[1], 8);
        char char0 = (char)char0_code;
        char char1 = (char)char1_code;

        if (is_reserved_char(char0) || is_reserved_char(char1)) {
            printf("Warning: cannot use reserved characters < > / 0 1 as custom chars\n");
            free(content);
            return 0;
        }

        if (char0 == char1) {
            printf("Warning: custom characters must be different\n");
            free(content);
            return 0;
        }

        interp->char_zero = char0;
        interp->char_one = char1;

    } else if (strcmp(op, "01") == 0) {
        char* parts[1];
        int part_count = 0;
        char* token = strtok(rest, " \t");
        while (token && part_count < 1) {
            parts[part_count++] = token;
            token = strtok(NULL, " \t");
        }

        if (part_count != 1) {
            printf("Warning: %s directive requires 1 parameter, got %d\n", op, part_count);
            free(content);
            return 0;
        }

        if (strlen(parts[0]) != 8) {
            printf("Warning: %s parameter must be 8-bit binary\n", op);
            free(content);
            return 0;
        }

        int new_width = parse_binary(parts[0], 8);
        if (new_width < 1) {
            printf("Warning: bit-width must be at least 1, got %d\n", new_width);
            free(content);
            return 0;
        }
        if (new_width > 1024) {
            printf("Warning: bit-width too large (max 1024), got %d\n", new_width);
            free(content);
            return 0;
        }

        interp->bit_width = new_width;
        interp->max_value = (uint64_t)((1ULL << new_width) - 1);
        if (new_width >= 64) {
            interp->max_value = (uint64_t)-1;
        }

    } else {
        printf("Warning: unknown preprocessor opcode: %s\n", op);
        free(content);
        return 0;
    }

    free(content);
    return 1;
}


/* Main Parsing */

static int parse_code(ZonaryInterpreter* interp, const char* code) {
    char* work = my_strdup(code);
    if (!work) {
        interp->had_error = 1;
        return 0;
    }

    /* Extract and parse preprocessor directives */
    char* ptr = work;
    while (*ptr) {
        if (*ptr == '/') {
            char* start = ptr;
            ptr++;
            while (*ptr && *ptr != '/') {
                ptr++;
            }
            char* end = ptr;
            if (end > start + 1) {
                char* dir = (char*)malloc(end - start);
                if (dir) {
                    memcpy(dir, start + 1, end - start - 1);
                    dir[end - start - 1] = '\0';
                    parse_preprocessor(interp, dir);
                    free(dir);
                }
            }
        } else {
            ptr++;
        }
    }

    /* Remove preprocessor blocks */
    char* src = work;
    char* dst = work;
    int in_slash = 0;
    while (*src) {
        if (*src == '/') { in_slash = !in_slash; src++; continue; }
        if (!in_slash) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    /* Remove comments < ... > */
    src = work;
    dst = work;
    int in_comment = 0;
    while (*src) {
        if (*src == '<') { in_comment = 1; src++; continue; }
        if (*src == '>') { in_comment = 0; src++; continue; }
        if (!in_comment) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    /* Remove whitespace */
    src = work;
    dst = work;
    while (*src) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    /* Convert custom chars to 0/1 */
    src = work;
    dst = work;
    while (*src) {
        if (*src == interp->char_zero) {
            *dst++ = '0';
        } else if (*src == interp->char_one) {
            *dst++ = '1';
        }
        src++;
    }
    *dst = '\0';

    int n = strlen(work);
    if (n == 0) {
        free(work);
        return 1;
    }

    /* Parse binary into instructions */
    int i = 0;
    int bw = interp->bit_width;

    while (i < n) {
        if (i + 3 > n) {
            if (i < n) {
                printf("Warning: %d leftover bits ignored\n", n - i);
            }
            break;
        }

        char op[4] = {work[i], work[i+1], work[i+2], '\0'};
        i += 3;

        if (strcmp(op, "000") == 0) {
            int need = bw + bw + 1;
            if (i + need > n) {
                printf("Warning: incomplete SET instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char reg_str[1024], param_str[1024];
            memcpy(reg_str, work + i, bw);
            reg_str[bw] = '\0';
            memcpy(param_str, work + i + bw, bw);
            param_str[bw] = '\0';
            int reg = parse_binary(reg_str, bw);
            int param = parse_binary(param_str, bw);
            int mode = work[i + bw*2] - '0';

            if ((uint64_t)reg >= (1ULL << bw) || (uint64_t)param >= (1ULL << bw)) {
                printf("Error: register or parameter out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("SET");
            interp->instr_p1[interp->instr_count] = reg;
            interp->instr_p2[interp->instr_count] = param;
            interp->instr_p3[interp->instr_count] = mode;
            interp->instr_count++;
            i += need;

        } else if (strcmp(op, "001") == 0) {
            int need = bw + bw + 1;
            if (i + need > n) {
                printf("Warning: incomplete ADD instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char reg_str[1024], param_str[1024];
            memcpy(reg_str, work + i, bw);
            reg_str[bw] = '\0';
            memcpy(param_str, work + i + bw, bw);
            param_str[bw] = '\0';
            int reg = parse_binary(reg_str, bw);
            int param = parse_binary(param_str, bw);
            int mode = work[i + bw*2] - '0';

            if ((uint64_t)reg >= (1ULL << bw) || (uint64_t)param >= (1ULL << bw)) {
                printf("Error: register or parameter out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("ADD");
            interp->instr_p1[interp->instr_count] = reg;
            interp->instr_p2[interp->instr_count] = param;
            interp->instr_p3[interp->instr_count] = mode;
            interp->instr_count++;
            i += need;

        } else if (strcmp(op, "010") == 0) {
            int need = bw + bw + 1;
            if (i + need > n) {
                printf("Warning: incomplete SUB instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char reg_str[1024], param_str[1024];
            memcpy(reg_str, work + i, bw);
            reg_str[bw] = '\0';
            memcpy(param_str, work + i + bw, bw);
            param_str[bw] = '\0';
            int reg = parse_binary(reg_str, bw);
            int param = parse_binary(param_str, bw);
            int mode = work[i + bw*2] - '0';

            if ((uint64_t)reg >= (1ULL << bw) || (uint64_t)param >= (1ULL << bw)) {
                printf("Error: register or parameter out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("SUB");
            interp->instr_p1[interp->instr_count] = reg;
            interp->instr_p2[interp->instr_count] = param;
            interp->instr_p3[interp->instr_count] = mode;
            interp->instr_count++;
            i += need;

        } else if (strcmp(op, "011") == 0) {
            int need = bw + 1 + 1;
            if (i + need > n) {
                printf("Warning: incomplete JMP instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char param_str[1024];
            memcpy(param_str, work + i, bw);
            param_str[bw] = '\0';
            int param = parse_binary(param_str, bw);
            int mode1 = work[i + bw] - '0';
            int mode2 = work[i + bw + 1] - '0';

            if ((uint64_t)param >= (1ULL << bw)) {
                printf("Error: parameter out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("JMP");
            interp->instr_p1[interp->instr_count] = param;
            interp->instr_p2[interp->instr_count] = mode1;
            interp->instr_p3[interp->instr_count] = mode2;
            interp->instr_count++;
            i += need;

        } else if (strcmp(op, "100") == 0) {
            int need = bw + bw + 1;
            if (i + need > n) {
                printf("Warning: incomplete IFZ instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char reg_str[1024], param_str[1024];
            memcpy(reg_str, work + i, bw);
            reg_str[bw] = '\0';
            memcpy(param_str, work + i + bw, bw);
            param_str[bw] = '\0';
            int reg = parse_binary(reg_str, bw);
            int param = parse_binary(param_str, bw);
            int mode = work[i + bw*2] - '0';

            if ((uint64_t)reg >= (1ULL << bw) || (uint64_t)param >= (1ULL << bw)) {
                printf("Error: register or parameter out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("IFZ");
            interp->instr_p1[interp->instr_count] = reg;
            interp->instr_p2[interp->instr_count] = param;
            interp->instr_p3[interp->instr_count] = mode;
            interp->instr_count++;
            i += need;

        } else if (strcmp(op, "101") == 0) {
            int need = bw + 2 + 1;
            if (i + need > n) {
                printf("Warning: incomplete OUT instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char param_str[1024];
            memcpy(param_str, work + i, bw);
            param_str[bw] = '\0';
            int param = parse_binary(param_str, bw);
            char mode_bits[3] = {work[i + bw], work[i + bw + 1], '\0'};
            int mode1 = parse_binary(mode_bits, 2);
            int mode2 = work[i + bw + 2] - '0';

            if ((uint64_t)param >= (1ULL << bw)) {
                printf("Error: parameter out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("OUT");
            interp->instr_p1[interp->instr_count] = param;
            interp->instr_p2[interp->instr_count] = mode1;
            interp->instr_p3[interp->instr_count] = mode2;
            interp->instr_count++;
            i += need;

        } else if (strcmp(op, "110") == 0) {
            int need = bw + 2;
            if (i + need > n) {
                printf("Warning: incomplete INP instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char reg_str[1024];
            memcpy(reg_str, work + i, bw);
            reg_str[bw] = '\0';
            int reg = parse_binary(reg_str, bw);
            char mode_bits[3] = {work[i + bw], work[i + bw + 1], '\0'};
            int mode = parse_binary(mode_bits, 2);

            if ((uint64_t)reg >= (1ULL << bw)) {
                printf("Error: register out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("INP");
            interp->instr_p1[interp->instr_count] = reg;
            interp->instr_p2[interp->instr_count] = mode;
            interp->instr_p3[interp->instr_count] = 0;
            interp->instr_count++;
            i += need;

        } else if (strcmp(op, "111") == 0) {
            int need = bw + 1;
            if (i + need > n) {
                printf("Warning: incomplete SYS instruction at bit %d, ignored\n", i - 3);
                break;
            }
            char param_str[1024];
            memcpy(param_str, work + i, bw);
            param_str[bw] = '\0';
            int param = parse_binary(param_str, bw);
            int mode = work[i + bw] - '0';

            if ((uint64_t)param >= (1ULL << bw)) {
                printf("Error: parameter out of range for bit-width %d\n", bw);
                interp->had_error = 1;
                free(work);
                return 0;
            }

            ensure_instr_capacity(interp);
            interp->instr_op[interp->instr_count] = my_strdup("SYS");
            interp->instr_p1[interp->instr_count] = param;
            interp->instr_p2[interp->instr_count] = mode;
            interp->instr_p3[interp->instr_count] = 0;
            interp->instr_count++;
            i += need;

        } else {
            printf("Error: unknown opcode %s at bit %d\n", op, i - 3);
            interp->had_error = 1;
            free(work);
            return 0;
        }
    }

    free(work);
    return 1;
}


/* Label Resolution */

static int resolve_labels(ZonaryInterpreter* interp) {
    interp->label_count = 0;

    for (int idx = 0; idx < interp->instr_count; idx++) {
        char* op = interp->instr_op[idx];
        if (strcmp(op, "JMP") == 0) {
            uint64_t param = interp->instr_p1[idx];
            int mode1 = (int)interp->instr_p2[idx];
            int mode2 = (int)interp->instr_p3[idx];

            if (mode1 == 1) {
                uint64_t label_num;
                if (mode2 == 0) {
                    label_num = param;
                } else {
                    label_num = get_reg(interp, (int)param);
                }

                if (!add_label(interp, label_num, idx + 1)) {
                    int existing = find_label(interp, label_num);
                    printf("Error: label %llu already defined at instruction %d\n",
                           (unsigned long long)label_num, existing);
                    interp->had_error = 1;
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int get_label_target(ZonaryInterpreter* interp, uint64_t label_num) {
    int target = find_label(interp, label_num);
    if (target < 0) {
        printf("Warning: label %llu not found\n", (unsigned long long)label_num);
    }
    return target;
}


/* Output Formatting */

static void format_value(ZonaryInterpreter* interp, uint64_t val, int mode) {
    val &= interp->max_value;
    int bw = interp->bit_width;

    if (mode == 0) {
        for (int i = bw - 1; i >= 0; i--) {
            printf("%c", (val & (1ULL << i)) ? '1' : '0');
        }
    } else if (mode == 1) {
        printf("%llu", (unsigned long long)val);
    } else if (mode == 2) {
        int hex_digits = max_int(1, (bw + 3) / 4);
        printf("%0*llX", hex_digits, (unsigned long long)val);
    } else {
        if (val >= 32 && val <= 126) {
            printf("%c", (char)val);
        } else {
            printf("\\x%02llX", (unsigned long long)val);
        }
    }
    fflush(stdout);
}


/* Input Parsing */

static int parse_input(ZonaryInterpreter* interp, const char* s, int mode) {
    char* trimmed = my_strdup(s);
    if (!trimmed) return -1;

    char* p = trimmed;
    while (*p && isspace((unsigned char)*p)) p++;
    char* end = p + strlen(p) - 1;
    while (end > p && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    if (strlen(p) == 0) {
        free(trimmed);
        return -1;
    }

    uint64_t val = 0;
    int valid = 1;
    int bw = interp->bit_width;

    if (mode == 0) {
        if ((int)strlen(p) != bw) {
            valid = 0;
        } else {
            for (int i = 0; i < bw && valid; i++) {
                if (p[i] != '0' && p[i] != '1') valid = 0;
            }
            if (valid) {
                val = 0;
                for (int i = 0; i < bw; i++) {
                    val = (val << 1) | (p[i] - '0');
                }
            }
        }
    } else if (mode == 1) {
        char* endptr;
        val = strtoull(p, &endptr, 10);
        if (*endptr != '\0') valid = 0;
    } else if (mode == 2) {
        int expected_digits = max_int(1, (bw + 3) / 4);
        char* clean = p;
        while (*clean == '0') clean++;
        if ((int)strlen(clean) > expected_digits) valid = 0;
        if (valid) {
            char* endptr;
            val = strtoull(p, &endptr, 16);
            if (*endptr != '\0') valid = 0;
        }
    } else {
        val = (unsigned char)p[0];
    }

    free(trimmed);

    if (!valid || val > interp->max_value) {
        return -1;
    }
    return (int)val;
}


/* Execution */

static int execute(ZonaryInterpreter* interp) {
    if (interp->had_error) return 0;
    if (interp->instr_count == 0) return 0;

    if (!resolve_labels(interp)) return 0;

    interp->pc = 0;
    interp->running = 1;
    uint64_t return_code = 0;

    char input_buf[1024];

    while (interp->running) {
        if (interp->pc >= interp->instr_count) {
            interp->running = 0;
            printf("\n---\n[Return Code: ");
            for (int i = interp->bit_width - 1; i >= 0; i--) {
                printf("%c", '0');
            }
            printf("]\n");
            break;
        }

        char* op = interp->instr_op[interp->pc];
        uint64_t p1 = interp->instr_p1[interp->pc];
        uint64_t p2 = interp->instr_p2[interp->pc];
        uint64_t p3 = interp->instr_p3[interp->pc];

        if (strcmp(op, "SET") == 0) {
            int reg = (int)p1;
            uint64_t param = p2;
            int mode = (int)p3;

            if (mode == 0) {
                set_reg(interp, reg, param);
            } else {
                set_reg(interp, reg, get_reg(interp, (int)param));
            }

        } else if (strcmp(op, "ADD") == 0) {
            int reg = (int)p1;
            uint64_t param = p2;
            int mode = (int)p3;

            uint64_t a = get_reg(interp, reg);
            uint64_t b = (mode == 0) ? param : get_reg(interp, (int)param);
            set_reg(interp, reg, a + b);

        } else if (strcmp(op, "SUB") == 0) {
            int reg = (int)p1;
            uint64_t param = p2;
            int mode = (int)p3;

            uint64_t a = get_reg(interp, reg);
            uint64_t b = (mode == 0) ? param : get_reg(interp, (int)param);
            set_reg(interp, reg, a - b);

        } else if (strcmp(op, "JMP") == 0) {
            uint64_t param = p1;
            int mode1 = (int)p2;
            int mode2 = (int)p3;

            if (mode1 == 0) {
                uint64_t label_num = (mode2 == 0) ? param : get_reg(interp, (int)param);
                int target = get_label_target(interp, label_num);
                if (target >= 0) {
                    interp->pc = target;
                    continue;
                }
            }

        } else if (strcmp(op, "IFZ") == 0) {
            int reg = (int)p1;
            uint64_t param = p2;
            int mode = (int)p3;

            if (get_reg(interp, reg) == 0) {
                uint64_t label_num = (mode == 0) ? param : get_reg(interp, (int)param);
                int target = get_label_target(interp, label_num);
                if (target >= 0) {
                    interp->pc = target;
                    continue;
                }
            }

        } else if (strcmp(op, "OUT") == 0) {
            uint64_t param = p1;
            int mode1 = (int)p2;
            int mode2 = (int)p3;

            uint64_t val = (mode2 == 0) ? param : get_reg(interp, (int)param);
            format_value(interp, val, mode1);

        } else if (strcmp(op, "INP") == 0) {
            int reg = (int)p1;
            int mode = (int)p2;

            if (fgets(input_buf, sizeof(input_buf), stdin)) {
                int val = parse_input(interp, input_buf, mode);
                if (val >= 0) {
                    set_reg(interp, reg, (uint64_t)val);
                } else {
                    set_reg(interp, reg, 0);
                }
            } else {
                set_reg(interp, reg, 0);
            }

        } else if (strcmp(op, "SYS") == 0) {
            uint64_t param = p1;
            int mode = (int)p2;

            if (mode == 0) {
                interp->running = 0;
                return_code = param;
                printf("\n---\n[Return Code: ");
                for (int i = interp->bit_width - 1; i >= 0; i--) {
                    printf("%c", (return_code & (1ULL << i)) ? '1' : '0');
                }
                printf("]\n");
            } else {
                sleep_ms((int)param);
            }
        }

        interp->pc++;
    }

    return (int)return_code;
}


/* Help and Version */

static void print_help(void) {
    printf("Zonary - v2.1 - TasKin\n");
    printf("Usage: zonary [OPTION]\n");
    printf("Options:\n");
    printf("  -h, --help            Show this help message and exit\n");
    printf("  -v, --version         Show version information and exit\n");
    printf("  -r, --run <file>      Run the specified Zonary source file (default behavior)\n");
    printf("\nExamples:\n");
    printf("  zonary example.zonary      Run the program\n");
    printf("  zonary -r example.zonary   Equivalent to above\n");
}


/* Main */

int main(int argc, char* argv[]) {
    /* No arguments: show help */
    if (argc == 1) {
        print_help();
        return 0;
    }

    /* Parse CLI flags */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        printf("Zonary Interpreter v2.1 - TasKin\n");
        printf("Github: github.com/TasKin-tk/Zonary\n");
        return 0;
    }

    /* Handle -r flag, or use the first argument directly */
    const char* filename = NULL;
    if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--run") == 0) {
        if (argc < 3) {
            printf("Error: missing file argument for %s\n", argv[1]);
            return 1;
        }
        filename = argv[2];
    } else {
        filename = argv[1];
    }

    char* code = read_file(filename);
    if (!code) {
        printf("Error: file not found: %s\n", filename);
        return 1;
    }

    ZonaryInterpreter* interp = interpreter_create();
    if (!interp) {
        printf("Error: failed to create interpreter\n");
        free(code);
        return 1;
    }

    if (!parse_code(interp, code)) {
        free(code);
        interpreter_destroy(interp);
        return 1;
    }

    execute(interp);

    free(code);
    interpreter_destroy(interp);
    return 0;
}

// TasKin Made
