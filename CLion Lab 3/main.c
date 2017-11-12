// List the full names of ALL group members at the top of your code.
//Logan Emerson, Aaron Linz, Matt Delude

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
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
long pgm_c = 0;//program counter
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
    char out[100];//what is to be passed down to next function, output
    int i;//incrementer for loop below
    int oc = 0;//counter for out char array
    int space = 0; //for counting more than 1 consecutive space
    int comma = 0; //for counting more than 1 consecutive comma
    //int leftp=0; //counter for # of left parentheses
    int paren = 0;//count for # of parentheses

    for (i = 0; i < 100; i++) {//need to search through the array and parse it correctly
        //we should only ever encounter 1 set of parentheses
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

        if (line[i] == 0x20 || line[i] == 0x2C) {//when it detects a space or comma
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
        if ((i - 1 - space) > 1) {//when checking for   , ,  need i to be greater than 2
            if ((line[i] == 0x2C) & (line[i - space] == 0x20) & (line[i - 1 - space] == 0x2C)) {
                printf("Syntax error detected: ', ,' %d: %s:", linecount, line);
                fprintf(output, "Syntax error detected: ', ,' %d: %s:", linecount, line);
                exit(0);//since, error, exit the program
            }
        }//end the if statement looking for , ,

            //else if(line[i]==0x2C) {}//when we detect a comma, do nothing
        else {//when we read anything but a comma, space, or parentheses
            out[oc] = line[i];
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
    char* ret="";//return string
    char* substr="";//first char after $
    strncpy(substr, reg + 1, 1);
    char* substr2="";//second char after $
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

char *regNumberConverter(char* input) {
    char *token[6];
    char *ret = "";
    char *substr;
    token[0] = strtok(input, " ");
    int i = 0;
    while (token[i] != NULL) {//fills token array with the words in the line
        token[i++] = strtok(NULL, " ");
    }
    for (int j = 0; j < i; j++) {
        strncpy(substr, token[i], 1);//stores the first char of the token in substr
        if (!strcmp(substr, "$")) {
            strcat(" ", ret);
            strcat(getRegNum(token[i]), ret);
        } else {
            strcat(" ", ret);
            strcat(token[i], ret);
        }
    }
    return ret;
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
    char* token[6];
    token[0]=strtok(input," ");
    int i=0;
    while(token[i]!=NULL){//fills token array with the words in the line
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
    if (!strcmp(token[0], "haltSimulation")) {
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
    //if(inLatch->inst.opcode==nop)
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
   if(((inLatch->cycles%n==0)&(inLatch->opcode!=mul))||((inLatch->cycles==m)&(inLatch->opcode==mul))) {
       inLatch->done=true;//we have reached the needed # of cycles to complete the EX stage of datapath
       switch (inLatch->opcode) {
           case beq:
               outLatch->opcode = inLatch->opcode;
               outLatch->reg2 = inLatch->reg2;
               outLatch->regResult = inLatch->regResult;
               outLatch->result = registers[inLatch->reg1].value - registers[inLatch->reg2].value;
               outLatch->cycles = inLatch->cycles;

           case add:
               outLatch->opcode = inLatch->opcode;
               outLatch->reg2 = inLatch->reg2;
               outLatch->regResult = inLatch->regResult;
               outLatch->result = registers[inLatch->reg1].value + registers[inLatch->reg2].value;
               outLatch->cycles = inLatch->cycles;
           case sub:
               outLatch->opcode = inLatch->opcode;
               outLatch->reg2 = inLatch->reg2;
               outLatch->regResult = inLatch->regResult;
               outLatch->result = registers[inLatch->reg1].value - registers[inLatch->reg2].value;
               outLatch->cycles = inLatch->cycles;
           case mul:
               outLatch->opcode = inLatch->opcode;
               outLatch->reg2 = inLatch->reg2;
               outLatch->regResult = inLatch->regResult;
               outLatch->result = registers[inLatch->reg1].value * registers[inLatch->reg2].value;
               outLatch->cycles = inLatch->cycles;
           case addi:
               outLatch->opcode = inLatch->opcode;
               outLatch->reg2 = inLatch->reg2;
               outLatch->regResult = inLatch->regResult;
               outLatch->result = registers[inLatch->reg1].value +
                                  inLatch->immediate;//adds the immediate to reg 1 and puts in result
               outLatch->cycles = inLatch->cycles;
           case lw:
               outLatch->opcode = inLatch->opcode;
               outLatch->reg2 = inLatch->reg2;
               outLatch->regResult = inLatch->regResult + inLatch->immediate;
               outLatch->result = 0;//puts value from reg 1 into result
               outLatch->cycles = inLatch->cycles;
           default:
               outLatch->opcode = inLatch->opcode;
               outLatch->reg2 = inLatch->reg2;
               outLatch->regResult = inLatch->regResult;
               outLatch->result = 0;
               outLatch->cycles = inLatch->cycles;
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
    if (((inLatch->opcode == (lw || sw)) & inLatch->cycles == c) || (inLatch->opcode != (lw || sw))) {
        inLatch->done=true;
        switch (inLatch->opcode) {
            case add:
            case sub:
            case mul:
            case addi:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = inLatch->result;
            case lw:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = registers[inLatch->regResult].value;
            default:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = inLatch->result;
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
    long pgm_c = 0;//program counter, need assertion that PC < 512, max # of lines in IM
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
    //char *line = malloc(sizeof(char) * 100);//temp array for holding the raw input of the text file
    char line[100];//array of chars that will hold the string input from file
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
            if(state1->done==false)ID(state1,stateT2);//then ID, state1 is IFLatchID, state 2 is IDLatchEX
            idUtil=+state2->cycles;

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
        if((stateT3->result==0)&(stateT3->opcode==beq)&(state2->done==true)&(state2->done==true)&(state2->done==true)){

        }
        //when all states done
        else if((stateT1->done==true)&(state2->done==true)&(state3->done==true)&(state4->done==true)){
            state1=stateT1;
            state2=stateT2;
            state3=stateT3;
            state4=stateT4;//update the states to the temporary ones


        }



        sim_cycle+=1;//increment cycle count
        if(state4->opcode==haltSimulation)break;
    }


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

    ////////////////////////////////////////////
    if(sim_mode==0){
        fprintf(output,"program name: %s\n",argv[5]);
        fprintf(output,"stage utilization: %f  %f  %f  %f  %f \n",
                ifUtil, idUtil, exUtil, memUtil, wbUtil);
        // add the (double) stage_counter/sim_cycle for each
        // stage following sequence IF ID EX MEM WB

        fprintf(output,"register values ");
        for (i=1;i<REG_NUM;i++){
            fprintf(output,"%ld  ",mips_reg[i]);
        }
        fprintf(output,"%ld\n",pgm_c);

    }
    //close input and output files at the end of the simulation
    fclose(input);
    fclose(output);
    return 0;
}







