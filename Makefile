# Compiler
CC = clang

# Compiler flags
CFLAGS = -O3

# Frameworks
FRAMEWORKS =

# Source file
SRC = main.c image_creator.c bitmap.c

# Output executable
OUT = main

# Default target
all: $(OUT)

# Link and create the executable
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(FRAMEWORKS) -o $(OUT) $(SRC)

# Clean target
clean:
	rm -f $(OUT)
