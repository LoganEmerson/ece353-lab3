// List the full names of ALL group members at the top of your code.
//Logan Emerson, Aaron Linz, Matt Delude

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
//feel free to add here any additional library names you may nee
#define SINGLE 1
#define BATCH 0
#define REG_NUM 32
#define MEM_SIZE 2048
#define WORDMAX 510


////////////////////////////
/////ABOVE IS TA CODE///////
/////BELOW IS OUR CODE//////
////////////////////////////
int linecount = 0;//number of lines
long pgm_c = 0;//program counter, need assertion that PC < 512, max # of lines in IM
FILE *input;
FILE *output;

typedef enum {
    add, sub, addi, mul, lw, sw, beq, haltSimulation
} Opcode;

struct Inst {
    Opcode opcode;
    int rs;
    int rt;
    int rd;
    int imm;
};

struct Register {
    int value;
    bool flag; // if flag == true, the register is safe
};

struct IFLatchID{ //latch between Instruction Fetch and Instruction Decode
    struct Inst inst;
    long cycles;
    bool done;
};

struct IDLatchEX {//Latch between instruction decode and Execute
    Opcode opcode;
    int reg1; //reg value
    int reg2; //reg value
    int regResult;
    int immediate;

    long cycles;
    bool done;
};

struct EXLatchM {//latch between Execute and Data Memory
    Opcode opcode;
    int reg2;
    int regResult;
    int result;

    long cycles;
    bool done;
};

struct MLatchWB {//Latch between memory and write back
    Opcode opcode;
    int regResult;
    int result;
    long cycles;
    bool done;
};
//assert( pc < 255 );
//Array of registers
struct Register registers[REG_NUM];
//Instruction memory
struct Inst iM[MEM_SIZE];
//Data memory
int dM[MEM_SIZE];
double ifUtil, idUtil,exUtil,memUtil,wbUtil;

int c, m, n;//made global so IF, ID, EX, MEM, and WB can access

