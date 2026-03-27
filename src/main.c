/*
 * MathSolverCE  v2.6
 * TI-84 Plus CE  |  CE C/C++ Toolchain
 *
 * Menus:
 *   Main Menu
 *     1. Equations & Inequalities
 *           1. Linear Equation
 *           2. Quadratic Equation
 *           3. Linear Inequality
 *           4. Three-Way Inequality
 *     2. Absolute Value
 *           1. |Ax+B| = C
 *           2. |Ax+B| op C
 *           3. ||Ax+B|-C| op D  (nested)
 *     3. Number Theory
 *           1. Prime Factorization.
 *           2. GCD / HCF
 *           3. LCM
 *           4. Perm. & Comb. 
 *           5. Binomial Theorem.
 * 
 *     4. Formula Reference
 *     5. Quit
 *
 * Key legend:
 *   [1-9]    select menu item
 *   [CLEAR]  back / quit at any point, including mid-input
 *   [ENTER]  confirm number entry
 *   [(-)  ]  toggle negative sign
 *   [.    ]  decimal point
 *   [DEL  ]  backspace
 */

#include <graphx.h>
#include <ti/getcsc.h>
#include <tice.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <ti/real.h>

/* ═══════════════════════════════════════════════
   PALETTE INDICES
   ═══════════════════════════════════════════════ */
#define COL_WHITE   0
#define COL_BLACK   1
#define COL_NAVY    2
#define COL_ORANGE  3
#define COL_GREEN   4
#define COL_RED     5
#define COL_GRAY    6
#define COL_LTGRAY  7
#define COL_YELLOW  8

static const uint16_t kPalette[] = {
    gfx_RGBTo1555(255, 255, 255),  /* 0  white    body bg           */
    gfx_RGBTo1555(  0,   0,   0),  /* 1  black    body text         */
    gfx_RGBTo1555( 10,  35,  88),  /* 2  navy     header / footer   */
    gfx_RGBTo1555(215, 115,   0),  /* 3  orange   accent / numbers  */
    gfx_RGBTo1555(  0, 148,  58),  /* 4  green    solution text     */
    gfx_RGBTo1555(195,  22,  22),  /* 5  red      no solution       */
    gfx_RGBTo1555(145, 145, 145),  /* 6  gray     dividers          */
    gfx_RGBTo1555(215, 215, 215),  /* 7  lt-gray  alt row tint      */
    gfx_RGBTo1555(255, 200,   0),  /* 8  yellow   header title      */
};

/* ═══════════════════════════════════════════════
   SCREEN LAYOUT
   ═══════════════════════════════════════════════ */
#define SCR_W        320
#define SCR_H        240
#define HDR_H         22
#define FTR_H         18
#define BODY_TOP      26
#define LINE_H        13
#define MAX_LINES     15
#define LMARGIN        8

static const double MATH_EPS = 1e-9;
static const char  *kOpStr[5] = { "<", "<=", ">", ">=", "=" };

/* Current body row (0-based), reset by startScreen() */
static int gCurrentLine = 0;

/* ═══════════════════════════════════════════════
   DRAW HELPERS
   ═══════════════════════════════════════════════ */

static void drawHeader(const char *title)
{
    gfx_SetColor(COL_NAVY);
    gfx_FillRectangle_NoClip(0, 0, SCR_W, HDR_H);
    gfx_SetColor(COL_ORANGE);
    gfx_FillRectangle_NoClip(0, HDR_H - 2, SCR_W, 2);
    int titleWidth = gfx_GetStringWidth(title);
    gfx_SetTextFGColor(COL_YELLOW);
    gfx_SetTextBGColor(COL_NAVY);
    gfx_PrintStringXY(title, (SCR_W - titleWidth) / 2, (HDR_H - 8) / 2);
}

static void drawFooter(const char *hint)
{
    int fy = SCR_H - FTR_H;
    gfx_SetColor(COL_GRAY);
    gfx_FillRectangle_NoClip(0, fy, SCR_W, 1);
    gfx_SetColor(COL_NAVY);
    gfx_FillRectangle_NoClip(0, fy + 1, SCR_W, FTR_H - 1);
    gfx_SetTextFGColor(COL_LTGRAY);
    gfx_SetTextBGColor(COL_NAVY);
    gfx_PrintStringXY(hint, LMARGIN, fy + (FTR_H - 8) / 2);
}

static void startScreen(const char *title, const char *footerHint)
{
    gfx_FillScreen(COL_WHITE);
    drawHeader(title);
    drawFooter(footerHint);
    gCurrentLine = 0;
}

static void blit(void) { gfx_BlitBuffer(); }

/* ─────────────────────────────────────────────
   Body-line printing helpers
   ───────────────────────────────────────────── */

static void printLine(const char *text, uint8_t colour)
{
    if (gCurrentLine >= MAX_LINES) return;
    int y = BODY_TOP + gCurrentLine * LINE_H;
    gfx_SetTextFGColor(colour);
    gfx_SetTextBGColor(COL_WHITE);
    gfx_PrintStringXY(text, LMARGIN, y);
    gCurrentLine++;
}

static void printLineIndented(const char *text, uint8_t colour, int extraX)
{
    if (gCurrentLine >= MAX_LINES) return;
    int y = BODY_TOP + gCurrentLine * LINE_H;
    gfx_SetTextFGColor(colour);
    gfx_SetTextBGColor(COL_WHITE);
    gfx_PrintStringXY(text, LMARGIN + extraX, y);
    gCurrentLine++;
}

static void printSubheader(const char *label)
{
    if (gCurrentLine >= MAX_LINES) return;
    int y = BODY_TOP + gCurrentLine * LINE_H;
    gfx_SetColor(COL_LTGRAY);
    gfx_FillRectangle_NoClip(0, y - 1, SCR_W, LINE_H + 1);
    gfx_SetTextFGColor(COL_NAVY);
    gfx_SetTextBGColor(COL_LTGRAY);
    gfx_PrintStringXY(label, LMARGIN, y);
    gCurrentLine++;
}

static void printDivider(void)
{
    if (gCurrentLine >= MAX_LINES) return;
    int y = BODY_TOP + gCurrentLine * LINE_H + LINE_H / 2;
    gfx_SetColor(COL_GRAY);
    gfx_FillRectangle_NoClip(LMARGIN, y, SCR_W - LMARGIN * 2, 1);
    gCurrentLine++;
}

static void printBlank(void)
{
    if (gCurrentLine < MAX_LINES) gCurrentLine++;
}

/* ═══════════════════════════════════════════════
   KEY / WAIT HELPERS
   ═══════════════════════════════════════════════ */

static uint8_t waitForKey(void)
{
    uint8_t key;
    do { key = os_GetCSC(); } while (key == 0);
    return key;
}

static void waitContinue(void)
{
    drawFooter("[ENTER] Back to Menu");
    blit();
    uint8_t key;
    do { key = waitForKey(); } while (key != sk_Enter && key != sk_Clear);
}

/* ═══════════════════════════════════════════════
   CANCELLATION FLAG
   ─────────────────────────────────────────────
   gUserCancelled is set to true by inputNumber()
   or inputOperator() when [CLEAR] is pressed.

   Every solver:
     1. calls RESET_CANCEL() at the very top
     2. places CHECK_CANCEL on the same line as
        (or the line immediately after) every
        inputNumber() / inputOperator() call.

   CHECK_CANCEL evaluates to an immediate return
   from the calling solver, dropping back to the
   sub-menu loop cleanly.
   ═══════════════════════════════════════════════ */

static bool gUserCancelled = false;

#define RESET_CANCEL()  (gUserCancelled = false)
#define CHECK_CANCEL    do { if (gUserCancelled) return; } while (0)

/* ═══════════════════════════════════════════════
   NUMBER INPUT
   [CLEAR] sets gUserCancelled = true and returns
   0.0 so the caller's CHECK_CANCEL fires.
   ═══════════════════════════════════════════════ */

#define MAX_INPUT_LEN 18

/*
 * inputNumber — enhanced for sequences & series.
 *
 * New keys during entry:
 *   [÷]  fraction mode — type p, press ÷, type q, press ENTER → p/q
 *   [^]  sqrt mode     — optionally type coefficient, press ^,
 *                        type integer radicand, press ENTER
 *                        e.g. "3 ^ 2 ENTER" = 3*sqrt(2) ≈ 4.2426
 *                             "^ 2 ENTER"   = sqrt(2)   ≈ 1.4142
 *   [DEL] in sub-mode backs out one char; if sub-buffer empty,
 *         exits back to the main number.
 *
 * All other existing behaviour (digits, decimal, neg, CLEAR, ENTER)
 * is completely unchanged.
 */
static double inputNumber(const char *prompt)
{
    char numBuf[MAX_INPUT_LEN + 1]; /* main value / numerator / sqrt coeff */
    char auxBuf[10 + 1];            /* denominator OR sqrt radicand        */
    int  numLen = 0, auxLen = 0;
    bool hasDot = false;
    bool fracMode = false;          /* true after ÷ pressed  */
    bool sqrtMode = false;          /* true after ^ pressed  */
    int  savedLine = gCurrentLine;

    numBuf[0] = '\0';
    auxBuf[0] = '\0';

    for (;;) {
        /* ── Redraw row ── */
        int rowY = BODY_TOP + savedLine * LINE_H;
        gfx_SetColor(COL_WHITE);
        gfx_FillRectangle_NoClip(0, rowY - 1, SCR_W, LINE_H + 2);

        int promptW = gfx_GetStringWidth(prompt);
        gfx_SetTextFGColor(COL_NAVY);
        gfx_SetTextBGColor(COL_WHITE);
        gfx_PrintStringXY(prompt, LMARGIN, rowY);

        /* Build display string based on current mode */
        char displayBuf[MAX_INPUT_LEN + 18];
        if (sqrtMode) {
            if (numLen > 0)
                snprintf(displayBuf, sizeof(displayBuf),
                         "%s*sqrt(%s_)", numBuf, auxBuf);
            else
                snprintf(displayBuf, sizeof(displayBuf),
                         "sqrt(%s_)", auxBuf);
        } else if (fracMode) {
            snprintf(displayBuf, sizeof(displayBuf),
                     "%s/%s_", numBuf, auxBuf);
        } else {
            snprintf(displayBuf, sizeof(displayBuf), "%s_", numBuf);
        }

        int valueX = LMARGIN + promptW + 4;
        gfx_SetTextFGColor(COL_BLACK);
        gfx_SetTextBGColor(COL_WHITE);
        gfx_PrintStringXY(displayBuf, valueX, rowY);

        gfx_SetColor(COL_GRAY);
        gfx_FillRectangle_NoClip(valueX, rowY + 9,
                                  SCR_W - valueX - LMARGIN, 1);
        blit();

        uint8_t key = waitForKey();

        /* ── Cancel ── */
        if (key == sk_Clear) {
            gUserCancelled = true;
            gCurrentLine   = savedLine + 1;
            return 0.0;
        }

        /* ── Confirm ── */
        if (key == sk_Enter) {
            gCurrentLine = savedLine + 1;
            if (sqrtMode) {
                double coeff = (numLen > 0) ? atof(numBuf) : 1.0;
                int    rad   = (auxLen > 0) ? abs((int)atof(auxBuf)) : 1;
                return coeff * sqrt((double)rad);
            }
            if (fracMode) {
                double num = (numLen > 0) ? atof(numBuf) : 0.0;
                double den = (auxLen > 0) ? atof(auxBuf) : 1.0;
                if (fabs(den) < 1e-15) den = 1.0;
                return num / den;
            }
            return (numLen == 0) ? 0.0 : atof(numBuf);
        }

        /* ── Backspace ── */
        if (key == sk_Del) {
            if (sqrtMode || fracMode) {
                if (auxLen > 0) {
                    auxBuf[--auxLen] = '\0';
                } else {
                    /* exit sub-mode, return to main number editing */
                    fracMode = sqrtMode = false;
                }
            } else if (numLen > 0) {
                if (numBuf[numLen - 1] == '.') hasDot = false;
                numBuf[--numLen] = '\0';
            }
            continue;
        }

        /* ── Enter fraction mode: press ÷ after typing numerator ── */
        if (key == sk_Div && !fracMode && !sqrtMode && numLen > 0) {
            fracMode  = true;
            auxLen    = 0;
            auxBuf[0] = '\0';
            continue;
        }

        /* ── Enter sqrt mode: press ^ (sk_Power) ── */
        if (key == sk_Power && !fracMode && !sqrtMode) {
            sqrtMode  = true;
            auxLen    = 0;
            auxBuf[0] = '\0';
            continue;
        }

        /* ── Negative toggle (main buffer only) ── */
        if (key == sk_Chs && !fracMode && !sqrtMode) {
            if (numLen > 0 && numBuf[0] == '-') {
                memmove(numBuf, numBuf + 1, (size_t)numLen);
                numLen--;
            } else if (numLen < MAX_INPUT_LEN - 1) {
                memmove(numBuf + 1, numBuf, (size_t)(numLen + 1));
                numBuf[0] = '-';
                numLen++;
            }
            continue;
        }

        /* ── Decimal point ──
           Allowed in main buffer and fraction denominator, not in sqrt radicand */
        if (key == sk_DecPnt && !sqrtMode) {
            if (!fracMode && !hasDot && numLen < MAX_INPUT_LEN - 1) {
                if (numLen == 0 || numBuf[numLen - 1] == '-') {
                    numBuf[numLen++] = '0'; numBuf[numLen] = '\0';
                }
                numBuf[numLen++] = '.'; numBuf[numLen] = '\0';
                hasDot = true;
            } else if (fracMode && auxLen < 9) {
                /* only add decimal once to denominator */
                bool auxHasDot = false;
                for (int i = 0; i < auxLen; i++)
                    if (auxBuf[i] == '.') { auxHasDot = true; break; }
                if (!auxHasDot) {
                    if (auxLen == 0) { auxBuf[auxLen++] = '0'; auxBuf[auxLen] = '\0'; }
                    auxBuf[auxLen++] = '.'; auxBuf[auxLen] = '\0';
                }
            }
            continue;
        }

        /* ── Digit keys ── */
        char digitChar = '\0';
        switch (key) {
            case sk_0: digitChar = '0'; break;
            case sk_1: digitChar = '1'; break;
            case sk_2: digitChar = '2'; break;
            case sk_3: digitChar = '3'; break;
            case sk_4: digitChar = '4'; break;
            case sk_5: digitChar = '5'; break;
            case sk_6: digitChar = '6'; break;
            case sk_7: digitChar = '7'; break;
            case sk_8: digitChar = '8'; break;
            case sk_9: digitChar = '9'; break;
            default:   break;
        }
        if (digitChar) {
            if ((fracMode || sqrtMode) && auxLen < 9) {
                auxBuf[auxLen++] = digitChar;
                auxBuf[auxLen]   = '\0';
            } else if (!fracMode && !sqrtMode && numLen < MAX_INPUT_LEN - 1) {
                numBuf[numLen++] = digitChar;
                numBuf[numLen]   = '\0';
            }
        }
    }
}
/*
 * Integer-only input — builds a uint32_t directly from
 * digit keys with no floating-point conversion at any step.
 * Sets gUserCancelled and returns 0 if CLEAR is pressed.
 * Returns false (via out-param) if the entered value would
 * overflow uint32_t (max 4294967295).
 */
static uint32_t inputUInt32(const char *prompt, bool *overflowed)
{
    char    inputBuf[11];   /* max 10 digits for uint32_t + NUL */
    int     inputLen  = 0;
    int     savedLine = gCurrentLine;
    *overflowed = false;

    inputBuf[0] = '\0';

    for (;;) {
        int rowY = BODY_TOP + savedLine * LINE_H;
        gfx_SetColor(COL_WHITE);
        gfx_FillRectangle_NoClip(0, rowY - 1, SCR_W, LINE_H + 2);

        int promptW = gfx_GetStringWidth(prompt);
        gfx_SetTextFGColor(COL_NAVY);
        gfx_SetTextBGColor(COL_WHITE);
        gfx_PrintStringXY(prompt, LMARGIN, rowY);

        char displayBuf[14];
        snprintf(displayBuf, sizeof(displayBuf), "%s_", inputBuf);
        gfx_SetTextFGColor(COL_BLACK);
        gfx_SetTextBGColor(COL_WHITE);
        int valueX = LMARGIN + promptW + 4;
        gfx_PrintStringXY(displayBuf, valueX, rowY);

        gfx_SetColor(COL_GRAY);
        gfx_FillRectangle_NoClip(valueX, rowY + 9,
                                  SCR_W - valueX - LMARGIN, 1);
        blit();

        uint8_t key = waitForKey();

        if (key == sk_Clear) {
            gUserCancelled = true;
            gCurrentLine   = savedLine + 1;
            return 0;
        }
        if (key == sk_Enter) {
            gCurrentLine = savedLine + 1;
            if (inputLen == 0) return 0;

            /* Parse digit-by-digit, checking overflow at every step */
            uint32_t result = 0;
            for (int i = 0; i < inputLen; i++) {
                uint32_t digit = (uint32_t)(inputBuf[i] - '0');
                /* Overflow check: result*10 + digit > 4294967295 */
                if (result > (4294967295UL - digit) / 10UL) {
                    *overflowed = true;
                    return 0;
                }
                result = result * 10UL + digit;
            }
            return result;
        }
        if (key == sk_Del && inputLen > 0) {
            inputBuf[--inputLen] = '\0';
            continue;
        }

        /* Digit keys only — no decimal, no negative */
        char digitChar = '\0';
        switch (key) {
            case sk_0: digitChar = '0'; break;
            case sk_1: digitChar = '1'; break;
            case sk_2: digitChar = '2'; break;
            case sk_3: digitChar = '3'; break;
            case sk_4: digitChar = '4'; break;
            case sk_5: digitChar = '5'; break;
            case sk_6: digitChar = '6'; break;
            case sk_7: digitChar = '7'; break;
            case sk_8: digitChar = '8'; break;
            case sk_9: digitChar = '9'; break;
            default:   break;
        }
        /* Max 10 digits (4294967295 is 10 digits) */
        if (digitChar && inputLen < 10) {
            inputBuf[inputLen++] = digitChar;
            inputBuf[inputLen]   = '\0';
        }
    }
}

/* ═══════════════════════════════════════════════
   OPERATOR SELECTION
   Returns 0=<  1=<=  2=>  3=>=
   [CLEAR] sets gUserCancelled = true, returns -1.
   ═══════════════════════════════════════════════ */

static int inputOperator(const char *promptLabel)
{
    int labelRow  = gCurrentLine;
    int choiceRow = gCurrentLine + 1;
    gCurrentLine += 2;

    gfx_SetTextFGColor(COL_NAVY);
    gfx_SetTextBGColor(COL_WHITE);
    gfx_PrintStringXY(promptLabel, LMARGIN, BODY_TOP + labelRow * LINE_H);

    gfx_SetTextFGColor(COL_ORANGE);
    gfx_SetTextBGColor(COL_WHITE);
    gfx_PrintStringXY("1:<  2:<=  3:>  4:>=  5:=", LMARGIN + 12, BODY_TOP + choiceRow * LINE_H);
    blit();

    for (;;) {
        uint8_t key = waitForKey();
        if (key == sk_Clear) { gUserCancelled = true; return -1; }
        if (key == sk_1) return 0;
        if (key == sk_2) return 1;
        if (key == sk_3) return 2;
        if (key == sk_4) return 3;
        if (key == sk_5) return 4;
    }
}

/* ═══════════════════════════════════════════════
   NUMBER FORMATTING
   ═══════════════════════════════════════════════ */

static void formatNumber(char *outBuf, size_t bufSize, double value)
{
    if (fabs(value) < MATH_EPS) {
        snprintf(outBuf, bufSize, "0");
    } else if (fabs(value - round(value)) < MATH_EPS * (1.0 + fabs(value))) {
        snprintf(outBuf, bufSize, "%.0f", round(value));
    } else {
        snprintf(outBuf, bufSize, "%.5g", value);
    }
}
/* ═══════════════════════════════════════════════
   FRACTION / SQRT DISPLAY UTILITIES
   ═══════════════════════════════════════════════ */

/*
 * Best rational p/q for val, with |q| <= maxDen,
 * via continued fractions.
 */
static void toFraction(double val, long *num, long *den, long maxDen)
{
    if (fabs(val) < MATH_EPS) { *num = 0; *den = 1; return; }
    int    negative = (val < 0.0);
    double x        = fabs(val);
    long   h0 = 1, h1 = 0, k0 = 0, k1 = 1;

    for (int i = 0; i < 64; i++) {
        long   a  = (long)x;
        long   h2 = a * h0 + h1;
        long   k2 = a * k0 + k1;
        if (k2 > maxDen || k2 < 0) break;
        h1 = h0; h0 = h2;
        k1 = k0; k0 = k2;
        double rem = x - (double)a;
        if (rem < 1e-12) break;
        x = 1.0 / rem;
    }
    if (k0 == 0) { k0 = 1; h0 = (long)fabs(val); }
    *num = negative ? -h0 : h0;
    *den = k0;
}

/*
 * Format as reduced fraction p/q if |error| < 1e-8,
 * otherwise fall back to formatNumber decimal.
 */
static void formatFraction(char *buf, size_t sz, double val)
{
    long n, d;
    toFraction(val, &n, &d, 9999);
    if (d > 0 && fabs((double)n / (double)d - val) < 1e-8 * (1.0 + fabs(val))) {
        if (d == 1) snprintf(buf, sz, "%ld",       n);
        else        snprintf(buf, sz, "%ld/%ld", n, d);
    } else {
        formatNumber(buf, sz, val);
    }
}

/*
 * Format val as  p*sqrt(k)/q  when a clean match is found
 * (radicand in {2,3,5,6,7,10,11,13}, coefficient denom <= 99).
 * Falls back to formatFraction, then formatNumber.
 *
 * Examples:
 *   sqrt(2)/2  →  "sqrt(2)/2"
 *   3*sqrt(5)  →  "3*sqrt(5)"
 *   0.75       →  "3/4"
 */
static void formatSqrtForm(char *buf, size_t sz, double val)
{
    if (fabs(val) < MATH_EPS) { snprintf(buf, sz, "0"); return; }

    /* Try plain fraction first */
    long fn, fd;
    toFraction(val, &fn, &fd, 9999);
    if (fd > 0 && fabs((double)fn / (double)fd - val) < 1e-9 * (1.0 + fabs(val))) {
        formatFraction(buf, sz, val); return;
    }

    /* Try  (rn/rd) * sqrt(k)  for small radicands */
    static const int kTab[] = { 2, 3, 5, 6, 7, 10, 11, 13, 14, 15, 0 };
    for (int i = 0; kTab[i]; i++) {
        int    k  = kTab[i];
        double sq = sqrt((double)k);
        double r  = val / sq;
        long   rn, rd;
        toFraction(r, &rn, &rd, 99);
        if (rd <= 0) continue;
        if (fabs((double)rn / (double)rd * sq - val) < 1e-7 * (1.0 + fabs(val))) {
            if (rd == 1) {
                if      (rn ==  1) snprintf(buf, sz, "sqrt(%d)", k);
                else if (rn == -1) snprintf(buf, sz, "-sqrt(%d)", k);
                else               snprintf(buf, sz, "%ld*sqrt(%d)", rn, k);
            } else {
                if      (rn ==  1) snprintf(buf, sz, "sqrt(%d)/%ld", k, rd);
                else if (rn == -1) snprintf(buf, sz, "-sqrt(%d)/%ld", k, rd);
                else               snprintf(buf, sz, "%ld*sqrt(%d)/%ld", rn, k, rd);
            }
            return;
        }
    }

    formatNumber(buf, sz, val); /* decimal fallback */
}


/* ═══════════════════════════════════════════════
   MENU ENGINE
   ═══════════════════════════════════════════════ */

static int showMenu(const char *title, const char *options[], int optionCount)
{
    for (;;) {
        startScreen(title, "[#] Select    [CLEAR] Back");

        for (int i = 0; i < optionCount && i < MAX_LINES; i++) {
            int     rowY  = BODY_TOP + i * LINE_H;
            uint8_t rowBg = (i % 2 == 1) ? COL_LTGRAY : COL_WHITE;

            if (i % 2 == 1) {
                gfx_SetColor(COL_LTGRAY);
                gfx_FillRectangle_NoClip(0, rowY - 1, SCR_W, LINE_H + 1);
            }
            char numBuf[4];
            snprintf(numBuf, sizeof(numBuf), "%d.", i + 1);
            gfx_SetTextFGColor(COL_ORANGE);
            gfx_SetTextBGColor(rowBg);
            gfx_PrintStringXY(numBuf, LMARGIN, rowY);
            gfx_SetTextFGColor(COL_BLACK);
            gfx_SetTextBGColor(rowBg);
            gfx_PrintStringXY(options[i], LMARGIN + 26, rowY);
        }
        blit();

        uint8_t key = waitForKey();
        if (key == sk_Clear) return -1;

        int sel = -1;
        switch (key) {
            case sk_1: sel = 0; break;
            case sk_2: sel = 1; break;
            case sk_3: sel = 2; break;
            case sk_4: sel = 3; break;
            case sk_5: sel = 4; break;
            case sk_6: sel = 5; break;
            case sk_7: sel = 6; break;
            case sk_8: sel = 7; break;
            case sk_9: sel = 8; break;
            default:   break;
        }
        if (sel >= 0 && sel < optionCount) return sel;
    }
}

/* ═══════════════════════════════════════════════
   SECTION 1 — EQUATIONS & INEQUALITIES
   ═══════════════════════════════════════════════ */

static void solveLinearEquation(void)
{
    RESET_CANCEL();
    startScreen("LINEAR EQUATION", "[CLEAR] Back");
    printSubheader("Ax + B = C");
    printBlank();

    double coeffA    = inputNumber("A = "); CHECK_CANCEL;
    double constantB = inputNumber("B = "); CHECK_CANCEL;
    double constantC = inputNumber("C = "); CHECK_CANCEL;
    printDivider();

    char numBuf[20], lineBuf[48];

    if (fabs(coeffA) < MATH_EPS) {
        bool isTrue = fabs(constantB - constantC) < MATH_EPS;
        printLine(isTrue ? "All Real Numbers" : "No Solution",
                  isTrue ? COL_GREEN : COL_RED);
    } else {
        double solutionX = (constantC - constantB) / coeffA;
        formatNumber(numBuf, sizeof(numBuf), solutionX);
        snprintf(lineBuf, sizeof(lineBuf), "x = %s", numBuf);
        printLine(lineBuf, COL_GREEN);
    }
    waitContinue();
}

static void solveQuadraticEquation(void)
{
    RESET_CANCEL();
    startScreen("QUADRATIC EQUATION", "[CLEAR] Back");
    printSubheader("Ax^2 + Bx + C = 0");
    printBlank();

    double coeffA = inputNumber("A = "); CHECK_CANCEL;
    double coeffB = inputNumber("B = "); CHECK_CANCEL;
    double coeffC = inputNumber("C = "); CHECK_CANCEL;
    printDivider();

    char n1[20], n2[20], lineBuf[52];

    if (fabs(coeffA) < MATH_EPS) {
        printLine("A=0: reduces to linear", COL_ORANGE);
        if (fabs(coeffB) > MATH_EPS) {
            formatNumber(n1, sizeof(n1), -coeffC / coeffB);
            snprintf(lineBuf, sizeof(lineBuf), "x = %s", n1);
            printLine(lineBuf, COL_GREEN);
        } else {
            bool trivTrue = fabs(coeffC) < MATH_EPS;
            printLine(trivTrue ? "All Real Numbers" : "No Solution",
                      trivTrue ? COL_GREEN : COL_RED);
        }
        waitContinue();
        return;
    }

    double disc  = coeffB * coeffB - 4.0 * coeffA * coeffC;
    double vertX = -coeffB / (2.0 * coeffA);
    double vertY = coeffA * vertX * vertX + coeffB * vertX + coeffC;

    formatNumber(n1, sizeof(n1), disc);
    snprintf(lineBuf, sizeof(lineBuf), "Discriminant = %s", n1);
    printLine(lineBuf, COL_BLACK);

    if (disc > MATH_EPS) {
        double sqrtD = sqrt(disc);
        double root1 = (-coeffB - sqrtD) / (2.0 * coeffA);
        double root2 = (-coeffB + sqrtD) / (2.0 * coeffA);
        printLine("Two Real Roots:", COL_ORANGE);
        formatNumber(n1, sizeof(n1), root1);
        snprintf(lineBuf, sizeof(lineBuf), "  x1 = %s", n1);
        printLine(lineBuf, COL_GREEN);
        formatNumber(n1, sizeof(n1), root2);
        snprintf(lineBuf, sizeof(lineBuf), "  x2 = %s", n1);
        printLine(lineBuf, COL_GREEN);
    } else if (fabs(disc) <= MATH_EPS) {
        printLine("Repeated Root:", COL_ORANGE);
        formatNumber(n1, sizeof(n1), vertX);
        snprintf(lineBuf, sizeof(lineBuf), "  x = %s", n1);
        printLine(lineBuf, COL_GREEN);
    } else {
        double imagPart = sqrt(-disc) / (2.0 * fabs(coeffA));
        printLine("Complex Roots:", COL_ORANGE);
        formatNumber(n1, sizeof(n1), vertX);
        formatNumber(n2, sizeof(n2), imagPart);
        snprintf(lineBuf, sizeof(lineBuf), "  %s +/- %si", n1, n2);
        printLine(lineBuf, COL_GREEN);
    }

    formatNumber(n1, sizeof(n1), vertX);
    formatNumber(n2, sizeof(n2), vertY);
    snprintf(lineBuf, sizeof(lineBuf), "Vertex: (%s, %s)", n1, n2);
    printLine(lineBuf, COL_BLACK);
    waitContinue();
}

