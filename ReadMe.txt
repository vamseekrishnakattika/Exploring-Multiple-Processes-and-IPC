The entire code for the project is in a single 'C' file 'Project1.c'. The test files are sample1.txt, sample2.txt, sample3.txt, sample4.txt, and sample5.txt.

Compile and Execution instructions:

The project is implemented using C language.

To compile the project we have use 'gcc' followed by the name of the file.
Ex: gcc Project1.c

If we want to enable the warnings we have to use 'gcc -Wall' followed by the name of the file 
Ex: gcc -Wall Project1.c

By default the output file generated is 'a.out'

If we want to change the output file name we can do the same using the following while compilation.
Ex: gcc -Wall Project1.c -o Project1

The above command compiles the file with all the warnings enabled and generates the output executable file 'Project1'

For execution we have to use the executable filename followed by the input test filename and the timer value.

The executable is either a.out or the name of the output file we have given while compilation.

The input file name and timer value are the command line arguments with input filename as argument 1 and timer value argument 2

The argument 0 will be the the executable file itself.

Ex: ./Project1 sample1.txt 20 or ./a.out sample1.txt 20

If we don't provide either the input file or the timer value or both the program will exit with a error message
'Valid Input Format: Executable Filename<>Input File<>Timer Value'

Samples and their expected outputs:  

sample1.txt 
   Tests the indexed load instructions.
   Prints two tables, one of A-Z, the other of 1-10.

sample2.txt
   Tests the call/ret instructions.
   Prints a face where the lines are printed using subroutine calls.

sample3.txt 
   Tests the int/iret instructions.
   The main loop is printing the letter A followed by a number

   that is being periodically incremented by the timer.
   The number will increment faster if the timer period is
   shorter.

sample4.txt
   Tests the proper operation of the user stack and system
   stack, and also tests that accessing system memory in 
   user mode gives an error and exits.

Sample5.txt 
    Tests the proper operation of some of the instructions.
    It prints all the printable ASCII characters and their ASCII values that is from 32 to 127.
   'DONE' is printed using system call 