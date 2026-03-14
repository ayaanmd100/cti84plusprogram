/*
 * Math Solver CE  v1.0
 * TI-84 Plus CE  |  CE C/C++ Toolchain
 *
 * Topics:
 *   - Linear & Quadratic Equations
 *   - Linear Inequalities
 *   - Absolute Value Equations & Inequalities
 *     (including nested abs-val forms)
 *   - Systems of Equations (2x2 and 3x3)
 *   - Coordinate Geometry (distance, midpoint,
 *     slope, line equations, parallel/perp.)
 *   - Formula Reference Pages
 *
 * Navigation:
 *   Number keys  -> select menu item
 *   [CLEAR]      -> back / quit
 *   [ENTER]      -> confirm input
 *   [(-)]        -> negative sign in input
 *   [DEL]        -> backspace in input
 *   [.]          -> decimal point in input
 */

#include <tice.h>
#include <ti/screen.h>
#include <ti/getcsc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────
   CONSTANTS
   ───────────────────────────────────────────── */
#define EPSILON     1e-9
#define MAX_NBUF    22      /* max chars in a number input buffer */
#define LINE_SZ     28      /* scratch-line buffer  (26 cols + \0 + 1) */

/* ─────────────────────────────────────────────
   UTILITY LAYER
   ───────────────────────────────────────────── */

/* Clear home screen */
static void clr(void)
{
    os_ClrHome();
}

/* Print string + newline */
static void pln(const char *s)
{
    os_PutStrFull(s);
    os_NewLine();
}

/* 26-char divider */
static void divider(void)
{
    pln("--------------------------");
}

/* Wait for any key, showing a prompt */
static void any_key(void)
{
    pln("[Press any key]");
    while (!os_GetCSC());
}

/* Format a double to string.
   Shows as integer when value is whole, else uses %g style. */
static void fmt(char *buf, size_t sz, double v)
{
    if (fabs(v) < EPSILON) {
        snprintf(buf, sz, "0");
    } else if (fabs(v - round(v)) < EPSILON * (1.0 + fabs(v))) {
        snprintf(buf, sz, "%.0f", round(v));
    } else {
        snprintf(buf, sz, "%.5g", v);
    }
}

/* Print a formatted double followed by a newline */
static void pdln(double v)
{
    char buf[32];
    fmt(buf, sizeof(buf), v);
    pln(buf);
}

/* ─────────────────────────────────────────────
   NUMBER INPUT
   Displays prompt, then lets the user type a
   floating-point number using digit keys.
   Returns the entered value (0.0 on empty Enter).
   ───────────────────────────────────────────── */
static double get_num(const char *prompt)
{
    char buf[MAX_NBUF + 1];
    int  len     = 0;
    bool hasDot  = false;
    uint8_t key;
    unsigned int row, col;

    os_PutStrFull(prompt);
    os_GetCursorPos(&row, &col);
    buf[0] = '\0';

    for (;;) {
        /* Redraw the buffer at saved cursor position */
        os_SetCursorPos(row, col);
        os_PutStrFull(buf);
        os_PutStrFull("_ ");            /* blinking-style cursor indicator */

        /* Block until a key arrives */
        do { key = os_GetCSC(); } while (!key);

        if (key == sk_Enter) {
            /* Accept whatever is in buffer (including empty → 0) */
            os_SetCursorPos(row, col);
            os_PutStrFull(buf);
            os_PutStrFull("   ");
            os_NewLine();
            return atof(buf);

        } else if (key == sk_Del && len > 0) {
            if (buf[len - 1] == '.') hasDot = false;
            buf[--len] = '\0';

        } else if (key == sk_Chs) {
            /* Toggle leading minus sign */
            if (len > 0 && buf[0] == '-') {
                memmove(buf, buf + 1, (size_t)len);
                len--;
            } else if (len < MAX_NBUF - 1) {
                memmove(buf + 1, buf, (size_t)(len + 1));
                buf[0] = '-';
                len++;
            }

        } else if (key == sk_DecPnt && !hasDot && len < MAX_NBUF - 1) {
            /* If buffer is empty, prepend '0' for clarity */
            if (len == 0 || buf[len-1] == '-') {
                buf[len++] = '0';
            }
            buf[len++] = '.';
            buf[len]   = '\0';
            hasDot = true;

        } else {
            /* Digit keys */
            char c = 0;
            switch (key) {
                case sk_0: c = '0'; break;
                case sk_1: c = '1'; break;
                case sk_2: c = '2'; break;
                case sk_3: c = '3'; break;
                case sk_4: c = '4'; break;
                case sk_5: c = '5'; break;
                case sk_6: c = '6'; break;
                case sk_7: c = '7'; break;
                case sk_8: c = '8'; break;
                case sk_9: c = '9'; break;
                default:   break;
            }
            if (c && len < MAX_NBUF - 1) {
                buf[len++] = c;
                buf[len]   = '\0';
            }
        }
    }
}

/* ─────────────────────────────────────────────
   MENU ENGINE
   Shows a titled list of options; returns the
   0-based index of the chosen item, or -1 if
   [CLEAR] was pressed (back / quit).
   ───────────────────────────────────────────── */