char *progScanner(char line[]) {
    //printf("Got to progScanner");
    char temp;
    int i;//incrementer for loop below
    char out[100];//what is to be passed down to next function, output
    for(i=0;i<100;i++)  out[i]=' ';
    int oc = 0;//counter for out char array
    int space = 0; //for counting more than 1 consecutive space
    int comma = 0; //for counting more than 1 consecutive comma
    int paren = 0;//count for # of parentheses

    for (i = 0; i < 100; i++) {//need to search through the array and parse it correctly
        //we should only ever encounter 1 set of parentheses
        if(line[i]==0x0D) {
            break;
        }//when a \ is detected, start of \r and \n, end line
        if (line[i] == 0x28 || line[i] == 0x29) {//when we encounter a parentheses, 28=(   29=)
            space = 0;//reset # of consecutive spaces
            if (line[i] == 0x28) {//if we encounter a left parentheses
                paren++;
                if (oc > 0) {//checking to see if previous character was a space, need oc to be >0
                    if (out[oc - 1] != 0x20) {//checking to see if previous char was a space
                        out[oc] = 0x20;//put a space if previous character was not a space
                        oc++;//increment oc
                    }//end check for space
                }//end check for oc>0
            }//end if statement for when we get leftp,

            if (line[i] == 0x29) { paren--; }//when we encounter a right parentheses
            if (paren >= 2 || paren <= -1) {//when we have more than two left parentheses or start with right, error
                printf("Mismatched parentheses detected on line number %d: %s:", linecount, line);
                fprintf(output, "Mismatched parentheses detected, ending program", linecount, line);
                exit(0);//since, error, exit the program
            }//end check for mismatched parentheses
        }//end if statement for parentheses

        else if (line[i] == 0x20 || line[i] == 0x2C) {//when it detects a space or comma
            if (line[i] == 0x20) { space++; }//if space, increment # of consecutive spaces
            if (oc == 0) {//if first character is a space
                if (line[i] == 0x2C) {//if first character is a comma, error, quit program
                    printf("Typo detected on line number %d: %s:", linecount, line);
                    fprintf(output, "Typo detected on line number %d: %s:", linecount, line);
                    exit(0);//since, error, exit the program
                }//do nothing, as output of progscanner should not have leading spaces
            } else {//when oc, output counter != 0, then put a space
                if (out[oc - 1] != 0x20) {//if previous character in output is not a space
                    out[oc] = 0x20;
                    oc++;//when we put something in output array, then increment
                }
            }//if not consecutive spaces, place the space in output
            space++;//increment number of spaces
        }
            /*
        else if ((i - 1 - space) > 1) {//when checking for   , ,  need i to be greater than 2
            if ((line[i] == 0x2C) & (line[i - space] == 0x20) & (line[i - 1 - space] == 0x2C)) {
                printf("Syntax error detected: ', ,' %d: %s:", linecount, line);
                fprintf(output, "Syntax error detected: ', ,' %d: %s:", linecount, line);
                exit(0);//since, error, exit the program
            }
        }//end the if statement looking for , ,
        */
        else {//when we read anything but a comma, space, or parentheses
            temp = tolower(line[i]);
            out[oc] = temp;
            oc++;
            paren = 0;//reset # of parentheses
            space = 0;//reset # of consecutive spaces
        }

    }
    char *outP = out;//transfers the character array out[] to a char pointer, outP
    return outP;
    /*This reads as input a pointer to a string holding the next
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
}

char* getRegNum(char* reg){//takes in a register in hte form of "$xx" and returns the equivalent register number
    char ret[2];//return string
    char substr=reg[1];//first char after $
    char *subP=&substr;
    char *subP2;

    //strncpy(substr, reg + 1, 1);
    char substr2;//second char after $
    int regNum=0;
    bool isCharReg=false;//register starts off with a letter
    //if ((!strcmp(substr,"Z")) || (!strcmp(substr,"a")) || (!strcmp(substr,"v")) || (!strcmp(substr,"t")) || (!strcmp(substr,"s")) || (!strcmp(substr,"k")) || (!strcmp(substr,"g")) || (!strcmp(substr,"f")) || (!strcmp(substr,"r"))){
    if ((substr == 'z') || (substr == 'a') || (substr == 'v') || (substr == 't') || (substr == 's') || (substr == 'k') || (substr == 'g') || (substr == 'f') || (substr == 'r')){
        //if ((substr ==( 'z'||'a' ||'v'||'t'||'s'||'k'||'g'||'f'||'r'))){
        isCharReg=true;
        //strncpy(substr2, reg + 2, 1);
        substr2=reg[2];
        subP2 = &reg[2];
    }
    if (!isCharReg) {//if it already is in number form, return it as is
        //strncpy(ret, reg + 1, strlen(reg) - 1);
        ret[0]=reg[1];
        if(isdigit(reg[2])){ret[1]=reg[2];}
        else ret[1]=NULL;
        return ret;
    }
    else{//otherwise, we must take the number equivalent of the register
        if((!strcmp(subP,"z")))
            return "0";
        else if((!strcmp(subP,"a"))){
            if((!strcmp(subP2,"0")))
                return "4";
            else if((!strcmp(subP2,"3")))
                return "7";
            else if((!strcmp(subP2,"1")))
                return "5";
            else if((!strcmp(subP2,"2")))
                return "6";
            else
                return "1";
        }
        else if((!strcmp(subP,"v"))){
            if((!strcmp(subP2,"0")))
                return "2";
            else
                return "3";
        }
        else if((!strcmp(subP,"t"))){
            if((!strcmp(subP2,"0")))
                return "8";
            else if((!strcmp(subP2,"7")))
                return "15";
            else if((!strcmp(subP2,"1")))
                return "9";
            else if((!strcmp(subP2,"2")))
                return "10";
            else if((!strcmp(subP2,"3")))
                return "11";
            else if((!strcmp(subP2,"4")))
                return "12";
            else if((!strcmp(subP2,"5")))
                return "13";
            else
                return "14";
        }
        else if((!strcmp(subP,"s"))){
            if((!strcmp(subP2,"0")))
                return "16";
            else if((!strcmp(subP2,"7")))
                return "23";
            else if((!strcmp(subP2,"1")))
                return "17";
            else if((!strcmp(subP2,"2")))
                return "18";
            else if((!strcmp(subP2,"3")))
                return "19";
            else if((!strcmp(subP2,"4")))
                return "20";
            else if((!strcmp(subP2,"5")))
                return "21";
            else if((!strcmp(subP2,"6")))
                return "22";
            else
                return "29";//sp
        }
        else if((!strcmp(subP,"k"))){
            if((!strcmp(subP2,"0")))
                return "26";//k0
            else
                return "27";//k1
        }
        else if((!strcmp(subP,"g")))
            return "28";//gp
        else if((!strcmp(subP,"f")))
            return "30";//fp
        else if((!strcmp(subP,"r")))
            return "31";//ra
        else
            return "*";//if we get *s then we know something isnt right
    }
}

char *regNumberConverter(char* input) {
    int i;
    char ret[20];
    char *spa = " ";
    char *substr[1];
    char copierC[100];
    strcpy(copierC, input);
    char *token[6];
    for(i=0;i<6;i++)  token[i]="";
    for(i=0;i<20;i++) ret[i]=NULL;
    token[0] = strtok(copierC, " ");
    i=1;
    while (token[i-1] != NULL && i <6) {//fills token array with the words in the line
        token[i++] = strtok(NULL, " ");
    }
    int j;
    char *tokenString;
    for (j = 0; j < i-1; j++) {
        tokenString = token[j];
        //memcpy(substr, tokenString,1);//stores the first char of the token in substr
        if (tokenString[0]==0x24) {
            strcat(ret,getRegNum(tokenString));
            strcat(ret, spa);
        } else {
            strcat(ret, tokenString);
            strcat(ret, spa);

        }
    }
    char *retu=&ret;
    return retu;
/* This function accepts as input the output of
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
}

struct Inst parser(char* input){
    int i;
    char copierC[100];
    strcpy(copierC, input);
    char* token[6];
    for(i=0;i<6;i++)  token[i]="";
    token[0]=strtok(copierC," ");
    i=1;
    while(token[i-1]!=NULL&&i<6){//fills token array with the words in the line
        token[i++]=strtok(NULL," ");
    }
    /*struct Inst {
        char* opcode;
        int rs;
        int rt;
        int rd;
        int immediate;
    };*/
    //IF LOGIC ERRORS,
    struct Inst retVal;
    if (!strcmp(token[0], "haltsimulation")) {
        retVal.opcode=haltSimulation;
        retVal.rs=0;
        retVal.rt=0;
        retVal.rd=0;
        retVal.imm=0;
    }
    else if (!strcmp(token[0], "add")) {
        retVal.opcode=add;
        retVal.rs=atoi(token[2]);
        retVal.rt= atoi(token[3]);
        retVal.rd=atoi(token[1]);
        retVal.imm=0;
    }
    else if (!strcmp(token[0], "sub")) {
        retVal.opcode=sub;
        retVal.rs=atoi(token[2]);
        retVal.rt= atoi(token[3]);
        retVal.rd=atoi(token[1]);
        retVal.imm=0;
    }
    else if (!strcmp(token[0], "addi")) {
        retVal.opcode=addi;
        retVal.rs=atoi(token[2]);
        retVal.rt=atoi(token[1]);
        retVal.rd=0;
        retVal.imm=atoi(token[3]);
    }
    else if (!strcmp(token[0], "mul")) {
        retVal.opcode=mul;
        retVal.rs=atoi(token[2]);
        retVal.rt= atoi(token[3]);
        retVal.rd=atoi(token[1]);
        retVal.imm=0;
    }
    else if (!strcmp(token[0], "lw")) {
        retVal.opcode=lw;
        retVal.rs=atoi(token[3]);
        retVal.rt=atoi(token[1]);
        retVal.rd=0;
        retVal.imm=atoi(token[2]);
    }
    else if (!strcmp(token[0], "sw")) {
        retVal.opcode=sw;
        retVal.rs=atoi(token[3]);
        retVal.rt=atoi(token[1]);
        retVal.rd=0;
        retVal.imm=atoi(token[2]);
    }
    else if (!strcmp(token[0], "beq")) {
        retVal.opcode=beq;
        retVal.rs=atoi(token[1]);
        retVal.rt=atoi(token[2]);
        retVal.rd=0;
        retVal.imm=atoi(token[3]);
    }
    else {
        printf("Error On Line Containing %s : The opcode %s is illegal", input, token[0]);
        exit(0);
    }
    if(retVal.imm>0x0000ffff){
        printf("Error On Line Containing %s : The immediate value %d is to large", input, retVal.imm);
        exit(0);
    }
    /*else if(){
        printf("Error On Line Containing %s : The immediate value %d is to large", input, retval.imm);
        exit(0);
    }*/
    return retVal;
    /* This function uses the output of regNumberConverter().
The instruction is returned as an inst struct with fields for each of the fields of MIPS
assembly instructions, namely opcode, rs, rt, rd, Imm. Of course, not all the fields
will be present in all instructions; for example, beq will have just two register and
one Imm fields.
Each of the fields of the inst struct will be an integer. You should use
the enumeration type to conveniently describe the opcodes, e.g., enum inst
{ADD,ADDI,SUB,MULT,BEQ,LW,SW}. You can assume that the assembly language
instr*/
}

