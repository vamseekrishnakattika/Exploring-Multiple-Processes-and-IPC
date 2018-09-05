/* The project will simulate a simple computer system consisting of a CPU and Memory.
The CPU and Memory will be simulated by separate processes that communicate.*/

#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

int memoryInitialize(int*, char*);
int readFromMemory(int,int);
int writeToMemory(int,int,int);
void endFunction();
int processInstruction(int*,int*,int*,int*,int*,int*P,int*,int*,int*);

int childPipe[2], parentPipe[2];//Pipes descriptors

int main(int argc, char *argv[]){

	//Checking for the input filename and timer value
    //If the input file name and timer values are not provided as
    //command line arguments exit with the message

	if(argc<3){
		printf("Valid Input Format: Executable Filename<>Input File<>Timer Value\n");
		return 0;						  
	}
	pid_t pid;                          // Process id. 0 for child process and other value for the parent process
	char *filename=argv[1];            // argument 1 in the command line arguments which is the input file
    int timerValue=atoi(argv[2]);     // argument 2 is the timer value
    int processorMode=0,iFlag=0;     // processor mode and timer interrupt flags
	// Decalring PC,AC,X,Y,IR and SP
	int PC=0;
	int AC=0;
	int X=0;
	int Y=0;
	int IR=0;
	int SP=1000;
	int executedInstructions=0;    // To count the number of instructions
    
    
    /* Pipes are used for communication between a child and its parent
        pipe[0] is to read the data in the pipe
        pipe[1] is used to write the data */

        if(pipe(childPipe)<0){
                printf("Pipe failed. Exiting the process\n"); // If Pipe fails exit with an error message
                exit(-1);
        }
	 if(pipe(parentPipe)<0){
	        printf("Pipe failed. Exiting the process\n");   //If Pipe fails exit with an error message 
		exit(-1);
		}

    /*
    The fork() function is used to create a new process from an existing process. The new process is called the child process, and the existing process is called the parent.
    You can tell which is which by checking the return value from fork(). The parent gets the child's pid returned to him, but the child gets 0 returned to him. 
    */

	pid=fork();
		switch(pid){
		case -1:{
			printf("Fork failed.Exiting the process\n");  // If Fork fails exit with an error message 
			exit(-1);
		}
		break;
		case 0:{

			/* Case 0. This is the child process. It reads data from the input file provided and then
			 It initializes the memory
			 It also reads the address from parent pipe what the parent need to access form the memory
			 and write it to child pipe so that parent can read. It can write it into the memory by reading the address and the data to be written  */

			int *memory=(int *)malloc(2000*sizeof(int));  // Allocating memory of 2000 integer array dynamically
			if(memory==NULL){
				printf("Unable to create memory: Exiting\n"); // unable to allocate, exit with error message
				exit(1);			
				}
			int  buf[3];
            memoryInitialize(memory,filename);
			while(1){
				read(parentPipe[0],buf,sizeof(buf));  // read from the parent
				if(buf[0]==0){
				 write(childPipe[1],&memory[buf[1]],sizeof(int)); // write to the parent in read function call 
				}
				else if(buf[0]==1){
					memory[buf[1]]=buf[2];
				write(childPipe[1],&buf[1],sizeof(int));  // write to parent in write function call
   				}
				else
                        break;           // if end instruction is executed exit
			}
			free(memory);  // free the memory allocated dynamically and close the pipes
			close(childPipe[0]);
			close(childPipe[1]);
			close(parentPipe[0]);
			close(parentPipe[1]);
			_exit(0);
			}
		break;
		default:{
          	int result=0;
            // This is the parent process

			while(1){

			IR=readFromMemory(PC,processorMode);   // reading the instruction form the memory
            result=processInstruction(&PC,&IR,&AC,&X,&Y,&SP,&processorMode,&executedInstructions,&iFlag);  // processing the instruction
			
            if(result==-1) // if end instruction is executed exit form the loop.
                break;
            if(result==1&&processorMode==0) // If the processor is not in the system mode then count the number of instructions executed
                executedInstructions++;

            // The following function will implement the timer interrupt

            if(processorMode==0&&timerValue==executedInstructions){
                            iFlag=1; 
                            IR=29;
                            processInstruction(&PC,&IR,&AC,&X,&Y,&SP,&processorMode,&executedInstructions,&iFlag);              
                                      
                }
            PC++; // increment the program counter
           }	
		       
		    close(childPipe[0]);    // close all the pipes
		    close(childPipe[1]);
			close(parentPipe[0]);
			close(parentPipe[1]);
            waitpid(-1,NULL,0);
            }
		break;
	  	}
	  
	return 0;
}

    /* The following is the function to initialize the memory. It reads a file which is specified in the command line arguments(argument 1) and stores the instructions and data
    in an integer array of 2000 elements whicha acts as the memory in this project*/
    
    // Takes pointer to memory and pointer to the file to read as arguments    
