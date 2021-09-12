#include <string.h>

double ten_pow(int pow) {
    double result = 1;
    for (int i = 0; i < pow; i++) {
        result = result * 10;
    }
    return result;
}

int findPeriod(char* str) {
    for (int i = 0; i < 256; i++) {
        if (str[i] == '.') return i;
        if (str[i] == 0) return -1;
    }

    return -1;
}

// characters to double
double ctod(char* str) {
    int isNeg = str[0] == '-';
    if (isNeg) {
        return -1.0 * ctod(str+1);
    }

    double result = 0;
    int periodPos = findPeriod(str);
    int endIndex = strlen(str);
    int endOfNatural = periodPos == -1 ? endIndex : periodPos;

    for (int i = 0; i < endOfNatural; i++) {
        result += ten_pow(i) * (str[endOfNatural-i-1] - '0');
    }

    if (periodPos != -1) {
        for (int i = periodPos + 1; i < endIndex; i++) {
            result += ((double) (str[i] - '0')) / ten_pow(i-periodPos);
        }
    }
    return result;
}