void IF(struct IFLatchID *inLatch){
    if(inLatch->cycles%c==0){
        struct Inst instruction=iM[pgm_c / 4];
        inLatch->inst=instruction;
        inLatch->done=true;
        ifUtil++;
    }
    else inLatch->done=false;
    /*if (instruction.opcode== haltSimulation){
        inLatch->inst=instruction;
        return 1;
    }
    if (inLatch->cycles == 0) {
        inLatch->cycles = cycles;
    }
    inLatch->cycles--;
    if (inLatch->cycles == 0){ //doesnt pass fetch untill cycles have dropped to 0
        inLatch->inst = instruction;
        return 0;
    } else {
        return 1;
    }*/
}

void ID(struct IFLatchID *inLatch,struct IDLatchEX *outLatch){
    /*
    struct Inst {
    Opcode opcode;
    int rs;
    int rt;
    int rd;
    int imm;
};
     struct IFLatchID{ //latch between Instruction Fetch and Instruction Decode
    struct Inst inst;
	int cycles;
};
     struct IDLatchEX {//Latch between instruction decode and Execute
        Opcode opcode;
        int reg1; //reg value
        int reg2; //reg value
        int regResult;
        int immediate;
        int cycles;
    };
*/
    /* switch (inLatch->inst.opcode) {
         case add:
         case sub:
         case mul:
         case addi:
             outLatch->opcode = inLatch->inst.opcode;
             outLatch->reg1 = inLatch->inst.rs;
             outLatch->reg2 = inLatch->inst.rt;
             outLatch->regResult = inLatch->inst.rd;
             outLatch->immediate = inLatch->inst.imm;
             outLatch->cycles = inLatch->cycles;

         case lw:
             outLatch->opcode = inLatch->inst.opcode;
             outLatch->reg1 = inLatch->inst.rs;
             outLatch->reg2 = inLatch->inst.rt;
             outLatch->regResult = inLatch->inst.rs;
             outLatch->immediate = inLatch->inst.imm;
             outLatch->cycles = inLatch->cycles;
         case beq:
             outLatch->opcode = inLatch->inst.opcode;
             outLatch->reg1 = inLatch->inst.rs;
             outLatch->reg2 = inLatch->inst.rt;
         case sw:
             outLatch->opcode = inLatch->inst.opcode;
             outLatch->reg1 = inLatch->inst.rs;
             outLatch->reg2 = inLatch->inst.rt;
         default:
             outLatch->opcode = inLatch->inst.opcode;
             outLatch->reg1 = 0;
             outLatch->reg2 = 0;
             outLatch->regResult = 0;
             outLatch->immediate = 0;
             outLatch->cycles = 0;
     }*/

    if(((registers[inLatch->inst.rt].flag) || (inLatch->inst.opcode==addi) || (inLatch->inst.opcode==lw) || (inLatch->inst.opcode==haltSimulation)) & (registers[inLatch->inst.rs].flag)){
        //if the flag value is true, the registers are good to go
        //this if runs if the flaggs are good, or it is an addi, halt, or lw
        outLatch->opcode = inLatch->inst.opcode;
        outLatch->immediate = inLatch->inst.imm;
        outLatch->reg1 = registers[inLatch->inst.rs].value;
        if((inLatch->inst.opcode==addi) || (inLatch->inst.opcode==lw) || (inLatch->inst.opcode==haltSimulation)) {
            outLatch->reg2 = 0;
        }
        else {
            registers[inLatch->inst.rt].value;
        }
        switch (inLatch->inst.opcode) {
            case add:
            case sub:
            case mul:
                outLatch->regResult = inLatch->inst.rd;
                registers[outLatch->regResult].flag = false;
                break;
            case addi:
            case lw:
                outLatch->regResult = inLatch->inst.rt;
                registers[outLatch->regResult].flag = false;
                break;
            case beq:
                outLatch->opcode = inLatch->inst.opcode;
                outLatch->reg1   = inLatch->inst.rs;
                outLatch->reg2   = inLatch->inst.rt;
                outLatch->immediate = inLatch->inst.imm;
                //Logan made some changes 11/12 1:52
            case sw:
                //incomplete
            case haltSimulation:
                outLatch->opcode = haltSimulation;
                outLatch->reg1 = 0;
                outLatch->reg2 = 0;
                outLatch->regResult = 0;
                outLatch->immediate = 0;
                //outLatch->cycles = 0;
            default:
                outLatch->regResult = 0;
                break;
        }
    }
    else{//NOP
        outLatch->opcode = add;
        outLatch->reg1 = 0;
        outLatch->reg2 = 0;
        outLatch->regResult = 0;
        outLatch->immediate = 0;
        outLatch->cycles = 0;

    }
    return;
}