static int menu(const char *title, const char **opts, int n)
{
    uint8_t key;
    for (;;) {
        clr();
        pln(title);
        divider();
        for (int i = 0; i < n; i++) {
            char line[LINE_SZ];
            snprintf(line, sizeof(line), "%d: %s", i + 1, opts[i]);
            pln(line);
        }

        do { key = os_GetCSC(); } while (!key);

        if (key == sk_Clear) return -1;

        int num = -1;
        switch (key) {
            case sk_1: num = 0; break;
            case sk_2: num = 1; break;
            case sk_3: num = 2; break;
            case sk_4: num = 3; break;
            case sk_5: num = 4; break;
            case sk_6: num = 5; break;
            case sk_7: num = 6; break;
            case sk_8: num = 7; break;
            case sk_9: num = 8; break;
            default:   break;
        }
        if (num >= 0 && num < n) return num;
    }
}

/* ─────────────────────────────────────────────
   INEQUALITY-OPERATOR SELECTION
   Returns 0 (<)  1 (<=)  2 (>)  3 (>=)  -1 (cancel)
   ───────────────────────────────────────────── */
static int pick_ineq_op(void)
{
    pln("Choose inequality:");
    pln("1:<  2:<=  3:>  4:>=");
    uint8_t key;
    do { key = os_GetCSC(); } while (!key);
    switch (key) {
        case sk_1: return 0;
        case sk_2: return 1;
        case sk_3: return 2;
        case sk_4: return 3;
        default:   return -1;
    }
}

static const char *op_str[4] = { "<", "<=", ">", ">=" };

/* ═══════════════════════════════════════════════
   SECTION 1 — EQUATIONS & INEQUALITIES
   ═══════════════════════════════════════════════ */

/* 1a. Linear equation  Ax + B = C */
static void eq_linear(void)
{
    clr();
    pln("LINEAR EQUATION");
    pln("Ax + B = C");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");

    char bA[16], bB[16], bC[16], line[LINE_SZ];
    fmt(bA, sizeof(bA), A);
    fmt(bB, sizeof(bB), B);
    fmt(bC, sizeof(bC), C);

    clr();
    pln("LINEAR EQUATION");
    divider();
    snprintf(line, sizeof(line), "(%s)x + (%s) = %s", bA, bB, bC);
    /* If it fits in 26 chars, print; else print on two lines */
    if ((int)strlen(line) <= 26) {
        pln(line);
    } else {
        snprintf(line, sizeof(line), "A=%s B=%s C=%s", bA, bB, bC);
        pln(line);
    }
    divider();

    if (fabs(A) < EPSILON) {
        if (fabs(B - C) < EPSILON) pln("All Real Numbers");
        else                        pln("No Solution");
    } else {
        double x = (C - B) / A;
        fmt(bA, sizeof(bA), x);
        snprintf(line, sizeof(line), "x = %s", bA);
        pln(line);
    }
    any_key();
}

/* 1b. Quadratic  Ax^2 + Bx + C = 0 */
static void eq_quadratic(void)
{
    clr();
    pln("QUADRATIC EQUATION");
    pln("Ax^2 + Bx + C = 0");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");

    char bA[16], bB[16], line[LINE_SZ];

    clr();
    pln("QUADRATIC EQUATION");
    divider();

    if (fabs(A) < EPSILON) {
        pln("A=0: not quadratic");
        if (fabs(B) > EPSILON) {
            double x = -C / B;
            fmt(bA, sizeof(bA), x);
            snprintf(line, sizeof(line), "Linear: x = %s", bA);
            pln(line);
        } else {
            if (fabs(C) < EPSILON) pln("All Reals (0=0)");
            else                    pln("No Solution");
        }
        any_key();
        return;
    }

    double D = B * B - 4.0 * A * C;
    fmt(bA, sizeof(bA), D);
    snprintf(line, sizeof(line), "Discrim. D = %s", bA);
    pln(line);

    double vx = -B / (2.0 * A);
    double vy = A * vx * vx + B * vx + C;

    if (D > EPSILON) {
        double sqD = sqrt(D);
        double x1 = (-B - sqD) / (2.0 * A);
        double x2 = (-B + sqD) / (2.0 * A);
        pln("Two Real Roots:");
        fmt(bA, sizeof(bA), x1);
        snprintf(line, sizeof(line), "  x1 = %s", bA);
        pln(line);
        fmt(bA, sizeof(bA), x2);
        snprintf(line, sizeof(line), "  x2 = %s", bA);
        pln(line);
    } else if (fabs(D) <= EPSILON) {
        pln("One Real Root (equal):");
        fmt(bA, sizeof(bA), vx);
        snprintf(line, sizeof(line), "  x = %s", bA);
        pln(line);
    } else {
        double im = sqrt(-D) / (2.0 * fabs(A));
        pln("Complex Roots:");
        fmt(bA, sizeof(bA), vx);
        fmt(bB, sizeof(bB), im);
        snprintf(line, sizeof(line), "  %s +/- %si", bA, bB);
        pln(line);
    }

    fmt(bA, sizeof(bA), vx);
    fmt(bB, sizeof(bB), vy);
    snprintf(line, sizeof(line), "V:(%s,%s)", bA, bB);
    pln(line);
    any_key();
}

