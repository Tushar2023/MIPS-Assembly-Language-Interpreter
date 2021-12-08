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
# Output Format of intermediate.cpp
For each clock cycle the simulator prints the changes in the memory, or the change in value of any register or any jump executed.
For DRAM requests the simulator also prints the interval of clock cycles taken to execute that request.
After the execution completes, the interpreter also prints statistics like the number of clock cycles, Number of row buffer updates and the number of times each instruction was executed. (Assuming that each instruction is executed in one clock cycle).


# advanced.cpp
The new implementation has all the previous features along with advanced features of  DRAM request manager for multicore processors, non-blocking memory access (subsequent instructions
don’t always wait for the previous instructions to complete).
# Input Format for advanced.cpp
Program asks for these quantities in the following order:
1. Row access delay
2. Column access delay
3. Number of cores/files
4. M (Upper limit of cycles)
5. File paths
# Output Format of advanced.cpp
Program prints the following:
1. Analysis of each core as to which instruction is run in each cycle.
2. Gives adequate info about each DRAM access (instruction being queued and
executed).
