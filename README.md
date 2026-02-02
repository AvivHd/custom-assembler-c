# Custom Assembler in C

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
├── include/ # Header files (.h)
├── src/ # Source files (.c)
├── .gitignore
├── LICENSE
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
To run the assembler on an assembly input file

```bash
./assembler input.as
```

### Clean
To remove compiled files and the executable
```bash
make clean
```

---

## What I Learned
- How a two-pass assembler works internally
- Designing and managing symbol tables in C
- Parsing and validating low-level languages
- Writing modular C code for a non-trivial system
- Managing memory explicitly and avoiding unnecessary allocations