/* 1c. Linear inequality  Ax + B {op} C */
static void ineq_linear(void)
{
    clr();
    pln("LINEAR INEQUALITY");
    pln("Ax + B {op} C");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");

    int op = pick_ineq_op();
    if (op < 0) return;

    char bA[16], bB[16], line[LINE_SZ];
    clr();
    pln("LINEAR INEQUALITY");
    divider();

    /* A = 0 case: constant inequality */
    if (fabs(A) < EPSILON) {
        bool res;
        switch (op) {
            case 0: res = (B <  C); break;
            case 1: res = (B <= C); break;
            case 2: res = (B >  C); break;
            default:res = (B >= C); break;
        }
        pln(res ? "All Real Numbers" : "No Solution");
        any_key();
        return;
    }

    double x = (C - B) / A;
    fmt(bA, sizeof(bA), x);

    /* If A < 0, flip direction */
    int eff = op;
    if (A < 0.0) {
        const int flip[4] = {2, 3, 0, 1};
        eff = flip[op];
    }

    /* Inequality-style solution */
    snprintf(line, sizeof(line), "x %s %s", op_str[eff], bA);
    pln(line);

    /* Interval notation */
    if      (eff == 0) snprintf(line, sizeof(line), "(-inf, %s)",  bA);
    else if (eff == 1) snprintf(line, sizeof(line), "(-inf, %s]",  bA);
    else if (eff == 2) snprintf(line, sizeof(line), "(%s, inf)",   bA);
    else               snprintf(line, sizeof(line), "[%s, inf)",   bA);
    pln(line);

    /* Show the boundary value in fraction form if near a nice fraction */
    fmt(bB, sizeof(bB), C - B);
    fmt(bA, sizeof(bA), A);
    snprintf(line, sizeof(line), "x = (%s)/(%s)", bB, bA);
    pln(line);

    any_key();
}

/* ═══════════════════════════════════════════════
   SECTION 2 — ABSOLUTE VALUE
   ═══════════════════════════════════════════════ */

/* Helper: solve |Ax+B| = rhs, print solutions labelled x{n}, x{n+1}.
   Returns the next available label index. */
static int solve_abs_eq(double A, double B, double rhs,
                         int xi, bool showLabel)
{
    char buf[16], line[LINE_SZ];
    if (rhs < -EPSILON) return xi;   /* negative RHS — no solution */

    if (fabs(A) < EPSILON) {
        /* |B| = rhs */
        if (fabs(fabs(B) - rhs) < EPSILON) {
            pln(showLabel ? "True for all x" : "All Reals");
        }
        /* else: no solution (no output) */
        return xi;
    }

    if (fabs(rhs) <= EPSILON) {
        /* |Ax+B| = 0  =>  x = -B/A */
        double x = -B / A;
        fmt(buf, sizeof(buf), x);
        snprintf(line, sizeof(line), "x%d = %s", xi, buf);
        pln(line);
        return xi + 1;
    }

    /* Two solutions: Ax+B = rhs  and  Ax+B = -rhs */
    double x1 = ( rhs - B) / A;
    double x2 = (-rhs - B) / A;
    fmt(buf, sizeof(buf), x1);
    snprintf(line, sizeof(line), "x%d = %s", xi, buf);
    pln(line);
    fmt(buf, sizeof(buf), x2);
    snprintf(line, sizeof(line), "x%d = %s", xi + 1, buf);
    pln(line);
    return xi + 2;
}

/* 2a.  |Ax + B| = C */
static void absval_eq(void)
{
    clr();
    pln("ABS VALUE EQUATION");
    pln("|Ax + B| = C");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");

    clr();
    pln("ABS VALUE EQUATION");
    divider();

    if (C < -EPSILON) {
        pln("No Solution");
        pln("(|expr| >= 0 always)");
    } else {
        int nextIdx = solve_abs_eq(A, B, C, 1, true);
        if (nextIdx == 1) pln("No Solution");
    }
    any_key();
}

/* 2b.  |Ax + B| {op} C */
static void absval_ineq(void)
{
    clr();
    pln("ABS VALUE INEQUALITY");
    pln("|Ax+B| {op} C");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");

    int op = pick_ineq_op();
    if (op < 0) return;

    bool less   = (op == 0 || op == 1);
    bool strict = (op == 0 || op == 2);
    char bL[16], bH[16], line[LINE_SZ];

    clr();
    pln("ABS VALUE INEQUALITY");
    divider();

    if (less) {
        /* |Ax+B| < C  (or <=)
           =>  -C < Ax+B < C
           =>  (-C-B)/A and (C-B)/A, sorted */
        if (C < EPSILON && strict) {
            pln("No Solution");
            pln("(|.| >= 0 > C)");
        } else if (fabs(A) < EPSILON) {
            /* |B| op C */
            bool ok = strict ? (fabs(B) < C) : (fabs(B) <= C);
            pln(ok ? "All Real Numbers" : "No Solution");
        } else {
            double v1 = (-C - B) / A;
            double v2 = ( C - B) / A;
            double lo = fmin(v1, v2);
            double hi = fmax(v1, v2);
            fmt(bL, sizeof(bL), lo);
            fmt(bH, sizeof(bH), hi);
            char lb = strict ? '(' : '[';
            char rb = strict ? ')' : ']';
            snprintf(line, sizeof(line), "%c%s, %s%c", lb, bL, bH, rb);
            pln(line);
            snprintf(line, sizeof(line),
                     strict ? "%s < x < %s" : "%s <= x <= %s", bL, bH);
            pln(line);
        }
    } else {
        /* |Ax+B| > C  (or >=) */
        if (C < -EPSILON) {
            pln("All Real Numbers");
        } else if (fabs(A) < EPSILON) {
            bool ok = strict ? (fabs(B) > C) : (fabs(B) >= C);
            pln(ok ? "All Real Numbers" : "No Solution");
        } else {
            double v1 = (-C - B) / A;
            double v2 = ( C - B) / A;
            double lo = fmin(v1, v2);
            double hi = fmax(v1, v2);
            fmt(bL, sizeof(bL), lo);
            fmt(bH, sizeof(bH), hi);
            char ob = strict ? ')' : ']';
            char ib = strict ? '(' : '[';
            snprintf(line, sizeof(line), "(-inf,%s%s", &ob, bL);
            /* Note: just embed bracket char */
            snprintf(line, sizeof(line), "(-inf,%c%s", ob, bL);
            pln(line);
            pln("  OR");
            snprintf(line, sizeof(line), "%s%c,inf)", bH, ib);
            pln(line);
            snprintf(line, sizeof(line),
                     strict ? "x<%s or x>%s" : "x<=%s or x>=%s", bL, bH);
            pln(line);
        }
    }
    any_key();
}

