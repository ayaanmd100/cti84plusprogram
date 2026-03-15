# ----------------------------
# Program Options
# ----------------------------

NAME = UltimateMathSolver
ICON = icon.png
DESCRIPTION  = "C"
COMPRESSED = YES
ARCHIVED = YES
CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz
COMPRESSED_MODE = zx0
LTO = YES
PREFER_OS_CRT = YES

# ----------------------------

include $(shell cedev-config --makefile)