int memoryInitialize(int *memory, char *filename){
    FILE *fp;
    int i=0;
    char *value;
    char buffer[100];
    fp=fopen(filename,"r");
    if(fp<0){
    	printf("Unable to read the file and write to memory"); //If it is unale to read the file then throw an error and exit
	exit(-1);
    } /* The following code reads the contents of the file and writes it to the array of 2000 integers. The first 1000 integer array(0 to 999) is used to store the user program
        The remaining 1000 array(1000-1999) is used to store the system code */
     while(fscanf(fp,"%[^\n]\n",buffer)!=EOF){
   	   value=strtok(buffer," ");
	   if(value[0]=='/')
	   	continue;
		if(value[0]=='.'){
		 int j=1;
		 i=0;
			while(j<strlen(value)-1){
			 i=10*i+(value[j]-'0');
				j++;
			}
			continue;
		}
		memory[i]=atoi(value);
        i++;	
	}
	value=NULL;
    fclose(fp);
    return 0;

}
    /* The following is a function to read data from the memory. It takes address as an argumens and returns the value store in that address. It also takes
      system mode as an argument to restrict the program accessing system code in the user mode   */

int readFromMemory(int address,int mode){
if(mode==0 && address>999){	
	
		printf("Memory violation: accessing system address %d in user mode\n",address);// If trying to access sytem code in user program, exit from the program showing the reason.
        endFunction();		
        exit(-1);
	}
else {
	int array[3];
	int data;
	array[0]=0;
	array[1]=address;
	array[2]=-1;
	write(parentPipe[1],array,sizeof(array));
	read(childPipe[0],&data,sizeof(int));
	return data;
    }
}

/* The following is a function to write data to the memory. It takes address and data to be written as the arguments and returns the data sored in that address. It also takes
      system mode as an argument to restrict the program accessing system code in the user mode   */

int writeToMemory(int address, int data, int mode){	
if(mode==0 && address>999){	
	
		printf("Memory violation: accessing system address %d in user mode\n",address);// If trying to access sytem code in user program, exit from the program showing the reason.
        endFunction();		
        exit(-1);
	}
else {
	int array[3];
	array[0]=1;
	array[1]=address;
	array[2]=data;
	write(parentPipe[1],array,sizeof(array));
	read(childPipe[0],&data,sizeof(int));
    return data;
    }
}
    /* This function is used to terminate the child process whenever the end instruction is executed. Yhis doesn't have any return value*/
void endFunction(){
	int array[3]={2,-1,-1};
	write(parentPipe[1],array,sizeof(array));
	
}
    /* This function is processes all the instructions that are provided in the program. This takes many arguments which are program counter, instruction register, accumulator, and registers X and Y,
     stack pointer, processor mode, number of instructions execute and a flag for timer*/