/* 2c.  |Ax + B| = |Cx + D| */
static void absval_eq_eq(void)
{
    clr();
    pln("|Ax+B| = |Cx+D|");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");
    double D = get_num("D= ");

    clr();
    pln("|Ax+B| = |Cx+D|");
    divider();

    char buf[16], line[LINE_SZ];
    bool found = false;

    /* Case 1: Ax+B = Cx+D  =>  (A-C)x = D-B */
    double a1 = A - C, b1 = D - B;
    if (fabs(a1) < EPSILON) {
        if (fabs(b1) < EPSILON) { pln("Case1: All Reals"); found = true; }
    } else {
        double x = b1 / a1;
        fmt(buf, sizeof(buf), x);
        snprintf(line, sizeof(line), "x1 = %s", buf);
        pln(line);
        found = true;
    }

    /* Case 2: Ax+B = -(Cx+D)  =>  (A+C)x = -D-B */
    double a2 = A + C, b2 = -D - B;
    if (fabs(a2) < EPSILON) {
        if (fabs(b2) < EPSILON) { pln("Case2: All Reals"); found = true; }
    } else {
        double x = b2 / a2;
        fmt(buf, sizeof(buf), x);
        snprintf(line, sizeof(line), "x2 = %s", buf);
        pln(line);
        found = true;
    }

    if (!found) pln("No Solution");
    any_key();
}

/* 2d.  Nested: ||Ax+B| - C| = D
   Reduces to |Ax+B| = C+D  and  |Ax+B| = C-D  */
static void absval_nested(void)
{
    clr();
    pln("NESTED ABS VALUE");
    pln("||Ax+B| - C| = D");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");
    double D = get_num("D= ");

    clr();
    pln("NESTED ABS VALUE");
    pln("||Ax+B| - C| = D");
    divider();

    if (D < -EPSILON) { pln("No Solution (D<0)"); any_key(); return; }

    /* ||Ax+B|-C|=D  =>  |Ax+B|-C = ±D
       =>  |Ax+B| = C+D  or  |Ax+B| = C-D */
    double rhs[2] = { C + D, C - D };
    int xi = 1;
    bool found = false;

    for (int k = 0; k < 2; k++) {
        if (rhs[k] < -EPSILON) continue;
        int next = solve_abs_eq(A, B, rhs[k], xi, false);
        if (next > xi) { found = true; xi = next; }
    }

    if (!found) pln("No Solution");
    any_key();
}

/* 2e.  Combined: |Ax+B| + C {op} D
   Reduces to |Ax+B| {op} (D-C)  */
static void absval_ineq_combined(void)
{
    clr();
    pln("ABS VAL + CONST INEQ");
    pln("|Ax+B| + C {op} D");
    divider();
    double A = get_num("A= ");
    double B = get_num("B= ");
    double C = get_num("C= ");
    double D = get_num("D= ");

    int op = pick_ineq_op();
    if (op < 0) return;

    /* Rewrite as |Ax+B| op (D-C) — borrow absval_ineq logic inline */
    double newC = D - C;
    bool less   = (op == 0 || op == 1);
    bool strict = (op == 0 || op == 2);
    char bL[16], bH[16], line[LINE_SZ];

    clr();
    pln("ABS VAL + CONST INEQ");
    divider();

    char bR[16];
    fmt(bR, sizeof(bR), newC);
    snprintf(line, sizeof(line), "|Ax+B| %s %s", op_str[op], bR);
    pln(line);
    divider();

    if (less) {
        if (newC < EPSILON && strict) { pln("No Solution"); }
        else if (fabs(A) < EPSILON) {
            bool ok = strict ? (fabs(B) < newC) : (fabs(B) <= newC);
            pln(ok ? "All Real Numbers" : "No Solution");
        } else {
            double v1 = (-newC - B) / A, v2 = (newC - B) / A;
            double lo = fmin(v1,v2), hi = fmax(v1,v2);
            fmt(bL,sizeof(bL),lo); fmt(bH,sizeof(bH),hi);
            char lb = strict?'(':'[', rb = strict?')':']';
            snprintf(line,sizeof(line),"%c%s, %s%c",lb,bL,bH,rb); pln(line);
            snprintf(line,sizeof(line),
                     strict?"%s<x<%s":"%s<=x<=%s",bL,bH); pln(line);
        }
    } else {
        if (newC < -EPSILON) { pln("All Real Numbers"); }
        else if (fabs(A) < EPSILON) {
            bool ok = strict?(fabs(B)>newC):(fabs(B)>=newC);
            pln(ok?"All Real Numbers":"No Solution");
        } else {
            double v1 = (-newC - B)/A, v2 = (newC - B)/A;
            double lo = fmin(v1,v2), hi = fmax(v1,v2);
            fmt(bL,sizeof(bL),lo); fmt(bH,sizeof(bH),hi);
            char ob = strict?')':']', ib = strict?'(':'[';
            snprintf(line,sizeof(line),"(-inf,%c%s",ob,bL); pln(line);
            pln("  OR");
            snprintf(line,sizeof(line),"%s%c,inf)",bH,ib); pln(line);
            snprintf(line,sizeof(line),
                     strict?"x<%s or x>%s":"x<=%s or x>=%s",bL,bH); pln(line);
        }
    }
    any_key();
}

