# Custom Assembler in C
This repository is intended for demonstration and learning purposes.

## Overview
This project implements a two-pass assembler written in C.
The assembler translates a custom assembly language into machine code,
simulating the core stages of a real assembler.

The project was developed as part of my Computer Science degree and focuses on
low-level programming, parsing, and careful memory management.

---

## Key Features
- Two-pass assembly process (symbol collection and instruction encoding)
- Support for labels, directives, and instructions
- Preprocessing stage for macro handling
- Symbol tables for labels and external references
- Detailed syntax and semantic error reporting
- Focus on minimizing dynamic memory allocations

---

## Project Structure
```
custom-assembler-c/
├── include/                 # Header files (.h)
├── src/                     # Source files (.c)
├── examples/                # Example inputs and outputs
│   ├── valid/
│   ├── errors/
│   └── outputs/
├── Makefile
└── README.md
```
---

## How to Build and Run

### Build
The project includes a Makefile for compilation.
To build the assembler, run:

```bash
make 
```

### Run
The assembler expects an input file with a `.as` extension.

```bash
./assembler examples/valid/simple.as
```

If the input is valid, the assembler generates:
- .am – preprocessed file
- .ob – object file
- .ent – entries file (if applicable)
- .ext – externals file (if applicable)


### Clean
To remove compiled files and the executable
```bash
make clean
```

### Error handling
- Files without a `.as` extension are rejected.
- Syntax and semantic errors are reported with line numbers.
- If an error occurs, no output files are generated.

Example:

```bash
./assembler examples/errors/invalid_extension.txt
```

---

## What I Learned
- How a two-pass assembler works internally
- Designing and managing symbol tables in C
- Parsing and validating low-level languages
- Writing modular C code for a non-trivial system
- Managing memory explicitly and avoiding unnecessary allocations