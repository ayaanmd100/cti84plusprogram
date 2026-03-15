/*
 * MathSolverCE  v2.2
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
 *     3. Formula Reference
 *     4. Quit
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
static const char  *kOpStr[4] = { "<", "<=", ">", ">=" };

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

static double inputNumber(const char *prompt)
{
    char inputBuf[MAX_INPUT_LEN + 1];
    int  inputLen  = 0;
    bool hasDot    = false;
    int  savedLine = gCurrentLine;

    inputBuf[0] = '\0';

    for (;;) {
        int rowY = BODY_TOP + savedLine * LINE_H;
        gfx_SetColor(COL_WHITE);
        gfx_FillRectangle_NoClip(0, rowY - 1, SCR_W, LINE_H + 2);

        int promptW = gfx_GetStringWidth(prompt);
        gfx_SetTextFGColor(COL_NAVY);
        gfx_SetTextBGColor(COL_WHITE);
        gfx_PrintStringXY(prompt, LMARGIN, rowY);

        char displayBuf[MAX_INPUT_LEN + 4];
        snprintf(displayBuf, sizeof(displayBuf), "%s_", inputBuf);
        gfx_SetTextFGColor(COL_BLACK);
        gfx_SetTextBGColor(COL_WHITE);
        int valueX = LMARGIN + promptW + 4;
        gfx_PrintStringXY(displayBuf, valueX, rowY);

        gfx_SetColor(COL_GRAY);
        gfx_FillRectangle_NoClip(valueX, rowY + 9, SCR_W - valueX - LMARGIN, 1);
        blit();

        uint8_t key = waitForKey();

        if (key == sk_Clear) {
            gUserCancelled = true;
            gCurrentLine   = savedLine + 1;
            return 0.0;
        }
        if (key == sk_Enter) {
            gCurrentLine = savedLine + 1;
            return (inputLen == 0) ? 0.0 : atof(inputBuf);
        }
        if (key == sk_Del && inputLen > 0) {
            if (inputBuf[inputLen - 1] == '.') hasDot = false;
            inputBuf[--inputLen] = '\0';
            continue;
        }
        if (key == sk_Chs) {
            if (inputLen > 0 && inputBuf[0] == '-') {
                memmove(inputBuf, inputBuf + 1, (size_t)inputLen);
                inputLen--;
            } else if (inputLen < MAX_INPUT_LEN - 1) {
                memmove(inputBuf + 1, inputBuf, (size_t)(inputLen + 1));
                inputBuf[0] = '-';
                inputLen++;
            }
            continue;
        }
        if (key == sk_DecPnt && !hasDot && inputLen < MAX_INPUT_LEN - 1) {
            if (inputLen == 0 || inputBuf[inputLen - 1] == '-') {
                inputBuf[inputLen++] = '0';
                inputBuf[inputLen]   = '\0';
            }
            inputBuf[inputLen++] = '.';
            inputBuf[inputLen]   = '\0';
            hasDot = true;
            continue;
        }

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
        if (digitChar && inputLen < MAX_INPUT_LEN - 1) {
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
    gfx_PrintStringXY("1:<  2:<=  3:>  4:>=", LMARGIN + 12, BODY_TOP + choiceRow * LINE_H);
    blit();

    for (;;) {
        uint8_t key = waitForKey();
        if (key == sk_Clear) { gUserCancelled = true; return -1; }
        if (key == sk_1) return 0;
        if (key == sk_2) return 1;
        if (key == sk_3) return 2;
        if (key == sk_4) return 3;
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

static void solveThreeWayInequality(void)
{
    RESET_CANCEL();
    startScreen("THREE-WAY INEQUALITY", "[CLEAR] Back");
    printSubheader("C1 op1  Ax+B  op2 C2");
    printBlank();

    double leftBound  = inputNumber("C1 = ");             CHECK_CANCEL;
    int    leftOp     = inputOperator("op1 (C1 op1 Ax+B):"); CHECK_CANCEL;
    double coeffA     = inputNumber("A = ");              CHECK_CANCEL;
    double constantB  = inputNumber("B = ");              CHECK_CANCEL;
    int    rightOp    = inputOperator("op2 (Ax+B op2 C2):"); CHECK_CANCEL;
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
    printAbsIneqSolution(A, B, C, op);
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

static void menuEquationsAndInequalities(void)
{
    const char *options[] = {
        "Linear Equation",
        "Quadratic Equation",
        "Linear Inequality",
        "Three-Way Inequality",
        "Back"
    };
    int sel;
    while ((sel = showMenu("EQUATIONS & INEQ.", options, 5)) >= 0) {
        switch (sel) {
            case 0: solveLinearEquation();     break;
            case 1: solveQuadraticEquation();  break;
            case 2: solveLinearInequality();   break;
            case 3: solveThreeWayInequality(); break;
            case 4: return;
        }
    }
}

static void menuAbsoluteValue(void)
{
    const char *options[] = {
        "|Ax+B| = C",
        "|Ax+B| op C",
        "||Ax+B|-C| op D",
        "Back"
    };
    int sel;
    while ((sel = showMenu("ABSOLUTE VALUE", options, 4)) >= 0) {
        switch (sel) {
            case 0: solveAbsEq();         break;
            case 1: solveAbsIneq();       break;
            case 2: solveNestedAbsIneq(); break;
            case 3: return;
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

/* ═══════════════════════════════════════════════
   MAIN MENU
   ═══════════════════════════════════════════════ */

static void runMainMenu(void)
{
    const char *options[] = {
        "Equations & Inequalities",
        "Absolute Value",
        "Formulas & Reference",
        "Quit"
    };
    int sel;
    for (;;) {
        sel = showMenu("MATH SOLVER CE", options, 4);
        if (sel < 0 || sel == 3) break;
        switch (sel) {
            case 0: menuEquationsAndInequalities(); break;
            case 1: menuAbsoluteValue();            break;
            case 2: menuReference();                break;
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
    printLine("MathSolverCE  v2.2", COL_NAVY);
    printBlank();
    printLine("Goodbye!", COL_ORANGE);
    blit();

    for (volatile int i = 0; i < 400000; i++);

    gfx_End();
    return 0;
}