/* ═══════════════════════════════════════════════
   SECTION 3 — SYSTEMS OF EQUATIONS
   ═══════════════════════════════════════════════ */

/* 3a.  2×2 via Cramer's Rule
   a1x + b1y = c1
   a2x + b2y = c2                               */
static void sys_2x2(void)
{
    clr();
    pln("2x2 SYSTEM");
    pln("a1x+b1y=c1");
    pln("a2x+b2y=c2");
    divider();

    pln("--- Equation 1 ---");
    double a1 = get_num("a1= ");
    double b1 = get_num("b1= ");
    double c1 = get_num("c1= ");
    pln("--- Equation 2 ---");
    double a2 = get_num("a2= ");
    double b2 = get_num("b2= ");
    double c2 = get_num("c2= ");

    clr();
    pln("2x2 SYSTEM SOLUTION");
    divider();

    double det = a1 * b2 - a2 * b1;
    char bX[16], bY[16], line[LINE_SZ];

    if (fabs(det) < EPSILON) {
        /* Check infinite vs no solution via row proportionality */
        bool inf = false;
        if (fabs(a2) > EPSILON) {
            double r = a1 / a2;
            inf = (fabs(b1 - r*b2) < EPSILON && fabs(c1 - r*c2) < EPSILON);
        } else if (fabs(a1) < EPSILON) {
            if (fabs(b2) > EPSILON) {
                double r = b1 / b2;
                inf = (fabs(c1 - r*c2) < EPSILON);
            } else {
                inf = (fabs(c1) < EPSILON && fabs(c2) < EPSILON);
            }
        }
        pln(inf ? "Infinite Solutions" : "No Solution");
    } else {
        double x = (c1 * b2 - c2 * b1) / det;
        double y = (a1 * c2 - a2 * c1) / det;
        fmt(bX, sizeof(bX), x);
        fmt(bY, sizeof(bY), y);
        snprintf(line, sizeof(line), "x = %s", bX);  pln(line);
        snprintf(line, sizeof(line), "y = %s", bY);  pln(line);
        snprintf(line, sizeof(line), "(%s, %s)", bX, bY); pln(line);
    }
    any_key();
}

/* 3b.  3×3 via Gaussian Elimination with partial pivoting
   ax + by + cz = d   (for 3 equations)           */
static void sys_3x3(void)
{
    double m[3][4];   /* augmented matrix */
    int i, j, k;
    const char *vname[3] = {"x","y","z"};
    const char *coef[4]  = {"a= ","b= ","c= ","d= "};

    clr();
    pln("3x3 SYSTEM");
    pln("ax+by+cz=d");
    divider();

    for (i = 0; i < 3; i++) {
        char hdr[16];
        snprintf(hdr, sizeof(hdr), "-- Equation %d --", i+1);
        pln(hdr);
        for (j = 0; j < 4; j++) {
            m[i][j] = get_num(coef[j]);
        }
    }

    /* Partial-pivot Gaussian elimination */
    for (i = 0; i < 3; i++) {
        /* Find best pivot in column i */
        int pivot = i;
        for (k = i+1; k < 3; k++) {
            if (fabs(m[k][i]) > fabs(m[pivot][i])) pivot = k;
        }
        if (pivot != i) {
            for (j = 0; j <= 3; j++) {
                double tmp = m[i][j]; m[i][j] = m[pivot][j]; m[pivot][j] = tmp;
            }
        }
        if (fabs(m[i][i]) < EPSILON) continue;
        for (k = i+1; k < 3; k++) {
            double f = m[k][i] / m[i][i];
            for (j = i; j <= 3; j++) m[k][j] -= f * m[i][j];
        }
    }

    /* Back substitution */
    double sol[3] = {0.0, 0.0, 0.0};
    bool valid = true;
    for (i = 2; i >= 0; i--) {
        if (fabs(m[i][i]) < EPSILON) { valid = false; break; }
        sol[i] = m[i][3];
        for (j = i+1; j < 3; j++) sol[i] -= m[i][j] * sol[j];
        sol[i] /= m[i][i];
    }

    clr();
    pln("3x3 SYSTEM SOLUTION");
    divider();

    if (!valid) {
        /* Infinite if last row is all zeros, else no solution */
        bool allZ = true;
        for (j = 0; j < 3; j++) if (fabs(m[2][j]) > EPSILON) { allZ=false; break; }
        pln((allZ && fabs(m[2][3]) < EPSILON) ? "Infinite Solutions" : "No Solution");
    } else {
        char buf[16], line[LINE_SZ];
        for (i = 0; i < 3; i++) {
            fmt(buf, sizeof(buf), sol[i]);
            snprintf(line, sizeof(line), "%s = %s", vname[i], buf);
            pln(line);
        }
        /* Abbreviated tuple display */
        char bx[8],by[8],bz[8];
        snprintf(bx,sizeof(bx),"%.4g",sol[0]);
        snprintf(by,sizeof(by),"%.4g",sol[1]);
        snprintf(bz,sizeof(bz),"%.4g",sol[2]);
        snprintf(line, sizeof(line), "(%s,%s,%s)", bx,by,bz);
        pln(line);
    }
    any_key();
}

