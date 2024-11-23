#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    if (ALUControl == 0) {  
        *ALUresult = A + B;
    }
    else if (ALUControl == 1) { 
        *ALUresult = A - B;
    }
    else if (ALUControl == 2) {  
        *ALUresult = (A < B) ? 1 : 0;
    }
    else if (ALUControl == 3) {  
        *ALUresult = ((unsigned)A < (unsigned)B) ? 1 : 0;
    }
    else if (ALUControl == 4) {  
        *ALUresult = A & B;
    }
    else if (ALUControl == 5) {  
        *ALUresult = A | B;
    }
    else if (ALUControl == 6) {  
        *ALUresult = B << 16;
    }
    else if (ALUControl == 7) {  
        *ALUresult = ~A;
    }
    if (*ALUresult == 0) {
        *Zero = 1;
    } 
    else {
        *Zero = 0;
    }
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    if (PC % 4 != 0){ // word alignment check
        return 1; //halt
    }

    *instruction = Mem[PC >> 2]; // mem not an array of words but bytes

    return 
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    // bits 31-26
    *op = (instruction >> 26) & 0x3F; 

    // bits 25-21
    *r1 = (instruction >> 21) & 0x1F;  

    // bits 20-16
    *r2 = (instruction >> 16) & 0x1F; 

    // bits 15-11
    *r3 = (instruction >> 11) & 0x1F;  

    // bits 5-0
    *funct = instruction & 0x3F;  

    // bits 15-0
    *offset = instruction & 0xFFFF;  

    // jump address, regardless of instruction type (bits 25-0)
    *jsec = instruction & 0x03FFFFFF;  
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemWrite = 0;
    controls->RegWrite = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 0;
    controls->ALUSrc = 0;

    
    if (op == 0) {  
        controls->RegDst = 1;       
        controls->RegWrite = 1;      
        controls->ALUOp = 7;         
    }
    else if (op == 2) {  
        controls->Jump = 1;         
    }
    else if (op == 4) {  
        controls->Branch = 1;      
        controls->ALUOp = 1;        
    }
    else if (op == 35) {  
        controls->MemRead = 1;     
        controls->RegWrite = 1;    
        controls->MemtoReg = 1;     
        controls->ALUSrc = 1;       
        controls->ALUOp = 0;       
    }
    else if (op == 43) {  
        controls->MemWrite = 1;     
        controls->ALUSrc = 1;      
        controls->ALUOp = 0; 
    }    
    else if (op == 8) {  
        controls->RegDst = 0;       
        controls->RegWrite = 1;     
        controls->ALUSrc = 1;       
        controls->ALUOp = 0;      
    }
    else {
        
        return 1;  
    }

    return 0;

}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    //it basically "reads" the register (which is the number thats in r1/r2's
    //position in the reg array), and copies it into data 1 and 2. "reading" it.)
    *data1 = Reg[r1]; 
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    if (offset >> 1 == 1) {  // if negative

        *extended_value = offset | 0xFFFF0000;  // fill with 1s
    } else {

        *extended_value = offset & 0x0000ffff;  // fill with 0s
    }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    if (ALUSrc == 1) {
        ALU(data1, extended_value, ALUOp, ALUresult, Zero); 
    } else {
        ALU(data1, data2, ALUOp, ALUresult, Zero);  
    }
    return 0;

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    //Halt condition: Memory is out of range
    if (ALUresult > 0xFFFF)
        return 1;
    //Halt condition: Address not word-aligned
    if (ALUresult % 4 != 0)
        return 1;

    if (MemWrite == 1) 
        Mem[ALUresult >> 2] = data2;    //This writes data2 to memory

    if (MemRead == 1)
        *memdata = Mem[ALUresult >> 2];  //Reads data from memory

    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    //We bring data from memory since RegWrite and MemtoReg are both 1 and assign this to r2
    if (RegWrite == 1 && MemtoReg == 1) 
        Reg[r2] = memdata;

    //We bring data from ALUresult since MemtoReg is not 1 and we assign this to r3
    if (RegWrite == 1 && MemtoReg == 0)
        Reg[r3] = ALUresult;
    
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC = *PC + 4; //we always add four regardless of instruction

    if (Jump == 1){//check if we need to jump
        *PC = (jsec << 2) | (*PC & 0xf0000000); //combines both jsec (address to jump to, shifted 2 bits to the left) with the upper four bits of PC
    }

    if (Branch == 1){ //if branch is true
        if (Zero == 1){ //if zero is true
            *PC = *PC + (extended_value << 2); //add pc to extended value and shift 2 so it goes from words to bits
        }
        
    }

}

