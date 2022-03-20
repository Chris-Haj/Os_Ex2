Mini Shell Simulation
Authored by Christopher Haj
207824772

==Description==
This program is used to simulate a mini shell that is built through two files, named ex2a.c and ex2b.c.
The C file ex2a.c is used to simulate some commands of the linux shell and passes out the same output as the actual shell, whereas the ex2b.c file is built
on-top of the first file to add another functionality of the shell to use a command that was already passed in through the use of !x, where x can be any number from 1 upto
the number of commands used through the run of the program.


Program DATABASE:
A text file by the name file.txt is used to store all commands entered by user (excluding !x commands).

functions:
1) loop: contains the main loop which asks user for input to process.
2) checkInput: this function checks what kind of input was entered and deals with it according to what kind it is (done, history, cd, other...),.
   if it is (other) then the input is sent into the wordCounter and creates an array of pointers of the return size of wordCounter+1, and is sent into executeCommand(...)
3) wordCounter: this function first checks if the command only contained the words done, history, or cd, if yes, it does the appropriate action, otherwise
   this function counts and returns the amount of words contained in the input.
4) executeCommand: this function places each word of the input in order into the array received from checkInput and sends the array into the execvp(...) built-in function
   to execute the command.
5) readHistory: this function reads and outputs the contents of the file.txt in a numbered fashion.
6) cmdFromHistory: this function is only used through an input of type !x which, executes the command in line x of file.txt if x is between [1,NumberOfLines]

==Program Files==
ex2a.c
ex2b.c

==How to compile?==
compile: gcc ex2a.c -o ex2a
compile: gcc ex2b.c -o ex2b



==Input:==
1) "history"
2) "cd"
3) "done"
4) Input is required to be some basic commands of the linux shell, some examples are (ls, echo, cal, mkdir, touch)
5) Another kind of input that is allowed is !x to use a command that was written before in the current run.

==Restricted inputs are:==
1) Input of only spaces.
2) Input of nothing.
3) Input where the first or last indexes are a space.
4) The command cd (will be available in a later program)
5) !x where x is a number greater current amount of lines in file.txt


==Output:==
There can be multiple kinds of output depending on which commands are passed in.
1) Through the input "history", the contents of file.txt are printed out.
2) If cd was entered, then it simply outputs an error "command not supported yet".
3) If done was entered, the program prints out how many commands were used and the total amount of words in all inputs were (excluding !x commands)