void EX(struct IDLatchEX *inLatch,struct EXLatchM *outLatch){
    /*
     struct IDLatchEX {//Latch between instruction decode and Execute
    Opcode opcode;
    int reg1; //reg value
    int reg2; //reg value
    int regResult;
    int immediate;
    int cycles;
};
struct EXLatchM {//latch between Execute and Data Memory
    Opcode opcode;
    int reg2;
    int regResult;
    int result;
    int cycles;
};
     */
    if((((inLatch->cycles%n)==0)&&(inLatch->opcode!=mul))||(((inLatch->cycles%m)==0)&&(inLatch->opcode==mul))) {
        inLatch->done=true;//we have reached the needed # of cycles to complete the EX stage of datapath
        switch (inLatch->opcode) {
            case beq:
                outLatch->opcode = inLatch->opcode;
                outLatch->reg2 = inLatch->reg2;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = registers[inLatch->reg1].value - registers[inLatch->reg2].value;
                outLatch->cycles = inLatch->cycles;
                break;

            case add:
                outLatch->opcode = inLatch->opcode;
                outLatch->reg2 = inLatch->reg2;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = registers[inLatch->reg1].value + registers[inLatch->reg2].value;
                outLatch->cycles = inLatch->cycles;
                break;
            case sub:
                outLatch->opcode = inLatch->opcode;
                outLatch->reg2 = inLatch->reg2;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = registers[inLatch->reg1].value - registers[inLatch->reg2].value;
                outLatch->cycles = inLatch->cycles;
                break;
            case mul:
                outLatch->opcode = inLatch->opcode;
                outLatch->reg2 = inLatch->reg2;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = registers[inLatch->reg1].value * registers[inLatch->reg2].value;
                outLatch->cycles = inLatch->cycles;
                break;
            case addi:
                outLatch->opcode = inLatch->opcode;
                outLatch->reg2 = inLatch->reg2;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = registers[inLatch->reg1].value +
                                   inLatch->immediate;//adds the immediate to reg 1 and puts in result
                outLatch->cycles = inLatch->cycles;
                break;
            case lw:
                outLatch->opcode = inLatch->opcode;
                outLatch->reg2 = inLatch->reg2;
                outLatch->regResult = inLatch->regResult + inLatch->immediate;
                outLatch->result = 0;//puts value from reg 1 into result
                outLatch->cycles = inLatch->cycles;
                break;
            default:
                outLatch->opcode = inLatch->opcode;
                outLatch->reg2 = inLatch->reg2;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = 0;
                outLatch->cycles = inLatch->cycles;
                break;
        }
        return;
    }
    else inLatch->done=false;
}