/* Flip operator index when dividing by a negative number */
static int flipOp(int op)
{
    const int kFlipTable[4] = {2, 3, 0, 1};
    return kFlipTable[op];
}

static void solveLinearInequality(void)
{
    RESET_CANCEL();
    startScreen("LINEAR INEQUALITY", "[CLEAR] Back");
    printSubheader("Ax + B  op  C");
    printBlank();

    double coeffA    = inputNumber("A = "); CHECK_CANCEL;
    double constantB = inputNumber("B = "); CHECK_CANCEL;
    double constantC = inputNumber("C = "); CHECK_CANCEL;
    int    op        = inputOperator("Operator:"); CHECK_CANCEL;

    printDivider();

    char numBuf[20], lineBuf[40];

    if (fabs(coeffA) < MATH_EPS) {
        bool satisfied;
        switch (op) {
            case 0: satisfied = (constantB <  constantC); break;
            case 1: satisfied = (constantB <= constantC); break;
            case 2: satisfied = (constantB >  constantC); break;
            default:satisfied = (constantB >= constantC); break;
        }
        printLine(satisfied ? "All Real Numbers" : "No Solution",
                  satisfied ? COL_GREEN : COL_RED);
        waitContinue();
        return;
    }

    double boundaryX = (constantC - constantB) / coeffA;
    int    resultOp  = (coeffA < 0.0) ? flipOp(op) : op;

    formatNumber(numBuf, sizeof(numBuf), boundaryX);
    snprintf(lineBuf, sizeof(lineBuf), "x %s %s", kOpStr[resultOp], numBuf);
    printLine(lineBuf, COL_GREEN);
    waitContinue();
}
/* Used only by solveThreeWayInequality — equal sign not valid here */
static int inputOperatorIneqOnly(const char *promptLabel)
{
    for (;;) {
        int op = inputOperator(promptLabel);
        if (gUserCancelled) return -1;
        if (op == 4) {
            /* flash a warning on the footer, then re-ask */
            drawFooter("= not valid here. Use <, <=, >, >=");
            blit();
            /* small delay so the user can read it */
            for (volatile int i = 0; i < 200000; i++);
            /* re-draw the screen minus the footer so the prompt reappears */
            drawFooter("[CLEAR] Back");
            continue;
        }
        return op;
    }
}
static void solveThreeWayInequality(void)
{
    RESET_CANCEL();
    startScreen("THREE-WAY INEQUALITY", "[CLEAR] Back");
    printSubheader("C1 op1  Ax+B  op2 C2");
    printBlank();

    double leftBound  = inputNumber("C1 = ");             CHECK_CANCEL;
    int leftOp  = inputOperatorIneqOnly("op1 (C1 op1 Ax+B):"); CHECK_CANCEL;
    double coeffA     = inputNumber("A = ");              CHECK_CANCEL;
    double constantB  = inputNumber("B = ");              CHECK_CANCEL;
    int rightOp = inputOperatorIneqOnly("op2 (Ax+B op2 C2):"); CHECK_CANCEL;
    double rightBound = inputNumber("C2 = ");             CHECK_CANCEL;

    printDivider();

    char lo[20], hi[20], lineBuf[52];

    /* A = 0: constant check only */
    if (fabs(coeffA) < MATH_EPS) {
        bool leftOk, rightOk;
        switch (leftOp) {
            case 0: leftOk  = (leftBound <  constantB); break;
            case 1: leftOk  = (leftBound <= constantB); break;
            case 2: leftOk  = (leftBound >  constantB); break;
            default:leftOk  = (leftBound >= constantB); break;
        }
        switch (rightOp) {
            case 0: rightOk = (constantB <  rightBound); break;
            case 1: rightOk = (constantB <= rightBound); break;
            case 2: rightOk = (constantB >  rightBound); break;
            default:rightOk = (constantB >= rightBound); break;
        }
        printLine((leftOk && rightOk) ? "All Real Numbers" : "No Solution",
                  (leftOk && rightOk) ? COL_GREEN : COL_RED);
        waitContinue();
        return;
    }

    /*
     * Left side  "C1 leftOp Ax+B"
     *   -> reverse operator -> "Ax+B revLeft C1"
     *   -> isolate x        -> "x revLeft (C1-B)/A"
     *   -> if A<0, flip again
     *
     * Right side "Ax+B rightOp C2"
     *   -> isolate x        -> "x rightOp (C2-B)/A"
     *   -> if A<0, flip
     */
    int    xOpLeft     = flipOp(leftOp);
    if (coeffA < 0.0) xOpLeft = flipOp(xOpLeft);
    double xBoundLeft  = (leftBound  - constantB) / coeffA;

    int    xOpRight    = rightOp;
    if (coeffA < 0.0) xOpRight = flipOp(xOpRight);
    double xBoundRight = (rightBound - constantB) / coeffA;

    bool leftIsLower  = (xOpLeft  == 2 || xOpLeft  == 3);
    bool rightIsLower = (xOpRight == 2 || xOpRight == 3);

    double finalLo, finalHi;
    bool   loStrict, hiStrict;

    if (leftIsLower && !rightIsLower) {
        finalLo  = xBoundLeft;  loStrict = (xOpLeft  == 2);
        finalHi  = xBoundRight; hiStrict = (xOpRight == 0);
    } else if (!leftIsLower && rightIsLower) {
        finalLo  = xBoundRight; loStrict = (xOpRight == 2);
        finalHi  = xBoundLeft;  hiStrict = (xOpLeft  == 0);
    } else {
        /* Both conditions point the same direction -> single half-line */
        double chosenBound;
        int    chosenOp;
        if (leftIsLower) {
            if (fabs(xBoundLeft - xBoundRight) < MATH_EPS) {
                chosenBound = xBoundLeft;
                chosenOp    = (xOpLeft == 2 || xOpRight == 2) ? 2 : 3;
            } else if (xBoundLeft > xBoundRight) {
                chosenBound = xBoundLeft;  chosenOp = xOpLeft;
            } else {
                chosenBound = xBoundRight; chosenOp = xOpRight;
            }
        } else {
            if (fabs(xBoundLeft - xBoundRight) < MATH_EPS) {
                chosenBound = xBoundLeft;
                chosenOp    = (xOpLeft == 0 || xOpRight == 0) ? 0 : 1;
            } else if (xBoundLeft < xBoundRight) {
                chosenBound = xBoundLeft;  chosenOp = xOpLeft;
            } else {
                chosenBound = xBoundRight; chosenOp = xOpRight;
            }
        }
        formatNumber(lo, sizeof(lo), chosenBound);
        snprintf(lineBuf, sizeof(lineBuf), "x %s %s", kOpStr[chosenOp], lo);
        printLine(lineBuf, COL_GREEN);
        waitContinue();
        return;
    }

    if (finalLo > finalHi + MATH_EPS) {
        printLine("No Solution", COL_RED);
        waitContinue();
        return;
    }

    if (fabs(finalLo - finalHi) < MATH_EPS) {
        if (loStrict || hiStrict) {
            printLine("No Solution", COL_RED);
        } else {
            formatNumber(lo, sizeof(lo), finalLo);
            snprintf(lineBuf, sizeof(lineBuf), "x = %s", lo);
            printLine(lineBuf, COL_GREEN);
        }
        waitContinue();
        return;
    }

    formatNumber(lo, sizeof(lo), finalLo);
    formatNumber(hi, sizeof(hi), finalHi);
    const char *loOpStr = loStrict ? "<"  : "<="; // Changed from > / >=
    const char *hiOpStr = hiStrict ? "<"  : "<=";
    char        loBrk   = loStrict ? '('  : '[';
    char        hiBrk   = hiStrict ? ')'  : ']';

    snprintf(lineBuf, sizeof(lineBuf), "%s %s x %s %s",
             lo, loOpStr, hiOpStr, hi);
    printLine(lineBuf, COL_GREEN);
    snprintf(lineBuf, sizeof(lineBuf), "%c%s, %s%c", loBrk, lo, hi, hiBrk);
    printLine(lineBuf, COL_BLACK);
    waitContinue();
}

/* ═══════════════════════════════════════════════
   SECTION 2 — ABSOLUTE VALUE HELPERS
   ═══════════════════════════════════════════════ */

/* Print solution lines for  |Ax+B| = rhs */
static void printAbsEqSolution(double A, double B, double rhs)
{
    char v1[20], v2[20], lineBuf[52];

    if (rhs < -MATH_EPS) {
        printLine("No Solution  (rhs < 0)", COL_RED);
        return;
    }
    if (fabs(A) < MATH_EPS) {
        bool ok = fabs(fabs(B) - rhs) < MATH_EPS;
        printLine(ok ? "All Real Numbers" : "No Solution",
                  ok ? COL_GREEN : COL_RED);
        return;
    }
    if (fabs(rhs) < MATH_EPS) {
        formatNumber(v1, sizeof(v1), -B / A);
        snprintf(lineBuf, sizeof(lineBuf), "x = %s", v1);
        printLine(lineBuf, COL_GREEN);
        return;
    }
    double root1 = ( rhs - B) / A;
    double root2 = (-rhs - B) / A;
    formatNumber(v1, sizeof(v1), root1);
    formatNumber(v2, sizeof(v2), root2);
    snprintf(lineBuf, sizeof(lineBuf), "x=%s  or  x=%s", v1, v2);
    printLine(lineBuf, COL_GREEN);
}

/*
 * Print solution lines for  |Ax+B| op rhs
 * opType:  0=<   1=<=   2=>   3=>=
 */
static void printAbsIneqSolution(double A, double B, double rhs, int opType)
{
    char v1[20], v2[20], lineBuf[52];
    bool isLess   = (opType == 0 || opType == 1);
    bool isStrict = (opType == 0 || opType == 2);

    if (isLess) {
        if (rhs < MATH_EPS && isStrict) {
            printLine("No Solution", COL_RED);
            return;
        }
        if (fabs(rhs) < MATH_EPS && !isStrict) {
            if (fabs(A) < MATH_EPS) {
                printLine(fabs(B) < MATH_EPS ? "All Reals" : "No Solution",
                          fabs(B) < MATH_EPS ? COL_GREEN : COL_RED);
            } else {
                formatNumber(v1, sizeof(v1), -B / A);
                snprintf(lineBuf, sizeof(lineBuf), "x = %s", v1);
                printLine(lineBuf, COL_GREEN);
            }
            return;
        }
        if (fabs(A) < MATH_EPS) {
            bool ok = isStrict ? (fabs(B) < rhs) : (fabs(B) <= rhs);
            printLine(ok ? "All Real Numbers" : "No Solution",
                      ok ? COL_GREEN : COL_RED);
            return;
        }
        double boundLo = (-rhs - B) / A;
        double boundHi = ( rhs - B) / A;
        if (boundLo > boundHi) { double t = boundLo; boundLo = boundHi; boundHi = t; }
        formatNumber(v1, sizeof(v1), boundLo);
        formatNumber(v2, sizeof(v2), boundHi);
        const char *opStr = isStrict ? "<" : "<=";
        snprintf(lineBuf, sizeof(lineBuf), "%s %s x %s %s", v1, opStr, opStr, v2);
        printLine(lineBuf, COL_GREEN);

    } else {
        if (rhs < -MATH_EPS) { printLine("All Real Numbers", COL_GREEN); return; }
        if (!isStrict && fabs(rhs) < MATH_EPS) { printLine("All Real Numbers", COL_GREEN); return; }
        if (fabs(A) < MATH_EPS) {
            bool ok = isStrict ? (fabs(B) > rhs) : (fabs(B) >= rhs);
            printLine(ok ? "All Real Numbers" : "No Solution",
                      ok ? COL_GREEN : COL_RED);
            return;
        }
        double boundLo = (-rhs - B) / A;
        double boundHi = ( rhs - B) / A;
        if (boundLo > boundHi) { double t = boundLo; boundLo = boundHi; boundHi = t; }
        formatNumber(v1, sizeof(v1), boundLo);
        formatNumber(v2, sizeof(v2), boundHi);
        const char *lopStr = isStrict ? "<"  : "<=";
        const char *ropStr = isStrict ? ">"  : ">=";
        snprintf(lineBuf, sizeof(lineBuf), "x %s %s  or  x %s %s",
                 lopStr, v1, ropStr, v2);
        printLine(lineBuf, COL_GREEN);
    }
}

/* ═══════════════════════════════════════════════
   SECTION 2 — ABSOLUTE VALUE SOLVERS
   ═══════════════════════════════════════════════ */

static void solveAbsEq(void)
{
    RESET_CANCEL();
    startScreen("|Ax + B| = C", "[CLEAR] Back");
    printSubheader("Enter A, B, C:");
    printBlank();

    double A = inputNumber("A = "); CHECK_CANCEL;
    double B = inputNumber("B = "); CHECK_CANCEL;
    double C = inputNumber("C = "); CHECK_CANCEL;
    printDivider();
    printAbsEqSolution(A, B, C);
    waitContinue();
}

static void solveAbsIneq(void)
{
    RESET_CANCEL();
    startScreen("|Ax+B|  op  C", "[CLEAR] Back");
    printSubheader("Enter A, B, C, then op:");
    printBlank();

    double A  = inputNumber("A = "); CHECK_CANCEL;
    double B  = inputNumber("B = "); CHECK_CANCEL;
    double C  = inputNumber("C = "); CHECK_CANCEL;
    int    op = inputOperator("Operator:"); CHECK_CANCEL;
    printDivider();
    if (op == 4) {
        printAbsEqSolution(A, B, C);
    } else {
        printAbsIneqSolution(A, B, C, op);
    }
    waitContinue();
}

/*
 * Nested absolute value:  ||Ax+B| - C| op D
 *
 * Let u = |Ax+B|  (always >= 0).
 *
 * LESS (< or <=):
 *   C-D < u < C+D  intersected with u>=0
 *   uHigh=C+D, uLow=C-D
 *   uHigh<=0  -> No Solution
 *   uLow<=0   -> |Ax+B| op uHigh  (single abs ineq)
 *   uLow>0    -> two symmetric sub-intervals on x-axis
 *
 * GREATER (> or >=):
 *   u < C-D  or  u > C+D  intersected with u>=0
 *   Part A: |Ax+B| < C-D  (only if C-D > 0)
 *   Part B: |Ax+B| > C+D
 */
static void solveNestedAbsIneq(void)
{
    RESET_CANCEL();
    startScreen("||Ax+B|-C|  op  D", "[CLEAR] Back");
    printSubheader("Enter A, B, C, D, then op:");
    printBlank();

    double A  = inputNumber("A = "); CHECK_CANCEL;
    double B  = inputNumber("B = "); CHECK_CANCEL;
    double C  = inputNumber("C = "); CHECK_CANCEL;
    double D  = inputNumber("D = "); CHECK_CANCEL;
    int    op = inputOperator("Operator:"); CHECK_CANCEL;

    printDivider();

/* ── Equality: ||Ax+B|-C| = D ──
 * |Ax+B| - C = +D  =>  |Ax+B| = C+D
 * |Ax+B| - C = -D  =>  |Ax+B| = C-D      */
    if (op == 4) {
        if (D < -MATH_EPS) {
        printLine("No Solution  (D < 0)", COL_RED);
        waitContinue();
        return;
    }
        double rhs1 = C + D;
        double rhs2 = C - D;

        bool has1 = (rhs1 >= -MATH_EPS);
        bool has2 = (fabs(rhs2) >= MATH_EPS) && (rhs2 >= -MATH_EPS);

        if (!has1 && !has2) {
            printLine("No Solution", COL_RED);
        } else {
            if (has1) printAbsEqSolution(A, B, rhs1);
            if (has1 && has2) printLineIndented("OR", COL_ORANGE, 4);
            if (has2) printAbsEqSolution(A, B, rhs2);
    }

    waitContinue();
    return;
    }

    bool isLess   = (op == 0 || op == 1);
    bool isStrict = (op == 0 || op == 2);
    char v1[20], v2[20], lineBuf[52];

    /* ── D < 0 ── */
    if (D < -MATH_EPS) {
        printLine(isLess ? "No Solution" : "All Real Numbers",
                  isLess ? COL_RED : COL_GREEN);
        waitContinue();
        return;
    }

    /* ── D = 0 ── */
    if (fabs(D) < MATH_EPS) {
        if (isStrict && isLess) {
            printLine("No Solution", COL_RED);
        } else if (!isStrict && isLess) {
            printLine("|Ax+B| = C:", COL_ORANGE);
            printAbsEqSolution(A, B, C);
        } else if (isStrict && !isLess) {
            printLine("All reals except:", COL_GREEN);
            printAbsEqSolution(A, B, C);
        } else {
            printLine("All Real Numbers", COL_GREEN);
        }
        waitContinue();
        return;
    }

    /* ── General case ── */

    if (isLess) {
        double uHigh = C + D;
        double uLow  = C - D;

        if (uHigh < -MATH_EPS || (isStrict && fabs(uHigh) < MATH_EPS)) {
            printLine("No Solution", COL_RED);
            waitContinue();
            return;
        }

        if (uLow <= MATH_EPS) {
            /* Lower bound is 0 — just a single abs inequality */
            printAbsIneqSolution(A, B, uHigh, isStrict ? 0 : 1);
            waitContinue();
            return;
        }

        /* uLow > 0: two sub-intervals */
        if (fabs(A) < MATH_EPS) {
            double absB   = fabs(B);
            bool   inHigh = isStrict ? (absB < uHigh) : (absB <= uHigh);
            bool   outLow = isStrict ? (absB > uLow)  : (absB >= uLow);
            printLine((inHigh && outLow) ? "All Real Numbers" : "No Solution",
                      (inHigh && outLow) ? COL_GREEN : COL_RED);
            waitContinue();
            return;
        }

        /* Boundaries from |Ax+B| = uHigh */
        double loH = (-uHigh - B) / A, hiH = ( uHigh - B) / A;
        if (loH > hiH) { double t = loH; loH = hiH; hiH = t; }

        /* Boundaries from |Ax+B| = uLow */
        double loL = (-uLow - B) / A, hiL = ( uLow - B) / A;
        if (loL > hiL) { double t = loL; loL = hiL; hiL = t; }

        char ob = isStrict ? '(' : '[';
        char cb = isStrict ? ')' : ']';

        bool seg1 = (loH < loL - MATH_EPS);
        bool seg2 = (hiL < hiH - MATH_EPS);

        if (!seg1 && !seg2) {
            printLine("No Solution", COL_RED);
            waitContinue();
            return;
        }
        if (seg1) {
            formatNumber(v1, sizeof(v1), loH);
            formatNumber(v2, sizeof(v2), loL);
            snprintf(lineBuf, sizeof(lineBuf), "%c%s, %s%c", ob, v1, v2, cb);
            printLine(lineBuf, COL_GREEN);
        }
        if (seg1 && seg2) {
            printLineIndented("OR", COL_ORANGE, 4);
        }
        if (seg2) {
            formatNumber(v1, sizeof(v1), hiL);
            formatNumber(v2, sizeof(v2), hiH);
            snprintf(lineBuf, sizeof(lineBuf), "%c%s, %s%c", ob, v1, v2, cb);
            printLine(lineBuf, COL_GREEN);
        }

    } else {
        /* Greater-than case */
        double uLow  = C - D;
        double uHigh = C + D;

        if (uHigh < -MATH_EPS) {
            printLine("All Real Numbers", COL_GREEN);
            waitContinue();
            return;
        }

        bool hasPartA = (uLow > MATH_EPS);
        int  partAOp  = isStrict ? 0 : 1;
        int  partBOp  = isStrict ? 2 : 3;

        if (hasPartA) {
            printLine("Part 1:", COL_ORANGE);
            printAbsIneqSolution(A, B, uLow, partAOp);
            printLineIndented("OR", COL_ORANGE, 4);
            printLine("Part 2:", COL_ORANGE);
        }
        printAbsIneqSolution(A, B, uHigh, partBOp);
    }

    waitContinue();
}

/* ═══════════════════════════════════════════════
   NUMBER THEORY — POLLARD'S RHO FACTORIZATION ALGORITHM o(N^1/4) runtime
   ═══════════════════════════════════════════════ */

static uint32_t gcd32(uint32_t a, uint32_t b)
{
    while (b) { uint32_t t = b; b = a % b; a = t; }
    return a;
}

static uint32_t lcm32(uint32_t a, uint32_t b)
{
    if (a == 0 || b == 0) return 0;
    return (a / gcd32(a, b)) * b;   /* divide first to avoid overflow */
}

/* nPr = n * (n-1) * ... * (n-r+1)
   Returns 0 on overflow or invalid input. */
static unsigned long long permutation(int n, int r)
{
    if (r < 0 || r > n || n < 0) return 0ULL;
    if (r == 0) return 1ULL;

    unsigned long long result = 1ULL;
    for (int i = 0; i < r; i++) {
        /* Overflow check before multiplying */
        if (result > 0xFFFFFFFFFFFFFFFFULL / (unsigned long long)(n - i)) {
            return 0ULL;   /* signal overflow */
        }
        result *= (unsigned long long)(n - i);
    }
    return result;
}

/* nCr computed iteratively — divides at each step to keep values small.
   C(n,r) = (n/1) * ((n-1)/2) * ((n-2)/3) * ... * ((n-r+1)/r)
   Each intermediate result is always an exact integer.
   Returns 0 on overflow or invalid input. */
static unsigned long long combination(int n, int r)
{
    if (r < 0 || r > n || n < 0) return 0ULL;
    if (r == 0 || r == n) return 1ULL;

    /* Use the smaller of r and n-r for fewer iterations */
    if (r > n - r) r = n - r;

    unsigned long long result = 1ULL;
    for (int i = 0; i < r; i++) {
        /* Overflow check before multiplying */
        if (result > 0xFFFFFFFFFFFFFFFFULL / (unsigned long long)(n - i)) {
            return 0ULL;   /* signal overflow */
        }
        result *= (unsigned long long)(n - i);
        result /= (unsigned long long)(i + 1);
    }
    return result;
}

static uint32_t pollardRho(uint32_t n)
{
    if (n % 2 == 0) return 2;
    uint32_t x = 2, y = 2, c = 1, d = 1;
    while (d == 1) {
        x = (uint32_t)(((uint64_t)x * x + c) % n);
        y = (uint32_t)(((uint64_t)y * y + c) % n);
        y = (uint32_t)(((uint64_t)y * y + c) % n);
        d = gcd32(x > y ? x - y : y - x, n);
    }
    if (d == n) {
        c = (c + 1) % n;
        x = 2; y = 2; d = 1;
        while (d == 1) {
            x = (uint32_t)(((uint64_t)x * x + c) % n);
            y = (uint32_t)(((uint64_t)y * y + c) % n);
            y = (uint32_t)(((uint64_t)y * y + c) % n);
            d = gcd32(x > y ? x - y : y - x, n);
        }
    }
    return (d == n) ? n : d;
}

static bool millerRabinIsPrime(uint32_t n)
{
    if (n < 2)  return false;
    if (n == 2 || n == 3 || n == 5 || n == 7) return true;
    if (n % 2 == 0) return false;
    uint32_t d = n - 1;
    int      r = 0;
    while (d % 2 == 0) { d /= 2; r++; }
    static const uint32_t witnesses[4] = {2, 3, 5, 7};
    for (int wi = 0; wi < 4; wi++) {
        uint32_t a = witnesses[wi];
        if (a >= n) continue;
        uint64_t base = a, exp = d, mod = n, result = 1;
        base %= mod;
        while (exp > 0) {
            if (exp & 1) result = result * base % mod;
            exp >>= 1;
            base = base * base % mod;
        }
        uint64_t x = result;
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int s = 0; s < r - 1; s++) {
            x = x * x % mod;
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;
    }
    return true;
}

#define MAX_FACTORS 32
static uint32_t factorList[MAX_FACTORS];
static int      factorCount = 0;

static void collectFactors(uint32_t n)
{
    if (n <= 1) return;
    if (millerRabinIsPrime(n)) {
        if (factorCount < MAX_FACTORS)
            factorList[factorCount++] = n;
        return;
    }
    uint32_t divisor = pollardRho(n);
    collectFactors(divisor);
    collectFactors(n / divisor);
}

static void sortFactors(void)
{
    for (int i = 1; i < factorCount; i++) {
        uint32_t key = factorList[i];
        int j = i - 1;
        while (j >= 0 && factorList[j] > key) {
            factorList[j + 1] = factorList[j];
            j--;
        }
        factorList[j + 1] = key;
    }
}

static bool calculateFactorization(uint32_t n)
{
    if (n < 2) return false;
    factorCount = 0;
    collectFactors(n);
    sortFactors();
    return true;
}

static void solveFactorize(void)
{
    RESET_CANCEL();
    startScreen("PRIME FACTORIZATION", "[CLEAR] Back");
    printSubheader("(Max: 4294967295");
    printBlank();

    bool overflow = false;
    uint32_t n = inputUInt32("N = ", &overflow); CHECK_CANCEL;

    if (overflow) {
        printDivider();
        printLine("Too large (max 4294967295)", COL_RED);
        waitContinue();
        return;
    }

if (!calculateFactorization(n)) {
    printLine("Enter N >= 2", COL_RED);
}

    char resultBuf[52];
    resultBuf[0] = '\0';
    int i = 0;
    bool firstTerm = true;

    while (i < factorCount) {
        uint32_t base = factorList[i];
        int      exp  = 0;
        while (i < factorCount && factorList[i] == base) { exp++; i++; }
        char term[20];
        if (exp == 1) snprintf(term, sizeof(term), "%lu", (unsigned long)base);
        else          snprintf(term, sizeof(term), "%lu^%d", (unsigned long)base, exp);
        if (!firstTerm)
            strncat(resultBuf, " * ", sizeof(resultBuf) - strlen(resultBuf) - 1);
        strncat(resultBuf, term, sizeof(resultBuf) - strlen(resultBuf) - 1);
        firstTerm = false;
    }

    char lineBuf[52];
    snprintf(lineBuf, sizeof(lineBuf), "N = %lu", (unsigned long)n);
    printLine(lineBuf, COL_BLACK);

    if (strlen(resultBuf) <= 36) {
        printLine(resultBuf, COL_GREEN);
    } else {
        char line1[52] = {0}, line2[52] = {0};
        int split = 36;
        while (split > 0 && !(resultBuf[split] == '*' && resultBuf[split-1] == ' '))
            split--;
        if (split > 0) {
            strncpy(line1, resultBuf, split - 1);
            strncpy(line2, resultBuf + split + 2, sizeof(line2) - 1);
        } else {
            strncpy(line1, resultBuf, 36);
            strncpy(line2, resultBuf + 36, sizeof(line2) - 1);
        }
        printLine(line1, COL_GREEN);
        printLine(line2, COL_GREEN);
    }

    snprintf(lineBuf, sizeof(lineBuf), "%d prime factor(s)", factorCount);
    printLine(lineBuf, COL_BLACK);
    waitContinue();
}

static void solveGCD(void)
{
    RESET_CANCEL();
    startScreen("GCD / HCF", "[CLEAR] Back");
    printSubheader("Greatest Common Divisor");
    printBlank();

    double countVal = inputNumber("How many numbers? "); CHECK_CANCEL;
    int count = (int)round(countVal);

    if (count < 2) {
        printDivider();
        printLine("Enter at least 2", COL_RED);
        waitContinue();
        return;
    }
    if (count > 10) {
        printDivider();
        printLine("Max 10 numbers", COL_RED);
        waitContinue();
        return;
    }

    uint32_t numbers[10];
    char prompt[12];
    for (int i = 0; i < count; i++) {
        snprintf(prompt, sizeof(prompt), "N%d = ", i + 1);
        double val = inputNumber(prompt); CHECK_CANCEL;
        numbers[i] = (uint32_t)fabs(round(val));
        if (numbers[i] == 0) {
            printDivider();
            printLine("0 not allowed", COL_RED);
            waitContinue();
            return;
        }
    }

    /* Accumulate GCD across all entered numbers */
    uint32_t result = numbers[0];
    for (int i = 1; i < count; i++)
        result = gcd32(result, numbers[i]);

    printDivider();

    char lineBuf[52];
    snprintf(lineBuf, sizeof(lineBuf), "GCD = %lu", (unsigned long)result);
    printLine(lineBuf, COL_GREEN);
    waitContinue();
}

static void solveLCM(void)
{
    RESET_CANCEL();
    startScreen("LCM", "[CLEAR] Back");
    printSubheader("Least Common Multiple");
    printBlank();

    double countVal = inputNumber("How many numbers? "); CHECK_CANCEL;
    int count = (int)round(countVal);

    if (count < 2) {
        printDivider();
        printLine("Enter at least 2", COL_RED);
        waitContinue();
        return;
    }
    if (count > 10) {
        printDivider();
        printLine("Max 10 numbers", COL_RED);
        waitContinue();
        return;
    }

    uint32_t numbers[10];
    char prompt[12];
    for (int i = 0; i < count; i++) {
        snprintf(prompt, sizeof(prompt), "N%d = ", i + 1);
        double val = inputNumber(prompt); CHECK_CANCEL;
        numbers[i] = (uint32_t)fabs(round(val));
        if (numbers[i] == 0) {
            printDivider();
            printLine("0 not allowed", COL_RED);
            waitContinue();
            return;
        }
    }

    /* Accumulate LCM across all entered numbers */
    uint32_t result = numbers[0];
    for (int i = 1; i < count; i++) {
        result = lcm32(result, numbers[i]);
        /* Guard against overflow — LCM can grow very fast */
        if (result == 0) {
            printDivider();
            printLine("Overflow: too large", COL_RED);
            waitContinue();
            return;
        }
    }

    printDivider();

    char lineBuf[52];
    snprintf(lineBuf, sizeof(lineBuf), "LCM = %lu", (unsigned long)result);
    printLine(lineBuf, COL_GREEN);
    waitContinue();
}

