// 定义无符号长整型的最大值
#define ULONG_MAX ((unsigned long)-1)

// 内联字符分类和转换函数
static inline int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static inline int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline int is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline char to_upper(char c) {
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

static inline int char_to_digit(char c, int base) {
    int digit;
    
    if (is_digit(c)) {
        digit = c - '0';
    } else if (is_alpha(c)) {
        digit = to_upper(c) - 'A' + 10;
    } else {
        return -1; // 无效字符
    }
    
    return (digit < base) ? digit : -1;
}

unsigned long strtoul(const char *str, char **endptr, int base) {
    const char *ptr = str;
    unsigned long result = 0;
    int sign = 1;
    int valid_digits = 0;
    int overflow = 0;
    
    // 1. 跳过前导空白字符
    while (is_whitespace(*ptr)) {
        ptr++;
    }
    
    // 2. 处理可选符号
    if (*ptr == '+') {
        ptr++;
    } else if (*ptr == '-') {
        sign = -1;
        ptr++;
    }
    
    // 3. 自动检测进制
    if (base == 0) {
        if (*ptr == '0') {
            char next_char = ptr[1];
            if (next_char == 'x' || next_char == 'X') {
                base = 16;
                ptr += 2; // 跳过 "0x"
            } else {
                base = 8;
                ptr++; // 只跳过 '0'
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        // 处理十六进制前缀
        if (*ptr == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) {
            ptr += 2;
        }
    }
    
    // 检查进制有效性
    if (base < 2 || base > 36) {
        if (endptr) *endptr = (char*)str;
        return 0;
    }
    
    // 4. 主转换循环
    unsigned long cutoff = ULONG_MAX / base;
    int cutlim = ULONG_MAX % base;
    
    while (*ptr != '\0') {
        int digit = char_to_digit(*ptr, base);
        
        if (digit == -1) {
            break; // 无效字符
        }
        
        valid_digits = 1;
        
        // 溢出检查（在乘法之前）
        if (!overflow) {
            if (result > cutoff || (result == cutoff && (unsigned long)digit > (unsigned long)cutlim)) {
                overflow = 1;
                result = ULONG_MAX;
            } else {
                result = result * base + (unsigned long)digit;
            }
        }
        
        ptr++;
    }
    
    // 5. 设置结束指针
    if (endptr) {
        *endptr = (char*)(valid_digits ? ptr : str);
    }
    
    // 6. 处理结果
    if (!valid_digits) {
        return 0;
    }
    
    if (overflow) {
        return ULONG_MAX;
    }
    
    // 处理负号（无符号数的特殊情况）
    if (sign < 0) {
        // 对于无符号数，负号通常表示取模运算
        // 这相当于 result = -result（在无符号上下文中的行为）
        result = (unsigned long)(-(long)result);
    }
    
    return result;
}