/* ═══════════════════════════════════════════════
   SECTION 4 — COORDINATE GEOMETRY
   ═══════════════════════════════════════════════ */

/* 4a.  Distance & Midpoint */
static void coord_dist_mid(void)
{
    clr();
    pln("DISTANCE & MIDPOINT");
    pln("Points (x1,y1),(x2,y2)");
    divider();
    double x1 = get_num("x1= ");
    double y1 = get_num("y1= ");
    double x2 = get_num("x2= ");
    double y2 = get_num("y2= ");

    double dx   = x2 - x1, dy = y2 - y1;
    double dsq  = dx*dx + dy*dy;
    double dist = sqrt(dsq);
    double mx   = (x1 + x2) * 0.5;
    double my   = (y1 + y2) * 0.5;

    char b1[16], b2[16], line[LINE_SZ];
    clr();
    pln("DISTANCE & MIDPOINT");
    divider();

    fmt(b1, sizeof(b1), dist);
    snprintf(line, sizeof(line), "d = %s", b1);     pln(line);
    fmt(b1, sizeof(b1), dsq);
    snprintf(line, sizeof(line), "  = sqrt(%s)", b1); pln(line);

    fmt(b1, sizeof(b1), mx);
    fmt(b2, sizeof(b2), my);
    snprintf(line, sizeof(line), "Mid = (%s, %s)", b1, b2); pln(line);
    any_key();
}

/* 4b.  Slope & Line Equations */
static void coord_slope_line(void)
{
    clr();
    pln("SLOPE & LINE EQ.");
    pln("Points (x1,y1),(x2,y2)");
    divider();
    double x1 = get_num("x1= ");
    double y1 = get_num("y1= ");
    double x2 = get_num("x2= ");
    double y2 = get_num("y2= ");

    char bM[16], bB[16], bX[16], bY[16], line[LINE_SZ];
    clr();
    pln("SLOPE & LINE EQ.");
    divider();

    if (fabs(x2 - x1) < EPSILON) {
        pln("Slope: Undefined");
        pln("(Vertical line)");
        fmt(bX, sizeof(bX), x1);
        snprintf(line, sizeof(line), "Eq: x = %s", bX); pln(line);
    } else {
        double m = (y2 - y1) / (x2 - x1);
        double b = y1 - m * x1;
        double xi = (fabs(m) > EPSILON) ? (-b / m) : 0.0; /* x-intercept */

        fmt(bM, sizeof(bM), m);
        fmt(bB, sizeof(bB), b);

        snprintf(line, sizeof(line), "m = %s", bM); pln(line);

        /* Slope-intercept: y = mx + b */
        snprintf(line, sizeof(line), "y = %sx + %s", bM, bB); pln(line);

        /* Point-slope (abbreviated for screen width) */
        fmt(bX, sizeof(bX), x1);
        fmt(bY, sizeof(bY), y1);
        snprintf(line, sizeof(line), "Pt-slope: m=%s", bM); pln(line);
        snprintf(line, sizeof(line), " pt=(%s,%s)", bX, bY); pln(line);

        /* y-intercept */
        snprintf(line, sizeof(line), "y-int: (0, %s)", bB); pln(line);

        /* x-intercept (only if not horizontal) */
        if (fabs(m) > EPSILON) {
            fmt(bX, sizeof(bX), xi);
            snprintf(line, sizeof(line), "x-int: (%s, 0)", bX); pln(line);
        }
    }
    any_key();
}