void MEM(struct EXLatchM *inLatch,struct MLatchWB *outLatch) {
    /*
    struct EXLatchM {//latch between Execute and Data Memory
    Opcode opcode;
    int reg2;
    int regResult;
    int result;

    int cycles;
};

struct MLatchWB {//Latch between memory and write back
    Opcode opcode;
    int regResult;
    int result;
};

     */
    if (((inLatch->opcode == (lw || sw))&&inLatch->cycles == c) || (inLatch->opcode != (lw || sw))) {
        inLatch->done=true;
        //regResult = rd register
        switch (inLatch->opcode) {
            case add:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = inLatch->result;
                break;
            case sub:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = inLatch->result;
                break;
            case mul:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = inLatch->result;
                break;
            case addi:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = inLatch->result;
                break;
            case lw:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = registers[inLatch->regResult].value;
                break;
            default:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = inLatch->result;
                break;
        }
        return;
    }
    else inLatch->done==false;
}

void WB(struct MLatchWB *inLatch){
    wbUtil++;//increment wb utilization
    registers[inLatch->regResult].value=inLatch->result;
    registers[inLatch->regResult].flag= true;
    inLatch->done=true;
    return;
}
/* These
functions simulate activity in each of the five pipeline stages. All data, structural,
and control hazards must be taken into account. Keep in mind that several operations
are multicycle and that these stages are themselves not pipelined. For
example, if an add takes 4 cycles, the next instruction cannot enter EX until these
cycles have elapsed. This, in turn, can cause IF to be blocked by ID. Branches will
be resolved in the EX stage. We will cover in the lectures some some issues related
to realizing these functions.
Keep in mind that the only requirement is that the simulator be cycle-by-cycle
register-accurate. You don’t have to simulate the control signals. So, you can
simply pass the instruction from one pipeline latch to the nxt.*/


