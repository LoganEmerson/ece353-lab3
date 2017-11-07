#include <stdio.h>

int main() {
	long cycleAdress=0;
    //here we can initialize all of our arrays and set values as well as launch off the program
    while (there are still cycles){
		cycle(cycleAdress);
		cycleAdress++;
	}
	return 0;
}
/*
* so the way i see this working is we have all 
* out registers set as global variables and 
* all of our local variables will be the variables
* on the stack.
*/
long regstr[];	/*a long is 32 bits(perfect). 
				 *Initialize it later to have 32 registers where $0=0. 
				 *maybe this should be a pointer.
				 */
String stages[][];/*there are 5 possable processes going on at once 
				 *and the state of which we shall store in this array
				 *this is how we will know if there needs to be any nops 
				 *1=IF
				 *2=ID
				 *3=EX
				 *4=MEM
				 *5=WB
				 *commands can move through this array like an queue so we will need a function for that
				 *the second dimension holds the command
				 */
	
void revalueStages(?){//this will re-evaluate the stages array assuming one cycle has passed
/*for most cases, we will just increment each array value by 1
 *when a stage hits 6(after WB) we will have to shift all the values up (stages[x]=stages[x-1
 *i figure we can represent nops as negative numbers starting at 0. this way if they we can still just increment them like the others. and if it is a negative number we can assume nothing is happening at that stage 
 */
}
void cycle(String command){//not sure how to describe this just yet
	for(int i=0;i<stages.length;i++){
		switch(stages[i]){
			case 1 :
			fetch(command);
			break;
			case 2 :
			decode(command);
			break;
			case 3 :
			exicute(command);
			break;
			case 4 :
			memoryAccess(command);
			break;
			case 5 :
			Write(command);
			break;
			default :
			wait();
		}
	}
	revaluateStages();
}
void fetch(){}
void decode(){}
void exicute(){}
void memoryAccess(){}
void Write(){}
void wait(){}
void add(){}
void sub(){}
void addi(){}
void lw(){}
void sw(){}
void beq(){}