/* 4c.  Parallel & Perpendicular Lines */
static void coord_parallel_perp(void)
{
    clr();
    pln("PARALLEL & PERP.");
    pln("Enter slope m of line:");
    divider();
    double m = get_num("m= ");

    char bM[16], bMP[16], bB[16], line[LINE_SZ];
    fmt(bM, sizeof(bM), m);

    clr();
    pln("PARALLEL & PERP.");
    divider();
    snprintf(line, sizeof(line), "Given: m = %s", bM);    pln(line);
    snprintf(line, sizeof(line), "Parallel: m = %s", bM); pln(line);

    if (fabs(m) < EPSILON) {
        pln("Perp: Undefined (vert.)");
    } else {
        double mp = -1.0 / m;
        fmt(bMP, sizeof(bMP), mp);
        snprintf(line, sizeof(line), "Perp: m = %s", bMP); pln(line);
    }

    /* Optional: line through a point */
    pln("Through point (p,q)?");
    pln("1: Yes   Other: No");
    uint8_t key;
    do { key = os_GetCSC(); } while (!key);

    if (key == sk_1) {
        double px = get_num("px= ");
        double py = get_num("py= ");

        /* Parallel: y = m*x + (py - m*px) */
        double bPar = py - m * px;
        fmt(bB, sizeof(bB), bPar);
        snprintf(line, sizeof(line), "Par: y=%sx+%s", bM, bB); pln(line);

        /* Perpendicular */
        if (fabs(m) > EPSILON) {
            double mp  = -1.0 / m;
            double bPp = py - mp * px;
            fmt(bMP, sizeof(bMP), mp);
            fmt(bB,  sizeof(bB),  bPp);
            snprintf(line, sizeof(line), "Perp: y=%sx+%s", bMP, bB); pln(line);
        } else {
            /* Horizontal line through (px, py) → perpendicular is vertical */
            fmt(bB, sizeof(bB), px);
            snprintf(line, sizeof(line), "Perp: x = %s", bB); pln(line);
        }
    }
    any_key();
}

/* 4d.  Collinearity / Angle between lines */
static void coord_angle_lines(void)
{
    clr();
    pln("ANGLE BETWEEN LINES");
    pln("Enter slopes m1, m2:");
    divider();
    double m1 = get_num("m1= ");
    double m2 = get_num("m2= ");

    char b1[16], line[LINE_SZ];
    clr();
    pln("ANGLE BETWEEN LINES");
    divider();

    if (fabs(1.0 + m1 * m2) < EPSILON) {
        pln("Lines are PERP (90 deg)");
    } else if (fabs(m1 - m2) < EPSILON) {
        pln("Lines are PARALLEL (0 deg)");
    } else {
        double tanA = fabs((m1 - m2) / (1.0 + m1 * m2));
        double angleRad = atan(tanA);
        double angleDeg = angleRad * (180.0 / M_PI);
        fmt(b1, sizeof(b1), angleDeg);
        snprintf(line, sizeof(line), "Angle = %s deg", b1); pln(line);

        fmt(b1, sizeof(b1), 180.0 - angleDeg);
        snprintf(line, sizeof(line), "Supp.  = %s deg", b1); pln(line);
    }
    any_key();
}

/* ═══════════════════════════════════════════════
   SECTION 5 — FORMULA REFERENCE  (display only)
   ═══════════════════════════════════════════════ */

static void ref_algebra(void)
{
    clr();
    pln("ALGEBRA FORMULAS");
    divider();
    pln("LINE: y = mx + b");
    pln("y - y1 = m(x - x1)");
    pln("Ax + By + C = 0");
    pln("QUAD: ax^2+bx+c=0");
    pln("x=(-b+/-sqt(D))/2a");
    pln("D = b^2 - 4ac");
    any_key();

    clr();
    pln("ALGEBRA FORMULAS 2");
    divider();
    pln("D>0: 2 real roots");
    pln("D=0: 1 real root");
    pln("D<0: complex roots");
    pln("Vertex: x = -b/2a");
    pln("  y = f(-b/2a)");
    pln("SPECIAL PRODUCTS:");
    any_key();

    clr();
    pln("SPECIAL FACTORS");
    divider();
    pln("a^2-b^2=(a+b)(a-b)");
    pln("a^3-b^3=");
    pln(" (a-b)(a^2+ab+b^2)");
    pln("a^3+b^3=");
    pln(" (a+b)(a^2-ab+b^2)");
    pln("(a+b)^2=a^2+2ab+b^2");
    any_key();
}

static void ref_geometry(void)
{
    clr();
    pln("GEOMETRY FORMULAS");
    divider();
    pln("CIRCLE:A=pi*r^2");
    pln("       C=2*pi*r");
    pln("SPHERE:V=(4/3)pi*r^3");
    pln("       SA=4*pi*r^2");
    pln("CYLINDER:V=pi*r^2*h");
    pln("CONE:V=(1/3)pi*r^2*h");
    any_key();

    clr();
    pln("GEOMETRY FORMULAS 2");
    divider();
    pln("RECT: A=L*W, P=2L+2W");
    pln("SQUARE: A=s^2, P=4s");
    pln("TRAP: A=0.5h(b1+b2)");
    pln("TRIANGLE: A=0.5*b*h");
    pln("Heron:s=(a+b+c)/2");
    pln(" A=sqrt(s(s-a)(s-b)");
    any_key();
}