/* Computes log10 of nPr using lgamma.
   nPr = n! / (n-r)!  =>  log10(nPr) = (lgamma(n+1) - lgamma(n-r+1)) / ln10 */
static double log10Permutation(int n, int r)
{
    if (r == 0) return 0.0;
    return (lgamma((double)(n + 1)) - lgamma((double)(n - r + 1))) / log(10.0);
}

/* Computes log10 of nCr using lgamma.
   nCr = n! / (r!(n-r)!)                                                      */
static double log10Combination(int n, int r)
{
    if (r == 0 || r == n) return 0.0;
    if (r > n - r) r = n - r;
    return (lgamma((double)(n + 1))
          - lgamma((double)(r + 1))
          - lgamma((double)(n - r + 1))) / log(10.0);
}

/* Formats a log10 value as  m.mmmm x 10^e  into outBuf */
static void formatScientific(char *outBuf, size_t bufSize, double log10val)
{
    int    exponent = (int)floor(log10val);
    double mantissa = pow(10.0, log10val - (double)exponent);
    if (mantissa < 1.0 - MATH_EPS) { mantissa *= 10.0; exponent--; }
    snprintf(outBuf, bufSize, "%.4fx10^%d", mantissa, exponent);
}

static void solvePermComb(void)
{
    RESET_CANCEL();
    startScreen("PERM. & COMBINATION", "[CLEAR] Back");
    printSubheader("nPr  and  nCr");
    printBlank();

    double totalVal = inputNumber("Total (n) = "); CHECK_CANCEL;
    double takenVal = inputNumber("Taken (r) = "); CHECK_CANCEL;

    int n = (int)round(totalVal);
    int r = (int)round(takenVal);

    printDivider();

    if (n < 0 || r < 0) {
        printLine("n and r must be >= 0", COL_RED);
        waitContinue();
        return;
    }
    if (r > n) {
        printLine("r must be <= n", COL_RED);
        waitContinue();
        return;
    }

    unsigned long long npr = permutation(n, r);
    unsigned long long ncr = combination(n, r);

    char lineBuf[52];
    char sciBuf[32];

    if (npr == 0 && r != 0) {
        formatScientific(sciBuf, sizeof(sciBuf), log10Permutation(n, r));
        snprintf(lineBuf, sizeof(lineBuf), "(order) nPr~%s", sciBuf);
        printLine(lineBuf, COL_ORANGE);
    } else {
        snprintf(lineBuf, sizeof(lineBuf), "(order) nPr = %llu", npr);
        printLine(lineBuf, COL_GREEN);
    }

    if (ncr == 0 && r != 0) {
        formatScientific(sciBuf, sizeof(sciBuf), log10Combination(n, r));
        snprintf(lineBuf, sizeof(lineBuf), "(no order) nCr~%s", sciBuf);
        printLine(lineBuf, COL_ORANGE);
    } else {
        snprintf(lineBuf, sizeof(lineBuf), "(no order) nCr = %llu", ncr);
        printLine(lineBuf, COL_GREEN);
    }

    waitContinue();
}

/*
 * Binomial Theorem solver using logarithms.
 *
 * Expands (ax + b)^n and finds the coefficient of x^p.
 *
 * The general term is:
 *   T(r+1) = C(n,r) * (ax)^(n-r) * b^r
 *          = C(n,r) * a^(n-r) * b^r * x^(n-r)
 *
 * So for the x^p term:  n - r = p  =>  r = n - p
 *
 * Coefficient k = C(n,r) * a^(n-r) * b^r
 *
 * We compute log10(|k|) using:
 *   log10(C(n,r)) = Σlog10(i) for i=(n-r+1..n) - Σlog10(i) for i=(1..r)
 *   log10(|a|^(n-r)) = (n-r) * log10(|a|)
 *   log10(|b|^r)     = r     * log10(|b|)
 *
 * Sign is tracked separately.
 */
static void solveBinomialCoeff(void)
{
    RESET_CANCEL();
    startScreen("BINOMIAL THEOREM", "[CLEAR] Back");
    printSubheader("(ax + b)^n, coeff of x^p");
    printBlank();

    double a = inputNumber("a = "); CHECK_CANCEL;
    double b = inputNumber("b = "); CHECK_CANCEL;
    double nVal = inputNumber("n = "); CHECK_CANCEL;
    double pVal = inputNumber("x^p, p = "); CHECK_CANCEL;

    printDivider();

    int n = (int)round(nVal);
    int p = (int)round(pVal);

    /* Basic validation */
    if (n < 0) {
        printLine("n must be >= 0", COL_RED);
        waitContinue();
        return;
    }
    if (p < 0 || p > n) {
        printLine("p must be 0 <= p <= n", COL_RED);
        waitContinue();
        return;
    }
    if (fabs(a) < MATH_EPS) {
        printLine("a cannot be 0", COL_RED);
        waitContinue();
        return;
    }

    int r = n - p;   /* which term of the expansion */

    /* log10(C(n,r)) via log sum — avoids computing the huge integer */
    double logCnr = (lgamma((double)(n + 1))
               - lgamma((double)(r + 1))
               - lgamma((double)(n - r + 1))) / log(10.0);

    /* log10(|a|^p) and log10(|b|^r) */
    double logA = (fabs(a) < MATH_EPS) ? 0.0 : (double)p * log10(fabs(a));
    double logB = (fabs(b) < MATH_EPS) ? 0.0 : (double)r * log10(fabs(b));

    double logCoeff = logCnr + logA + logB;

    /* Determine sign:
       sign of a^p * b^r  =  sign(a)^p * sign(b)^r */
    bool aNeg = (a < 0.0);
    bool bNeg = (b < 0.0);
    bool resultNeg = (aNeg && (p % 2 != 0)) || (bNeg && (r % 2 != 0));

    /* Special case: b = 0 and r > 0 -> coefficient is 0 */
    if (fabs(b) < MATH_EPS && r > 0) {
        printLine("Coefficient = 0", COL_GREEN);
        printLine("(b=0 kills this term)", COL_BLACK);
        waitContinue();
        return;
    }

    /* Convert log10(|k|) to  m * 10^e  where 1 <= m < 10 */
    int    exponent = (int)floor(logCoeff);
    double mantissa = pow(10.0, logCoeff - (double)exponent);

    /* Floating point edge: mantissa can land just below 1.0 */
    if (mantissa < 1.0 - MATH_EPS) {
        mantissa *= 10.0;
        exponent--;
    }

    char lineBuf[52];
    char mBuf[20];

    /* Show which term was selected */
    snprintf(lineBuf, sizeof(lineBuf), "r = %d  (term %d)", r, r + 1);
    printLine(lineBuf, COL_BLACK);

    /* Show C(n,r) log value as a sanity reference */
    snprintf(lineBuf, sizeof(lineBuf), "log10|k| = %.4f", logCoeff);
    printLine(lineBuf, COL_BLACK);

    /* Scientific notation to 4 significant figures */
    snprintf(mBuf, sizeof(mBuf), "%.4f", mantissa);
    snprintf(lineBuf, sizeof(lineBuf), "k = %s%sx10^%d",
             resultNeg ? "-" : "", mBuf, exponent);
    printLine(lineBuf, COL_GREEN);

    /* If small enough, also show the exact integer */
    if (logCoeff < 15.0) {
     double exact = (resultNeg ? -1.0 : 1.0) * pow(10.0, logCoeff);
    /* Use %g to preserve decimals — shows 3.75 not 4 */
    char exactBuf[24];
    snprintf(exactBuf, sizeof(exactBuf), "%.10g", exact);
    snprintf(lineBuf, sizeof(lineBuf), "k = %s (exact)", exactBuf);
    printLine(lineBuf, COL_GREEN);
}

    waitContinue();
}

static void solvePascalTriangle(void)
{
    RESET_CANCEL();
    startScreen("PASCAL'S TRIANGLE", "[CLEAR] Back");
    printSubheader("Enter row number:");
    printBlank();

    double nVal = inputNumber("Row N = "); CHECK_CANCEL;
    int n = (int)round(nVal);

    if (n < 0) {
        printDivider();
        printLine("N must be >= 0", COL_RED);
        waitContinue();
        return;
    }
    if (n > 60) {
        printDivider();
        printLine("Max N = 60", COL_RED);
        waitContinue();
        return;
    }

    if (n <= 10) {
        /* ── Visual mode: show full triangle rows 0..N centred ── */
        startScreen("PASCAL'S TRIANGLE", "[ENTER] Done");

        for (int row = 0; row <= n; row++) {
            if (gCurrentLine >= MAX_LINES) break;

            /* Compute this row iteratively from the previous value */
            unsigned long long vals[11];
            vals[0] = 1;
            for (int k = 1; k <= row; k++)
                vals[k] = vals[k-1]
                        * (unsigned long long)(row - k + 1)
                        / (unsigned long long)k;

            /* Build the row string */
            char rowStr[52];
            rowStr[0] = '\0';
            for (int k = 0; k <= row; k++) {
                char numStr[10];
                snprintf(numStr, sizeof(numStr), "%llu", vals[k]);
                if (k > 0)
                    strncat(rowStr, " ", sizeof(rowStr) - strlen(rowStr) - 1);
                strncat(rowStr, numStr, sizeof(rowStr) - strlen(rowStr) - 1);
            }

            /* Centre on screen using pixel width */
            int strW = gfx_GetStringWidth(rowStr);
            int x    = (SCR_W - strW) / 2;
            if (x < LMARGIN) x = LMARGIN;
            int y = BODY_TOP + gCurrentLine * LINE_H;
            gfx_SetTextFGColor(COL_BLACK);
            gfx_SetTextBGColor(COL_WHITE);
            gfx_PrintStringXY(rowStr, x, y);
            gCurrentLine++;
        }
        waitContinue();

    } else {
        /*
         * ── List mode: show k = 0 .. floor(N/2) with values, paged ──
         *
         * Pascal's triangle is symmetric: C(N,k) = C(N,N-k), so
         * showing the first half is enough. The symmetry note is
         * printed on the last page.
         *
         * For N <= 60 all C(N,k) values fit in uint64, so no
         * scientific notation is needed, but the overflow guard
         * is kept as a safety net.
         */
        int halfCount    = n / 2 + 1;          /* k = 0..floor(N/2) */
        int linesPerPage = MAX_LINES - 2;       /* 2 lines for header */
        int pages        = (halfCount + linesPerPage - 1) / linesPerPage;

        for (int page = 0; page < pages; page++) {
            char titleBuf[32];
            snprintf(titleBuf, sizeof(titleBuf),
                     "ROW %d  (%d/%d)", n, page + 1, pages);
            startScreen(titleBuf, "[ENTER] Next / Done");
            printSubheader("k      C(N,k)");

            int start = page * linesPerPage;
            int end   = start + linesPerPage;
            if (end > halfCount) end = halfCount;

            for (int k = start; k < end; k++) {
                char lineBuf[52];
                unsigned long long val = combination(n, k);

                /* Overflow fallback — won't trigger for N<=60 */
                if (val == 0 && k > 0 && k < n) {
                    char sciBuf[24];
                    formatScientific(sciBuf, sizeof(sciBuf),
                                     log10Combination(n, k));
                    snprintf(lineBuf, sizeof(lineBuf),
                             "%-4d  ~%s", k, sciBuf);
                    printLine(lineBuf, COL_ORANGE);
                } else {
                    snprintf(lineBuf, sizeof(lineBuf),
                             "%-4d  %llu", k, val);
                    printLine(lineBuf, COL_GREEN);
                }
            }

            /* Symmetry reminder on last page */
            if (page == pages - 1) {
                printBlank();
                printLine("C(N,k) = C(N, N-k)", COL_ORANGE);
            }
            waitContinue();
        }
    }
}

/*
 * System of Linear Inequalities solver.
 *
 * Each inequality is entered as:  Ax + By op C
 *
 * Strategy:
 *   1. Find all intersection points of every pair of boundary lines.
 *   2. Keep only those that satisfy ALL inequalities (feasible vertices).
 *   3. Sort vertices by angle around their centroid (convex hull order).
 *   4. Apply the shoelace formula to compute the area.
 */

#define MAX_INEQ      8
#define MAX_VERTICES  64   /* C(8,2) = 28 pairs, plus axis intersections */

typedef struct {
    double x, y;
} Point2D;

/* Check whether point p satisfies inequality Ax + By op C */
static bool satisfiesIneq(double A, double B, double C, int op, Point2D p)
{
    double lhs = A * p.x + B * p.y;
    switch (op) {
        case 0: return lhs <  C - MATH_EPS;
        case 1: return lhs <= C + MATH_EPS;
        case 2: return lhs >  C + MATH_EPS;
        case 3: return lhs >= C - MATH_EPS;
        default: return false;
    }
}

/* Intersect lines A1x+B1y=C1 and A2x+B2y=C2.
   Returns true and sets p if lines are not parallel. */
static bool intersectLines(double A1, double B1, double C1,
                           double A2, double B2, double C2,
                           Point2D *p)
{
    double det = A1 * B2 - A2 * B1;
    if (fabs(det) < MATH_EPS) return false;
    p->x = (C1 * B2 - C2 * B1) / det;
    p->y = (A1 * C2 - A2 * C1) / det;
    return true;
}

/* Sort vertices by angle around centroid — puts them in convex hull order */
static void sortVerticesByAngle(Point2D *verts, int count)
{
    /* Find centroid */
    double cx = 0.0, cy = 0.0;
    for (int i = 0; i < count; i++) { cx += verts[i].x; cy += verts[i].y; }
    cx /= count; cy /= count;

    /* Bubble sort by atan2 angle — count is small so this is fine */
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            double a1 = atan2(verts[j].y   - cy, verts[j].x   - cx);
            double a2 = atan2(verts[j+1].y - cy, verts[j+1].x - cx);
            if (a1 > a2) {
                Point2D tmp = verts[j];
                verts[j]    = verts[j+1];
                verts[j+1]  = tmp;
            }
        }
    }
}

/* Shoelace formula — vertices must be in convex order */
static double polygonArea(Point2D *verts, int count)
{
    double area = 0.0;
    for (int i = 0; i < count; i++) {
        int j = (i + 1) % count;
        area += verts[i].x * verts[j].y;
        area -= verts[j].x * verts[i].y;
    }
    return fabs(area) * 0.5;
}

static void solveSystemOfInequalities(void)
{
    RESET_CANCEL();
    startScreen("SYSTEM OF INEQ.", "[CLEAR] Back");
    printSubheader("Each: Ax + By op C");
    printBlank();

    double nVal = inputNumber("# of inequalities: "); CHECK_CANCEL;
    int n = (int)round(nVal);

    if (n < 2 || n > MAX_INEQ) {
        printDivider();
        printLine("Enter 2 to 8", COL_RED);
        waitContinue();
        return;
    }

    double A[MAX_INEQ], B[MAX_INEQ], C[MAX_INEQ];
    int    op[MAX_INEQ];
    char   prompt[12];

       for (int i = 0; i < n; i++) {
        /* Fresh screen for each inequality so inputs never overflow */
        char titleBuf[28];
        snprintf(titleBuf, sizeof(titleBuf),
                 "INEQ. %d of %d", i + 1, n);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("Ax + By  op  C");
        printBlank();

        snprintf(prompt, sizeof(prompt), "A%d = ", i + 1);
        A[i] = inputNumber(prompt); CHECK_CANCEL;

        snprintf(prompt, sizeof(prompt), "B%d = ", i + 1);
        B[i] = inputNumber(prompt); CHECK_CANCEL;

        snprintf(prompt, sizeof(prompt), "C%d = ", i + 1);
        C[i] = inputNumber(prompt); CHECK_CANCEL;

        snprintf(prompt, sizeof(prompt), "op%d:", i + 1);
        op[i] = inputOperator(prompt); CHECK_CANCEL;
    }

    /* ── Find all candidate vertices ── */
    Point2D verts[MAX_VERTICES];
    int     vertCount = 0;

    for (int i = 0; i < n && vertCount < MAX_VERTICES; i++) {
        for (int j = i + 1; j < n && vertCount < MAX_VERTICES; j++) {
            Point2D p;
            if (!intersectLines(A[i], B[i], C[i],
                                A[j], B[j], C[j], &p)) continue;

            /* Reject if coordinates are unreasonably large */
            if (fabs(p.x) > 1e6 || fabs(p.y) > 1e6) continue;

            /* Check this point satisfies every inequality */
            bool feasible = true;
            for (int k = 0; k < n; k++) {
                if (!satisfiesIneq(A[k], B[k], C[k], op[k], p)) {
                    feasible = false;
                    break;
                }
            }
            if (!feasible) continue;

            /* Deduplicate — skip if already in list */
            bool duplicate = false;
            for (int d = 0; d < vertCount; d++) {
                if (fabs(verts[d].x - p.x) < MATH_EPS &&
                    fabs(verts[d].y - p.y) < MATH_EPS) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) verts[vertCount++] = p;
        }
    }

    printDivider();

    if (vertCount < 3) {
        printLine("No bounded region found", COL_RED);
        printLine("Check inequalities", COL_BLACK);
        waitContinue();
        return;
    }

    sortVerticesByAngle(verts, vertCount);
    double area = polygonArea(verts, vertCount);

    /* ── Display vertices ── */
    char lineBuf[52];
    snprintf(lineBuf, sizeof(lineBuf), "Vertices (%d):", vertCount);
    printLine(lineBuf, COL_ORANGE);

    for (int i = 0; i < vertCount && gCurrentLine < MAX_LINES - 2; i++) {
        char xBuf[12], yBuf[12];
        formatNumber(xBuf, sizeof(xBuf), verts[i].x);
        formatNumber(yBuf, sizeof(yBuf), verts[i].y);
        snprintf(lineBuf, sizeof(lineBuf), "  (%s, %s)", xBuf, yBuf);
        printLine(lineBuf, COL_BLACK);
    }

    if (vertCount > MAX_LINES - 3) {
        printLine("  (more — see above)", COL_GRAY);
    }

    /* ── Display area ── */
    printDivider();
    char areaBuf[20];
    formatNumber(areaBuf, sizeof(areaBuf), area);
    snprintf(lineBuf, sizeof(lineBuf), "Area = %s sq units", areaBuf);
    printLine(lineBuf, COL_GREEN);

    waitContinue();
}

/*
 * Number Base Conversion
 *
 * Supports bases 2 through 36.
 * Digits 0-9 are entered as numbers.
 * For bases > 10, the user enters digit values numerically
 * separated by spaces — e.g. base-16 number "1A3" is
 * entered as three digits: 1, 10, 3.
 *
 * Two modes:
 *   1. Convert FROM any base TO base 10
 *   2. Convert FROM base 10 TO any base
 */

/* Convert a base-10 value to a digit character: 0-9, A-Z */
static char digitToChar(int d)
{
    if (d < 10) return '0' + d;
    return 'A' + (d - 10);
}

static void solveBaseToDecimal(void)
{
    RESET_CANCEL();
    startScreen("BASE -> DECIMAL", "[CLEAR] Back");
    printSubheader("Convert base N -> base 10");
    printBlank();

    double baseVal = inputNumber("Source base = "); CHECK_CANCEL;
    int base = (int)round(baseVal);

    if (base < 2 || base > 36) {
        printDivider();
        printLine("Base must be 2 to 36", COL_RED);
        waitContinue();
        return;
    }

    double digitCountVal = inputNumber("# of digits = "); CHECK_CANCEL;
    int digitCount = (int)round(digitCountVal);

    if (digitCount < 1 || digitCount > 12) {
        printDivider();
        printLine("1 to 12 digits only", COL_RED);
        waitContinue();
        return;
    }

    /* Collect each digit on a fresh screen */
    int digits[12];
    for (int i = 0; i < digitCount; i++) {
        char titleBuf[28];
        snprintf(titleBuf, sizeof(titleBuf),
                 "DIGIT %d of %d (base %d)", i + 1, digitCount, base);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("Enter digit value 0-N");
        printBlank();

        /* Show digits entered so far */
        for (int j = 0; j < i; j++) {
            char prevBuf[20];
            snprintf(prevBuf, sizeof(prevBuf),
                     "d%d = %c", j + 1, digitToChar(digits[j]));
            printLine(prevBuf, COL_GRAY);
        }

        char prompt[12];
        snprintf(prompt, sizeof(prompt), "d%d = ", i + 1);
        double dVal = inputNumber(prompt); CHECK_CANCEL;
        int d = (int)round(dVal);

        if (d < 0 || d >= base) {
            printDivider();
            char errBuf[32];
            snprintf(errBuf, sizeof(errBuf),
                     "Digit must be 0 to %d", base - 1);
            printLine(errBuf, COL_RED);
            waitContinue();
            return;
        }
        digits[i] = d;
    }

    /* Convert: most significant digit first */
    unsigned long long result = 0;
    bool overflow = false;
    for (int i = 0; i < digitCount; i++) {
        if (result > (0xFFFFFFFFFFFFFFFFULL - digits[i]) / (unsigned long long)base) {
            overflow = true;
            break;
        }
        result = result * (unsigned long long)base + (unsigned long long)digits[i];
    }

    /* Build the original number string for display */
    char origBuf[16];
    origBuf[0] = '\0';
    for (int i = 0; i < digitCount; i++) {
        char c[2] = { digitToChar(digits[i]), '\0' };
        strncat(origBuf, c, sizeof(origBuf) - strlen(origBuf) - 1);
    }

    startScreen("BASE -> DECIMAL", "[ENTER] Done");
    printSubheader("Result");
    printBlank();

    char lineBuf[52];
    snprintf(lineBuf, sizeof(lineBuf), "(%s) base %d", origBuf, base);
    printLine(lineBuf, COL_BLACK);

    if (overflow) {
        printLine("= Overflow (too large)", COL_RED);
    } else {
        snprintf(lineBuf, sizeof(lineBuf), "= %llu (base 10)",
                 result);
        printLine(lineBuf, COL_GREEN);
    }

    waitContinue();
}

static void solveDecimalToBase(void)
{
    RESET_CANCEL();
    startScreen("DECIMAL -> BASE", "[CLEAR] Back");
    printSubheader("Convert base 10 -> base N");
    printBlank();

    double numVal  = inputNumber("Number (base 10) = "); CHECK_CANCEL;
    double baseVal = inputNumber("Target base = ");      CHECK_CANCEL;

    int base = (int)round(baseVal);
    unsigned long long num = (unsigned long long)fabs(round(numVal));
    bool negative = (numVal < 0.0);

    printDivider();

    if (base < 2 || base > 36) {
        printLine("Base must be 2 to 36", COL_RED);
        waitContinue();
        return;
    }

    if (num == 0) {
        printLine("Result: 0", COL_GREEN);
        waitContinue();
        return;
    }

    /* Repeated division */
    char resultBuf[52];
    resultBuf[0] = '\0';
    char tmp[52];
    tmp[0] = '\0';

    unsigned long long n = num;
    while (n > 0) {
        int rem = (int)(n % (unsigned long long)base);
        char c[2] = { digitToChar(rem), '\0' };
        /* Prepend digit */
        memmove(tmp + 1, tmp, strlen(tmp) + 1);
        tmp[0] = c[0];
        n /= (unsigned long long)base;
    }

    if (negative) {
        snprintf(resultBuf, sizeof(resultBuf), "-%s", tmp);
    } else {
        snprintf(resultBuf, sizeof(resultBuf), "%s", tmp);
    }

    char lineBuf[52];
    snprintf(lineBuf, sizeof(lineBuf), "%llu (base 10)", num);
    printLine(lineBuf, COL_BLACK);

    snprintf(lineBuf, sizeof(lineBuf), "= %s (base %d)",
             resultBuf, base);
    printLine(lineBuf, COL_GREEN);

    waitContinue();
}

static void solveBaseToBase(void)
{
    RESET_CANCEL();
    startScreen("BASE -> BASE", "[CLEAR] Back");
    printSubheader("Convert base A -> base B");
    printBlank();

    /*
     * Strategy: convert source -> decimal -> target.
     * Reuses the logic from the two functions above
     * but inline, since we need the intermediate value.
     */

    double srcBaseVal = inputNumber("Source base = "); CHECK_CANCEL;
    int srcBase = (int)round(srcBaseVal);

    double digitCountVal = inputNumber("# of digits = "); CHECK_CANCEL;
    int digitCount = (int)round(digitCountVal);

    if (srcBase < 2 || srcBase > 36) {
        printDivider();
        printLine("Base must be 2 to 36", COL_RED);
        waitContinue();
        return;
    }
    if (digitCount < 1 || digitCount > 12) {
        printDivider();
        printLine("1 to 12 digits only", COL_RED);
        waitContinue();
        return;
    }

    int digits[12];
    for (int i = 0; i < digitCount; i++) {
        char titleBuf[28];
        snprintf(titleBuf, sizeof(titleBuf),
                 "DIGIT %d of %d (base %d)", i + 1, digitCount, srcBase);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("Enter digit value 0-N");
        printBlank();

        for (int j = 0; j < i; j++) {
            char prevBuf[20];
            snprintf(prevBuf, sizeof(prevBuf),
                     "d%d = %c", j + 1, digitToChar(digits[j]));
            printLine(prevBuf, COL_GRAY);
        }

        char prompt[12];
        snprintf(prompt, sizeof(prompt), "d%d = ", i + 1);
        double dVal = inputNumber(prompt); CHECK_CANCEL;
        int d = (int)round(dVal);

        if (d < 0 || d >= srcBase) {
            printDivider();
            char errBuf[32];
            snprintf(errBuf, sizeof(errBuf),
                     "Digit must be 0 to %d", srcBase - 1);
            printLine(errBuf, COL_RED);
            waitContinue();
            return;
        }
        digits[i] = d;
    }

    /* Source -> decimal */
    unsigned long long decimal = 0;
    bool overflow = false;
    for (int i = 0; i < digitCount; i++) {
        if (decimal > (0xFFFFFFFFFFFFFFFFULL - digits[i])
                      / (unsigned long long)srcBase) {
            overflow = true; break;
        }
        decimal = decimal * (unsigned long long)srcBase
                + (unsigned long long)digits[i];
    }

    startScreen("BASE -> BASE", "[CLEAR] Back");
    printSubheader("Target base");
    printBlank();

    double tgtBaseVal = inputNumber("Target base = "); CHECK_CANCEL;
    int tgtBase = (int)round(tgtBaseVal);

    if (tgtBase < 2 || tgtBase > 36) {
        printDivider();
        printLine("Base must be 2 to 36", COL_RED);
        waitContinue();
        return;
    }

    /* Decimal -> target */
    char tmp[52];
    tmp[0] = '\0';
    unsigned long long n = decimal;
    if (n == 0) {
        tmp[0] = '0'; tmp[1] = '\0';
    } else {
        while (n > 0) {
            int rem = (int)(n % (unsigned long long)tgtBase);
            memmove(tmp + 1, tmp, strlen(tmp) + 1);
            tmp[0] = digitToChar(rem);
            n /= (unsigned long long)tgtBase;
        }
    }

    /* Build original number string */
    char origBuf[16];
    origBuf[0] = '\0';
    for (int i = 0; i < digitCount; i++) {
        char c[2] = { digitToChar(digits[i]), '\0' };
        strncat(origBuf, c, sizeof(origBuf) - strlen(origBuf) - 1);
    }

    printDivider();

    char lineBuf[52];
    snprintf(lineBuf, sizeof(lineBuf),
             "(%s) base %d", origBuf, srcBase);
    printLine(lineBuf, COL_BLACK);

    if (overflow) {
        printLine("Overflow in conversion", COL_RED);
    } else {
        snprintf(lineBuf, sizeof(lineBuf),
                 "= %llu (base 10)", decimal);
        printLine(lineBuf, COL_BLACK);

        snprintf(lineBuf, sizeof(lineBuf),
                 "= %s (base %d)", tmp, tgtBase);
        printLine(lineBuf, COL_GREEN);
    }

    waitContinue();
}

/*
 * Find-the-Base solver.
 *
 * Given:
 *   - Number A represented in base x  (digits entered by user)
 *   - Number B represented in base y  (digits entered by user)
 *   - x - y = diff  (user enters the difference)
 *
 * Strategy:
 *   Let y be the smaller base. Then x = y + diff.
 *   For each candidate y starting from 2, evaluate:
 *     polyA(x) == polyB(y)
 *   Stop at the first match. Also enforce:
 *     y  > max digit in B
 *     x  > max digit in A
 */

/* Evaluate a number given its digits[] (most significant first)
   in a given base. Returns 0 on overflow. */
static unsigned long long evalInBase(int *digits, int count, int base)
{
    unsigned long long val = 0;
    for (int i = 0; i < count; i++) {
        if (val > (0xFFFFFFFFFFFFFFFFULL - digits[i])
                  / (unsigned long long)base)
            return 0;   /* overflow sentinel */
        val = val * (unsigned long long)base
            + (unsigned long long)digits[i];
    }
    return val;
}

