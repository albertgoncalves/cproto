#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* NOTE: See `http://www.cs.utsa.edu/~wagner/CS3723/tiny/rdparse.html`. */
/* NOTE:
 *           GRAMMAR
 *  -------------------------
 *   F ---> char | '(' E ')'
 *   S ---> F '^' S | F
 *   T ---> S {('*'|'/') S}
 *   E ---> T {('+'|'-') T}
 *   P ---> E ';'
 */

static char TOKEN;
static char LEVEL = 0;
static char POS = 0;

static void print_indent(char level) {
    while (0 < level--) {
        printf("| ");
    }
}

static void print_enter(char x) {
    print_indent(LEVEL++);
    printf("+-%c: Enter, \t", x);
    printf("Next == %c\n", TOKEN);
}

static void print_leave(char x) {
    print_indent(--LEVEL);
    printf("+-%c: Leave, \t", x);
    printf("Next == %c\n", TOKEN);
}

#define ERROR                               \
    printf("\nError, character %d\n", POS); \
    exit(EXIT_FAILURE)

static void advance_token(void) {
    do {
        ++POS;
        TOKEN = (char)getchar();
    } while (isspace(TOKEN));
}

void F(void);
void S(void);
void T(void);
void E(void);

#define PRINT_ENTER_LEAVE(x, block) \
    {                               \
        print_enter(x);             \
        block;                      \
        print_leave(x);             \
    }

void F(void) {
    PRINT_ENTER_LEAVE('F', {
        if (isalpha(TOKEN)) {
            advance_token();
        } else if (TOKEN == '(') {
            advance_token();
            E();
            if (TOKEN == ')') {
                advance_token();
            } else {
                ERROR;
            }
        } else {
            ERROR;
        }
    });
}

void T(void) {
    PRINT_ENTER_LEAVE('T', {
        S();
        while ((TOKEN == '*') || (TOKEN == '/')) {
            advance_token();
            S();
        }
    });
}

void S(void) {
    PRINT_ENTER_LEAVE('S', {
        F();
        if (TOKEN == '^') {
            advance_token();
            S();
        }
    });
}

void E(void) {
    PRINT_ENTER_LEAVE('E', {
        T();
        while ((TOKEN == '+') || (TOKEN == '-')) {
            advance_token();
            T();
        }
    });
}

static void P(void) {
    advance_token();
    print_enter('P');
    E();
    if (TOKEN != ';') {
        ERROR;
    } else {
        print_leave('P');
        printf("\nSuccessful parse!\n");
    }
}

int main(void) {
    P();
    return EXIT_SUCCESS;
}
