// Author: Shikang Xu; ECE 353 TA

// List the full names of ALL group members at the top of your code.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
//feel free to add here any additional library names you may need
#define SINGLE 1
#define BATCH 0
#define REG_NUM 32
#define MEMSIZE 2048

main (int argc, char *argv[]){
    int sim_mode=0;//mode flag, 1 for single-cycle, 0 for batch
    int c,m,n;
    int i;//for loop counter
    long mips_reg[REG_NUM];
    long pgm_c=0;//program counter
    long sim_cycle=0;//simulation cycle counter
    //define your own counter for the usage of each pipeline stage here

    int test_counter=0;
    FILE *input=NULL;
    FILE *output=NULL;
    printf("The arguments are:");

    for(i=1;i<argc;i++){
        printf("%s ",argv[i]);
    }
    printf("\n");
    if(argc==7){
        if(strcmp("-s",argv[1])==0){
            sim_mode=SINGLE;
        }
        else if(strcmp("-b",argv[1])==0){
            sim_mode=BATCH;
        }
        else{
            printf("Wrong sim mode chosen\n");
            exit(0);
        }

        m=atoi(argv[2]);
        n=atoi(argv[3]);
        c=atoi(argv[4]);
        input=fopen(argv[5],"r");
        output=fopen(argv[6],"w");

    }

    else{
        printf("Usage: ./sim-mips -s m n c input_name output_name (single-sysle mode)\n or \n ./sim-mips -b m n c input_name  output_name(batch mode)\n");
        printf("m,n,c stand for number of cycles needed by multiplication, other operation, and memory access, respectively\n");
        exit(0);
    }
    if(input==NULL){
        printf("Unable to open input or output file\n");
        exit(0);
    }
    if(output==NULL){
        printf("Cannot create output file\n");
        exit(0);
    }
    //initialize registers and program counter
    if(sim_mode==1){
        for (i=0;i<REG_NUM;i++){
            mips_reg[i]=0;
        }
    }

    //start your code from here





    ///////////////////////////////////////////

    //output code 2: the following code will output the register
    //value to screen at every cycle and wait for the ENTER key
    //to be pressed; this will make it proceed to the next cycle
    printf("cycle: %d ",sim_cycle);
    if(sim_mode==1){
        for (i=1;i<REG_NUM;i++){
            printf("%d  ",mips_reg[i]);
        }
    }
    printf("%d\n",pgm_c);
    pgm_c+=4;
    sim_cycle+=1;
    test_counter++;
    printf("press ENTER to continue\n");
    while(getchar() != '\n');

    ////////////////////////////////////////////
    if(sim_mode==0){
        fprintf(output,"program name: %s\n",argv[5]);
        fprintf(output,"stage utilization: %f  %f  %f  %f  %f \n",
                ifUtil, idUtil, exUtil, memUtil, wbUtil);
        // add the (double) stage_counter/sim_cycle for each
        // stage following sequence IF ID EX MEM WB

        fprintf(output,"register values ");
        for (i=1;i<REG_NUM;i++){
            fprintf(output,"%d  ",mips_reg[i]);
        }
        fprintf(output,"%d\n",pgm_c);

    }
    //close input and output files at the end of the simulation
    fclose(input);
    fclose(output);
    return 0;
}

////////////////////////////
/////ABOVE IS TA CODE///////
/////BELOW IS OUR CODE//////
////////////////////////////

struct Command {
    char* opcode;
    int rs;
    int rt;
    int rd;
    int immediate;
};

struct Register {
    int value;
    bool flag; // if flag == true, the register is safe
};

struct LatchA {
    struct Command command;
	int cycles;
};

struct LatchB {
    char* opcode;
    int reg1; //reg value
    int reg2; //reg value
    int regResult;
    int immediate;

    int cycles;
};

struct LatchC {
    char* opcode;
    int reg2;
    int regResult;
    int result;

    int cycles;
};

struct LatchD {
    char* opcode;
    int regResult;
    int result;
};
long pgm_c = 0;//program counter
//Array of registers
struct Register registers[REG_NUM];
//Instruction memory
struct Commands iM[MEM_SIZE];
//Data memory
int dM[MEM_SIZE];
int legalCommand(struct Command command){
}
char *progScanner(...){} /*This reads as input a pointer to a string holding the next
line from the assembly language program, using the fgets() library function to do
so. progScanner() removes all duplicate spaces, parentheses, and commas from
it from it and a pointer to the resulting character string will be returned. Items
will be separated in this character string solely by a single space. For example
add $s0, $s1, $s2 will be transformed to add $s0 $s1 $s2. The instruction
lw $s0, 8($t0) will be converted to lw $s0 8 $t0. If, in a load or store instruction,
mismatched parentheses are detected (e.g., 8($t0( instead of 8($t0) or a missing
or extra ), ), this should be reported and the simulation should then stop.
In this simulator, we will assume that consecutive commas with nothing in between
(e.g., ,,) are a typo for a single comma, and not flag them as an error; such
consecutive commas will be treated as a single comma.*/