static void solveFindTheBase(void)
{
    RESET_CANCEL();
    startScreen("FIND THE BASE", "[CLEAR] Back");
    printSubheader("Same number, two bases");
    printBlank();

    double diffVal = inputNumber("Base difference (x-y) = ");
    CHECK_CANCEL;
    int diff = (int)round(diffVal);
    if (diff < 1) {
        printDivider();
        printLine("Difference must be >= 1", COL_RED);
        waitContinue();
        return;
    }

    /* Enter digits of number in larger base (base x) */
    double countAVal = inputNumber("# digits in larger base = ");
    CHECK_CANCEL;
    int countA = (int)round(countAVal);
    if (countA < 1 || countA > 10) {
        printDivider();
        printLine("1 to 10 digits only", COL_RED);
        waitContinue();
        return;
    }

    int digitsA[10];
    int maxDigitA = 0;
    for (int i = 0; i < countA; i++) {
        char titleBuf[32];
        snprintf(titleBuf, sizeof(titleBuf),
                 "LARGER BASE: digit %d of %d", i + 1, countA);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("Most significant first");
        printBlank();

        for (int j = 0; j < i; j++) {
            char prev[20];
            snprintf(prev, sizeof(prev), "d%d = %d", j + 1, digitsA[j]);
            printLine(prev, COL_GRAY);
        }

        char prompt[10];
        snprintf(prompt, sizeof(prompt), "d%d = ", i + 1);
        double dv = inputNumber(prompt); CHECK_CANCEL;
        digitsA[i] = (int)round(dv);
        if (digitsA[i] < 0) {
            printDivider();
            printLine("Digit must be >= 0", COL_RED);
            waitContinue();
            return;
        }
        if (digitsA[i] > maxDigitA) maxDigitA = digitsA[i];
    }

    /* Enter digits of number in smaller base (base y) */
    double countBVal = inputNumber("# digits in smaller base = ");
    CHECK_CANCEL;
    int countB = (int)round(countBVal);
    if (countB < 1 || countB > 10) {
        printDivider();
        printLine("1 to 10 digits only", COL_RED);
        waitContinue();
        return;
    }

    int digitsB[10];
    int maxDigitB = 0;
    for (int i = 0; i < countB; i++) {
        char titleBuf[32];
        snprintf(titleBuf, sizeof(titleBuf),
                 "SMALLER BASE: digit %d of %d", i + 1, countB);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("Most significant first");
        printBlank();

        for (int j = 0; j < i; j++) {
            char prev[20];
            snprintf(prev, sizeof(prev), "d%d = %d", j + 1, digitsB[j]);
            printLine(prev, COL_GRAY);
        }

        char prompt[10];
        snprintf(prompt, sizeof(prompt), "d%d = ", i + 1);
        double dv = inputNumber(prompt); CHECK_CANCEL;
        digitsB[i] = (int)round(dv);
        if (digitsB[i] < 0) {
            printDivider();
            printLine("Digit must be >= 0", COL_RED);
            waitContinue();
            return;
        }
        if (digitsB[i] > maxDigitB) maxDigitB = digitsB[i];
    }

    /* Search for y starting from the minimum valid base */
    int minY = maxDigitB + 1;          /* y must be > every digit in B */
    if (minY < 2) minY = 2;

    bool found = false;
    char lineBuf[52];

    startScreen("FIND THE BASE", "[ENTER] Done");
    printSubheader("Result");
    printBlank();

    for (int y = minY; y <= 1000; y++) {
        int x = y + diff;

        /* x must also be valid for digits in A */
        if (x <= maxDigitA) continue;

        unsigned long long valA = evalInBase(digitsA, countA, x);
        unsigned long long valB = evalInBase(digitsB, countB, y);

        /* Skip overflow sentinels */
        if (valA == 0 && countA > 1) continue;
        if (valB == 0 && countB > 1) continue;

        if (valA == valB) {
            snprintf(lineBuf, sizeof(lineBuf),
                     "Larger base x = %d", x);
            printLine(lineBuf, COL_BLACK);

            snprintf(lineBuf, sizeof(lineBuf),
                     "Smaller base y = %d", y);
            printLine(lineBuf, COL_BLACK);

            printDivider();

            snprintf(lineBuf, sizeof(lineBuf),
                     "Value (base 10) = %llu", valA);
            printLine(lineBuf, COL_GREEN);

            found = true;
            break;
        }
    }

    if (!found) {
        printLine("No solution found", COL_RED);
        printLine("(checked y = 2..1000)", COL_BLACK);
    }

    waitContinue();
}

/*
 * z|Ax+B| = w|Cx+D|
 *
 * Splits into two linear cases:
 *   Case 1:  z(Ax+B) =  w(Cx+D)  =>  (zA-wC)x = wD-zB
 *   Case 2:  z(Ax+B) = -w(Cx+D)  =>  (zA+wC)x = -wD-zB
 *
 * Each case is a linear equation — solved directly.
 * Solutions are validated by substituting back into
 * the original equation to reject extraneous roots.
 */
static void solveAbsEqAbs(void)
{
    RESET_CANCEL();
    startScreen("z|Ax+B| = w|Cx+D|", "[CLEAR] Back");
    printSubheader("Enter z, A, B, w, C, D:");
    printBlank();

    double z = inputNumber("z = "); CHECK_CANCEL;
    double A = inputNumber("A = "); CHECK_CANCEL;
    double B = inputNumber("B = "); CHECK_CANCEL;
    double w = inputNumber("w = "); CHECK_CANCEL;
    double C = inputNumber("C = "); CHECK_CANCEL;
    double D = inputNumber("D = "); CHECK_CANCEL;

    printDivider();

    /* Validate: z and w must be non-zero for a meaningful equation */
    if (fabs(z) < MATH_EPS || fabs(w) < MATH_EPS) {
        printLine("z and w cannot be 0", COL_RED);
        waitContinue();
        return;
    }

    char v1[20], v2[20], lineBuf[52];
    bool foundAny = false;

    /* ── Case 1: z(Ax+B) = w(Cx+D)
     *   zAx + zB = wCx + wD
     *   (zA - wC)x = wD - zB                        */
    double coeff1 = z * A - w * C;
    double rhs1   = w * D - z * B;

    if (fabs(coeff1) < MATH_EPS) {
        /* Degenerate: 0*x = rhs1 */
        if (fabs(rhs1) < MATH_EPS) {
            printLine("Case 1: All Reals", COL_GREEN);
            foundAny = true;
        }
        /* else: no solution from case 1, silently skip */
    } else {
        double x1 = rhs1 / coeff1;

        /* Validate by substituting back */
        double lhs = z * fabs(A * x1 + B);
        double rhs = w * fabs(C * x1 + D);

        if (fabs(lhs - rhs) < MATH_EPS * (1.0 + fabs(lhs))) {
            formatNumber(v1, sizeof(v1), x1);
            snprintf(lineBuf, sizeof(lineBuf), "x = %s", v1);
            printLine(lineBuf, COL_GREEN);
            foundAny = true;
        }
    }

    /* ── Case 2: z(Ax+B) = -w(Cx+D)
     *   zAx + zB = -wCx - wD
     *   (zA + wC)x = -wD - zB                       */
    double coeff2 = z * A + w * C;
    double rhs2   = -w * D - z * B;

    if (fabs(coeff2) < MATH_EPS) {
        if (fabs(rhs2) < MATH_EPS) {
            printLine("Case 2: All Reals", COL_GREEN);
            foundAny = true;
        }
    } else {
        double x2 = rhs2 / coeff2;

        /* Validate by substituting back */
        double lhs = z * fabs(A * x2 + B);
        double rhs = w * fabs(C * x2 + D);

        if (fabs(lhs - rhs) < MATH_EPS * (1.0 + fabs(lhs))) {
            /* Only print if different from case 1 solution */
            bool isDuplicate = false;
            if (fabs(coeff1) > MATH_EPS) {
                double x1 = rhs1 / coeff1;
                if (fabs(x2 - x1) < MATH_EPS) isDuplicate = true;
            }
            if (!isDuplicate) {
                formatNumber(v2, sizeof(v2), x2);
                snprintf(lineBuf, sizeof(lineBuf), "x = %s", v2);
                printLine(lineBuf, COL_GREEN);
                foundAny = true;
            }
        }
    }

    if (!foundAny) {
        printLine("No Solution", COL_RED);
    }

    waitContinue();
}

/* ═══════════════════════════════════════════════
   THEORY OF EQUATIONS
   ═══════════════════════════════════════════════ */

/*
 * Evaluate a polynomial at x=val.
 * coeffs[0] = leading (degree n), coeffs[n] = constant.
 * Uses Horner's method.
 */
static double evalPoly(double *coeffs, int degree, double val)
{
    double result = 0.0;
    for (int i = 0; i <= degree; i++)
        result = result * val + coeffs[i];
    return result;
}

/* ─────────────────────────────────────────────
   REMAINDER & FACTOR THEOREM
   ─────────────────────────────────────────────
   Two modes:
   A) All coefficients known — evaluate P(r) directly.
      Output: remainder, and whether (x-r) is a factor.
   B) One coefficient is unknown (k) — user tells us
      which power holds k ONCE up front, then enters
      all other coefficients, then enters r and the
      desired remainder. Solver finds k.
   ───────────────────────────────────────────── */
