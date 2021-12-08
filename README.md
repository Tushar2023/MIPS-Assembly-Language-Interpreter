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

# intermediate.cpp
The code has an additional feature to that of basic.cpp. This interpreter implements memory request ordering for better performance with same results.
# Input Format for intermediate.cpp
The program takes in the name of the text file (NOT machine instructions) which consist of MIPS assembly language program, the value of ROW_ACCESS_DELAY and the value of COLUMN_ACCESS_DELAY as arguments on the command line.
The allowed instructions in the program are: add, sub, mul, beq, bne, slt, j, lw, sw, and addi.
The immediate values stored should be integers only.
For j, bne, beq commands provide the label to which you want to jump to.
# Output Format for intermediate.cpp
For each clock cycle the simulator prints the changes in the memory, or the change in value of any register or any jump executed.
For DRAM requests the simulator also prints the interval of clock cycles taken to execute that request.
After the execution completes, the interpreter also prints statistics like the number of clock cycles, Number of row buffer updates and the number of times each instruction was executed. (Assuming that each instruction is executed in one clock cycle).
# Strengths of intermediate.cpp
The implementation takes the minimum delay as possible by doing reordering of input DRAM requests. The implementation succesfully handles the features like labels and ofsets in the input. The order of the basic instructions remain unchanged even after reordering of the DRAM requests. The final register values as same as expected with less delay than usual.