static void ref_coord(void)
{
    clr();
    pln("COORD GEO FORMULAS");
    divider();
    pln("Distance:");
    pln("d=sqrt((x2-x1)^2");
    pln("       +(y2-y1)^2)");
    pln("Midpoint:");
    pln("M=((x1+x2)/2,");
    pln("   (y1+y2)/2)");
    any_key();

    clr();
    pln("COORD GEO FORMULAS 2");
    divider();
    pln("Slope: m=(y2-y1)/");
    pln("         (x2-x1)");
    pln("|| lines: m1 = m2");
    pln("_|_ lines: m1*m2=-1");
    pln("Angle: tan(A)=");
    pln(" |m1-m2|/|1+m1*m2|");
    any_key();

    clr();
    pln("CONICS");
    divider();
    pln("Circle:(x-h)^2+");
    pln("  (y-k)^2=r^2");
    pln("Ellipse:(x-h)^2/a^2");
    pln("  +(y-k)^2/b^2=1");
    pln("Parabola:(x-h)^2=");
    pln("  4p(y-k)");
    any_key();
}

static void ref_inequalities(void)
{
    clr();
    pln("INEQUALITY RULES");
    divider();
    pln("Add/Sub: same side");
    pln("Mul/Div >0: same dir");
    pln("Mul/Div <0: FLIP dir");
    pln("|x| < a => -a<x<a");
    pln("|x| > a =>");
    pln(" x<-a  or  x>a");
    any_key();

    clr();
    pln("SYSTEMS — INEQ.");
    divider();
    pln("Graph each boundary.");
    pln("Shade the region that");
    pln("satisfies BOTH (AND)");
    pln("or EITHER (OR).");
    pln("Test a point to check");
    pln("which region to shade.");
    any_key();
}

/* ═══════════════════════════════════════════════
   MENU HIERARCHY
   ═══════════════════════════════════════════════ */

static void menu_equations(void)
{
    const char *opts[] = {
        "Linear  (Ax+B=C)",
        "Quadratic Eq.",
        "Linear Inequality",
        "Back"
    };
    int sel;
    while ((sel = menu("EQUATIONS & INEQ.", opts, 4)) >= 0) {
        switch (sel) {
            case 0: eq_linear();    break;
            case 1: eq_quadratic(); break;
            case 2: ineq_linear();  break;
            case 3: return;
        }
    }
}

static void menu_absval(void)
{
    const char *opts[] = {
        "|Ax+B| = C",
        "|Ax+B| {op} C",
        "|Ax+B| = |Cx+D|",
        "Nested ||Ax+B|-C|=D",
        "|Ax+B|+C {op} D",
        "Back"
    };
    int sel;
    while ((sel = menu("ABSOLUTE VALUE", opts, 6)) >= 0) {
        switch (sel) {
            case 0: absval_eq();            break;
            case 1: absval_ineq();          break;
            case 2: absval_eq_eq();         break;
            case 3: absval_nested();        break;
            case 4: absval_ineq_combined(); break;
            case 5: return;
        }
    }
}

static void menu_systems(void)
{
    const char *opts[] = {
        "2x2 System",
        "3x3 System",
        "Back"
    };
    int sel;
    while ((sel = menu("SYSTEMS OF EQ.", opts, 3)) >= 0) {
        switch (sel) {
            case 0: sys_2x2(); break;
            case 1: sys_3x3(); break;
            case 2: return;
        }
    }
}

static void menu_coord(void)
{
    const char *opts[] = {
        "Distance & Midpoint",
        "Slope & Line Eq.",
        "Parallel & Perp.",
        "Angle Btwn. Lines",
        "Back"
    };
    int sel;
    while ((sel = menu("COORDINATE GEOMETRY", opts, 5)) >= 0) {
        switch (sel) {
            case 0: coord_dist_mid();      break;
            case 1: coord_slope_line();    break;
            case 2: coord_parallel_perp(); break;
            case 3: coord_angle_lines();   break;
            case 4: return;
        }
    }
}

static void menu_reference(void)
{
    const char *opts[] = {
        "Algebra Formulas",
        "Geometry Formulas",
        "Coord. Geometry",
        "Inequality Rules",
        "Back"
    };
    int sel;
    while ((sel = menu("FORMULAS & REF.", opts, 5)) >= 0) {
        switch (sel) {
            case 0: ref_algebra();      break;
            case 1: ref_geometry();     break;
            case 2: ref_coord();        break;
            case 3: ref_inequalities(); break;
            case 4: return;
        }
    }
}

/* ─────────────────────────────────────────────
   MAIN MENU  (mirrors ACT Helper's top-level)
   ───────────────────────────────────────────── */
static void menu_main(void)
{
    const char *opts[] = {
        "Equations & Ineq.",
        "Absolute Value",
        "Systems of Eq.",
        "Coordinate Geometry",
        "Formulas & Ref.",
        "Quit"
    };
    int sel;
    for (;;) {
        sel = menu("MATH SOLVER CE", opts, 6);
        if (sel < 0 || sel == 5) break;
        switch (sel) {
            case 0: menu_equations(); break;
            case 1: menu_absval();    break;
            case 2: menu_systems();   break;
            case 3: menu_coord();     break;
            case 4: menu_reference(); break;
        }
    }
}

/* ─────────────────────────────────────────────
   ENTRY POINT
   ───────────────────────────────────────────── */
int main(void)
{
    os_ClrHome();
    menu_main();

    /* Exit screen — mirrors reference program's quit label */
    os_ClrHome();
    pln("MATH SOLVER CE");
    divider();
    pln("Thank you for using");
    pln("Math Solver CE.");
    pln("Goodbye!");

    /* Brief pause before returning to TI-OS */
    for (volatile int i = 0; i < 500000; i++);

    os_ClrHome();
    return 0;
}