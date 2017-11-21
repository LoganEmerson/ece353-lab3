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
#define WORDMAX 512
////////////////////////////
/////ABOVE IS TA CODE///////
/////BELOW IS OUR CODE//////
////////////////////////////
int linecount = 0;//number of lines
long pgm_c = 0;//program counter, need assertion that PC < 512, max # of lines in IM
bool stall;
FILE *input;
FILE *output;

typedef enum {
    nop, add, sub, addi, mul, lw, sw, beq, haltSimulation
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
struct Inst iM[WORDMAX];
//Data memory
int dM[WORDMAX];
double ifUtil, idUtil,exUtil,memUtil,wbUtil;

int c, m, n;//made global so IF, ID, EX, MEM, and WB can access

char *progScanner(char line[]) {
    char temp;
    int i;//incrementer for loop below
    char out[100];//what is to be passed down to next function, output
    for(i=0;i<100;i++)  out[i]=' ';
    int oc = 0;//counter for out char array
    int space = 0; //for counting more than 1 consecutive space
    int paren = 0;//count for # of parentheses
    for (i = 0; i < 100; i++) {//need to search through the array and parse it correctly
        //we should only ever encounter 1 set of parentheses
        if(line[i]==0x0D) {
            out[oc]=NULL;
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
        else {//when we read anything but a comma, space, or parentheses
            temp = tolower(line[i]);
            out[oc] = temp;
            oc++;
            //paren = 0;//reset # of parentheses
            space = 0;//reset # of consecutive spaces
        }

    }
    char *outP = out;//transfers the character array out[] to a char pointer, outP
    return outP;
}

char* getRegNum(char* reg){//takes in a register in hte form of "$xx" and returns the equivalent register number
    char ret[2];//return string
    char substr=reg[1];//first char after $

    char *subP=&substr;
    char *subP2;
    char subC=reg[1];

    //strncpy(substr, reg + 1, 1);
    char substr2;//second char after $
    int regNum=0;
    bool isCharReg=false;//register starts off with a letter
    //if ((!strcmp(substr,"Z")) || (!strcmp(substr,"a")) || (!strcmp(substr,"v")) || (!strcmp(substr,"t")) || (!strcmp(substr,"s")) || (!strcmp(substr,"k")) || (!strcmp(substr,"g")) || (!strcmp(substr,"f")) || (!strcmp(substr,"r"))){
    if ((substr == 'z') || (substr == 'a') || (substr == 'v') || (substr == 't') || (substr == 's') || (substr == 'k') || (substr == 'g') || (substr == 'f') || (substr == 'r')){
        isCharReg=true;
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
            else if((!strcmp(subP2,"6")))
                return "14";
            else if((!strcmp(subP2,"8")))
                return "24";
            else if((!strcmp(subP2,"9")))
                return "25";
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
            strcat(ret,getRegNum(token[j]));
            strcat(ret, spa);
        } else {
            strcat(ret, tokenString);
            strcat(ret, spa);

        }
    }
    char *retu=&ret;
    return retu;
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
    return retVal;
}

void IF(struct IFLatchID *inLatch){
    if(inLatch->cycles%c==0){
        struct Inst instruction=iM[pgm_c / 4];
        inLatch->inst=instruction;
        inLatch->done=true;
        ifUtil=ifUtil+c;
    }
    else inLatch->done=false;
}

void ID(struct IFLatchID *inLatch,struct IDLatchEX *outLatch){
    idUtil++;
    //if(((registers[inLatch->inst.rt].flag) || (inLatch->inst.opcode==addi) || (inLatch->inst.opcode==lw) || (inLatch->inst.opcode==haltSimulation)) & (registers[inLatch->inst.rs].flag)){
    if(inLatch->inst.opcode==haltSimulation){
        outLatch->opcode = inLatch->inst.opcode;
        outLatch->regResult = inLatch->inst.rd;
        outLatch->reg1 = inLatch->inst.rs;
        outLatch->reg2 = inLatch->inst.rt;
    }
    else if((registers[inLatch->inst.rt].flag)/*&&(registers[inLatch->inst.rs].flag)&&(registers[inLatch->inst.rd].flag)*/){
        //if the flag value is true, the registers are good to go
        //this if runs if the flaggs are good, or it is an addi, halt, or lw
        outLatch->opcode = inLatch->inst.opcode;
        outLatch->immediate = inLatch->inst.imm;
        outLatch->reg1 = registers[inLatch->inst.rs].value;
        if((inLatch->inst.opcode==addi) || (inLatch->inst.opcode==lw) || (inLatch->inst.opcode==haltSimulation)) {
            outLatch->reg2 = 0;
        }
        else {
            outLatch->reg2=registers[inLatch->inst.rt].value;
        }
        switch (inLatch->inst.opcode) {
            /*
            case sub:
            case mul:
                outLatch->regResult = inLatch->inst.rd;
                registers[outLatch->regResult].flag = false;
                break;
                */
            case addi:
                outLatch->opcode = inLatch->inst.opcode;
                outLatch->immediate = inLatch->inst.imm;
                outLatch->reg1 = inLatch->inst.rs;
                outLatch->regResult = inLatch->inst.rt;
                //if(outLatch->regResult!=0) registers[outLatch->regResult].flag=false;
                break;
            case lw:
                outLatch->opcode = inLatch->inst.opcode;
                outLatch->immediate = inLatch->inst.imm;
                outLatch->reg1 = inLatch->inst.rs;
                outLatch->regResult = inLatch->inst.rt;
                //registers[outLatch->regResult].flag=false;
                break;
            case sw:
                outLatch->opcode = inLatch->inst.opcode;
                outLatch->immediate = inLatch->inst.imm;
                outLatch->reg1 = inLatch->inst.rs;
                outLatch->regResult = inLatch->inst.rt;
               // registers[outLatch->regResult].flag=false;
                break;

            case beq:
                outLatch->opcode = inLatch->inst.opcode;
                outLatch->reg1   = inLatch->inst.rs;
                outLatch->reg2   = inLatch->inst.rt;
                outLatch->immediate = inLatch->inst.imm;
                //Logan made some changes 11/12 1:52
            default:
                outLatch->opcode = inLatch->inst.opcode;
                outLatch->regResult = inLatch->inst.rd;
                outLatch->reg1 = inLatch->inst.rs;
                outLatch->reg2 = inLatch->inst.rt;
                //if(outLatch->regResult!=0) registers[outLatch->regResult].flag=false;
                //registers[outLatch->regResult].flag=false;
                break;
        }
        if(outLatch->regResult!=0) registers[outLatch->regResult].flag=false;
    }
    else{//NOP
        outLatch->opcode = add;
        outLatch->reg1 = 0;
        outLatch->reg2 = 0;
        outLatch->regResult = 0;
        outLatch->immediate = 0;
        outLatch->cycles = 0;
        //stall=true;

    }
    return;
}

void EX(struct IDLatchEX *inLatch,struct EXLatchM *outLatch){
    if((inLatch->reg1==0)&&(inLatch->reg2==0)&&(inLatch->regResult==0)&&((inLatch->opcode==add)||(inLatch->opcode==nop))){//nop
        outLatch->opcode = nop;
        outLatch->reg2 = inLatch->reg2;
        outLatch->regResult = inLatch->regResult;
        outLatch->result = registers[inLatch->reg1].value + registers[inLatch->reg2].value;
        outLatch->cycles = inLatch->cycles;
        inLatch->done=true;
        }
    else if((((inLatch->cycles%n)==0)&&(inLatch->opcode!=mul))||(((inLatch->cycles%m)==0)&&(inLatch->opcode==mul))) {
        inLatch->done=true;//we have reached the needed # of cycles to complete the EX stage of datapath
        if(inLatch->opcode==mul)exUtil=+m;
        else if(inLatch->opcode!=mul)exUtil=exUtil+n;
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
                //outLatch->reg2 = inLatch->reg2;
                outLatch->result = registers[inLatch->reg1].value + inLatch->immediate;
                outLatch->regResult = inLatch->regResult;//puts value from reg 1 into result
                outLatch->cycles = inLatch->cycles;
                break;

            case sw:
                outLatch->opcode = inLatch->opcode;
                outLatch->result = registers[inLatch->reg1].value + inLatch->immediate;
                outLatch->regResult = inLatch->regResult;//puts value from reg 1 into result
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
    if (((inLatch->opcode ==lw)&& inLatch->cycles == c) || (inLatch->opcode != lw)) {
        if(inLatch->opcode == lw){memUtil=+c;}
        else if(inLatch->opcode == sw){memUtil=+c;}
        inLatch->done=true;
        switch (inLatch->opcode) {
            /*
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
                */
            case lw:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                outLatch->result = dM[inLatch->result];
                break;
            case sw:
                outLatch->opcode = inLatch->opcode;
                outLatch->regResult = inLatch->regResult;
                dM[inLatch->result]=registers[inLatch->regResult].value;
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
    switch (inLatch->opcode){
        case beq:
            break;

        case sw:
            break;

        case nop:
            break;

        default:
            registers[inLatch->regResult].value=inLatch->result;
            registers[inLatch->regResult].flag= true;
            wbUtil++;
            break;
    }
    inLatch->done=true;
    return;
}
int main (int argc, char *argv[]) {
    stall=false;
    int sim_mode = 0;//mode flag, 1 for single-cycle, 0 for batch
    int i;//for loop counter
    //long mips_reg[REG_NUM];
    long sim_cycle = 1;//simulation cycle counter
    //define your own counter for the usage of each pipeline stage here
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

    char *line = malloc(sizeof(char) * 100);//temp array for holding the raw input of the text file
    char tline[100];//array of chars that will hold the string input from file
    while (fgets(line, 100, input)) {//keep getting lines from input file
        strcpy(tline, line);
        if(tline[0]!=0x0D) {
            iM[linecount] = parser(regNumberConverter(progScanner(line)));// store the completed instruction into I
            linecount++;//increment the linecounter for the IM
        }
        }//end the while statement that fills IM

    ///////////////////////////////////////////
    //Initializing the pipelined datapath, setting all stages to nops. add 0 0 0 is considered nop
    //since the nop command is not native to the MIPS ISA
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
    state3->opcode=nop;
    state3->result=0;
    state3->regResult=0;
    state3->cycles=0;
    state3->reg2=0;

    struct MLatchWB *state4 = malloc(sizeof(struct MLatchWB));
    state4->opcode=nop;
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
    stall=false;
    while(true){
        assert(registers[0].value==0);//zero register should always =0
        registers[0].flag=true;
        assert(pgm_c<MEM_SIZE);//program counter should never exceed the memory size
        //if(registers[state4->regResult].flag==false) stall = false;
        state4->cycles++;
        if(state4->done==false) WB(state4);//Do write back first, state4 is MLatchWB

        state3->cycles++;
        if(state3->done==false) MEM(state3,stateT4);//then mem, state3 is EXLatchM, state4 is MLatchWB

        state2->cycles++;
        if(state2->done==false) EX(state2,stateT3);//then execute, state2 is IDLatchEX, state3 is EXLatchM

        //When we need to branch
        if((stateT3->result==0)&&(stateT3->opcode==beq)&&(state2->done==true)&&(state3->done==true)&&(state4->done==true)){
            pgm_c = pgm_c +4+ 4*(state2->immediate);
            //stall = false;//stop stalling
            state1->cycles++;
            ID(state1, stateT2);//decode the NOP

            stateT1->cycles++;
            IF(stateT1);//IF new pgm_C
        }
        else {//when there is no branch to take
            state1->cycles++;
            if(state1->done==false){
                ID(state1,stateT2);
            }//then ID, state1 is IFLatchID, state 2 is IDLatchEX

            if((stateT2->opcode==beq)){//if there is a branch detected in the ID stage, NOP until resolved
                stall = true;
                if((state2->done==true)&&(state3->done==true)&&(state4->done==true)){
                    state2->done=false;
                    state3->done=false;
                    state4->done=false;
                    state2->cycles=0;
                    state3->cycles=0;
                    state4->cycles=0;
                    state2=stateT2;
                    state3=stateT3;
                    state4=stateT4;//update the states to the temporary ones
                    state1->inst.opcode=nop;
                    state1->inst.rd=0;
                    state1->inst.rt=0;
                    state1->inst.rs=0;
                    state1->done=true;
                    state1->inst.imm=0;

                }
            }
            else if(stall==false){
                stateT1->cycles++;
                if(stateT1->done==false)IF(stateT1);//finally IF
            }


        }//when branch not taken, or no branch at all, pgm_c is just incremeted by 4
            //when all states done

        if((stateT1->done==true)&&(state2->done==true)&&(state3->done==true)&&(state4->done==true)){//all stages done
            stateT1->done=false;
            state2->done=false;
            state3->done=false;
            state4->done=false;
            stateT1->cycles=0;
            state2->cycles=0;
            state3->cycles=0;
            state4->cycles=0;
            state1=stateT1;
            state2=stateT2;
            state3=stateT3;
            state4=stateT4;//update the states to the temporary ones

            if(registers[state1->inst.rt].flag==false||registers[state1->inst.rs].flag==false) {//detecing RAW data haz
                if((state1->inst.rt==state2->regResult)||(state1->inst.rd==state2->regResult)){
                state1->inst.opcode=nop;
                state1->inst.rd=0;
                state1->inst.rt=0;
                state1->inst.rs=0;
                state1->done=true;
                state1->inst.imm=0;
                //make a nop and do not incremenet PC
            }}
            else {
                if (stall == false) pgm_c = pgm_c + 4;
                stall = false;
            }
        }
        if(sim_mode==1){
            printf("cycle: %d register value: ",sim_cycle);
            for (i=1;i<REG_NUM;i++){
                printf("%d  ",registers[i].value);
            }
            printf("program counter: %d\n",pgm_c);
            printf("press ENTER to continue\n");
            while(getchar() != '\n');
        }
        if(state4->opcode==haltSimulation)break;
        sim_cycle+=1;//increment cycle count
    }

    ////////////////////////////////////////////
    ifUtil=ifUtil/sim_cycle;
    idUtil=idUtil/sim_cycle;
    exUtil=exUtil/sim_cycle;
    memUtil=memUtil/sim_cycle;
    wbUtil=wbUtil/sim_cycle;
    if(sim_mode==0){
        fprintf(output,"program name: %s\n",argv[5]);
        fprintf(output,"stage utilization: %f  %f  %f  %f  %f \n",
                ifUtil, idUtil, exUtil, memUtil, wbUtil);

        fprintf(output,"register values ");
        for (i=1;i<REG_NUM;i++){
            fprintf(output,"%ld  ",registers[i].value);
        }
        fprintf(output,"%ld\n",pgm_c);
    }

    //PRINT TO CONSOLE
    printf("program name: %s\n",argv[5]);
    printf("stage utilization: %f  %f  %f  %f  %f \n",
           ifUtil, idUtil, exUtil, memUtil, wbUtil);

    printf("register values ");
    for (i=1;i<REG_NUM;i++){
        printf("%ld  ",registers[i].value);
    }
    printf("%ld\n",pgm_c);
    //close input and output files at the end of the simulation
    printf("FINISHED SIMULTATION\n");
    printf("press ENTER to continue\n");
    while(getchar() != '\n');
    fclose(input);
    fclose(output);
    return 0;
}