char *regNumberConverter(char* input){
    char* token[6];
    char* ret="";
    char* substr;
    token[0]=strtok(input," ");
    int i=0;
    while(token[i]!=NULL){//fills token array with the words in the line
        token[i++]=strtok(NULL," ");
    }
    for(int j=0;j<i;j++){
        strncpy(substr,token[i],1);//stores the first char of the token in substr
        if(!strcmp(substr,"$")){
            strcat(" ",ret);
            strcat(getRegNum(token[i]),ret);
        }
        else{
            strcat(" ",ret);
            strcat(token[i],ret);
        }
    }
    return ret;
} /* This function accepts as input the output of
the progScanner() function and returns a pointer to a character string in which all
register names are converted to numbers.
MIPS assembly allows you to specify either the name or the number of a register.
For example, both $zero and $0 are the zero register; $t0 and $8 both refer to register
8,and so on. Your parser should be able to handle either representation. (Use the
table of registers in Hennessy and Patterson or look up the register numbers online.)
The code scans down the string looking for the $ delimiter. Whatever is to the right
of this (and to the left of a space or the end of string) is the label or number of the
register. If the register is specified as a number (e.g., $5), then the $ is stripped and
5 is left behind. If it is specified as a register name (e.g., $s0), the name is replaced
by the equivalent register number). If an illegal register name is detected (e.g., $y5)
or the register number is out of bounds (e.g., $987), an error is reported and the
simulator halts*/
char* getRegNum(char* reg){//takes in a register in hte form of "$xx" and returns the equivalent register number
    char* ret;//return string
    char* substr;//first char after $
    strncpy(substr, reg + 1, 1);
    char* substr2;//second char after $
    int regNum=0;
    bool isCharReg=false;//register starts off with a letter
    if ((!strcmp(substr,"Z")) || (!strcmp(substr,"a")) || (!strcmp(substr,"v")) || (!strcmp(substr,"t")) || (!strcmp(substr,"s")) || (!strcmp(substr,"k")) || (!strcmp(substr,"g")) || (!strcmp(substr,"f")) || (!strcmp(substr,"r"))){
        isCharReg=true;
        strncpy(substr2, reg + 2, 1);
    }
    if (!isCharReg) {//if it already is in number form, return it as is
        strncpy(ret, reg + 1, strlen(reg) - 1);
        return ret;
    }
    else{//otherwise, we must take the number equivalent of the register
        if((!strcmp(substr,"Z")))
            return "0";
        else if((!strcmp(substr,"a"))){
            if((!strcmp(substr2,"0")))
                return "4";
            else if((!strcmp(substr2,"3")))
                return "7";
            else if((!strcmp(substr2,"1")))
                return "5";
            else if((!strcmp(substr2,"2")))
                return "6";
            else
                return "1";
        }
        else if((!strcmp(substr,"v"))){
            if((!strcmp(substr2,"0")))
                return "2";
            else
                return "3";
        }
        else if((!strcmp(substr,"t"))){
            if((!strcmp(substr2,"0")))
                return "8";
            else if((!strcmp(substr2,"7")))
                return "15";
            else if((!strcmp(substr2,"1")))
                return "9";
            else if((!strcmp(substr2,"2")))
                return "10";
            else if((!strcmp(substr2,"3")))
                return "11";
            else if((!strcmp(substr2,"4")))
                return "12";
            else if((!strcmp(substr2,"5")))
                return "13";
            else
                return "14";
        }
        else if((!strcmp(substr,"s"))){
            if((!strcmp(substr2,"0")))
                return "16";
            else if((!strcmp(substr2,"7")))
                return "23";
            else if((!strcmp(substr2,"1")))
                return "17";
            else if((!strcmp(substr2,"2")))
                return "18";
            else if((!strcmp(substr2,"3")))
                return "19";
            else if((!strcmp(substr2,"4")))
                return "20";
            else if((!strcmp(substr2,"5")))
                return "21";
            else if((!strcmp(substr2,"6")))
                return "22";
            else
                return "29";//sp
        }
        else if((!strcmp(substr,"k"))){
            if((!strcmp(substr2,"0")))
                return "26";//k0
            else
                return "27";//k1
        }
        else if((!strcmp(substr,"g")))
            return "28";//gp
        else if((!strcmp(substr,"f")))
            return "30";//fp
        else if((!strcmp(substr,"r")))
            return "31";//ra
        else
            return "*";//if we get *s then we know something isnt right
    }
}

struct inst parser(...){} /* This function uses the output of regNumberConverter().
The instruction is returned as an inst struct with fields for each of the fields of MIPS
assembly instructions, namely opcode, rs, rt, rd, Imm. Of course, not all the fields
will be present in all instructions; for example, beq will have just two register and
one Imm fields.
Each of the fields of the inst struct will be an integer. You should use
the enumeration type to conveniently describe the opcodes, e.g., enum inst
{ADD,ADDI,SUB,MULT,BEQ,LW,SW}. You can assume that the assembly language
instr*/

void IF(...){}
void ID(...){}
void EX(..){}
void MEM(...){}
void WB(...){} /* These
functions simulate activity in each of the five pipeline stages. All data, structural,
and control hazards must be taken into account. Keep in mind that several operations
are multicycle and that these stages are themselves not pipelined. For
example, if an add takes 4 cycles, the next instruction cannot enter EX until these
cycles have elapsed. This, in turn, can cause IF to be blocked by ID. Branches will
be resolved in the EX stage. We will cover in the lectures some some issues related
to realizing these functions.
Keep in mind that the only requirement is that the simulator be cycle-by-cycle
register-accurate. You don’t have to simulate the control signals. So, you can
simply pass the instruction from one pipeline latch to the next.*/






