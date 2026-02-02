CC = gcc

CFLAGS = -pedantic -ansi -Wall -std=c90

# Executable name
EXEC = assembler

# Source files
SRC = assembler.c encode.c firstpass.c output.c parser.c preprocessor.c sec_pass.c tables.c

# Object files
OBJ = assembler.o encode.o firstpass.o output.o parser.o preprocessor.o sec_pass.o tables.o

# Default target
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC)

# Compile each source file into an object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