int main (int argc, char *argv[]) {
    int sim_mode = 0;//mode flag, 1 for single-cycle, 0 for batch
    int i;//for loop counter
    long mips_reg[REG_NUM];
    long sim_cycle = 1;//simulation cycle counter
    //define your own counter for the usage of each pipeline stage here

    int test_counter = 0;
    printf("The arguments are:");

    for (i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    if (argc == 7) {
        if (strcmp("-s", argv[1]) == 0) {
            sim_mode = SINGLE;
        } else if (strcmp("-b", argv[1]) == 0) {
            sim_mode = BATCH;
        } else {
            printf("Wrong sim mode chosen\n");
            exit(0);
        }

        m = atoi(argv[2]);
        n = atoi(argv[3]);
        c = atoi(argv[4]);
        input = fopen(argv[5], "r");
        output = fopen(argv[6], "w");

    } else {
        printf("Usage: ./sim-mips -s m n c input_name output_name (single-sysle mode)\n or \n ./sim-mips -b m n c input_name  output_name(batch mode)\n");
        printf("m,n,c stand for number of cycles needed by multiplication, other operation, and memory access, respectively\n");
        exit(0);
    }
    if (input == NULL) {
        printf("Unable to open input or output file\n");
        exit(0);
    }
    if (output == NULL) {
        printf("Cannot create output file\n");
        exit(0);
    }
    //initialize registers and program counter
    if (sim_mode == 1) {
        for (i = 0; i < REG_NUM; i++) {
            mips_reg[i] = 0;
        }
    }
    char *line = malloc(sizeof(char) * 100);//temp array for holding the raw input of the text file
    //char line[100];//array of chars that will hold the string input from file
    char *command;//pointer to char string with the final command with registers converted to numbers
    while (fgets(line, 100, input)) {//keep getting lines from input file
        iM[linecount] = parser(regNumberConverter(progScanner(line)));// store the completed instruction into I
        linecount++;//increment the linecounter for the IM
    }//end the while statement that fills IM

    ///////////////////////////////////////////
    //Initializing the pipelined datapath, setting all stages to nops. add 0 0 0 is considered nop
    //since the nop command is not native to the MIPS ISA

    /*
    struct Inst *nop = malloc(sizeof(struct Inst));//defining a nop command
    nop->opcode=add;
    nop->rd=0;
    nop->imm=0;
    nop->rs=0;
    nop->rt=0;
*/
    struct IFLatchID *state1 = malloc(sizeof(struct IFLatchID));
    state1->inst.opcode=add;
    state1->inst.rd=0;
    state1->inst.rs=0;
    state1->inst.rt=0;
    state1->inst.imm=0;
    state1->cycles=0;

    struct IDLatchEX *state2 = malloc(sizeof(struct IDLatchEX));
    state2->opcode=add;
    state2->reg1=0;
    state2->reg2=0;
    state2->regResult=0;
    state2->cycles=0;
    state2->immediate=0;

    struct EXLatchM *state3 = malloc(sizeof(struct EXLatchM));
    state3->opcode=add;
    state3->result=0;
    state3->regResult=0;
    state3->cycles=0;
    state3->reg2=0;

    struct MLatchWB *state4 = malloc(sizeof(struct MLatchWB));
    state4->opcode=add;
    state4->regResult=0;
    state4->result=0;

    int inc;
    for(inc=0;inc<32;inc++) {//initialize all registers to 0.
        registers[inc].value=0;
        registers[inc].flag=true;
    }
    //some temporary states, for when stages are waiting for other stages to finish
    struct MLatchWB *stateT4 = malloc(sizeof(struct MLatchWB));
    struct EXLatchM *stateT3 = malloc(sizeof(struct EXLatchM));
    struct IDLatchEX *stateT2 = malloc(sizeof(struct IDLatchEX));
    struct IFLatchID *stateT1 = malloc(sizeof(struct IFLatchID));
    /////////////////////////////////////////////////////
    ////This will be main loop that executes program/////
    /////////////////////////////////////////////////////
    while(true){

        state4->cycles=sim_cycle;
        if(state4->done==false) WB(state4);//Do write back first, state4 is MLatchWB
        //if(state4->done==true) wbUtil=+1;//wb only ever takes 1 cycle

        state3->cycles=sim_cycle;
        if(state3->done==false) MEM(state3,stateT4);//then mem, state3 is EXLatchM, state4 is MLatchWB
        //memUtil=+state4->cycles;

        state2->cycles=sim_cycle;
        if(state2->done==false) EX(state2,stateT3);//then execute, state2 is IDLatchEX, state3 is EXLatchM
        // exUtil=+state3->cycles;

        if((stateT3->result==0)&(stateT3->opcode==beq)){//when there is a beq, and result is 0, then do the branch
            //pgm_c=+(4+state2->immediate); //pc = pc + 4 + immediate
        }
        else {//when there is no branch to take/ no beq detected
            //pgm_c=+4;//increment the PC normally
            state1->cycles=sim_cycle;
            if(state1->done==false){
                ID(state1,stateT2);
                idUtil=+state2->cycles;
            }//then ID, state1 is IFLatchID, state 2 is IDLatchEX

            if(stateT2->opcode==beq){//if there is a branch detected in the ID stage, NOP until resolved
                //state1->inst=*nop;
                stateT1->inst.opcode=add;
                stateT1->inst.rd=0;
                stateT1->inst.rs=0;
                stateT1->inst.rt=0;
                stateT1->inst.imm=0;
                stateT1->cycles=sim_cycle;
                stateT1->done=true;
            }
            else {
                stateT1->cycles=sim_cycle;
                if(stateT1->done==false)IF(stateT1);//finally IF
            }
        }//when branch not taken, or no branch at all, pgm_c is just incremeted by 4
        //if the EX stage says we have a branch that we need to take
        if((stateT3->result==0)&&(stateT3->opcode==beq)&&(state2->done==true)&&(state3->done==true)&&(state4->done==true)){

        }
            //when all states done
        else if((stateT1->done==true)&&(state2->done==true)&&(state3->done==true)&&(state4->done==true)){
            stateT1->done=false;
            state2->done=false;
            state3->done=false;
            state4->done=false;
            state1=stateT1;
            state2=stateT2;
            state3=stateT3;
            state4=stateT4;//update the states to the temporary ones
            pgm_c=+4;

            if(sim_mode==1){
                printf("cycle: %d register value: ",sim_cycle);
                for (i=1;i<REG_NUM;i++){
                    printf("%d  ",registers[i].value);
                }
                printf("program counter: %d\n",pgm_c);
                printf("press ENTER to continue\n");
                while(getchar() != '\n');
            }
        }
        if(state4->opcode==haltSimulation)break;
        sim_cycle+=1;//increment cycle count
    }
    /*
    //output code 2: the following code will output the register
    //value to screen at every cycle and wait for the ENTER key
    //to be pressed; this will make it proceed to the next cycle

    if(sim_mode==1){
        printf("cycle: %ld ",sim_cycle);
        for (i=1;i<REG_NUM;i++){
            printf("%ld  ",mips_reg[i]);
        }
    }
    printf("%ld\n",pgm_c);
    pgm_c+=4;
    sim_cycle+=1;
    test_counter++;
    printf("press ENTER to continue\n");
    while(getchar() != '\n');
*/
    ////////////////////////////////////////////
    if(sim_mode==0){
        ifUtil=ifUtil/sim_cycle;
        idUtil=idUtil/sim_cycle;
        exUtil=exUtil/sim_cycle;
        memUtil=memUtil/sim_cycle;
        wbUtil=wbUtil/sim_cycle;
        fprintf(output,"program name: %s\n",argv[5]);
        fprintf(output,"stage utilization: %f  %f  %f  %f  %f \n",
                ifUtil, idUtil, exUtil, memUtil, wbUtil);

        fprintf(output,"register values ");
        for (i=1;i<REG_NUM;i++){
            fprintf(output,"%ld  ",registers[i].value);
        }
        fprintf(output,"%ld\n",pgm_c);

        //PRINT TO CONSOLE
        printf("program name: %s\n",argv[5]);
        printf("stage utilization: %f  %f  %f  %f  %f \n",
               ifUtil, idUtil, exUtil, memUtil, wbUtil);

        printf("register values ");
        for (i=1;i<REG_NUM;i++){
            printf("%ld  ",registers[i].value);
        }
        printf("%ld\n",pgm_c);

    }
    //close input and output files at the end of the simulation
    printf("FINISHED SIMULTATION\n");
    printf("press ENTER to continue\n");
    while(getchar() != '\n');
    fclose(input);
    fclose(output);
    return 0;
}







