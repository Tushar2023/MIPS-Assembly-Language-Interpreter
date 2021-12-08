# MIPS-Assembly-Language-Interpreter
The repository contains contains 3 files all are working codes of MIPS interpreter at different levels containing more and more features as we progress.

# basic.cpp
The code for an interpreter for a subset of MIPS assembly language instructions. The program takes in the name of the text file (NOT machine instructions) which consist of MIPS assembly language program, as an arguement to run the interpreter on.
# Input Formar for basic.cpp
The allowed instructions in the program are: add, sub, mul, beq, bne, slt, j, lw, sw, and addi.
The data memory address for lw & sw commands should lie between (524288-1048576).
The immediate values stored should be integers only.
For j, bne, beq commands provide the line number to which you want to jump to.
The total number of instructions run after the completion should not be more than 131,072, for the
execution to be successful.
# Output Format of basic.cpp
The interpreter prints the Register File contents(32 register values in Hexadecimal format) after executing each instruction.
After the execution completes, the interpreter also prints statistics like the number of clock cycles and the number of times each instruction was executed. (Assuming that each instruction is executed in one clock cycle).