static void solveRemainderFactor(void)
{
    RESET_CANCEL();
    startScreen("REMAINDER/FACTOR THM", "[CLEAR] Back");
    printSubheader("P(x) divided by (x-r)");
    printBlank();

    double degVal = inputNumber("Degree of P(x) = "); CHECK_CANCEL;
    int deg = (int)round(degVal);

    if (deg < 1 || deg > 5) {
        printDivider();
        printLine("Degree must be 1 to 5", COL_RED);
        waitContinue();
        return;
    }

    /* ── Step 1: ask ONCE whether there is an unknown k ── */
    startScreen("REMAINDER/FACTOR THM", "[CLEAR] Back");
    printSubheader("Unknown coefficient?");
    printBlank();
    printLine("1: Yes, there is a k", COL_BLACK);
    printLine("2: No, all known",      COL_BLACK);
    blit();

    uint8_t hasKChoice;
    do { hasKChoice = waitForKey(); }
    while (hasKChoice != sk_1 && hasKChoice != sk_2 && hasKChoice != sk_Clear);
    if (hasKChoice == sk_Clear) { gUserCancelled = true; return; }
    bool hasK = (hasKChoice == sk_1);

    int kPos = -1;   /* index into coeffs[] where k lives */

    /* ── Step 2: if k exists, ask which power — ONCE ── */
    if (hasK) {
        startScreen("REMAINDER/FACTOR THM", "[CLEAR] Back");
        printSubheader("Which power holds k?");
        printBlank();
        double kPowVal = inputNumber("k is coeff of x^? = "); CHECK_CANCEL;
        int kPower = (int)round(kPowVal);
        if (kPower < 0 || kPower > deg) {
            printDivider();
            printLine("Invalid power for degree", COL_RED);
            waitContinue();
            return;
        }
        kPos = deg - kPower;   /* convert power to array index */
    }

    /* ── Step 3: collect all coefficients, one per screen ──
       The k slot is labelled and force-stored as 0.
       No "is this k?" prompt ever appears here.            */
    double coeffs[6] = {0};

    for (int i = 0; i <= deg; i++) {
        char titleBuf[32];
        snprintf(titleBuf, sizeof(titleBuf), "COEFF of x^%d", deg - i);
        startScreen(titleBuf, "[CLEAR] Back");

        if (i == kPos) {
            printSubheader("This is the k term");
            printLine("(enter 0 as placeholder)", COL_GRAY);
        } else {
            printSubheader("Enter coefficient");
        }
        printBlank();

        /* Show all previously entered coefficients */
        for (int j = 0; j < i; j++) {
            char prev[32];
            if (j == kPos) {
                snprintf(prev, sizeof(prev), "  x^%d: k", deg - j);
            } else {
                char nb[16];
                formatNumber(nb, sizeof(nb), coeffs[j]);
                snprintf(prev, sizeof(prev), "  x^%d: %s", deg - j, nb);
            }
            printLine(prev, COL_GRAY);
        }

        char prompt[16];
        snprintf(prompt, sizeof(prompt), "x^%d coeff = ", deg - i);
        double cv = inputNumber(prompt); CHECK_CANCEL;

        /* Force k slot to 0 regardless of what user types */
        coeffs[i] = (i == kPos) ? 0.0 : cv;
    }

    /* ── Step 4: enter divisor root r ── */
    startScreen("REMAINDER/FACTOR THM", "[CLEAR] Back");
    printSubheader("Divisor is (x - r)");
    printBlank();

    /* Show polynomial entered so far */
    for (int j = 0; j <= deg; j++) {
        char prev[32];
        if (j == kPos) {
            snprintf(prev, sizeof(prev), "  x^%d: k", deg - j);
        } else {
            char nb[16];
            formatNumber(nb, sizeof(nb), coeffs[j]);
            snprintf(prev, sizeof(prev), "  x^%d: %s", deg - j, nb);
        }
        printLine(prev, COL_GRAY);
    }
    printBlank();

    double r = inputNumber("r = "); CHECK_CANCEL;

    /* ── Step 5: if Mode B, ask desired remainder ── */
    double desiredRem = 0.0;
    if (hasK) {
        printLine("Desired remainder:", COL_NAVY);
        printLine("(0 for factor theorem)", COL_GRAY);
        desiredRem = inputNumber("Remainder = "); CHECK_CANCEL;
    }

    /* ── Step 6: compute and display ── */
    printDivider();

    char lineBuf[52], n1[20];

    if (!hasK) {
        /* ── Mode A: evaluate P(r), report remainder ── */
        double rem = evalPoly(coeffs, deg, r);

        formatNumber(n1, sizeof(n1), r);
        snprintf(lineBuf, sizeof(lineBuf), "P(%s) =", n1);
        printLine(lineBuf, COL_BLACK);

        formatNumber(n1, sizeof(n1), rem);
        snprintf(lineBuf, sizeof(lineBuf), "Remainder = %s", n1);
        printLine(lineBuf, fabs(rem) < MATH_EPS ? COL_GREEN : COL_ORANGE);

        if (fabs(rem) < MATH_EPS) {
            printLine("(x-r) IS a factor", COL_GREEN);
        } else {
            printLine("(x-r) is NOT a factor", COL_RED);
        }

    } else {
        /* ── Mode B: solve for k ──
         *
         * P(r) with k=0 gives S (the known part).
         * The k term contributes k * r^(deg-kPos).
         * So:  S + k * r^(deg-kPos) = desiredRem
         *      k = (desiredRem - S) / r^(deg-kPos)
         */
        int   kPow    = deg - kPos;
        double kContrib = 1.0;
        for (int i = 0; i < kPow; i++) kContrib *= r;

        double S = evalPoly(coeffs, deg, r);   /* coeffs[kPos]=0 */

        if (fabs(kContrib) < MATH_EPS) {
            /* r^kPow = 0 means k vanishes at this r — unsolvable */
            printLine("k term = 0 at r=0", COL_RED);
            printLine("Cannot solve for k", COL_RED);
        } else {
            double k = (desiredRem - S) / kContrib;

            formatNumber(n1, sizeof(n1), k);
            snprintf(lineBuf, sizeof(lineBuf), "k = %s", n1);
            printLine(lineBuf, COL_GREEN);

            /* Verify: substitute k back and evaluate */
            coeffs[kPos] = k;
            double check = evalPoly(coeffs, deg, r);
            formatNumber(n1, sizeof(n1), check);
            snprintf(lineBuf, sizeof(lineBuf), "Verify P(r) = %s", n1);
            printLine(lineBuf,
                      fabs(check - desiredRem) < 1e-6 ? COL_GRAY : COL_RED);

            /* Factor theorem bonus: tell user if (x-r) is a factor */
            if (fabs(desiredRem) < MATH_EPS) {
                printLine("(x-r) IS a factor", COL_GREEN);
            }
        }
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   2. FACTOR THEOREM — MULTIPLE KNOWN FACTORS
   Given n factors (x-r1),(x-r2),..., all of degree-n
   polynomial Ax^n + Bx^(n-1) + ... find all unknowns.
   ───────────────────────────────────────────── */
static void solveMultipleFactors(void)
{
    RESET_CANCEL();
    startScreen("MULTIPLE FACTORS", "[CLEAR] Back");
    printSubheader("Known factors -> coeffs");
    printBlank();

    double degVal = inputNumber("Polynomial degree = "); CHECK_CANCEL;
    int deg = (int)round(degVal);

    if (deg < 2 || deg > 5) {
        printDivider();
        printLine("Degree must be 2 to 5", COL_RED);
        waitContinue();
        return;
    }

    double nFactVal = inputNumber("# of known factors = "); CHECK_CANCEL;
    int nFact = (int)round(nFactVal);

    if (nFact < 1 || nFact > deg) {
        printDivider();
        printLine("Must be 1 to degree", COL_RED);
        waitContinue();
        return;
    }

    /* Collect roots from factors (x - ri) */
    double roots[5];
    for (int i = 0; i < nFact; i++) {
        char titleBuf[32];
        snprintf(titleBuf, sizeof(titleBuf),
                 "FACTOR %d of %d", i + 1, nFact);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("Factor is (x - r)");
        printBlank();

        for (int j = 0; j < i; j++) {
            char prev[24];
            char nb[12];
            formatNumber(nb, sizeof(nb), roots[j]);
            snprintf(prev, sizeof(prev), "  r%d = %s", j+1, nb);
            printLine(prev, COL_GRAY);
        }

        char prompt[10];
        snprintf(prompt, sizeof(prompt), "r%d = ", i + 1);
        roots[i] = inputNumber(prompt); CHECK_CANCEL;
    }

    /* Collect known polynomial coefficients (unknowns entered as 0) */
    startScreen("MULTIPLE FACTORS", "[CLEAR] Back");
    printSubheader("Enter poly coefficients");
    printLine("(Enter 0 for unknowns)", COL_GRAY);
    printBlank();

    double coeffs[6] = {0};
    bool   isUnknown[6] = {false};

    /* Leading coefficient is always known (usually 1) */
    coeffs[0] = inputNumber("Leading coeff = "); CHECK_CANCEL;

    for (int i = 1; i <= deg; i++) {
        char prompt[16];
        snprintf(prompt, sizeof(prompt), "x^%d coeff = ", deg - i);
        double cv = inputNumber(prompt); CHECK_CANCEL;
        coeffs[i] = cv;

        /* If entered 0, ask if unknown */
        if (fabs(cv) < MATH_EPS) {
            drawFooter("Is this unknown? 1=Yes 2=No");
            blit();
            uint8_t kc;
            do { kc = waitForKey(); }
            while (kc != sk_1 && kc != sk_2 && kc != sk_Clear);
            if (kc == sk_Clear) { gUserCancelled = true; return; }
            if (kc == sk_1) isUnknown[i] = true;
        }
    }

    /* For each known root r, P(r)=0 gives one equation.
     * For simple cases (one or two unknowns), solve directly. */
    int unknownCount = 0;
    int unknownIdx[5];
    for (int i = 0; i <= deg; i++)
        if (isUnknown[i]) unknownIdx[unknownCount++] = i;

    printDivider();
    char lineBuf[52], n1[20], n2[20];

    if (unknownCount == 0) {
        /* Just verify the factors */
        bool allValid = true;
        for (int i = 0; i < nFact; i++) {
            double val = evalPoly(coeffs, deg, roots[i]);
            if (fabs(val) > 1e-6) allValid = false;
        }
        printLine(allValid ? "All factors verified" : "Factor mismatch",
                  allValid ? COL_GREEN : COL_RED);

    } else if (unknownCount == 1 && nFact >= 1) {
        /* One unknown: use first root equation
         * S + unknownCoeff * r^power = 0
         * unknownCoeff = -S / r^power                */
        int ui    = unknownIdx[0];
        int power = deg - ui;
        double r  = roots[0];

        double rPow = 1.0;
        for (int i = 0; i < power; i++) rPow *= r;

        double S = evalPoly(coeffs, deg, r);

        if (fabs(rPow) < MATH_EPS) {
            printLine("Cannot solve: r^p=0", COL_RED);
        } else {
            double unknown = -S / rPow;
            formatNumber(n1, sizeof(n1), unknown);
            snprintf(lineBuf, sizeof(lineBuf),
                     "x^%d coeff = %s", power, n1);
            printLine(lineBuf, COL_GREEN);

            /* Verify remaining roots */
            coeffs[ui] = unknown;
            bool allOk = true;
            for (int i = 1; i < nFact; i++) {
                double val = evalPoly(coeffs, deg, roots[i]);
                if (fabs(val) > 1e-6) allOk = false;
            }
            if (!allOk) printLine("Warning: check roots", COL_RED);

            /* Print full polynomial */
            printBlank();
            printLine("Full polynomial:", COL_ORANGE);
            char polyStr[52];
            polyStr[0] = '\0';
            for (int i = 0; i <= deg; i++) {
                char term[16];
                int pw = deg - i;
                formatNumber(n1, sizeof(n1), coeffs[i]);
                if (pw == 0)      snprintf(term, sizeof(term), "%s", n1);
                else if (pw == 1) snprintf(term, sizeof(term), "%sx", n1);
                else              snprintf(term, sizeof(term), "%sx^%d", n1, pw);
                if (i > 0 && coeffs[i] >= 0)
                    strncat(polyStr, "+", sizeof(polyStr)-strlen(polyStr)-1);
                strncat(polyStr, term, sizeof(polyStr)-strlen(polyStr)-1);
            }
            printLine(polyStr, COL_BLACK);
        }

    } else if (unknownCount == 2 && nFact >= 2) {
        /* Two unknowns: 2x2 system from two root equations
         * coeffs at ui0 and ui1 are unknown.
         * P(r0) = S0 + k0*r0^p0 + k1*r0^p1 = 0
         * P(r1) = S1 + k0*r1^p0 + k1*r1^p1 = 0
         */
        int ui0 = unknownIdx[0], ui1 = unknownIdx[1];
        int p0  = deg - ui0,     p1  = deg - ui1;
        double r0 = roots[0],    r1  = roots[1];

        double r0p0=1.0, r0p1=1.0, r1p0=1.0, r1p1=1.0;
        for (int i=0;i<p0;i++){r0p0*=r0; r1p0*=r1;}
        for (int i=0;i<p1;i++){r0p1*=r0; r1p1*=r1;}

        double S0 = evalPoly(coeffs, deg, r0);
        double S1 = evalPoly(coeffs, deg, r1);

        /* a11*k0 + a12*k1 = b1
           a21*k0 + a22*k1 = b2 */
        double a11=r0p0, a12=r0p1, b1=-S0;
        double a21=r1p0, a22=r1p1, b2=-S1;
        double det = a11*a22 - a12*a21;

        if (fabs(det) < MATH_EPS) {
            printLine("System is singular", COL_RED);
        } else {
            double k0 = (b1*a22 - b2*a12) / det;
            double k1 = (a11*b2 - a21*b1) / det;

            formatNumber(n1, sizeof(n1), k0);
            snprintf(lineBuf, sizeof(lineBuf), "x^%d coeff = %s", p0, n1);
            printLine(lineBuf, COL_GREEN);

            formatNumber(n1, sizeof(n1), k1);
            snprintf(lineBuf, sizeof(lineBuf), "x^%d coeff = %s", p1, n1);
            printLine(lineBuf, COL_GREEN);

            /* Ordered triple if degree 3 */
            if (deg == 3) {
                coeffs[ui0] = k0; coeffs[ui1] = k1;
                formatNumber(n1, sizeof(n1), coeffs[1]);
                formatNumber(n2, sizeof(n2), coeffs[2]);
                char n3[20];
                formatNumber(n3, sizeof(n3), coeffs[3]);
                snprintf(lineBuf, sizeof(lineBuf),
                         "(%s, %s, %s)", n1, n2, n3);
                printLine(lineBuf, COL_BLACK);
            }
        }
    } else {
        printLine("Too many unknowns here", COL_RED);
        printLine("Use: Poly from Roots", COL_ORANGE);
        printLine("Enter same roots there", COL_BLACK);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   3. BUILD POLYNOMIAL FROM ROOTS
   ───────────────────────────────────────────── */
static void solvePolyFromRoots(void)
{
    RESET_CANCEL();
    startScreen("POLY FROM ROOTS", "[CLEAR] Back");
    printSubheader("Enter roots -> polynomial");
    printBlank();

    double nVal = inputNumber("# of roots = "); CHECK_CANCEL;
    int n = (int)round(nVal);

    if (n < 1 || n > 5) {
        printDivider();
        printLine("1 to 5 roots only", COL_RED);
        waitContinue();
        return;
    }

    double leadVal = inputNumber("Leading coeff = "); CHECK_CANCEL;

    double roots[5];
    for (int i = 0; i < n; i++) {
        char titleBuf[24];
        snprintf(titleBuf, sizeof(titleBuf), "ROOT %d of %d", i+1, n);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("(x - root) is a factor");
        printBlank();

        for (int j = 0; j < i; j++) {
            char prev[20]; char nb[12];
            formatNumber(nb, sizeof(nb), roots[j]);
            snprintf(prev, sizeof(prev), "  r%d = %s", j+1, nb);
            printLine(prev, COL_GRAY);
        }

        char prompt[10];
        snprintf(prompt, sizeof(prompt), "r%d = ", i+1);
        roots[i] = inputNumber(prompt); CHECK_CANCEL;
    }

    /* Multiply out (x-r1)(x-r2)...(x-rn) using convolution */
    double poly[6] = {0};
    poly[0] = 1.0;   /* start with 1 */

    for (int i = 0; i < n; i++) {
        /* Multiply current poly by (x - roots[i]) */
        double newPoly[6] = {0};
        for (int j = 0; j <= i; j++) {
            newPoly[j]   += poly[j];            /* x * poly[j-1] */
            newPoly[j+1] += poly[j] * (-roots[i]); /* -r * poly[j] */
        }
        for (int j = 0; j <= i+1; j++) poly[j] = newPoly[j];
    }

    /* Scale by leading coefficient */
    for (int i = 0; i <= n; i++) poly[i] *= leadVal;

    printDivider();
    printLine("Polynomial:", COL_ORANGE);

    char lineBuf[52], n1[20];

    for (int i = 0; i <= n && gCurrentLine < MAX_LINES - 1; i++) {
        int pw = n - i;
        formatNumber(n1, sizeof(n1), poly[i]);
        if (pw == 0)
            snprintf(lineBuf, sizeof(lineBuf), "  constant = %s", n1);
        else if (pw == 1)
            snprintf(lineBuf, sizeof(lineBuf), "  x^1 coeff = %s", n1);
        else
            snprintf(lineBuf, sizeof(lineBuf), "  x^%d coeff = %s", pw, n1);
        printLine(lineBuf, COL_GREEN);
    }

    /* Sum of coefficients */
    double sumCoeffs = 0.0;
    for (int i = 0; i <= n; i++) sumCoeffs += poly[i];
    formatNumber(n1, sizeof(n1), sumCoeffs);
    snprintf(lineBuf, sizeof(lineBuf), "Sum of coeffs = %s", n1);
    printLine(lineBuf, COL_BLACK);

    waitContinue();
}

/* ─────────────────────────────────────────────
   4. GENERAL VIETA'S RELATIONS
   Degree 2 or 3 — compute ALL root relations
   including sum of squares, used for Q20 2013
   ───────────────────────────────────────────── */
static void solveVietasFormulas(void)
{
    RESET_CANCEL();
    startScreen("VIETA'S FORMULAS", "[CLEAR] Back");
    printSubheader("All root relations");
    printBlank();

    double degVal = inputNumber("Degree (2 or 3) = "); CHECK_CANCEL;
    int deg = (int)round(degVal);

    if (deg != 2 && deg != 3) {
        printDivider();
        printLine("Enter 2 or 3", COL_RED);
        waitContinue();
        return;
    }

    char lineBuf[52], n1[20];

    if (deg == 2) {
        startScreen("VIETA'S (QUADRATIC)", "[CLEAR] Back");
        printSubheader("Ax^2 + Bx + C = 0");
        printBlank();

        double A = inputNumber("A = "); CHECK_CANCEL;
        double B = inputNumber("B = "); CHECK_CANCEL;
        double C = inputNumber("C = "); CHECK_CANCEL;

        if (fabs(A) < MATH_EPS) {
            printDivider();
            printLine("A cannot be 0", COL_RED);
            waitContinue();
            return;
        }

        double sumR   = -B / A;
        double prodR  = C / A;
        double sumSqR = sumR * sumR - 2.0 * prodR;
        double disc   = B*B - 4.0*A*C;

        printDivider();

        formatNumber(n1, sizeof(n1), sumR);
        snprintf(lineBuf, sizeof(lineBuf), "r1+r2       = %s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), prodR);
        snprintf(lineBuf, sizeof(lineBuf), "r1*r2       = %s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), sumSqR);
        snprintf(lineBuf, sizeof(lineBuf), "r1^2+r2^2   = %s", n1);
        printLine(lineBuf, COL_GREEN);

        /* |r1 - r2| = sqrt(disc) / |A| */
        if (disc >= -MATH_EPS) {
            double absDiff = sqrt(fabs(disc)) / fabs(A);
            formatNumber(n1, sizeof(n1), absDiff);
            snprintf(lineBuf, sizeof(lineBuf), "|r1-r2|     = %s", n1);
            printLine(lineBuf, COL_GREEN);
        } else {
            printLine("|r1-r2|     = complex", COL_ORANGE);
        }

        /* Sum of cubes: (r1+r2)^3 - 3r1r2(r1+r2) */
        double sumCuR = sumR*sumR*sumR - 3.0*prodR*sumR;
        formatNumber(n1, sizeof(n1), sumCuR);
        snprintf(lineBuf, sizeof(lineBuf), "r1^3+r2^3   = %s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), disc);
        snprintf(lineBuf, sizeof(lineBuf), "Discriminant= %s", n1);
        printLine(lineBuf, COL_BLACK);

    } else {
        startScreen("VIETA'S (CUBIC)", "[CLEAR] Back");
        printSubheader("Ax^3+Bx^2+Cx+D=0");
        printBlank();

        double A = inputNumber("A = "); CHECK_CANCEL;
        double B = inputNumber("B = "); CHECK_CANCEL;
        double C = inputNumber("C = "); CHECK_CANCEL;
        double D = inputNumber("D = "); CHECK_CANCEL;

        if (fabs(A) < MATH_EPS) {
            printDivider();
            printLine("A cannot be 0", COL_RED);
            waitContinue();
            return;
        }

        double e1 = -B / A;             /* r1+r2+r3          */
        double e2 =  C / A;             /* r1r2+r1r3+r2r3    */
        double e3 = -D / A;             /* r1*r2*r3          */

        double sumSqR  = e1*e1 - 2.0*e2;           /* r1^2+r2^2+r3^2  */
        double sumCuR  = e1*e1*e1 - 3.0*e1*e2
                       + 3.0*e3;                    /* r1^3+r2^3+r3^3  */        
        double sumSqProd = e2*e2 - 2.0*e1*e3;       /* r1^2r2^2+...    */

        printDivider();

        formatNumber(n1, sizeof(n1), e1);
        snprintf(lineBuf, sizeof(lineBuf), "r1+r2+r3     = %s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), e2);
        snprintf(lineBuf, sizeof(lineBuf), "r1r2+r1r3+r2r3=%s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), e3);
        snprintf(lineBuf, sizeof(lineBuf), "r1*r2*r3     = %s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), sumSqR);
        snprintf(lineBuf, sizeof(lineBuf), "r1^2+r2^2+r3^2=%s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), sumCuR);
        snprintf(lineBuf, sizeof(lineBuf), "r1^3+r2^3+r3^3=%s", n1);
        printLine(lineBuf, COL_GREEN);

        formatNumber(n1, sizeof(n1), sumSqProd);
        snprintf(lineBuf, sizeof(lineBuf), "r1^2r2^2+...=%s", n1);
        printLine(lineBuf, COL_GREEN);
    }

    waitContinue();
}

/*
 * Q20-2013 style: polynomial has symbolic parameters a, b.
 * Structure: x^3 + P(a)*x^2 + Q(a)*x + R(a,b) = 0
 * User tells us which coefficients depend on a single
 * integer parameter 'a', and the solver scans a over a range
 * to compute the desired root relation at each value.
 *
 * Handles the pattern:  x^3 + a*x^2 + a*x + (C + b) = 0
 * where b is free (so D can be ignored in sum-of-squares).
 *
 * Sum of squares of roots = a^2 - 2a  (from Vieta's, b cancels)
 */
static void solveVietasParametric(void)
{
    RESET_CANCEL();
    startScreen("VIETA'S PARAMETRIC", "[CLEAR] Back");
    printSubheader("x^3 + ax^2 + ax + ... = 0");
    printBlank();

    printLine("Coefficients as multiples", COL_NAVY);
    printLine("of parameter a:", COL_NAVY);
    printBlank();

    /* User enters: B = mB*a + cB,  C = mC*a + cC
       D is ignored (free parameter b absorbs it)    */
    double mB = inputNumber("B = mB*a, mB = "); CHECK_CANCEL;
    double cB = inputNumber("  + cB, cB = ");   CHECK_CANCEL;
    double mC = inputNumber("C = mC*a, mC = "); CHECK_CANCEL;
    double cC = inputNumber("  + cC, cC = ");   CHECK_CANCEL;

    double aMin = inputNumber("a range: min = "); CHECK_CANCEL;
    double aMax = inputNumber("         max = "); CHECK_CANCEL;
    double aStep = inputNumber("    step (e.g.2) = "); CHECK_CANCEL;

    if (fabs(aStep) < MATH_EPS) {
        printDivider();
        printLine("Step cannot be 0", COL_RED);
        waitContinue();
        return;
    }

    startScreen("VIETA'S PARAMETRIC", "[ENTER] Done");
    printSubheader("a    r1^2+r2^2+r3^2");

    /* A=1 always (monic cubic) */
    double bestSumSq  = -1e18;
    double bestA      = aMin;
    bool   first      = true;

    char lineBuf[52], n1[20], n2[20];

    for (double a = aMin; a <= aMax + MATH_EPS; a += aStep) {
        double B = mB * a + cB;
        double C = mC * a + cC;
        /* e1 = -B/1, e2 = C/1 */
        double e1 = -B;
        double e2 =  C;
        double sumSqR = e1*e1 - 2.0*e2;

        if (sumSqR > bestSumSq || first) {
            bestSumSq = sumSqR;
            bestA     = a;
            first     = false;
        }

        if (gCurrentLine < MAX_LINES - 2) {
            formatNumber(n1, sizeof(n1), a);
            formatNumber(n2, sizeof(n2), sumSqR);
            snprintf(lineBuf, sizeof(lineBuf), "a=%-5s  %s", n1, n2);
            printLine(lineBuf, COL_BLACK);
        }
    }

    printDivider();
    formatNumber(n1, sizeof(n1), bestA);
    formatNumber(n2, sizeof(n2), bestSumSq);
    snprintf(lineBuf, sizeof(lineBuf), "Max at a=%s: %s", n1, n2);
    printLine(lineBuf, COL_GREEN);

    waitContinue();
}

/* ─────────────────────────────────────────────
   5. GENERAL ROOT CONDITION FINDER
   Given Ax^2+Bx+C=0, one coeff unknown,
   user picks what root condition is known,
   solver finds the unknown coefficient.
   ───────────────────────────────────────────── */
static void solveRootCondition(void)
{
    RESET_CANCEL();
    startScreen("ROOT CONDITION", "[CLEAR] Back");
    printSubheader("Quadratic, find coeff");
    printBlank();

    printLine("Which coeff is unknown?", COL_NAVY);
    printLine("1:A  2:B  3:C", COL_ORANGE);
    blit();
    uint8_t uc;
    do { uc = waitForKey(); }
    while (uc != sk_1 && uc != sk_2 && uc != sk_3 && uc != sk_Clear);
    if (uc == sk_Clear) { gUserCancelled = true; return; }
    int unknownCoeff = -1;
    if      (uc == sk_1) unknownCoeff = 1;
    else if (uc == sk_2) unknownCoeff = 2;
    else if (uc == sk_3) unknownCoeff = 3;

    double A=0, B=0, C=0;
    if (unknownCoeff != 1) { A = inputNumber("A = "); CHECK_CANCEL; }
    if (unknownCoeff != 2) { B = inputNumber("B = "); CHECK_CANCEL; }
    if (unknownCoeff != 3) { C = inputNumber("C = "); CHECK_CANCEL; }

    /* Pick condition type */
    /* Use showMenu so number keys work reliably */
    const char *condOptions[] = {
        "r1+r2 = S  (sum)",
        "r1*r2 = P  (product)",
        "|r1-r2| = d  (diff)",
        "r1^2+r2^2 = Q",
        "r2 = r1^2",
        "r2 = k*r1  (ratio)"
    };
    int condType = showMenu("KNOWN CONDITION", condOptions, 6);
    if (condType < 0) return;
    condType++;   /* showMenu returns 0-based; code uses 1-based */

    double condVal = 0.0;
    if (condType != 5) {
        startScreen("ROOT CONDITION", "[CLEAR] Back");
        printSubheader("Enter the known value:");
        printBlank();
        char prompt[24];
        switch (condType) {
            case 1: snprintf(prompt,sizeof(prompt),"S (sum) = ");      break;
            case 2: snprintf(prompt,sizeof(prompt),"P (product) = ");  break;
            case 3: snprintf(prompt,sizeof(prompt),"|r1-r2| = ");      break;
            case 4: snprintf(prompt,sizeof(prompt),"Q (sum sq) = ");   break;
            case 6: snprintf(prompt,sizeof(prompt),"k (ratio r2/r1) ="); break;
            default: snprintf(prompt,sizeof(prompt),"Value = ");        break;
        }
        condVal = inputNumber(prompt); CHECK_CANCEL;
    }

    startScreen("ROOT CONDITION", "[ENTER] Done");
    printSubheader("Result");
    printBlank();
    printDivider();
    char lineBuf[52], n1[20];

    /*
     * Vieta's for Ax^2+Bx+C=0:
     *   sum  = -B/A
     *   prod =  C/A
     *
     * Express condition in terms of the unknown coefficient.
     */
    double result = 0.0;
    bool   valid  = true;

    if (unknownCoeff == 2) {
        /* Unknown is B.  sum = -B/A  =>  B = -A*sum */
        switch (condType) {
            case 1: result = -A * condVal;            break; /* B=-A*S */
            case 2: result = B;  valid=false; break;  /* prod=C/A, no B */
            case 3: /* |r1-r2|=d: disc=d²A²
                       B²-4AC = d²A²  =>  B=sqrt(d²A²+4AC) */
            {
                double underB = condVal*condVal*A*A + 4.0*A*C;
                if (underB < -MATH_EPS) { valid=false; break; }
                result = sqrt(fabs(underB));
                formatNumber(n1, sizeof(n1), result);
                snprintf(lineBuf, sizeof(lineBuf), "B = +/- %s", n1);
                printLine(lineBuf, COL_GREEN);
                formatNumber(n1, sizeof(n1), -result);
                snprintf(lineBuf, sizeof(lineBuf), "  or B = %s", n1);
                printLine(lineBuf, COL_GREEN);
                waitContinue();
                return;
            }
            case 4: /* sum^2-2prod=Q => (B/A)^2-2C/A=Q => B^2=A^2(Q+2C/A) */
            {
                double underB = A*A*(condVal + 2.0*C/A);
                if (underB < -MATH_EPS) { valid=false; break; }
                result = sqrt(fabs(underB));
                formatNumber(n1, sizeof(n1), result);
                snprintf(lineBuf, sizeof(lineBuf), "B = +/- %s", n1);
                printLine(lineBuf, COL_GREEN);
                formatNumber(n1, sizeof(n1), -result);
                snprintf(lineBuf, sizeof(lineBuf), "  or B = %s", n1);
                printLine(lineBuf, COL_GREEN);
                waitContinue();
                return;
            }
            case 5: /* r2=r1^2: r1^3=C/A, B=-A(r1+r1^2) */
            {
                double r1 = cbrt(C / A);
                double r2 = r1 * r1;
                result    = -A * (r1 + r2);

                /* Show the roots so the user can verify */
                char rb1[20], rb2[20];
                formatNumber(rb1, sizeof(rb1), r1);
                formatNumber(rb2, sizeof(rb2), r2);
                snprintf(lineBuf, sizeof(lineBuf), "r1=%s, r2=%s", rb1, rb2);
                printLine(lineBuf, COL_GRAY);
                break;
            }
            case 6: /* r2=k*r1: sum=(1+k)r1=-B/A, prod=k*r1^2=C/A */
            {
                /* r1^2=C/(Ak), r1=-B/(A(1+k)) => B^2/(A^2(1+k)^2)=C/(Ak) */
                /* B = -A*(1+k)*r1, r1=cbrt... use prod to find r1 */
                double r1sq = C / (A * condVal);
                if (r1sq < -MATH_EPS) { valid=false; break; }
                double r1 = (r1sq >= 0) ? sqrt(r1sq) : -sqrt(-r1sq);
                /* pick sign from sum */
                /* r1 + k*r1 = -B/A  =>  B = -A*r1*(1+condVal) */
                /* But we don't know sign of r1 yet — give both */
                result = -A * r1 * (1.0 + condVal);
                formatNumber(n1, sizeof(n1), result);
                snprintf(lineBuf, sizeof(lineBuf), "B = %s", n1);
                printLine(lineBuf, COL_GREEN);
                formatNumber(n1, sizeof(n1), -result);
                snprintf(lineBuf, sizeof(lineBuf), "  or B = %s", n1);
                printLine(lineBuf, COL_GREEN);
                waitContinue();
                return;
            }
            default: valid=false; break;
        }

    } else if (unknownCoeff == 3) {
        /* Unknown is C.  prod = C/A */
        switch (condType) {
            case 1: valid=false; break; /* sum doesn't involve C */
            case 2: result = A * condVal; break; /* C=A*P */
            case 3: /* disc=B^2-4AC=d^2A^2 => C=(B^2-d^2A^2)/(4A) */
                result = (B*B - condVal*condVal*A*A) / (4.0*A);
                break;
            case 4: /* Q=sum^2-2prod => C=A*(sum^2-Q)/2=A*((B/A)^2-Q)/2 */
                result = A * ((B/A)*(B/A) - condVal) / 2.0;
                break;
           case 5:
            {
                double disc2 = 1.0 - 4.0*(B/A);
                if (disc2 < -MATH_EPS) { valid=false; break; }
                double r1a = (-1.0 + sqrt(fabs(disc2))) / 2.0;
                double r1b = (-1.0 - sqrt(fabs(disc2))) / 2.0;
                double Ca  = A * r1a * r1a * r1a;
                double Cb  = A * r1b * r1b * r1b;

                char rb1[20], rb2[20];
                formatNumber(rb1, sizeof(rb1), r1a);
                formatNumber(rb2, sizeof(rb2), r1a*r1a);
                snprintf(lineBuf, sizeof(lineBuf), "r1=%s r2=%s", rb1, rb2);
                printLine(lineBuf, COL_GRAY);

                formatNumber(n1, sizeof(n1), Ca);
                snprintf(lineBuf, sizeof(lineBuf), "C = %s", n1);
                printLine(lineBuf, COL_GREEN);

                formatNumber(rb1, sizeof(rb1), r1b);
                formatNumber(rb2, sizeof(rb2), r1b*r1b);
                snprintf(lineBuf, sizeof(lineBuf), "r1=%s r2=%s", rb1, rb2);
                printLine(lineBuf, COL_GRAY);

                formatNumber(n1, sizeof(n1), Cb);
                snprintf(lineBuf, sizeof(lineBuf), "  or C = %s", n1);
                printLine(lineBuf, COL_GREEN);
                waitContinue();
                return;
            }
            case 6: /* r2=k*r1: prod=k*r1^2=C/A; sum=(1+k)r1=-B/A
                       r1=-B/(A(1+k)), C=A*k*r1^2 */
            {
                if (fabs(1.0+condVal) < MATH_EPS) { valid=false; break; }
                double r1 = -B / (A*(1.0+condVal));
                result = A * condVal * r1 * r1;
                break;
            }
            default: valid=false; break;
        }

    } else {
        /* Unknown is A */
        switch (condType) {
            case 1: /* sum=-B/A=S => A=-B/S */
                if (fabs(condVal)<MATH_EPS){valid=false;break;}
                result = -B / condVal;
                break;
            case 2: /* prod=C/A=P => A=C/P */
                if (fabs(condVal)<MATH_EPS){valid=false;break;}
                result = C / condVal;
                break;
            case 3: /* disc=B^2-4AC=d^2A^2 => A(4C+d^2A)=B^2 — quadratic in A */
            {
                double dd = condVal*condVal;
                /* dd*A^2 + 4C*A - B^2 = 0 */
                if (fabs(dd) < MATH_EPS) { valid=false; break; }
                double disc2 = 16.0*C*C + 4.0*dd*B*B;
                if (disc2 < -MATH_EPS) { valid=false; break; }
                double A1 = (-4.0*C + sqrt(fabs(disc2))) / (2.0*dd);
                double A2 = (-4.0*C - sqrt(fabs(disc2))) / (2.0*dd);
                formatNumber(n1, sizeof(n1), A1);
                snprintf(lineBuf, sizeof(lineBuf), "A = %s", n1);
                printLine(lineBuf, COL_GREEN);
                formatNumber(n1, sizeof(n1), A2);
                snprintf(lineBuf, sizeof(lineBuf), "  or A = %s", n1);
                printLine(lineBuf, COL_GREEN);
                waitContinue();
                return;
            }
            default: valid=false; break;
        }
    }

    if (!valid) {
        printLine("Cannot determine from", COL_RED);
        printLine("this condition alone", COL_RED);
    } else {
        char label[4] = {'A'+unknownCoeff-1, '\0', '\0', '\0'};
        formatNumber(n1, sizeof(n1), result);
        snprintf(lineBuf, sizeof(lineBuf), "%s = %s", label, n1);
        printLine(lineBuf, COL_GREEN);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   6. COMMON ROOT — GENERAL (any degree up to 3)
   ───────────────────────────────────────────── */
static void solveCommonRoot(void)
{
    RESET_CANCEL();
    startScreen("COMMON ROOT", "[CLEAR] Back");
    printSubheader("Two polys, same unknown k");
    printBlank();

    double degVal = inputNumber("Degree = "); CHECK_CANCEL;
    int deg = (int)round(degVal);

    if (deg < 2 || deg > 3) {
        printDivider();
        printLine("Degree 2 or 3 only", COL_RED);
        waitContinue();
        return;
    }

    /* Both polynomials share the same k in the SAME position */
    printLine("Same k in same position", COL_NAVY);
    printLine("in both polynomials.", COL_NAVY);

    double p1[4]={0}, p2[4]={0};
    int kPos = -1;

    for (int poly = 0; poly < 2; poly++) {
        char titleBuf[20];
        snprintf(titleBuf, sizeof(titleBuf), "POLYNOMIAL %d", poly+1);
        startScreen(titleBuf, "[CLEAR] Back");
        printSubheader("Enter 0 for k position");
        printBlank();

        for (int i = 0; i <= deg; i++) {
            char prompt[14];
            snprintf(prompt, sizeof(prompt), "x^%d coeff = ", deg-i);
            double cv = inputNumber(prompt); CHECK_CANCEL;
            (poly==0 ? p1 : p2)[i] = cv;

            if (fabs(cv) < MATH_EPS && kPos < 0 && poly == 0) {
                drawFooter("Is this k? 1=Yes 2=No");
                blit();
                uint8_t kc;
                do { kc = waitForKey(); }
                while (kc!=sk_1 && kc!=sk_2 && kc!=sk_Clear);
                if (kc==sk_Clear){gUserCancelled=true;return;}
                if (kc==sk_1) kPos = i;
            }
        }
        if (poly==0 && kPos >= 0) p2[kPos] = 0;  /* mark same position */
    }

    if (kPos < 0) {
        printDivider();
        printLine("No k position marked", COL_RED);
        waitContinue();
        return;
    }

    /* Subtract p1 - p2 to eliminate k term:
     * Since k appears in same position in both, it cancels.
     * Result is a polynomial of degree < deg with no unknowns.
     * Solve it for the common root x0.                         */
    double diff[4] = {0};
    for (int i = 0; i <= deg; i++) diff[i] = p1[i] - p2[i];
    /* diff[kPos] = 0 by construction */

    /* Find the root of the difference polynomial */
    /* For degree 2/3 difference is degree (deg-1) at most */
    /* Find effective degree of diff */
    int effDeg = 0;
    for (int i = 0; i <= deg; i++) {
    if (fabs(diff[i]) > MATH_EPS) {
        effDeg = deg - i;
        break;
    }
}

    double x0 = 0.0;
    bool   found = false;
    char   lineBuf[52], n1[20], n2[20];

    printDivider();

    if (effDeg == 1) {
        /* Linear: a*x + b = 0 */
        int ai = -1, bi = -1;
        for (int i = 0; i <= deg; i++) {
            if (deg-i == 1 && fabs(diff[i])>MATH_EPS) ai=i;
            if (deg-i == 0 && fabs(diff[i])>MATH_EPS) bi=i;
        }
        if (ai >= 0 && fabs(diff[ai]) > MATH_EPS) {
            x0 = (bi>=0 ? -diff[bi] : 0.0) / diff[ai];
            found = true;
        }
    } else if (effDeg == 2) {
        /* Quadratic: use quadratic formula */
        double qa=0, qb=0, qc=0;
        for (int i=0;i<=deg;i++){
            if (deg-i==2) qa=diff[i];
            if (deg-i==1) qb=diff[i];
            if (deg-i==0) qc=diff[i];
        }
        double disc2 = qb*qb-4.0*qa*qc;
        if (disc2 >= -MATH_EPS) {
            x0 = (-qb+sqrt(fabs(disc2)))/(2.0*qa);
            found = true;
            /* Also try the other root */
            double x0b = (-qb-sqrt(fabs(disc2)))/(2.0*qa);
            formatNumber(n1, sizeof(n1), x0);
            formatNumber(n2, sizeof(n2), x0b);
            snprintf(lineBuf, sizeof(lineBuf),
                     "Candidate roots: %s, %s", n1, n2);
            printLine(lineBuf, COL_BLACK);
        }
    }

    if (!found) {
        printLine("Cannot find common root", COL_RED);
        waitContinue();
        return;
    }

    formatNumber(n1, sizeof(n1), x0);
    snprintf(lineBuf, sizeof(lineBuf), "Common root x = %s", n1);
    printLine(lineBuf, COL_BLACK);

    /* Substitute x0 into p1 to find k:
     * p1(x0) = S + k * x0^(deg-kPos) = 0
     * k = -S / x0^(deg-kPos)              */
    int kPow = deg - kPos;
    double rPow = 1.0;
    for (int i = 0; i < kPow; i++) rPow *= x0;

    double S = evalPoly(p1, deg, x0); /* p1[kPos]=0 already */

    if (fabs(rPow) < MATH_EPS) {
        printLine("k indeterminate (x0=0)", COL_ORANGE);
    } else {
        double k = -S / rPow;
        formatNumber(n1, sizeof(n1), k);
        snprintf(lineBuf, sizeof(lineBuf), "k = %s", n1);
        printLine(lineBuf, COL_GREEN);

        /* Verify in p2 */
        p2[kPos] = k;
        double checkP2 = evalPoly(p2, deg, x0);
        formatNumber(n1, sizeof(n1), checkP2);
        snprintf(lineBuf, sizeof(lineBuf), "Verify P2(x0) = %s", n1);
        printLine(lineBuf, fabs(checkP2)<1e-6 ? COL_GRAY : COL_RED);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   7. NEGATIVE RECIPROCAL ROOTS — GENERAL degree
   ───────────────────────────────────────────── */
static void solveNegRecipRoots(void)
{
    RESET_CANCEL();
    startScreen("NEG. RECIPROCAL ROOTS", "[CLEAR] Back");
    printSubheader("f(x)->g(x), roots=-1/ri");
    printBlank();

    double degVal = inputNumber("Degree (2 or 3) = "); CHECK_CANCEL;
    int deg = (int)round(degVal);

    if (deg != 2 && deg != 3) {
        printDivider();
        printLine("Degree 2 or 3 only", COL_RED);
        waitContinue();
        return;
    }

    printLine("f(x): most sig. first", COL_NAVY);
    double coeffs[4] = {0};
    for (int i = 0; i <= deg; i++) {
        char prompt[14];
        snprintf(prompt, sizeof(prompt), "x^%d coeff = ", deg-i);
        coeffs[i] = inputNumber(prompt); CHECK_CANCEL;
    }

    if (fabs(coeffs[deg]) < MATH_EPS) {
        printDivider();
        printLine("Constant term = 0", COL_RED);
        printLine("(root 0 has no recip)", COL_BLACK);
        waitContinue();
        return;
    }

    /*
     * Substitute x = -1/t into f(t) and multiply by t^deg:
     * For f(x) = a0*x^n + a1*x^(n-1) + ... + an
     * g(t) = an*t^n - a(n-1)*t^(n-1) + ... + (-1)^n * a0
     * i.e., reverse and alternate signs.
     */
    double gCoeffs[4] = {0};
    for (int i = 0; i <= deg; i++) {
        double sign = ((deg - i) % 2 == 0) ? 1.0 : -1.0;
        gCoeffs[i] = sign * coeffs[deg - i];
    }

    /* Make leading coefficient positive */
    if (gCoeffs[0] < 0)
        for (int i = 0; i <= deg; i++) gCoeffs[i] = -gCoeffs[i];

    /* Reduce if all integers */
    bool allInt = true;
    long long iCoeffs[4] = {0};
    for (int i = 0; i <= deg; i++) {
        iCoeffs[i] = (long long)round(gCoeffs[i]);
        if (fabs(gCoeffs[i] - iCoeffs[i]) > MATH_EPS) allInt = false;
    }

    if (allInt) {
        long long g = llabs(iCoeffs[0]);
        for (int i = 1; i <= deg; i++)
            if (iCoeffs[i])
                g = (long long)gcd32((uint32_t)llabs(g),
                                     (uint32_t)llabs(iCoeffs[i]));
        if (g > 1)
            for (int i = 0; i <= deg; i++) iCoeffs[i] /= g;
    }

    printDivider();
    printLine("g(x) coefficients:", COL_ORANGE);

    char lineBuf[52], n1[20];
    long long sumI = 0;
    double    sumD = 0.0;

    for (int i = 0; i <= deg; i++) {
        int pw = deg - i;
        if (allInt) {
            snprintf(lineBuf, sizeof(lineBuf),
                     "  x^%d: %lld", pw, iCoeffs[i]);
            sumI += iCoeffs[i];
        } else {
            formatNumber(n1, sizeof(n1), gCoeffs[i]);
            snprintf(lineBuf, sizeof(lineBuf), "  x^%d: %s", pw, n1);
            sumD += gCoeffs[i];
        }
        printLine(lineBuf, COL_GREEN);
    }

    if (allInt) {
        snprintf(lineBuf, sizeof(lineBuf),
                 "A+B+C+D = %lld", sumI);
        printLine(lineBuf, COL_BLACK);
    } else {
        formatNumber(n1, sizeof(n1), sumD);
        snprintf(lineBuf, sizeof(lineBuf), "Sum of coeffs = %s", n1);
        printLine(lineBuf, COL_BLACK);
    }

    waitContinue();
}

/*
 * Radical Simplifier — nth root of any positive integer.
 *
 * Algorithm:
 *   1. Factorize X using the existing Pollard's rho engine.
 *   2. For each prime p with exponent e in the factorization:
 *        outside *= p ^ (e / n)      <- perfect nth power part
 *        inside  *= p ^ (e % n)      <- leftover stays under radical
 *   3. Result:  outside * nthroot(inside)
 *
 * Example n=2, X=12:
 *   12 = 2^2 * 3^1
 *   p=2: outside *= 2^(2/2)=2^1=2,  inside *= 2^(2%2)=2^0=1
 *   p=3: outside *= 3^(3/2)=3^0=1,  inside *= 3^(3%1)=3^1=3
 *   => 2 * sqrt(3)
 *
 * Example n=3, X=108:
 *   108 = 2^2 * 3^3
 *   p=2: outside*=2^(2/3)=1,  inside*=2^(2%3)=4
 *   p=3: outside*=3^(3/3)=3,  inside*=3^(3%3)=1
 *   => 3 * cbrt(4)
 */
/*
 * Radical Simplifier — nth root of any positive integer.
 *
 * Algorithm:
 *   1. Factorize X using the existing Pollard's rho engine.
 *   2. For each prime p with exponent e in the factorization:
 *        outside *= p ^ (e / n)      <- perfect nth power part
 *        inside  *= p ^ (e % n)      <- leftover stays under radical
 *   3. Result:  outside * nthroot(inside)
 *   4. Always show decimal approximation.
 *   5. Use continued fractions to find best p/q approx (q<=9999),
 *      clearly marked as approximate when the result is irrational.
 *
 * Example n=2, X=12:
 *   12 = 2^2 * 3^1
 *   outside=2, inside=3 => 2 * sqrt(3)
 *   decimal = 3.46410...
 *   fraction ~ 97/28  (approx)
 */
static void solveRadicalSimplifier(void)
{
    RESET_CANCEL();
    startScreen("RADICAL SIMPLIFIER", "[CLEAR] Back");
    printSubheader("Simplify n-th root of X");
    printBlank();

    /* Root index n */
    bool overflow = false;
    uint32_t n = inputUInt32("Root index n = ", &overflow); CHECK_CANCEL;

    if (overflow || n < 2) {
        printDivider();
        printLine("n must be >= 2", COL_RED);
        waitContinue();
        return;
    }

    /* Radicand X */
    uint32_t X = inputUInt32("Radicand X = ", &overflow); CHECK_CANCEL;

    if (overflow) {
        printDivider();
        printLine("Too large (max 4294967295)", COL_RED);
        waitContinue();
        return;
    }
    if (X < 1) {
        printDivider();
        printLine("X must be >= 1", COL_RED);
        waitContinue();
        return;
    }

    printDivider();

    char lineBuf[52];

    /* ── X = 1: trivially 1 ── */
    if (X == 1) {
        char rootLabel[12];
        if      (n == 2) snprintf(rootLabel, sizeof(rootLabel), "sqrt");
        else if (n == 3) snprintf(rootLabel, sizeof(rootLabel), "cbrt");
        else             snprintf(rootLabel, sizeof(rootLabel), "%u-root", (unsigned)n);

        snprintf(lineBuf, sizeof(lineBuf), "%s(1) = 1", rootLabel);
        printLine(lineBuf, COL_GREEN);
        printLine("= 1.0 (decimal)", COL_BLACK);
        /* No fraction needed — it is exactly 1 */
        waitContinue();
        return;
    }

    /* ── Factorize X ── */
    if (!calculateFactorization(X)) {
        printLine("X must be >= 2", COL_RED);
        waitContinue();
        return;
    }

    /* ── Walk factor list, extract perfect nth powers ── */
    uint32_t outside = 1;
    uint32_t inside  = 1;
    bool     outsideOverflow = false;
    bool     insideOverflow  = false;

    int i = 0;
    while (i < factorCount) {
        uint32_t base = factorList[i];
        int      exp  = 0;
        while (i < factorCount && factorList[i] == base) { exp++; i++; }

        int outsideExp = exp / (int)n;
        int insideExp  = exp % (int)n;

        for (int k = 0; k < outsideExp; k++) {
            if (outside > 0xFFFFFFFFUL / base) { outsideOverflow = true; break; }
            outside *= base;
        }
        for (int k = 0; k < insideExp; k++) {
            if (inside > 0xFFFFFFFFUL / base) { insideOverflow = true; break; }
            inside *= base;
        }
    }

    /* ── Build root label ── */
    char rootLabel[12];
    if      (n == 2) snprintf(rootLabel, sizeof(rootLabel), "sqrt");
    else if (n == 3) snprintf(rootLabel, sizeof(rootLabel), "cbrt");
    else             snprintf(rootLabel, sizeof(rootLabel), "%u-root", (unsigned)n);

    /* ── Show original expression ── */
    snprintf(lineBuf, sizeof(lineBuf), "%s(%lu) =", rootLabel, (unsigned long)X);
    printLine(lineBuf, COL_BLACK);

    /* ── Show simplified radical form ── */
    bool isExact = (inside == 1);   /* perfect nth power — whole number result */

    if (outsideOverflow || insideOverflow) {
        printLine("Overflow in simplification", COL_RED);
    } else if (isExact) {
        snprintf(lineBuf, sizeof(lineBuf), "%lu  (exact integer)",
                 (unsigned long)outside);
        printLine(lineBuf, COL_GREEN);
    } else if (outside == 1) {
        snprintf(lineBuf, sizeof(lineBuf), "%s(%lu)",
                 rootLabel, (unsigned long)inside);
        printLine(lineBuf, COL_ORANGE);
        printLine("Already fully simplified", COL_BLACK);
    } else {
        snprintf(lineBuf, sizeof(lineBuf), "%lu * %s(%lu)",
                 (unsigned long)outside, rootLabel, (unsigned long)inside);
        printLine(lineBuf, COL_GREEN);
    }

    /* ── Always show decimal approximation ── */
    double decimalVal = (double)outside * pow((double)inside, 1.0 / (double)n);
    char   decBuf[24];
    snprintf(decBuf, sizeof(decBuf), "%.10g", decimalVal);
    snprintf(lineBuf, sizeof(lineBuf), "= %s (decimal)", decBuf);
    printLine(lineBuf, COL_BLACK);

    /* ── Fraction approximation via continued fractions ──
     *
     * Only shown when result is NOT an exact integer.
     * Clearly marked "approx" whenever inside != 1.
     * Stops when denominator exceeds 9999 or error < 1e-7.
     */
    if (!isExact && !outsideOverflow && !insideOverflow) {
        double  val = decimalVal;
        long    h0=1, h1=0;
        long    k0=0, k1=1;
        bool    foundFrac = false;

        for (int step = 0; step < 30; step++) {
            long   a  = (long)floor(val);
            long   h2 = a * h0 + h1;
            long   k2 = a * k0 + k1;

            if (k2 > 9999) break;

            double approxErr = fabs((double)h2 / (double)k2 - decimalVal);

            if (approxErr < 1e-7) {
                char fracBuf[40];
                if (k2 == 1) {
                    /* Should not reach here since isExact=false, but guard anyway */
                    snprintf(fracBuf, sizeof(fracBuf), "= %ld (exact)", h2);
                    printLine(fracBuf, COL_GREEN);
                } else {
                    /* Always label as approx — the result is irrational */
                    snprintf(fracBuf, sizeof(fracBuf), "~ %ld/%ld (approx frac)",
                             h2, k2);
                    printLine(fracBuf, COL_GRAY);
                }
                foundFrac = true;
                break;
            }

            double remainder = val - (double)a;
            if (fabs(remainder) < 1e-10) break;

            h1=h0; h0=h2;
            k1=k0; k0=k2;
            val = 1.0 / remainder;
        }

        if (!foundFrac) {
            printLine("(no simple fraction <= 9999)", COL_GRAY);
        }
    }

    /* ── Show factorization for reference ── */
    printBlank();
    printLine("Factorization:", COL_ORANGE);

    char factBuf[52];
    factBuf[0] = '\0';
    int fi = 0;
    bool firstTerm = true;
    while (fi < factorCount) {
        uint32_t base = factorList[fi];
        int      exp  = 0;
        while (fi < factorCount && factorList[fi] == base) { exp++; fi++; }
        char term[16];
        if (exp == 1) snprintf(term, sizeof(term), "%lu", (unsigned long)base);
        else          snprintf(term, sizeof(term), "%lu^%d", (unsigned long)base, exp);
        if (!firstTerm)
            strncat(factBuf, " * ", sizeof(factBuf)-strlen(factBuf)-1);
        strncat(factBuf, term, sizeof(factBuf)-strlen(factBuf)-1);
        firstTerm = false;
    }
    printLine(factBuf, COL_BLACK);

    waitContinue();
}

/* ═══════════════════════════════════════════════
   SEQUENCES AND SERIES
   ═══════════════════════════════════════════════ */

/*
 * Arithmetic Sequence solver — all 10 valid 3-of-5 combinations.
 *
 * Variables:  a1  d  n  an  Sn
 * Formulas:
 *   an = a1 + (n-1)*d
 *   Sn = n/2 * (a1 + an)   [equivalently  n*a1 + d*n*(n-1)/2]
 *
 * The solver iterates rules until no more progress, then handles
 * the two quadratic-in-n cases explicitly.
 */
static void solveArithmeticSequence(void)
{
    RESET_CANCEL();
    startScreen("ARITHMETIC SEQUENCE", "[CLEAR] Back");
    printSubheader("an = a1 + (n-1)d");
    printBlank();

    printLine("Mark 3 known variables:", COL_NAVY);
    printLine("1:a1  2:d  3:n  4:an  5:Sn", COL_ORANGE);
    printLine("Type 3 digits then ENTER", COL_GRAY);
    blit();

    /* ── 3-digit variable selection ── */
    char sel[4] = "";
    int  selLen  = 0;
    int  savedLine = gCurrentLine;

    for (;;) {
        int rowY = BODY_TOP + savedLine * LINE_H;
        gfx_SetColor(COL_WHITE);
        gfx_FillRectangle_NoClip(0, rowY - 1, SCR_W, LINE_H + 2);
        char disp[8];
        snprintf(disp, sizeof(disp), "%s_", sel);
        gfx_SetTextFGColor(COL_BLACK);
        gfx_SetTextBGColor(COL_WHITE);
        gfx_PrintStringXY(disp, LMARGIN, rowY);
        blit();

        uint8_t key = waitForKey();
        if (key == sk_Clear) { gUserCancelled = true; return; }
        if (key == sk_Enter && selLen == 3) break;
        if (key == sk_Del && selLen > 0) { sel[--selLen] = '\0'; continue; }

        char digit = '\0';
        if      (key == sk_1) digit = '1'; else if (key == sk_2) digit = '2';
        else if (key == sk_3) digit = '3'; else if (key == sk_4) digit = '4';
        else if (key == sk_5) digit = '5';

        if (digit && selLen < 3) {
            bool dup = false;
            for (int i = 0; i < selLen; i++)
                if (sel[i] == digit) { dup = true; break; }
            if (!dup) { sel[selLen++] = digit; sel[selLen] = '\0'; }
        }
    }
    gCurrentLine = savedLine + 1;

    bool knowA1 = false, knowD  = false, knowN  = false,
         knowAN = false, knowSN = false;
    for (int i = 0; i < 3; i++) {
        switch (sel[i]) {
            case '1': knowA1=true; break; case '2': knowD=true;  break;
            case '3': knowN=true;  break; case '4': knowAN=true; break;
            case '5': knowSN=true; break;
        }
    }

    /* ── Collect values ── */
    double a1=0, d=0, n=0, an=0, Sn=0;

    startScreen("ARITHMETIC SEQUENCE", "[CLEAR] Back");
    printSubheader("Enter known values:");
    printBlank();

    if (knowA1) { a1 = inputNumber("a1 = "); CHECK_CANCEL; }
    if (knowD)  { d  = inputNumber("d  = "); CHECK_CANCEL; }
    if (knowN)  { n  = inputNumber("n  = "); CHECK_CANCEL; }
    if (knowAN) { an = inputNumber("an = "); CHECK_CANCEL; }
    if (knowSN) { Sn = inputNumber("Sn = "); CHECK_CANCEL; }

    /* ── Solve: iterate until no more progress ── */
    bool progress = true;
    for (int pass = 0; pass < 20 && progress; pass++) {
        progress = false;

        /* From  an = a1 + (n-1)*d */
        if (!knowAN && knowA1 && knowD && knowN) {
            an = a1 + (n-1.0)*d; knowAN=true; progress=true;
        }
        if (!knowA1 && knowAN && knowD && knowN) {
            a1 = an - (n-1.0)*d; knowA1=true; progress=true;
        }
        if (!knowD && knowA1 && knowAN && knowN && fabs(n-1.0) > MATH_EPS) {
            d = (an-a1)/(n-1.0); knowD=true; progress=true;
        }
        if (!knowN && knowA1 && knowAN && knowD && fabs(d) > MATH_EPS) {
            n = (an-a1)/d + 1.0; knowN=true; progress=true;
        }

        /* From  Sn = n/2*(a1+an)  */
        if (!knowSN && knowN && knowA1 && knowAN) {
            Sn = n/2.0*(a1+an); knowSN=true; progress=true;
        }
        if (!knowN && knowSN && knowA1 && knowAN && fabs(a1+an) > MATH_EPS) {
            n = 2.0*Sn/(a1+an); knowN=true; progress=true;
        }
        if (!knowA1 && knowSN && knowN && knowAN && fabs(n) > MATH_EPS) {
            a1 = 2.0*Sn/n - an; knowA1=true; progress=true;
        }
        if (!knowAN && knowSN && knowN && knowA1 && fabs(n) > MATH_EPS) {
            an = 2.0*Sn/n - a1; knowAN=true; progress=true;
        }

        /* From  Sn = n*a1 + d*n*(n-1)/2  */
        if (!knowSN && knowA1 && knowD && knowN) {
            Sn = n*a1 + d*n*(n-1.0)/2.0; knowSN=true; progress=true;
        }
        if (!knowA1 && knowSN && knowD && knowN && fabs(n) > MATH_EPS) {
            a1 = (2.0*Sn - d*n*(n-1.0)) / (2.0*n);
            knowA1=true; progress=true;
        }
        if (!knowD && knowSN && knowA1 && knowN && fabs(n*(n-1.0)) > MATH_EPS) {
            d = 2.0*(Sn - n*a1) / (n*(n-1.0));
            knowD=true; progress=true;
        }

        /* Quadratic: find n from {a1, d, Sn}
         *   d*n^2 + (2*a1 - d)*n - 2*Sn = 0                */
        if (!knowN && knowA1 && knowD && knowSN && fabs(d) > MATH_EPS) {
            double A = d, B = 2.0*a1 - d, C = -2.0*Sn;
            double disc = B*B - 4.0*A*C;
            if (disc >= -MATH_EPS) {
                double sq = sqrt(fabs(disc));
                double n1 = (-B + sq) / (2.0*A);
                double n2 = (-B - sq) / (2.0*A);
                double nSel = 0.0; bool found = false;
                if (n1 > 0.5 && fabs(n1-round(n1)) < 0.01) { nSel=round(n1); found=true; }
                else if (n2 > 0.5 && fabs(n2-round(n2)) < 0.01) { nSel=round(n2); found=true; }
                if (found) { n=nSel; knowN=true; progress=true; }
            }
        }

        /* Quadratic: find n from {d, an, Sn}
         *   d*n^2 - (2*an + d)*n + 2*Sn = 0                */
        if (!knowN && knowD && knowAN && knowSN && fabs(d) > MATH_EPS) {
            double A = d, B = -(2.0*an + d), C = 2.0*Sn;
            double disc = B*B - 4.0*A*C;
            if (disc >= -MATH_EPS) {
                double sq = sqrt(fabs(disc));
                double n1 = (-B + sq) / (2.0*A);
                double n2 = (-B - sq) / (2.0*A);
                double nSel = 0.0; bool found = false;
                if (n1 > 0.5 && fabs(n1-round(n1)) < 0.01) { nSel=round(n1); found=true; }
                else if (n2 > 0.5 && fabs(n2-round(n2)) < 0.01) { nSel=round(n2); found=true; }
                if (found) { n=nSel; knowN=true; progress=true; }
            }
        }
    }

    /* ── Display ── */
    startScreen("ARITHMETIC SEQUENCE", "[ENTER] Done");
    printSubheader("Results");
    printBlank();

    char nb[28], lineBuf[52];

    if (!knowA1 || !knowD || !knowN || !knowAN || !knowSN) {
        printLine("Could not solve fully.", COL_RED);
        printLine("Try a different 3 vars.", COL_BLACK);
        waitContinue();
        return;
    }

    formatFraction(nb, sizeof(nb), a1);
    snprintf(lineBuf, sizeof(lineBuf), "a1 = %s", nb); printLine(lineBuf, COL_GREEN);

    formatFraction(nb, sizeof(nb), d);
    snprintf(lineBuf, sizeof(lineBuf), "d  = %s", nb); printLine(lineBuf, COL_GREEN);

    formatNumber(nb, sizeof(nb), round(n));
    snprintf(lineBuf, sizeof(lineBuf), "n  = %s", nb); printLine(lineBuf, COL_GREEN);

    formatFraction(nb, sizeof(nb), an);
    snprintf(lineBuf, sizeof(lineBuf), "an = %s", nb); printLine(lineBuf, COL_GREEN);

    formatFraction(nb, sizeof(nb), Sn);
    snprintf(lineBuf, sizeof(lineBuf), "Sn = %s", nb); printLine(lineBuf, COL_GREEN);

    waitContinue();
}
/*
 * Geometric Sequence solver.
 *
 * Variables:  a1  r  n  an  Sn
 * Formulas:
 *   an = a1 * r^(n-1)
 *   Sn = a1*(1-r^n)/(1-r)   r != 1
 *   Sn = a1*n                r == 1
 *   S∞ = a1/(1-r)            |r| < 1
 *
 * New rules added vs original:
 *   n from {a1,r,an}  → n = log(an/a1)/log(r) + 1
 *   n from {a1,r,Sn}  → n = log(1-Sn(1-r)/a1)/log(r)
 */
static void solveGeometricSequence(void)
{
    RESET_CANCEL();
    startScreen("GEOMETRIC SEQUENCE", "[CLEAR] Back");
    printSubheader("an = a1 * r^(n-1)");
    printBlank();

    printLine("Mark known variables:", COL_NAVY);
    printLine("1:a1  2:r  3:n  4:an  5:Sn", COL_ORANGE);
    printLine("Type 2-3 digits then ENTER", COL_GRAY);
    blit();

    char sel[4] = "";
    int  selLen  = 0;
    int  savedLine = gCurrentLine;

    for (;;) {
        int rowY = BODY_TOP + savedLine * LINE_H;
        gfx_SetColor(COL_WHITE);
        gfx_FillRectangle_NoClip(0, rowY - 1, SCR_W, LINE_H + 2);
        char disp[8];
        snprintf(disp, sizeof(disp), "%s_", sel);
        gfx_SetTextFGColor(COL_BLACK);
        gfx_SetTextBGColor(COL_WHITE);
        gfx_PrintStringXY(disp, LMARGIN, rowY);
        blit();

        uint8_t key = waitForKey();
        if (key == sk_Clear) { gUserCancelled = true; return; }
        if (key == sk_Enter && selLen >= 2) break;
        if (key == sk_Del && selLen > 0) { sel[--selLen] = '\0'; continue; }

        char digit = '\0';
        if      (key == sk_1) digit = '1'; else if (key == sk_2) digit = '2';
        else if (key == sk_3) digit = '3'; else if (key == sk_4) digit = '4';
        else if (key == sk_5) digit = '5';

        if (digit && selLen < 3) {
            bool dup = false;
            for (int i = 0; i < selLen; i++)
                if (sel[i] == digit) { dup = true; break; }
            if (!dup) { sel[selLen++] = digit; sel[selLen] = '\0'; }
        }
    }
    gCurrentLine = savedLine + 1;

    bool knowA1=false, knowR=false, knowN=false, knowAN=false, knowSN=false;
    for (int i = 0; sel[i]; i++) {
        switch (sel[i]) {
            case '1': knowA1=true; break; case '2': knowR=true;  break;
            case '3': knowN=true;  break; case '4': knowAN=true; break;
            case '5': knowSN=true; break;
        }
    }

    double a1=0, r=0, n=0, an=0, Sn=0;

    startScreen("GEOMETRIC SEQUENCE", "[CLEAR] Back");
    printSubheader("Enter known values:");
    printBlank();

    if (knowA1) { a1 = inputNumber("a1 = "); CHECK_CANCEL; }
    if (knowR)  { r  = inputNumber("r  = "); CHECK_CANCEL; }
    if (knowN)  { n  = inputNumber("n  = "); CHECK_CANCEL; }
    if (knowAN) { an = inputNumber("an = "); CHECK_CANCEL; }
    if (knowSN) { Sn = inputNumber("Sn = "); CHECK_CANCEL; }

    /* ── Solve ── */
    bool progress = true;
    for (int pass = 0; pass < 20 && progress; pass++) {
        progress = false;

        /* an = a1 * r^(n-1) */
        if (!knowAN && knowA1 && knowR && knowN) {
            an = a1 * pow(r, n-1.0); knowAN=true; progress=true;
        }
        if (!knowA1 && knowAN && knowR && knowN) {
            double rp = pow(r, n-1.0);
            if (fabs(rp) > MATH_EPS) { a1 = an/rp; knowA1=true; progress=true; }
        }
        if (!knowR && knowAN && knowA1 && knowN
                && fabs(a1) > MATH_EPS && fabs(n-1.0) > MATH_EPS) {
            r = pow(an/a1, 1.0/(n-1.0)); knowR=true; progress=true;
        }
        /* n from {a1, r, an} via log */
        if (!knowN && knowA1 && knowR && knowAN
                && fabs(a1) > MATH_EPS && r > 0.0 && fabs(r-1.0) > MATH_EPS) {
            double ratio = an / a1;
            if (ratio > 0.0) {
                double nCalc = log(ratio) / log(r) + 1.0;
                if (nCalc > 0.5) { n = round(nCalc); knowN=true; progress=true; }
            }
        }

        /* Sn = a1*(1-r^n)/(1-r) */
        if (!knowSN && knowA1 && knowR && knowN) {
            if (fabs(r-1.0) < MATH_EPS) Sn = a1*n;
            else Sn = a1*(1.0-pow(r,n))/(1.0-r);
            knowSN=true; progress=true;
        }
        if (!knowA1 && knowSN && knowR && knowN) {
            if (fabs(r-1.0) < MATH_EPS) {
                if (fabs(n) > MATH_EPS) { a1 = Sn/n; knowA1=true; progress=true; }
            } else {
                double denom = (1.0-pow(r,n))/(1.0-r);
                if (fabs(denom) > MATH_EPS) { a1 = Sn/denom; knowA1=true; progress=true; }
            }
        }
        /* n from {a1, r, Sn} via log */
        if (!knowN && knowA1 && knowR && knowSN
                && fabs(a1) > MATH_EPS && r > 0.0 && fabs(r-1.0) > MATH_EPS) {
            double x = 1.0 - Sn*(1.0-r)/a1;
            if (x > 0.0) {
                double nCalc = log(x) / log(r);
                if (nCalc > 0.5) { n = round(nCalc); knowN=true; progress=true; }
            }
        }
    }

    /* ── Display ── */
    startScreen("GEOMETRIC SEQUENCE", "[ENTER] Done");
    printSubheader("Results");
    printBlank();

    char nb[28], lineBuf[52];
    bool anyKnown = knowA1 || knowR || knowN || knowAN || knowSN;

    if (!anyKnown) {
        printLine("Could not solve.", COL_RED);
        waitContinue(); return;
    }

    if (knowA1) {
        formatFraction(nb, sizeof(nb), a1);
        snprintf(lineBuf, sizeof(lineBuf), "a1 = %s", nb); printLine(lineBuf, COL_GREEN);
    }
    if (knowR) {
        formatFraction(nb, sizeof(nb), r);
        snprintf(lineBuf, sizeof(lineBuf), "r  = %s", nb); printLine(lineBuf, COL_GREEN);
    }
    if (knowN) {
        formatNumber(nb, sizeof(nb), round(n));
        snprintf(lineBuf, sizeof(lineBuf), "n  = %s", nb); printLine(lineBuf, COL_GREEN);
    }
    if (knowAN) {
        formatFraction(nb, sizeof(nb), an);
        snprintf(lineBuf, sizeof(lineBuf), "an = %s", nb); printLine(lineBuf, COL_GREEN);
    }
    if (knowSN) {
        formatFraction(nb, sizeof(nb), Sn);
        snprintf(lineBuf, sizeof(lineBuf), "Sn = %s", nb); printLine(lineBuf, COL_GREEN);
    }
    if (knowA1 && knowR && fabs(r) < 1.0 - MATH_EPS) {
        double Si = a1 / (1.0-r);
        formatFraction(nb, sizeof(nb), Si);
        snprintf(lineBuf, sizeof(lineBuf), "S-inf = %s", nb);
        printLine(lineBuf, COL_BLACK);
    }

    waitContinue();
}
/*
 * Arithmetic Means solver.
 * Insert k arithmetic means between two values a and b.
 * The full sequence is: a, m1, m2, ..., mk, b
 * Total terms = k+2, so d = (b-a)/(k+1).
 */
static void solveArithmeticMeans(void)
{
    RESET_CANCEL();
    startScreen("ARITHMETIC MEANS", "[CLEAR] Back");
    printSubheader("Insert k means between a,b");
    printBlank();

    double a = inputNumber("First value a = ");  CHECK_CANCEL;
    double b = inputNumber("Last value b  = ");  CHECK_CANCEL;
    double k = inputNumber("# of means k  = ");  CHECK_CANCEL;

    int numMeans = (int)round(k);
    if (numMeans < 1) {
        printDivider();
        printLine("k must be >= 1", COL_RED);
        waitContinue();
        return;
    }

    double d = (b - a) / (double)(numMeans + 1);

   startScreen("ARITHMETIC MEANS", "[ENTER] Done");
    printSubheader("Inserts k means: a,m1..mk,b");
    printBlank();

    char nb[28], lineBuf[52];

    formatFraction(nb, sizeof(nb), d);
    snprintf(lineBuf, sizeof(lineBuf), "d = %s", nb);
    printLine(lineBuf, COL_BLACK);
    printBlank();

    printLine("Full sequence:", COL_ORANGE);
    for (int i = 0; i <= numMeans + 1 && gCurrentLine < MAX_LINES - 1; i++) {
        double val = a + (double)i * d;
        formatFraction(nb, sizeof(nb), val);
        snprintf(lineBuf, sizeof(lineBuf), "  term %d = %s", i + 1, nb);
        printLine(lineBuf, COL_GREEN);
    }
    if (numMeans + 2 > MAX_LINES - 3)
        printLine("  (truncated)", COL_GRAY);

    double sumMeans = 0.0;
    for (int i = 1; i <= numMeans; i++) sumMeans += a + (double)i * d;
    formatFraction(nb, sizeof(nb), sumMeans);
    snprintf(lineBuf, sizeof(lineBuf), "Sum of means = %s", nb);
    printLine(lineBuf, COL_BLACK);

    waitContinue();
}

/*
 * Geometric Means solver.
 * Insert k geometric means between a and b.
 * r = (b/a)^(1/(k+1))
 */
static void solveGeometricMeans(void)
{
    RESET_CANCEL();
    startScreen("GEOMETRIC MEANS", "[CLEAR] Back");
    printSubheader("Insert k means between a,b");
    printBlank();

    double a = inputNumber("First value a = "); CHECK_CANCEL;
    double b = inputNumber("Last value b  = "); CHECK_CANCEL;
    double k = inputNumber("# of means k  = "); CHECK_CANCEL;

    int numMeans = (int)round(k);

    printDivider();
    char nb[20], lineBuf[52];

    if (numMeans < 1) {
        printLine("k must be >= 1", COL_RED);
        waitContinue();
        return;
    }
    if (fabs(a) < MATH_EPS) {
        printLine("a cannot be 0", COL_RED);
        waitContinue();
        return;
    }
    if ((a < 0 && b > 0) || (a > 0 && b < 0)) {
        printLine("a and b must have same sign", COL_RED);
        waitContinue();
        return;
    }

    double r = pow(b / a, 1.0 / (double)(numMeans + 1));

    startScreen("GEOMETRIC MEANS", "[ENTER] Done");
    printSubheader("Inserts k means: a,m1..mk,b");
    printBlank();


    formatFraction(nb, sizeof(nb), r);
    snprintf(lineBuf, sizeof(lineBuf), "r = %s", nb);
    printLine(lineBuf, COL_BLACK);
    printBlank();

    printLine("Full sequence:", COL_ORANGE);
    for (int i = 0; i <= numMeans + 1 && gCurrentLine < MAX_LINES - 1; i++) {
        double val = a * pow(r, (double)i);
        formatFraction(nb, sizeof(nb), val);
        snprintf(lineBuf, sizeof(lineBuf), "  term %d = %s", i + 1, nb);
        printLine(lineBuf, COL_GREEN);
    }

    if (numMeans == 1) {
        double gm = sqrt(fabs(a * b));
        if (a < 0) gm = -gm;
        formatSqrtForm(nb, sizeof(nb), gm);
        snprintf(lineBuf, sizeof(lineBuf), "Geo mean = %s", nb);
        printLine(lineBuf, COL_BLACK);
    }

    waitContinue();
}

/*
 * Harmonic Mean solver.
 * H = n / (1/a1 + 1/a2 + ... + 1/an)
 * Output shown as reduced fraction when possible.
 */
static void solveHarmonicMean(void)
{
    RESET_CANCEL();
    startScreen("HARMONIC MEAN", "[CLEAR] Back");
    printSubheader("H = n/(1/a1+...+1/an)");
    printBlank();

    double countVal = inputNumber("How many values? "); CHECK_CANCEL;
    int count = (int)round(countVal);

    if (count < 2) {
        printDivider();
        printLine("Enter at least 2", COL_RED);
        waitContinue();
        return;
    }
    if (count > 10) {
        printDivider();
        printLine("Max 10 values", COL_RED);
        waitContinue();
        return;
    }

    double recSum = 0.0;
    char prompt[12];
    for (int i = 0; i < count; i++) {
        snprintf(prompt, sizeof(prompt), "a%d = ", i + 1);
        double val = inputNumber(prompt); CHECK_CANCEL;
        if (fabs(val) < MATH_EPS) {
            printDivider();
            printLine("Values cannot be 0", COL_RED);
            waitContinue();
            return;
        }
        recSum += 1.0 / val;
    }

    double H = (double)count / recSum;

    printDivider();
    char nb[24], lineBuf[52];

    /* Fraction form */
    formatFraction(nb, sizeof(nb), H);
    snprintf(lineBuf, sizeof(lineBuf), "H = %s", nb);
    printLine(lineBuf, COL_GREEN);

    /* Also show decimal if it differs from fraction string */
    char decBuf[20];
    formatNumber(decBuf, sizeof(decBuf), H);
    if (strcmp(nb, decBuf) != 0) {
        snprintf(lineBuf, sizeof(lineBuf), "  = %s (decimal)", decBuf);
        printLine(lineBuf, COL_BLACK);
    }

    waitContinue();
}
/*
 * Sum of Special Series.
 * Covers:
 *   1. Sum of first n integers:        n(n+1)/2
 *   2. Sum of first n squares:         n(n+1)(2n+1)/6
 *   3. Sum of first n cubes:           [n(n+1)/2]^2
 *   4. Sum of first n odd integers:    n^2
 *   5. Sum of first n even integers:   n(n+1)
 *   6. Triangular number sum:          sum of 1+2+...+n = n(n+1)/2
 */
static void solveSpecialSeries(void)
{
    RESET_CANCEL();

    const char *opts[] = {
        "Sum 1+2+...+n",
        "Sum 1^2+2^2+...+n^2",
        "Sum 1^3+2^3+...+n^3",
        "Sum odd: 1+3+...+(2n-1)",
        "Sum even: 2+4+...+2n",
        "Back"
    };
    int sel = showMenu("SPECIAL SERIES", opts, 6);
    if (sel < 0 || sel == 5) return;

    startScreen("SPECIAL SERIES", "[CLEAR] Back");
    printSubheader("Enter n:");
    printBlank();

    double nVal = inputNumber("n = "); CHECK_CANCEL;
    int n = (int)round(nVal);

    if (n < 1) {
        printDivider();
        printLine("n must be >= 1", COL_RED);
        waitContinue();
        return;
    }

    printDivider();
    char nb[24], lineBuf[52];
    double result = 0.0;

    switch (sel) {
        case 0:
            result = (double)n * (n + 1) / 2.0;
            snprintf(lineBuf, sizeof(lineBuf), "1+2+...+%d =", n);
            break;
        case 1:
            result = (double)n * (n + 1) * (2 * n + 1) / 6.0;
            snprintf(lineBuf, sizeof(lineBuf), "1^2+...+%d^2 =", n);
            break;
        case 2: {
            double tri = (double)n * (n + 1) / 2.0;
            result = tri * tri;
            snprintf(lineBuf, sizeof(lineBuf), "1^3+...+%d^3 =", n);
            break;
        }
        case 3:
            result = (double)n * n;
            snprintf(lineBuf, sizeof(lineBuf), "Sum first %d odds =", n);
            break;
        case 4:
            result = (double)n * (n + 1);
            snprintf(lineBuf, sizeof(lineBuf), "Sum first %d evens =", n);
            break;
        default: break;
    }

    printLine(lineBuf, COL_BLACK);
    formatNumber(nb, sizeof(nb), result);
    printLine(nb, COL_GREEN);
    waitContinue();
}


/* ═══════════════════════════════════════════════
   ADVANCED SEQUENCES & SERIES SOLVERS
   ═══════════════════════════════════════════════ */

/* ─────────────────────────────────────────────
   AP: TWO GIVEN TERMS  →  find a1, d, T_n, S_n
   e.g. T7 = -4, T77 = -214, find S777
   ───────────────────────────────────────────── */
static void solveAPTwoTerms(void)
{
    RESET_CANCEL();
    startScreen("AP: TWO GIVEN TERMS", "[CLEAR] Back");
    printSubheader("T_p = Vp, T_q = Vq");
    printBlank();

    double pDbl = inputNumber("p (1st position) = "); CHECK_CANCEL;
    double vp   = inputNumber("T_p value        = "); CHECK_CANCEL;
    double qDbl = inputNumber("q (2nd position) = "); CHECK_CANCEL;
    double vq   = inputNumber("T_q value        = "); CHECK_CANCEL;
    double nDbl = inputNumber("Find S_n, n =      "); CHECK_CANCEL;

    int p = (int)round(pDbl);
    int q = (int)round(qDbl);
    int n = (int)round(nDbl);

    printDivider();
    char nb[24], lb[52];

    if (p == q) { printLine("p and q must differ", COL_RED); waitContinue(); return; }
    if (n < 1)  { printLine("n must be >= 1",       COL_RED); waitContinue(); return; }

    double d  = (vq - vp) / (double)(q - p);
    double a1 = vp - (double)(p - 1) * d;
    double an = a1 + (double)(n - 1) * d;
    double Sn = (double)n / 2.0 * (a1 + an);

    formatFraction(nb, sizeof(nb), a1);
    snprintf(lb, sizeof(lb), "a1 = %s", nb);        printLine(lb, COL_GREEN);

    formatFraction(nb, sizeof(nb), d);
    snprintf(lb, sizeof(lb), "d  = %s", nb);        printLine(lb, COL_GREEN);

    formatFraction(nb, sizeof(nb), an);
    snprintf(lb, sizeof(lb), "a%d = %s", n, nb);   printLine(lb, COL_GREEN);

    formatFraction(nb, sizeof(nb), Sn);
    snprintf(lb, sizeof(lb), "S%d = %s", n, nb);   printLine(lb, COL_GREEN);

    waitContinue();
}

/* ─────────────────────────────────────────────
   CONSECUTIVE ODD / EVEN INTEGER SUM
   Mode A: known first & last  →  find sum & count
   Mode B: known first & sum   →  find last term
   (AP with d = 2)
   ───────────────────────────────────────────── */
static void solveConsecutiveOddEven(void)
{
    RESET_CANCEL();
    startScreen("ODD/EVEN INT. SUM", "[CLEAR] Back");
    printSubheader("Consecutive odd or even, d=2");
    printBlank();

    printLine("1: Find sum (know first, last)", COL_BLACK);
    printLine("2: Find last (know first, sum)",  COL_BLACK);
    blit();

    uint8_t mc;
    do { mc = waitForKey(); }
    while (mc != sk_1 && mc != sk_2 && mc != sk_Clear);
    if (mc == sk_Clear) { gUserCancelled = true; return; }

    startScreen("ODD/EVEN INT. SUM", "[CLEAR] Back");
    printSubheader("d = 2  (consecutive parity)");
    printBlank();

    char nb[24], lb[52];

    if (mc == sk_1) {
        /* ── Mode A ── */
        double fDbl = inputNumber("First term = "); CHECK_CANCEL;
        double lDbl = inputNumber("Last term  = "); CHECK_CANCEL;
        int first = (int)round(fDbl);
        int last  = (int)round(lDbl);

        printDivider();
        if (((last - first) % 2) != 0) {
            printLine("Must have same parity", COL_RED);
            waitContinue(); return;
        }
        int    k   = (last - first) / 2 + 1;
        double sum = (double)k * (double)(first + last) / 2.0;

        snprintf(lb, sizeof(lb), "# terms = %d", k);  printLine(lb, COL_BLACK);
        formatFraction(nb, sizeof(nb), sum);
        snprintf(lb, sizeof(lb), "Sum = %s", nb);      printLine(lb, COL_GREEN);

    } else {
        /* ── Mode B: find last term given first and desired sum ──
         *
         * Let k = number of terms.
         * last  = first + 2(k-1)
         * sum   = k*(first + last)/2
         *       = k*(2*first + 2(k-1))/2
         *       = k*(first + k - 1)
         * → k^2 + (first-1)*k - sum = 0
         */
        double fDbl   = inputNumber("First term = "); CHECK_CANCEL;
        double sumDbl = inputNumber("Sum        = "); CHECK_CANCEL;
        int    first  = (int)round(fDbl);
        double sum    = sumDbl;

        double bCoef = (double)(first - 1);
        double disc  = bCoef * bCoef + 4.0 * sum;

        printDivider();
        if (disc < -MATH_EPS) {
            printLine("No real solution", COL_RED);
            waitContinue(); return;
        }

        /* Two roots; pick the positive integer one */
        double k1 = (-bCoef + sqrt(fabs(disc))) / 2.0;
        double k2 = (-bCoef - sqrt(fabs(disc))) / 2.0;
        int ki = -1;
        if (k1 > 0.5 && fabs(k1 - round(k1)) < 0.01) ki = (int)round(k1);
        else if (k2 > 0.5 && fabs(k2 - round(k2)) < 0.01) ki = (int)round(k2);

        if (ki < 1) {
            printLine("No integer solution", COL_RED);
            waitContinue(); return;
        }

        int last = first + 2 * (ki - 1);

        snprintf(lb, sizeof(lb), "# terms = %d", ki);  printLine(lb, COL_BLACK);
        snprintf(lb, sizeof(lb), "Last term = %d", last);
        printLine(lb, COL_GREEN);

        /* Verification */
        double chk = (double)ki * (double)(first + last) / 2.0;
        snprintf(lb, sizeof(lb), "Verify: sum = %.0f", chk);
        printLine(lb, fabs(chk - sum) < 0.5 ? COL_GRAY : COL_RED);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   AP ARITHMETIC MEANS BETWEEN TWO ENDPOINTS
   Given n-term AP with a1 and a_n known,
   finds sum of the (n-2) interior terms.
   e.g. 5-term AP, a1=18, a5=-10 → sum of a2+a3+a4
   ───────────────────────────────────────────── */
static void solveArithMeansSum(void)
{
    RESET_CANCEL();
    startScreen("SUM OF ARITH. MEANS", "[CLEAR] Back");
    printSubheader("n-term AP, sum middle terms");
    printBlank();

    double nDbl    = inputNumber("Total terms n    = "); CHECK_CANCEL;
    double firstDbl = inputNumber("First term a1    = "); CHECK_CANCEL;
    double lastDbl  = inputNumber("Last term a_n    = "); CHECK_CANCEL;

    int n = (int)round(nDbl);
    printDivider();

    if (n < 3) {
        printLine("Need n >= 3 for means", COL_RED);
        waitContinue(); return;
    }

    /* Sum of ALL n terms */
    double Sn   = (double)n / 2.0 * (firstDbl + lastDbl);
    /* Remove first and last to get sum of the (n-2) means */
    double sumMeans = Sn - firstDbl - lastDbl;

    char nb[24], lb[52];

    double d = (lastDbl - firstDbl) / (double)(n - 1);
    formatFraction(nb, sizeof(nb), d);
    snprintf(lb, sizeof(lb), "d = %s", nb);             printLine(lb, COL_BLACK);

    snprintf(lb, sizeof(lb), "S%d (all)  = %.4g", n, Sn);
    printLine(lb, COL_BLACK);

    formatFraction(nb, sizeof(nb), sumMeans);
    snprintf(lb, sizeof(lb), "Sum of %d means = %s", n - 2, nb);
    printLine(lb, COL_GREEN);

    /* List the means for reference */
    if (n - 2 <= 6) {
        printBlank();
        printLine("Means:", COL_ORANGE);
        for (int i = 1; i <= n - 2 && gCurrentLine < MAX_LINES - 1; i++) {
            double val = firstDbl + (double)i * d;
            formatFraction(nb, sizeof(nb), val);
            snprintf(lb, sizeof(lb), "  a%d = %s", i + 1, nb);
            printLine(lb, COL_GREEN);
        }
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   ALTERNATING SIGN SERIES  (magnitudes form AP)
   T_k = (-1)^(k+1) * (a + (k-1)*d)
   e.g. 2, -4, 6, -8, ...  (a=2, d=2)
   or  -2, 4, -6, 8, ...   (a=2, d=2, first negative)

   Mode A: enter n directly.
   Mode B: enter last term value → solver finds n.

   Closed-form:
     n even → S = -sign1 * (n/2) * d
     n odd  → S = sign1 * (a + ((n-1)/2)*d)
               which equals a1 + sign1*(n-1)/2 * d
   ───────────────────────────────────────────── */
static void solveAlternatingSeries(void)
{
    RESET_CANCEL();
    startScreen("ALTERNATING SERIES", "[CLEAR] Back");
    printSubheader("+a,-b,+c,... magnitudes AP");
    printBlank();

    printLine("1: Know n",                 COL_BLACK);
    printLine("2: Know last term (find n)", COL_BLACK);
    blit();

    uint8_t mc;
    do { mc = waitForKey(); }
    while (mc != sk_1 && mc != sk_2 && mc != sk_Clear);
    if (mc == sk_Clear) { gUserCancelled = true; return; }

    startScreen("ALTERNATING SERIES", "[CLEAR] Back");
    printSubheader("Enter series parameters:");
    printBlank();

    double a1 = inputNumber("First term (signed)    = "); CHECK_CANCEL;
    double d  = inputNumber("d of magnitudes (d>=0) = "); CHECK_CANCEL;

    char nb[24], lb[52];
    int n;

    if (mc == sk_2) {
        double anDbl  = inputNumber("Last term (signed)     = "); CHECK_CANCEL;
        double a_abs  = fabs(a1);
        double an_abs = fabs(anDbl);

        if (fabs(d) < MATH_EPS) {
            /* Constant magnitude — all terms equal, find count */
            if (fabs(a_abs - an_abs) > 1e-6) {
                printDivider(); printLine("d=0 but |last|!=|first|", COL_RED);
                waitContinue(); return;
            }
            /* n undefined without more info */
            printDivider(); printLine("d=0: n not uniquely found", COL_RED);
            waitContinue(); return;
        }
        double kf = (an_abs - a_abs) / d + 1.0;
        n = (int)round(kf);
        if (n < 1 || fabs(kf - (double)n) > 0.02) {
            printDivider(); printLine("No integer n found", COL_RED);
            waitContinue(); return;
        }
        snprintf(lb, sizeof(lb), "n = %d terms", n);
        printLine(lb, COL_BLACK);
    } else {
        double nDbl = inputNumber("# of terms n           = "); CHECK_CANCEL;
        n = (int)round(nDbl);
        if (n < 1) {
            printDivider(); printLine("n must be >= 1", COL_RED);
            waitContinue(); return;
        }
    }

    printDivider();

    double sign1 = (a1 >= 0.0) ? 1.0 : -1.0;
    double absa1 = fabs(a1);
    double sum;

    if (n % 2 == 0)
        sum = -sign1 * (double)(n / 2) * d;
    else
        sum = sign1 * (absa1 + (double)((n - 1) / 2) * d);

    /* Show last term */
    double lastAbs  = absa1 + (double)(n - 1) * d;
    double lastSign = sign1 * ((n % 2 == 1) ? 1.0 : -1.0);
    formatFraction(nb, sizeof(nb), lastSign * lastAbs);
    snprintf(lb, sizeof(lb), "Last term = %s", nb); printLine(lb, COL_BLACK);

    formatFraction(nb, sizeof(nb), sum);
    snprintf(lb, sizeof(lb), "Sum = %s", nb);        printLine(lb, COL_GREEN);

    waitContinue();
}

/* ─────────────────────────────────────────────
   ALTERNATING SQUARES SUM
   n^2 - (n-1)^2 + (n-2)^2 - ... +/- 1^2
   = n(n+1)/2  for any positive integer n.
   ───────────────────────────────────────────── */
static void solveAlternatingSquares(void)
{
    RESET_CANCEL();
    startScreen("ALT. SQUARES SUM", "[CLEAR] Back");
    printSubheader("n^2-(n-1)^2+...+/-1^2");
    printBlank();

    double nDbl = inputNumber("Starting n = "); CHECK_CANCEL;
    int n = (int)round(nDbl);

    printDivider();
    char lb[52];

    if (n < 1) { printLine("n must be >= 1", COL_RED); waitContinue(); return; }

    /*
     * Proof:
     *   Pair consecutive terms when n is even:
     *   (n^2-(n-1)^2) + ((n-2)^2-(n-3)^2) + ...
     *   Each pair (2k)^2-(2k-1)^2 = 4k-1, summing k=1..n/2 gives n(n+1)/2.
     *   For odd n, add the last term 1 to the sum for n-1: (n-1)n/2 + 1 = n(n+1)/2.
     *   So the formula n(n+1)/2 holds for all n >= 1.
     */
    long long result = (long long)n * ((long long)n + 1LL) / 2LL;

    if (n >= 2)
        snprintf(lb, sizeof(lb), "%d^2-%d^2+...+/-1^2 =", n, n - 1);
    else
        snprintf(lb, sizeof(lb), "1^2 =");
    printLine(lb, COL_BLACK);

    snprintf(lb, sizeof(lb), "%lld",  result); printLine(lb, COL_GREEN);
    printLine("Formula: n(n+1)/2",             COL_GRAY);

    waitContinue();
}

/* ─────────────────────────────────────────────
   SQRT(2) GEOMETRIC SEQUENCE
   Terms of the form a + b*sqrt(2).
   Uses exact ring arithmetic — no rounding.
   (p + q*sqrt2) * (c + d*sqrt2)
       = (pc+2qd) + (pd+qc)*sqrt2
   ───────────────────────────────────────────── */
static void solveSqrt2Sequence(void)
{
    RESET_CANCEL();
    startScreen("SQRT(2) GEO. SEQ.", "[CLEAR] Back");
    printSubheader("T_n = a + b*sqrt(2)");
    printBlank();

    printLine("Enter T1 = a + b*sqrt(2):", COL_NAVY);
    double a1a = inputNumber("T1 coefficient a = "); CHECK_CANCEL;
    double a1b = inputNumber("T1 coefficient b = "); CHECK_CANCEL;

    printLine("Enter ratio r = c + d*sqrt(2):", COL_NAVY);
    double rc  = inputNumber("r coefficient c  = "); CHECK_CANCEL;
    double rd  = inputNumber("r coefficient d  = "); CHECK_CANCEL;

    double nDbl = inputNumber("Find term T_n, n = "); CHECK_CANCEL;
    int n = (int)round(nDbl);

    printDivider();
    char nb1[20], nb2[20], lb[52];

    if (n < 1) { printLine("n must be >= 1", COL_RED); waitContinue(); return; }

    /*
     * Compute r^(n-1) by repeated multiplication in the ring Z[sqrt2].
     * Start with rPow = 1 + 0*sqrt2, multiply by r n-1 times.
     */
    double rA = 1.0, rB = 0.0;
    for (int i = 0; i < n - 1; i++) {
        double newA = rA * rc + 2.0 * rB * rd;
        double newB = rA * rd + rB * rc;
        rA = newA;
        rB = newB;
    }

    /* T_n = T1 * r^(n-1) in the ring */
    double tnA = a1a * rA + 2.0 * a1b * rB;
    double tnB = a1a * rB + a1b * rA;

    snprintf(lb, sizeof(lb), "T%d = a + b*sqrt(2):", n);
    printLine(lb, COL_BLACK);

    formatFraction(nb1, sizeof(nb1), tnA);
    snprintf(lb, sizeof(lb), "a = %s", nb1);  printLine(lb, COL_GREEN);

    formatFraction(nb2, sizeof(nb2), tnB);
    snprintf(lb, sizeof(lb), "b = %s", nb2);  printLine(lb, COL_GREEN);

    snprintf(lb, sizeof(lb), "(%s, %s)", nb1, nb2);
    printLine(lb, COL_GREEN);

    char decBuf[20];
    formatNumber(decBuf, sizeof(decBuf), tnA + tnB * 1.41421356237);
    snprintf(lb, sizeof(lb), "~ %s (decimal)", decBuf);
    printLine(lb, COL_BLACK);

    waitContinue();
}

/* ─────────────────────────────────────────────
   GEO SEQUENCE: SUM OF TWO SPECIFIC TERMS
   Computes T_p + T_q with fraction output.
   e.g. a1=2/3, r=1/2, find T4+T5
   ───────────────────────────────────────────── */
static void solveGeoSumTwoTerms(void)
{
    RESET_CANCEL();
    startScreen("GEO: T_p + T_q", "[CLEAR] Back");
    printSubheader("Find T_p + T_q as fraction");
    printBlank();

    double a1   = inputNumber("a1 = "); CHECK_CANCEL;
    double r    = inputNumber("r  = "); CHECK_CANCEL;
    double pDbl = inputNumber("p  = "); CHECK_CANCEL;
    double qDbl = inputNumber("q  = "); CHECK_CANCEL;

    int p = (int)round(pDbl);
    int q = (int)round(qDbl);

    printDivider();
    char nb[24], lb[52];

    if (p < 1 || q < 1) {
        printLine("p, q must be >= 1", COL_RED);
        waitContinue(); return;
    }

    double Tp   = a1 * pow(r, (double)(p - 1));
    double Tq   = a1 * pow(r, (double)(q - 1));
    double sumT = Tp + Tq;

    formatFraction(nb, sizeof(nb), Tp);
    snprintf(lb, sizeof(lb), "T%d = %s", p, nb);        printLine(lb, COL_BLACK);

    formatFraction(nb, sizeof(nb), Tq);
    snprintf(lb, sizeof(lb), "T%d = %s", q, nb);        printLine(lb, COL_BLACK);

    printDivider();
    formatFraction(nb, sizeof(nb), sumT);
    snprintf(lb, sizeof(lb), "T%d+T%d = %s", p, q, nb); printLine(lb, COL_GREEN);

    waitContinue();
}

/* ─────────────────────────────────────────────
   INFINITE GEOMETRIC SERIES — FIND RATIO r
   Given a1 and S_inf, solves for r:
   S_inf = a1 / (1 - r)  →  r = 1 - a1/S_inf
   e.g. a1=4/3, S_inf=4/3  →  r=0
   ───────────────────────────────────────────── */
static void solveInfiniteGeoFindR(void)
{
    RESET_CANCEL();
    startScreen("INF. GEO: FIND r", "[CLEAR] Back");
    printSubheader("S_inf=a1/(1-r), given S_inf");
    printBlank();

    double a1   = inputNumber("a1 (first term) = "); CHECK_CANCEL;
    double Sinf = inputNumber("S_inf           = "); CHECK_CANCEL;

    printDivider();
    char nb[24], lb[52];

    if (fabs(Sinf) < MATH_EPS) {
        printLine("S_inf cannot be 0", COL_RED); waitContinue(); return;
    }
    if (fabs(a1) < MATH_EPS) {
        printLine("a1 cannot be 0",   COL_RED); waitContinue(); return;
    }

    double r = 1.0 - a1 / Sinf;

    formatFraction(nb, sizeof(nb), r);
    snprintf(lb, sizeof(lb), "r = %s", nb); printLine(lb, COL_GREEN);

    if      (fabs(r) < 1.0 - MATH_EPS)         printLine("|r|<1: converges", COL_GRAY);
    else if (fabs(fabs(r) - 1.0) < MATH_EPS)   printLine("|r|=1: diverges",  COL_ORANGE);
    else                                         printLine("|r|>1: diverges",  COL_RED);

    /* Verification */
    if (fabs(1.0 - r) > MATH_EPS) {
        double chk = a1 / (1.0 - r);
        formatFraction(nb, sizeof(nb), chk);
        snprintf(lb, sizeof(lb), "Verify S_inf = %s", nb);
        printLine(lb, COL_GRAY);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   TWO-NUMBER MEANS (AM, GM, HM) WITH RATIOS
   Displays all three means and GM/AM ratio.
   e.g. GM(6,12) / AM(6,12)
   ───────────────────────────────────────────── */
static void solveTwoNumberMeans(void)
{
    RESET_CANCEL();
    startScreen("TWO-NUMBER MEANS", "[CLEAR] Back");
    printSubheader("AM, GM, HM for a and b");
    printBlank();

    double a = inputNumber("a = "); CHECK_CANCEL;
    double b = inputNumber("b = "); CHECK_CANCEL;

    printDivider();
    char nb[24], lb[52];

    double AM = (a + b) / 2.0;
    formatFraction(nb, sizeof(nb), AM);
    snprintf(lb, sizeof(lb), "AM = %s", nb); printLine(lb, COL_GREEN);

    if (a * b >= 0.0) {
        double GM = (a < 0.0 && b < 0.0)
                  ? -sqrt(a * b)
                  :  sqrt(fabs(a * b));

        /* Try to display GM cleanly */
        char gmBuf[20];
        formatFraction(gmBuf, sizeof(gmBuf), GM);
        char decBuf[20];
        formatNumber(decBuf, sizeof(decBuf), GM);
        if (strcmp(gmBuf, decBuf) == 0)
            snprintf(lb, sizeof(lb), "GM = %s", decBuf);
        else
            snprintf(lb, sizeof(lb), "GM = %s = %s", gmBuf, decBuf);
        printLine(lb, COL_GREEN);

        /* GM / AM */
        if (fabs(AM) > MATH_EPS) {
            char rBuf[20];
            formatFraction(rBuf, sizeof(rBuf), GM / AM);
            char rDec[20];
            formatNumber(rDec, sizeof(rDec), GM / AM);
            if (strcmp(rBuf, rDec) == 0)
                snprintf(lb, sizeof(lb), "GM/AM = %s", rDec);
            else
                snprintf(lb, sizeof(lb), "GM/AM = %s = %s", rBuf, rDec);
            printLine(lb, COL_BLACK);
        }
    } else {
        printLine("GM: undefined (neg. product)", COL_ORANGE);
    }

    if (fabs(a) > MATH_EPS && fabs(b) > MATH_EPS && fabs(a + b) > MATH_EPS) {
        double HM = 2.0 * a * b / (a + b);
        formatFraction(nb, sizeof(nb), HM);
        snprintf(lb, sizeof(lb), "HM = %s", nb); printLine(lb, COL_GREEN);
    } else {
        printLine("HM: undefined (zero denom.)", COL_ORANGE);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   TRIANGULAR SERIES EQUATION
   Solves:  1+3+6+...+T_n  =  k*(1+2+...+n)
   where T_i = i(i+1)/2  (ith triangular number)

   LHS = Σ_{i=1}^{n} i(i+1)/2 = n(n+1)(n+2)/6
   RHS = k * n(n+1)/2

   Dividing both sides by n(n+1)/2  (n > 0):
       (n+2)/3 = k
   →   n = 3k - 2
   ───────────────────────────────────────────── */
static void solveTriangularEquation(void)
{
    RESET_CANCEL();
    startScreen("TRIANGULAR SERIES EQN", "[CLEAR] Back");
    printSubheader("1+3+6+...+T_n = k*(1+...+n)");
    printBlank();

    printLine("Formula: n = 3k - 2", COL_NAVY);
    printBlank();

    double kDbl = inputNumber("k = "); CHECK_CANCEL;
    int k = (int)round(kDbl);

    printDivider();
    char lb[52];

    if (k < 1) { printLine("k must be >= 1", COL_RED); waitContinue(); return; }

    int n = 3 * k - 2;
    snprintf(lb, sizeof(lb), "n = 3(%d) - 2 = %d", k, n);
    printLine(lb, COL_GREEN);

    /* Verify numerically */
    double LHS = (double)n * (n + 1.0) * (n + 2.0) / 6.0;
    double RHS = (double)k * (double)n * (n + 1.0) / 2.0;
    snprintf(lb, sizeof(lb), "LHS=%.0f  RHS=%.0f", LHS, RHS);
    printLine(lb, fabs(LHS - RHS) < 1.0 ? COL_GRAY : COL_RED);

    waitContinue();
}

/* ═══════════════════════════════════════════════
   LOGARITHMS & EXPONENTIALS
   Covers all log/expo question types from the PDF:
     • Evaluate log_b(x)             log20(23), log(5)
     • Solve log_b(x)=c              log2(k)=9 → k=512
     • Exponential equation          2^(7k+3)=8^(2k)
     • Log equation                  3+log3(3k+2)=5+2log3(k)
     • Nested log = 0                log2(log3(log5(x)))=0
     • Expo-Log combo (u-sub)        100^x−12·100^(−x)=4
   ═══════════════════════════════════════════════ */

/* ─────────────────────────────────────────────
   1. EVALUATE log_b(x)
   b=0 → natural log (ln).  Full 14-digit output.
   ───────────────────────────────────────────── */
static void solveEvalLog(void)
{
    RESET_CANCEL();
    startScreen("EVALUATE LOG", "[CLEAR] Back");
    printSubheader("Compute log_b(x) precisely");
    printBlank();

    printLine("(Enter b=0 for ln / base e)", COL_GRAY);
    double b = inputNumber("Base b = "); CHECK_CANCEL;
    double x = inputNumber("x      = "); CHECK_CANCEL;
    printDivider();

    char lb[52], nb[28], bl[12];
    baseLabel(bl, sizeof(bl), b);

    if ((fabs(b) > MATH_EPS && (b <= 0 || fabs(b-1.0) < 0.001)) || x <= 0) {
        printLine("Need: b>0, b!=1, x>0", COL_RED);
        waitContinue(); return;
    }

    snprintf(lb, sizeof(lb), "%s(%g) =", bl, x);
    printLine(lb, COL_BLACK);

    /* High-precision string */
    char rStr[20];
    fmtLog(rStr, sizeof(rStr), b, x);
    printLine(rStr, COL_GREEN);

    /* Also try fraction form */
    float fv = rLogBase(b, x);
    formatFraction(nb, sizeof(nb), (double)fv);
    if (strcmp(nb, rStr) != 0) {
        snprintf(lb, sizeof(lb), "= %s (frac)", nb);
        printLine(lb, COL_GREEN);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   2. SOLVE  log_b(x) = c   →   x = b^c
   Displays decimal + fraction + sqrt form.
   ───────────────────────────────────────────── */
static void solveFindXLog(void)
{
    RESET_CANCEL();
    startScreen("SOLVE log_b(x) = c", "[CLEAR] Back");
    printSubheader("Find x = b^c");
    printBlank();

    printLine("(b=0 → solve ln(x)=c)", COL_GRAY);
    double b = inputNumber("Base b = "); CHECK_CANCEL;
    double c = inputNumber("RHS c  = "); CHECK_CANCEL;
    printDivider();

    char lb[52], nb[28], bl[12];
    baseLabel(bl, sizeof(bl), b);

    snprintf(lb, sizeof(lb), "%s(x)=%g  =>  x =", bl, c);
    printLine(lb, COL_BLACK);

    /* High-precision result */
    char rStr[20];
    fmtPow(rStr, sizeof(rStr), b, c);
    printLine(rStr, COL_GREEN);

    float fv = rPowBase(b, c);
    /* Fraction form */
    formatFraction(nb, sizeof(nb), (double)fv);
    if (strcmp(nb, rStr) != 0) {
        snprintf(lb, sizeof(lb), "= %s (frac)", nb); printLine(lb, COL_GREEN);
    }
    /* Sqrt form */
    formatSqrtForm(nb, sizeof(nb), (double)fv);
    char dec[20]; formatNumber(dec, sizeof(dec), (double)fv);
    if (strcmp(nb, dec) != 0 && strcmp(nb, rStr) != 0) {
        snprintf(lb, sizeof(lb), "= %s (exact)", nb); printLine(lb, COL_GREEN);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   3. EXPONENTIAL EQUATION
   a^(m1*x+c1) = b^(m2*x+c2)
   Take ln both sides → linear in x:
     x*(m1*lnA − m2*lnB) = c2*lnB − c1*lnA

   Works for same or related bases, e.g.:
     2^(7k+3) = 8^(2k)   [8=2^3, both reduce to ln2]
     4^(6k+2) = 16^k
   ───────────────────────────────────────────── */
static void solveExpEquation(void)
{
    RESET_CANCEL();
    startScreen("EXPONENTIAL EQ.", "[CLEAR] Back");
    printSubheader("a^(m1x+c1) = b^(m2x+c2)");
    printBlank();

    printLine("LHS: a^(m1*x + c1)", COL_NAVY);
    double a  = inputNumber("a  = "); CHECK_CANCEL;
    double m1 = inputNumber("m1 = "); CHECK_CANCEL;
    double c1 = inputNumber("c1 = "); CHECK_CANCEL;

    printLine("RHS: b^(m2*x + c2)", COL_NAVY);
    double b  = inputNumber("b  = "); CHECK_CANCEL;
    double m2 = inputNumber("m2 = "); CHECK_CANCEL;
    double c2 = inputNumber("c2 = "); CHECK_CANCEL;

    printDivider();
    char lb[52], nb[28];

    if (a<=0||b<=0||fabs(a-1.0)<0.001||fabs(b-1.0)<0.001) {
        printLine("Bases must be >0, !=1", COL_RED);
        waitContinue(); return;
    }

    real_t rA  = os_FloatToReal((float)a);
    real_t rB  = os_FloatToReal((float)b);
    real_t lnA = os_RealLog(&rA);
    real_t lnB = os_RealLog(&rB);
    float flnA = os_RealToFloat(&lnA);
    float flnB = os_RealToFloat(&lnB);

    double coeffX = m1*(double)flnA - m2*(double)flnB;
    double rhsV   = c2*(double)flnB - c1*(double)flnA;

    if (fabs(coeffX) < 1e-9) {
        printLine(fabs(rhsV)<1e-9 ? "Identity: all x" : "No solution",
                  fabs(rhsV)<1e-9 ? COL_GREEN : COL_RED);
        waitContinue(); return;
    }

    double x = rhsV / coeffX;
    formatFraction(nb, sizeof(nb), x);
    snprintf(lb, sizeof(lb), "x = %s", nb); printLine(lb, COL_GREEN);

    char dec[20]; formatNumber(dec, sizeof(dec), x);
    if (strcmp(nb, dec) != 0) {
        snprintf(lb, sizeof(lb), "  = %s", dec); printLine(lb, COL_BLACK);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   4. LOG EQUATION (general)
   A + cL*log_b(mL*x+dL) = B + cR*log_b(mR*x+dR)

   Rearranges to:
     (mLx+dL)^cL = base^(B−A) * (mRx+dR)^cR

   Handles cL,cR ∈ {1,2}:
     cL=1,cR=1 → linear
     cL=1,cR=2 → quadratic  (e.g. 2018 Q5)
     cL=2,cR=1 → quadratic

   Domain check enforces all log arguments > 0.
   Reports sum of valid solutions for quadratic cases.
   ───────────────────────────────────────────── */
static void solveLogEquation(void)
{
    RESET_CANCEL();
    startScreen("LOG EQUATION", "[CLEAR] Back");
    printSubheader("A+cL*log_b(mLx+dL)=B+cR*...");
    printBlank();

    double base = inputNumber("Base b   = "); CHECK_CANCEL;

    printLine("LHS: A + cL * log(mL*x+dL)", COL_NAVY);
    double A  = inputNumber("  A   = "); CHECK_CANCEL;
    double cL = inputNumber("  cL  = "); CHECK_CANCEL;
    double mL = inputNumber("  mL  = "); CHECK_CANCEL;
    double dL = inputNumber("  dL  = "); CHECK_CANCEL;

    printLine("RHS: B + cR * log(mR*x+dR)", COL_NAVY);
    double B  = inputNumber("  B   = "); CHECK_CANCEL;
    double cR = inputNumber("  cR  = "); CHECK_CANCEL;
    double mR = inputNumber("  mR  = "); CHECK_CANCEL;
    double dR = inputNumber("  dR  = "); CHECK_CANCEL;

    printDivider();
    char lb[52], nb[28];

    if (base<=0||fabs(base-1.0)<0.001) {
        printLine("Invalid base", COL_RED); waitContinue(); return;
    }

    /* bK = base^(B-A) */
    float bK = rPowBase(base, B - A);

    bool found = false;

    /* ── cL=1, cR=1: mLx+dL = bK*(mRx+dR) → linear ── */
    if (fabs(cL-1.0)<0.1 && fabs(cR-1.0)<0.1) {
        double coeff = mL - (double)bK*mR;
        double rhsv  = (double)bK*dR - dL;
        if (fabs(coeff)<MATH_EPS) {
            printLine(fabs(rhsv)<MATH_EPS?"All x valid":"No solution",
                      fabs(rhsv)<MATH_EPS?COL_GREEN:COL_RED);
            waitContinue(); return;
        }
        double x = rhsv / coeff;
        if (mL*x+dL<=0||mR*x+dR<=0) {
            printLine("Solution rejected", COL_RED);
            printLine("(log arg <= 0)", COL_ORANGE);
        } else {
            formatFraction(nb,sizeof(nb),x);
            snprintf(lb,sizeof(lb),"x = %s",nb); printLine(lb,COL_GREEN);
            found=true;
        }

    /* ── cL=1, cR=2: mLx+dL = bK*(mRx+dR)^2 → quadratic ── */
    } else if (fabs(cL-1.0)<0.1 && fabs(cR-2.0)<0.1) {
        double qa = (double)bK*mR*mR;
        double qb = 2.0*(double)bK*mR*dR - mL;
        double qc = (double)bK*dR*dR - dL;
        if (fabs(qa)<MATH_EPS) {
            double x = (fabs(qb)>MATH_EPS)?-qc/qb:0.0;
            formatFraction(nb,sizeof(nb),x);
            snprintf(lb,sizeof(lb),"x = %s",nb); printLine(lb,COL_GREEN);
            found=true; goto logEqDone;
        }
        double disc = qb*qb - 4.0*qa*qc;
        if (disc<-MATH_EPS){printLine("No real solution",COL_RED);waitContinue();return;}
        double sq=sqrt(fabs(disc));
        double roots[2]={(-qb+sq)/(2.0*qa),(-qb-sq)/(2.0*qa)};
        double sumV=0.0; int cnt=0;
        for (int i=0;i<2;i++) {
            double x=roots[i];
            if (mL*x+dL>MATH_EPS && mR*x+dR>MATH_EPS) {
                formatFraction(nb,sizeof(nb),x);
                snprintf(lb,sizeof(lb),"x = %s",nb); printLine(lb,COL_GREEN);
                sumV+=x; cnt++; found=true;
            }
        }
        if (cnt==2) {
            formatFraction(nb,sizeof(nb),sumV);
            snprintf(lb,sizeof(lb),"Sum of sols = %s",nb); printLine(lb,COL_BLACK);
        }

    /* ── cL=2, cR=1: (mLx+dL)^2 = bK*(mRx+dR) → quadratic ── */
    } else if (fabs(cL-2.0)<0.1 && fabs(cR-1.0)<0.1) {
        double qa = mL*mL;
        double qb = 2.0*mL*dL - (double)bK*mR;
        double qc = dL*dL - (double)bK*dR;
        if (fabs(qa)<MATH_EPS){
            double x=(fabs(qb)>MATH_EPS)?-qc/qb:0.0;
            formatFraction(nb,sizeof(nb),x);
            snprintf(lb,sizeof(lb),"x = %s",nb); printLine(lb,COL_GREEN);
            found=true; goto logEqDone;
        }
        double disc=qb*qb-4.0*qa*qc;
        if (disc<-MATH_EPS){printLine("No real solution",COL_RED);waitContinue();return;}
        double sq=sqrt(fabs(disc));
        double roots[2]={(-qb+sq)/(2.0*qa),(-qb-sq)/(2.0*qa)};
        for (int i=0;i<2;i++) {
            double x=roots[i];
            if (mL*x+dL>MATH_EPS && mR*x+dR>MATH_EPS) {
                formatFraction(nb,sizeof(nb),x);
                snprintf(lb,sizeof(lb),"x = %s",nb); printLine(lb,COL_GREEN);
                found=true;
            }
        }
    } else {
        printLine("cL, cR must each be 1 or 2", COL_ORANGE);
        printLine("Combine logs first if >2", COL_GRAY);
    }

    logEqDone:
    if (!found && gCurrentLine < MAX_LINES-1)
        printLine("No valid solution", COL_RED);
    waitContinue();
}

/* ─────────────────────────────────────────────
   5. NESTED LOG = 0
   Handles 1, 2, or 3 levels of nesting.

   Level 3:  log_a(log_b(log_c(x))) = 0
     step 1: log_b(log_c(x)) = 1   [a^0 = 1]
     step 2: log_c(x) = b           [b^1 = b]
     step 3: x = c^b

   Also solves a SYSTEM of three simultaneous
   nested-log-zero equations (as in 2018 Q18):
     log_a1(log_b1(log_c1(x))) = 0  →  x = c1^b1
     log_a2(log_b2(log_c2(y))) = 0  →  y = c2^b2
     log_a3(log_b3(log_c3(z))) = 0  →  z = c3^b3
   ───────────────────────────────────────────── */
static void solveNestedLogZero(void)
{
    RESET_CANCEL();
    startScreen("NESTED LOG = 0", "[CLEAR] Back");
    printSubheader("Choose mode:");
    printBlank();

    printLine("1: Single equation", COL_BLACK);
    printLine("2: System (3 eqs, 2018 Q18)", COL_BLACK);
    blit();
    uint8_t mc;
    do { mc=waitForKey(); }
    while(mc!=sk_1&&mc!=sk_2&&mc!=sk_Clear);
    if (mc==sk_Clear){gUserCancelled=true;return;}

    startScreen("NESTED LOG = 0", "[CLEAR] Back");
    printSubheader("Enter bases:");
    printBlank();

    char lb[52], nb[28];

    if (mc==sk_1) {
        /* Single equation: choose 1-3 nesting levels */
        printLine("Nesting levels (1/2/3):", COL_NAVY);
        blit();
        uint8_t lv;
        do { lv=waitForKey(); }
        while(lv!=sk_1&&lv!=sk_2&&lv!=sk_3&&lv!=sk_Clear);
        if (lv==sk_Clear){gUserCancelled=true;return;}

        double a=0,b=0,c=0;
        if (lv>=sk_1){a=inputNumber("Outer base a = ");CHECK_CANCEL;}
        if (lv>=sk_2){b=inputNumber("Mid   base b = ");CHECK_CANCEL;}
        if (lv>=sk_3){c=inputNumber("Inner base c = ");CHECK_CANCEL;}

        printDivider();

        if (lv==sk_1) {
            printLine("log_a(x)=0 → x=1",COL_GRAY);
            printLine("x = 1",COL_GREEN);
        } else if (lv==sk_2) {
            printLine("Inner log = 1 → x = b",COL_GRAY);
            formatNumber(nb,sizeof(nb),b);
            snprintf(lb,sizeof(lb),"x = %s",nb); printLine(lb,COL_GREEN);
        } else {
            printLine("x = c^b",COL_GRAY);
            char rStr[20]; fmtPow(rStr,sizeof(rStr),c,b);
            snprintf(lb,sizeof(lb),"x = %g^%g = %s",c,b,rStr);
            printLine(lb,COL_GREEN);
            float fv=rPowBase(c,b);
            formatFraction(nb,sizeof(nb),(double)fv);
            if(strcmp(nb,rStr)!=0){
                snprintf(lb,sizeof(lb),"  = %s",nb); printLine(lb,COL_GREEN);
            }
        }

    } else {
        /* System of 3 triple-nested equations */
        printLine("Eq1: log_a1(log_b1(log_c1(x)))=0", COL_NAVY);
        double a1=inputNumber("a1=");CHECK_CANCEL;
        double b1=inputNumber("b1=");CHECK_CANCEL;
        double c1=inputNumber("c1=");CHECK_CANCEL;

        printLine("Eq2: log_a2(log_b2(log_c2(y)))=0", COL_NAVY);
        double a2=inputNumber("a2=");CHECK_CANCEL;
        double b2=inputNumber("b2=");CHECK_CANCEL;
        double c2=inputNumber("c2=");CHECK_CANCEL;

        printLine("Eq3: log_a3(log_b3(log_c3(z)))=0", COL_NAVY);
        double a3=inputNumber("a3=");CHECK_CANCEL;
        double b3=inputNumber("b3=");CHECK_CANCEL;
        double c3=inputNumber("c3=");CHECK_CANCEL;

        (void)a1;(void)a2;(void)a3; /* outer base cancels */

        printDivider();

        char rStr[20];
        fmtPow(rStr,sizeof(rStr),c1,b1);
        snprintf(lb,sizeof(lb),"x = %g^%g = %s",c1,b1,rStr); printLine(lb,COL_GREEN);

        fmtPow(rStr,sizeof(rStr),c2,b2);
        snprintf(lb,sizeof(lb),"y = %g^%g = %s",c2,b2,rStr); printLine(lb,COL_GREEN);

        fmtPow(rStr,sizeof(rStr),c3,b3);
        snprintf(lb,sizeof(lb),"z = %g^%g = %s",c3,b3,rStr); printLine(lb,COL_GREEN);

        float fx=rPowBase(c1,b1);
        float fy=rPowBase(c2,b2);
        float fz=rPowBase(c3,b3);
        formatFraction(nb,sizeof(nb),(double)(fx+fy+fz));
        snprintf(lb,sizeof(lb),"x+y+z = %s",nb); printLine(lb,COL_BLACK);
    }

    waitContinue();
}

/* ─────────────────────────────────────────────
   6. EXPO-LOG COMBO  (u-substitution)
   b^x + k*b^(−x) = c
   Let u = b^x (u>0):  u^2 − c·u + k = 0
   Solve quadratic → each positive u:
     x = log_b(u)
   If base=10, also shows "x = log(k)".

   Example: 100^x − 12·100^(−x) = 4
     Base=100, k=−12, c=4
     u=6 → x=log100(6) = log(sqrt(6))
   ───────────────────────────────────────────── */
static void solveExpoLogCombo(void)
{
    RESET_CANCEL();
    startScreen("EXPO-LOG COMBO", "[CLEAR] Back");
    printSubheader("b^x + k*b^(-x) = c");
    printBlank();

    printLine("(negative k for minus term)", COL_GRAY);
    double base = inputNumber("Base b  = "); CHECK_CANCEL;
    double k    = inputNumber("Coeff k = "); CHECK_CANCEL;
    double c    = inputNumber("RHS c   = "); CHECK_CANCEL;
    printDivider();

    char lb[52], nb[28], bl[12];
    baseLabel(bl,sizeof(bl),base);

    if (base<=0||fabs(base-1.0)<0.001) {
        printLine("Invalid base",COL_RED); waitContinue(); return;
    }

    /* u^2 - cu + k = 0 */
    double disc = c*c - 4.0*k;
    if (disc<-MATH_EPS){printLine("No real u (disc<0)",COL_RED);waitContinue();return;}

    double sq=sqrt(fabs(disc));
    double u[2]={(c+sq)/2.0,(c-sq)/2.0};
    bool found=false;

    for (int i=0;i<2;i++) {
        if (u[i]<=MATH_EPS) continue;

        /* x = log_base(u) */
        char xStr[20]; fmtLog(xStr,sizeof(xStr),base,u[i]);
        float xfloat = rLogBase(base,u[i]);

        formatFraction(nb,sizeof(nb),(double)xfloat);
        if (strcmp(nb,xStr)==0)
            snprintf(lb,sizeof(lb),"x = %s",xStr);
        else
            snprintf(lb,sizeof(lb),"x = %s = %s",nb,xStr);
        printLine(lb,COL_GREEN);

        /* For base=10: problem often asks "x = log(k), find k" */
        if (fabs(base-10.0)<0.01) {
            /* u = 10^x = k (but only if x = log10(k) makes u=k) */
            /* More useful: show u in sqrt form */
            char sf[24]; formatSqrtForm(sf,sizeof(sf),u[i]);
            snprintf(lb,sizeof(lb),"  (u=%g, x=log(%s))",u[i],sf);
            printLine(lb,COL_BLACK);
        }
        found=true;
    }
    if (!found) printLine("No positive u",COL_RED);
    waitContinue();
}

/* ─────────────────────────────────────────────
   LOGARITHMS MENU
   ───────────────────────────────────────────── */
static void menuLogarithms(void)
{
    const char *options[] = {
        "Evaluate log_b(x)",
        "Solve log_b(x) = c",
        "Exponential Equation",
        "Log Equation",
        "Nested Log = 0",
        "Expo-Log Combo",
        "Back"
    };
    int sel;
    while ((sel=showMenu("LOG & EXPONENTIALS", options, 7)) >= 0) {
        switch (sel) {
            case 0: solveEvalLog();       break;
            case 1: solveFindXLog();      break;
            case 2: solveExpEquation();   break;
            case 3: solveLogEquation();   break;
            case 4: solveNestedLogZero(); break;
            case 5: solveExpoLogCombo();  break;
            case 6: return;
        }
    }
}

/* ─────────────────────────────────────────────
   ADVANCED SERIES SUB-MENU
   ───────────────────────────────────────────── */
static void menuAdvancedSeries(void)
{
    const char *options[] = {
        "AP: Two Given Terms",
        "Consecutive Odd/Even",
        "Sum of Arith. Means",
        "Alternating Series",
        "Alternating Squares",
        "Sqrt(2) Sequence",
        "Geo: T_p + T_q",
        "Inf. Geo: Find r",
        "Two-Number Means (AM/GM/HM)",
        "Triangular Eqn",
        "Back"
    };

    /* showMenu displays up to 9 items at once (keys 1-9).
       This menu has 11 entries so we show it in two pages. */
    for (;;) {
        /* Page 1: items 0-8 (keys 1-9) */
        int sel = showMenu("ADVANCED SERIES (1/2)", options, 9);
        if (sel < 0) return;
        if (sel == 8) {
            /* Key 9 on page 1 = "Two-Number Means" — intentional */
            solveTwoNumberMeans(); continue;
        }
        switch (sel) {
            case 0: solveAPTwoTerms();          break;
            case 1: solveConsecutiveOddEven();  break;
            case 2: solveArithMeansSum();        break;
            case 3: solveAlternatingSeries();    break;
            case 4: solveAlternatingSquares();   break;
            case 5: solveSqrt2Sequence();        break;
            case 6: solveGeoSumTwoTerms();       break;
            case 7: solveInfiniteGeoFindR();     break;
        }
        /* After any solver on page 1, loop back to the top. */

        /* "More..." prompt for items 9-10 */
        startScreen("ADVANCED SERIES (2/2)", "[CLEAR] Back");
        printLine("1: Two-Number Means (AM/GM/HM)", COL_BLACK);
        printLine("2: Triangular Series Eqn",        COL_BLACK);
        printLine("3: Back to Sequences menu",        COL_BLACK);
        blit();

        uint8_t key2;
        do { key2 = waitForKey(); }
        while (key2 != sk_1 && key2 != sk_2 && key2 != sk_3 && key2 != sk_Clear);
        if (key2 == sk_Clear || key2 == sk_3) return;
        if (key2 == sk_1) solveTwoNumberMeans();
        if (key2 == sk_2) solveTriangularEquation();
    }
}
/* ── Menu ── */
static void menuSequencesAndSeries(void)
{
    const char *options[] = {
        "Arithmetic Sequence",
        "Geometric Sequence",
        "Arithmetic Means",
        "Geometric Means",
        "Harmonic Mean",
        "Special Series",
        "Advanced Series",
        "Back"
    };
    int sel;
    while ((sel = showMenu("SEQUENCES & SERIES", options, 8)) >= 0) {
        switch (sel) {
            case 0: solveArithmeticSequence(); break;
            case 1: solveGeometricSequence();  break;
            case 2: solveArithmeticMeans();    break;
            case 3: solveGeometricMeans();     break;
            case 4: solveHarmonicMean();       break;
            case 5: solveSpecialSeries();      break;
            case 6: menuAdvancedSeries(); break;
            case 7: return;
        }
    }
}
/* ═══════════════════════════════════════════════
   SECTION 3 — FORMULA REFERENCE
   ═══════════════════════════════════════════════ */

static void showRefAlgebra(void)
{
    startScreen("ALGEBRA FORMULAS", "[ENTER] Next");
    printSubheader("Linear & Quadratic");
    printLine("Line:  y = mx + b",          COL_BLACK);
    printLine("y-y1 = m(x-x1)",             COL_BLACK);
    printLine("Quad: ax^2+bx+c=0",          COL_BLACK);
    printLine("x=(-b+/-sqrt(D))/2a",        COL_BLACK);
    printLine("D = b^2 - 4ac",              COL_BLACK);
    printDivider();
    printLine("D>0: 2 real roots",          COL_BLACK);
    printLine("D=0: 1 repeated root",       COL_BLACK);
    printLine("D<0: complex roots",         COL_BLACK);
    printLine("Vertex x = -b/2a",           COL_BLACK);
    waitContinue();

    startScreen("ALGEBRA FORMULAS 2", "[ENTER] Done");
    printSubheader("Special Factors");
    printLine("a^2-b^2=(a+b)(a-b)",         COL_BLACK);
    printLine("a^3-b^3=",                    COL_BLACK);
    printLine(" (a-b)(a^2+ab+b^2)",          COL_BLACK);
    printLine("a^3+b^3=",                    COL_BLACK);
    printLine(" (a+b)(a^2-ab+b^2)",          COL_BLACK);
    printLine("(a+b)^2=a^2+2ab+b^2",        COL_BLACK);
    printLine("(a-b)^2=a^2-2ab+b^2",        COL_BLACK);
    waitContinue();
}

static void showRefInequalityRules(void)
{
    startScreen("INEQUALITY RULES", "[ENTER] Done");
    printSubheader("Key Rules");
    printLine("Add/subtract both sides",    COL_BLACK);
    printLine("Mul/div by +: same dir",     COL_BLACK);
    printLine("Mul/div by -: FLIP dir",     COL_RED);
    printDivider();
    printSubheader("Absolute Value");
    printLine("|x| < a =>",                 COL_BLACK);
    printLine("  -a < x < a",               COL_GREEN);
    printLine("|x| > a =>",                 COL_BLACK);
    printLine("  x<-a  or  x>a",            COL_GREEN);
    printLine("|x| = a =>",                 COL_BLACK);
    printLine("  x=a  or  x=-a",            COL_GREEN);
    waitContinue();
}

static void showRefGeometry(void)
{
    startScreen("GEOMETRY FORMULAS", "[ENTER] Next");
    printSubheader("Circles & Spheres");
    printLine("Circle: A=pi*r^2",           COL_BLACK);
    printLine("        C=2*pi*r",           COL_BLACK);
    printLine("Sphere: V=(4/3)pi*r^3",      COL_BLACK);
    printLine("        SA=4*pi*r^2",        COL_BLACK);
    printDivider();
    printSubheader("Prisms & Cones");
    printLine("Cylinder: V=pi*r^2*h",       COL_BLACK);
    printLine("Cone: V=(1/3)pi*r^2*h",      COL_BLACK);
    printLine("Rect: A=LW   P=2L+2W",       COL_BLACK);
    printLine("Trap: A=0.5h(b1+b2)",        COL_BLACK);
    waitContinue();

    startScreen("GEOMETRY 2", "[ENTER] Done");
    printSubheader("Triangles");
    printLine("A=0.5*b*h",                  COL_BLACK);
    printLine("A=0.5ab*sin(C)",             COL_BLACK);
    printLine("Heron: s=(a+b+c)/2",         COL_BLACK);
    printLine(" A=sqrt(s(s-a)(s-b)(s-c))",  COL_BLACK);
    printDivider();
    printSubheader("Coord Geometry");
    printLine("d=sqrt((x2-x1)^2",           COL_BLACK);
    printLine("       +(y2-y1)^2)",          COL_BLACK);
    printLine("m=(y2-y1)/(x2-x1)",          COL_BLACK);
    printLine("||: m1=m2  _|_: m1*m2=-1",   COL_BLACK);
    waitContinue();
}

/* ═══════════════════════════════════════════════
   SUB-MENUS
   ═══════════════════════════════════════════════ */

static void menuTheoryOfEquations(void)
{
    const char *options[] = {
        "Remainder/Factor Thm",
        "Multiple Factors",
        "Poly from Roots",
        "Vieta's Formulas",
        "Vieta's Parametric",
        "Root Condition Finder",
        "Common Root",
        "Neg. Reciprocal Roots",
        "Back"
    };
    int sel;
    while ((sel = showMenu("THEORY OF EQUATIONS", options, 8)) >= 0) {
        switch (sel) {
            case 0: solveRemainderFactor();  break;
            case 1: solveMultipleFactors();  break;
            case 2: solvePolyFromRoots();    break;
            case 3: solveVietasFormulas();   break;
            case 4: solveVietasParametric(); break;
            case 5: solveRootCondition();    break;
            case 6: solveCommonRoot();       break;
            case 7: solveNegRecipRoots();    break;
            case 8: return;
        }
    }
}

static void menuEquationsAndInequalities(void)
{
    const char *options[] = {
        "Linear Equation",
        "Quadratic Equation",
        "Linear Inequality",
        "Three-Way Inequality",
        "System of Inequalties",
        "Back"
    };
    int sel;
    while ((sel = showMenu("EQUATIONS & INEQ.", options, 6)) >= 0) {
        switch (sel) {
            case 0: solveLinearEquation();     break;
            case 1: solveQuadraticEquation();  break;
            case 2: solveLinearInequality();   break;
            case 3: solveThreeWayInequality(); break;
            case 4: solveSystemOfInequalities(); break;
            case 5: return;
        }
    }
}

static void menuAbsoluteValue(void)
{
    const char *options[] = {
        "|Ax+B| = C",
        "|Ax+B| op C",
        "||Ax+B|-C| op D",
        "z|Ax+B| = w|Cx+D|",
        "Back"
    };
    int sel;
    while ((sel = showMenu("ABSOLUTE VALUE", options, 5)) >= 0) {
        switch (sel) {
            case 0: solveAbsEq();         break;
            case 1: solveAbsIneq();       break;
            case 2: solveNestedAbsIneq(); break;
            case 3: solveAbsEqAbs(); break;
            case 4: return;
        }
    }
}

static void menuReference(void)
{
    const char *options[] = {
        "Algebra Formulas",
        "Inequality Rules",
        "Geometry Formulas",
        "Back"
    };
    int sel;
    while ((sel = showMenu("FORMULAS & REF.", options, 4)) >= 0) {
        switch (sel) {
            case 0: showRefAlgebra();         break;
            case 1: showRefInequalityRules(); break;
            case 2: showRefGeometry();        break;
            case 3: return;
        }
    }
}

static void menuBaseConversion(void)
{
    const char *options[] = {
        "Base N -> Decimal",
        "Decimal -> Base N",
        "Base A -> Base B",
        "Find the Base",
        "Back"
    };
    int sel;
    while ((sel = showMenu("BASE CONVERSION", options, 5)) >= 0) {
        switch (sel) {
            case 0: solveBaseToDecimal(); break;
            case 1: solveDecimalToBase(); break;
            case 2: solveBaseToBase();    break;
            case 3: solveFindTheBase(); break;
            case 4: return;
        }
    }
}

static void menuNumberTheory(void)
{
    const char *options[] = {
        "Prime Factorization",
        "GCD / HCF",
        "LCM",
        "Perm. & Combination",
        "Binomial Theorem - Coeff",
        "Binomial Theorem - Pascal's Triangle",
        "Radical Simplifier",
        "Base Conversion",
        "Back"
    };
    int sel;
    while ((sel = showMenu("NUMBER THEORY", options, 9)) >= 0) {
        switch (sel) {
            case 0: solveFactorize(); break;
            case 1: solveGCD();       break;
            case 2: solveLCM();       break;
            case 3: solvePermComb();  break;
            case 4: solveBinomialCoeff(); break;
            case 5: solvePascalTriangle();break;
            case 6: solveRadicalSimplifier();break;
            case 7: menuBaseConversion();break;
            case 8: return;
        }
    }
}

/* ═══════════════════════════════════════════════
   MAIN MENU
   ═══════════════════════════════════════════════ */

static void runMainMenu(void)
{
    const char *options[] = {
        "Equations & Inequalities",
        "Absolute Value",
        "Number Theory",
        "Theory of Equations",
        "Sequences & Series",
        "Log and Exponential",
        "Formulas & Reference",
        "Quit"
    };
    int sel;
    for (;;) {
        sel = showMenu("MATH SOLVER CE", options, 7);
        if (sel < 0 || sel == 6) break;
        switch (sel) {
            case 0: menuEquationsAndInequalities(); break;
            case 1: menuAbsoluteValue();            break;
            case 2: menuNumberTheory(); break;
            case 3: menuTheoryOfEquations(); break;
            case 4: menuSequencesAndSeries(); break;
            case 5: menuLogarithms();    break;
            case 6: menuReference();break;
            case 7: return;
        }
    }
}

/* ═══════════════════════════════════════════════
   ENTRY POINT
   ═══════════════════════════════════════════════ */

int main(void)
{
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetPalette(kPalette, sizeof(kPalette), 0);

    runMainMenu();

    /* Goodbye screen */
    startScreen("MATH SOLVER CE", "");
    printBlank();
    printSubheader("Thank you for using");
    printLine("MathSolverCE  v2.6 ", COL_NAVY);
    printBlank();
    printLine("Goodbye!", COL_ORANGE);
    blit();

    for (volatile int i = 0; i < 400000; i++);

    gfx_End();
    return 0;
}