int processInstruction(int *PC,int *IR, int *AC, int *X, int *Y, int *SP,int *processorMode,int *instructions,int *iFlag){
            
            int port,stackPointer;// Local variables

			switch(*IR){
				case 1:*PC=*PC+1;       // Load value - Load the value into the AC
					*AC=readFromMemory(*PC,*processorMode); 
					break;

				case 2:*PC=*PC+1;      // Load addr - Load the value at the address into the AC
					*AC=readFromMemory(readFromMemory(*PC,*processorMode),*processorMode);
					break;

				case 3:*PC=*PC+1;       // LoadInd addr - Load the value from the address found in the given address into the AC
                                        // (for example, if LoadInd 500, and 500 contains 100, then load from 100).
					*AC=readFromMemory(readFromMemory(readFromMemory(*PC,*processorMode),*processorMode),*processorMode);
					break;

				case 4:*PC=*PC+1;       // LoadIdxX addr - Load the value at (address+X) into the AC
                                        // (for example, if LoadIdxX 500, and X contains 10, then load from 510)
					*AC=readFromMemory(*X+readFromMemory(*PC,*processorMode),*processorMode);
					break;

				case 5:*PC=*PC+1;	    // LoadIdxY addr - Load the value at (address+Y) into the AC
					*AC=readFromMemory(*Y+readFromMemory(*PC,*processorMode),*processorMode);
					break;

				case 6:*AC=readFromMemory(*X+*SP,*processorMode);       // LoadSpX - Load from (Sp+X) into the AC (if SP is 990, and X is 1, load from 991).
					break;

				case 7:*PC=*PC+1;       // Store addr - Store the value in the AC into the address
					  writeToMemory(readFromMemory(*PC,*processorMode),*AC,*processorMode);			 
					break;

				case 8:*AC=rand()%100;      // Get - Gets a random int from 1 to 100 into the AC
					break;

				case 9:*PC=*PC+1;       // Put port - If port=1, writes AC as an int to the screen
                                         //           If port=2, writes AC as a char to the screen
					port=readFromMemory(*PC,*processorMode);
					if(port==1)
					  printf("%d",*AC);
					else
					  printf("%c",*AC);
					break;

				case 10:*AC+=*X;        //  AddX - Add the value in X to the AC
					break;

				case 11:*AC+=*Y;        //  AddY - Add the value in Y to the AC
					break;

				case 12:*AC=*AC-*X;     //  SubX - Subtract the value in X from the AC
					break;

				case 13:*AC=*AC-*Y;     //  SubY - Subtract the value in Y from the AC
					break;

				case 14:*X=*AC;         // / CopyToX - Copy the value in the AC to X                  
					break;

				case 15:*AC=*X;         // CopyFromX - Copy the value in X to the AC
               		break;

				case 16:*Y=*AC;         // CopyToY - Copy the value in the AC to Y
					break;

				case 17:*AC=*Y;         // CopyFromY - Copy the value in Y to the AC
					break;

				case 18:*SP=*AC;        // CopyToSp - Copy the value in Ac to the SP
					break;

				case 19:*AC=*SP;        // CopyFromSp - Copy the value in SP to the AC
					break;

				case 20:*PC=readFromMemory(*PC+1,*processorMode);             // Jump addr - Jump to the address
					*PC=*PC-1;
					break;

				case 21:if(*AC==0){                                          // JumpIfEqual addr - Jump to the address only if the value in the AC is zero
					   *PC=readFromMemory(*PC+1,*processorMode);
					   *PC=*PC-1;
					}
					 else
 					   *PC=*PC+1;
					break;

				case 22:if(*AC!=0){                                         // JumpIfNotEqual addr - Jump to the address only if the value in the AC is not zero
					   *PC=readFromMemory(*PC+1,*processorMode);
					   *PC=*PC-1;
					}
					 else
 					   *PC=*PC+1;
					break;

				case 23:writeToMemory(--(*SP),*PC+2,*processorMode);        // Call addr - Push return address onto stack, jump to the address
					*PC=readFromMemory(*PC+1,*processorMode)-1;				
					break;

				case 24:*PC=readFromMemory((*SP)++,*processorMode)-1;       // Ret - Pop return address from the stack, jump to the address
					break;

				case 25:*X=*X+1;        // IncX - Increment the value in X
					break;

				case 26:*X=*X-1;        // DecX - Decrement the value in X
					break;

				case 27:writeToMemory(--(*SP),*AC,*processorMode);      // Push - Push AC onto stack
					break;

				case 28:*AC=readFromMemory((*SP)++,*processorMode);     // Pop - Pop from stack into AC
					break;

                //Int - This  piece of code implements interrupt processing. Interrupts are disabled when the processor is in system mode
				case 29:    *processorMode=1;
                            stackPointer=(*SP);
                            *SP=2000;
                            writeToMemory(--(*SP),stackPointer,*processorMode);                 
                            writeToMemory(--(*SP),(*PC),*processorMode);
                            if(*iFlag==1){
                                     
                                     *PC=999;         
                                }

                            else{
                                     *PC=1499;
                            }
                    break;
                // Iret - This piece of code performs the operation of returning form interrupt.
				case 30:    *PC=readFromMemory((*SP)++,*processorMode);
                            *SP=readFromMemory((*SP)++,*processorMode);  
                            *processorMode=0;
                            if(*iFlag==1){
                                *instructions=0;
                                *iFlag=0;
                                }
                            return 0;
                    break;	
                // End - When this instruction is executed both the parent and child processes will end 	
				case 50:endFunction();
					   return -1;;
					break;
				
	}
	return 1;
}

