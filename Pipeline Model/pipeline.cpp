//
//  pipeline.cpp
//  Pipe
//
//  Created by Sharmi on 5/13/18.
//  Copyright © 2018 Sharmi. All rights reserved.
//

#include<iostream>
#include<fstream>
#include<cmath>
#include<iomanip>

using namespace std;

#define LOAD    0   //'b0000000
#define STORE   32  //'b0100000
#define BRANCH  96  //'b1100000
#define JALR    100 //'b1100100
#define J       104 //'b1101000
#define JAL     108 //'b1101100
#define OP_IMM  16  //'b0010000
#define OP      48  //'b0110000
#define SYSTEM1 112 //'b1110000
#define SYSTEM2 116 //'b1110100
#define AUIPC   18  //'b0010100
#define LUI     52  //'b0110100
#define ALU_IMM 24  //'b0011000
#define ALU_OP  56  //'b0111000
#define UNKNOWN 128 //'b1111111


//Given an array pointer and the number of elements to take into consideration,
//returns the max value.
int max(int a[], int size) {
  int max = a[0];
  for(int i=1; i<size; i++) {
    if(a[i] > max) {
      max = a[i];
    }
  }//for
  return max;
}

int main(int argc, const char * argv[]) {
    //Variables to fetch 32bit addr and data from the addr and data trace files. 
    int addr=0;
    int data=0;
    long data_long=0;
    int first_run = 0;
    
    //Width variables
    int OPCODE_WIDTH=7;
    int RD_WIDTH=5;
    int RS1_WIDTH=5;
    int RS2_WIDTH=5;
    int FUNCT3_WIDTH=3;
    int FUNCT7_WIDTH = 7;
    int IMMEDIATE_WIDTH_I = 12;
    int IMMEDIATE_WIDTH_S1 = 5;
    int IMMEDIATE_WIDTH_S2 = 7;
    int IMMEDIATE_WIDTH_U = 20;

    //Mask variables
    int opcode_mask=0;
    int rd_mask=0;
    int funct3_mask=0;
    int rs1_mask=0;
    int rs2_mask=0;
    int funct7_mask =0;
    int immediate_mask_i = 0;
    int immediate_mask_s1=0;
    int immediate_mask_s2 = 0;
    int immediate_mask_u =0;
    int opcode=0;

    //Instruction Fields
    int op  = 0;
    int rs1 = 0;
    int rs2 = 0;
    int rd  = 0;
    int taken_branch = 0;
    int clock_cycle = 0;
    int IF_clock_cycle = 0;
    int ID_clock_cycle = 0;
    int EX_clock_cycle = 0;
    int MEM_clock_cycle = 0;
    int WB_clock_cycle = 0;
    int prev_IF_clock_cycle = 0;
    int prev_ID_clock_cycle = 0;
    int prev_EX_clock_cycle = 0;
    int prev_MEM_clock_cycle = 0;
    int prev_WB_clock_cycle = 0i;
    int max_arr[] = {0,0,0,0};
    int rs1_stall = 0;
    int rs2_stall = 0;

    //Statistical Data
    int num_data_cache_load_acc = 0;
    int num_data_cache_store_acc = 0;
    int num_instr_cache_acc = 0;
    int num_instr_cache_hits = 0;
    int num_data_cache_hits = 0;
    int num_alu_instr = 0;
    int num_branch_instr = 0;
    int num_load_instr = 0;
    int num_store_instr = 0;
    int num_all_other_instr = 0; //There arent anyother instructions. CHECK!!!
    int num_taken_branches = 0;
    int num_fwd_from_EX_stage = 0;
    int num_fwd_from_MEM_stage = 0;
    int num_cycles_stalled_in_ID_stage = 0;
    int total_num_of_cycles = 0;

    //Calculations
    double hit_ratio = 0;
    double data_hit_ratio= 0;
    double ipc=0;
    double branchper = 0;
    double Alufre = 0;
    double loadfre = 0;
    double storefre = 0;
    double branchfre=0;
    double stallper=0;

    //Scoreboard
    //Register 0 to 31.
    int scbd[32][4];
    int scbd_br[2] = {0,0};

    //Open files for I/O
    cout << "Opening the addr and data trace files to read and opening the decoded_trace file for intermediate output." << endl << endl;
    std::fstream myaddrfile("inst_addr_riscv_trace_project_2.txt",std::ios_base::in);
    std::fstream mydatafile("inst_data_riscv_trace_project_2.txt",std::ios_base::in);
    std::fstream mydecoded_out("decoded_trace.txt", std::ios_base::out);
    std::fstream mydebug_out("debug_trace.txt", std::ios_base::out);
    
    cout << "Calculating the mask for the fields in any supported instruction.." << endl;
    opcode_mask=(pow(2,OPCODE_WIDTH)-1);
    rd_mask=(pow(2,RD_WIDTH)-1);
    funct3_mask=(pow(2,FUNCT3_WIDTH)-1);
    rs1_mask=(pow(2,RS1_WIDTH)-1);
    rs2_mask=(pow(2,RS2_WIDTH)-1);
    funct7_mask=(pow(2,FUNCT7_WIDTH)-1);
    immediate_mask_i=(pow(2,IMMEDIATE_WIDTH_I)-1);
    immediate_mask_s1=(pow(2,IMMEDIATE_WIDTH_S1)-1);
    immediate_mask_s2=(pow(2,IMMEDIATE_WIDTH_S2)-1);
    immediate_mask_u=(pow(2,IMMEDIATE_WIDTH_U)-1);
    
    mydebug_out<<"Opcode mask:\t" << hex << opcode_mask << endl;
    mydebug_out<<"Funct3 mask:\t" << hex << funct3_mask << endl;
    mydebug_out<<"Funct7 mask:\t" << hex << funct7_mask << endl;
    mydebug_out<<"Destination register mask(Rd):\t" << hex << rd_mask << endl;
    mydebug_out<<"Source reister mask(Rs1):\t" << hex << rs1_mask << endl;
    mydebug_out<<"Source register mask(Rs2):\t" << hex << rs2_mask << endl;
    mydebug_out<<"Immediate field mask(I-Type):\t" << hex << immediate_mask_i << endl;
    mydebug_out<<"Immediate field mask(S-Type, imm[4:0], data[11:7]):\t" << hex << immediate_mask_s1 << endl;
    mydebug_out<<"Immediate field mask(S-Type, imm[11:5], data[31:25]):\t" << hex << immediate_mask_s2 << endl;
    mydebug_out<<"Immediate field mask(U-Type, imm[31:12]):\t" << hex << immediate_mask_u << endl << endl;
        
    //Initialize Scoreboard
    cout << "Initializing 32x4 Scoreboard array.\n" << endl;
    for(int i=0; i<32; i++) {
      for(int j=0; j<4; j++) {
        scbd[i][j] = 0;
      }
    }
    
    //Header file for decoded_trace file
    mydecoded_out << "ADDR\t\t" << "DATA\t\t" << "OP(RAW)\t" << "TYPE\t\t" << "OPCODE\t" << "Rd\t" << "Rs1\t" << "Rs2\t" << "BRANCH" << endl;
    
    int prev_addr = 0x0;
    while (mydatafile >> std::hex >> data_long && myaddrfile >> std::hex >> addr)
    {
        //Initialize the prev_addr to current addr for the first run.
        //There is no prev addr for the first instruction.
        if (first_run ==0) {
          prev_addr = addr - 0x4;
          first_run = 1;
        }
        //Convert long to integer. Need 64 bits to read as each character is considered a byte and the data is 8 characters long.
        data = int(data_long);
        opcode = data & opcode_mask & 0xfffffffc;//Getting rid of bits 1 and 0 in 6:0 of opcode.
        //cout << "OPCODE: " << opcode << endl;
	mydecoded_out << setfill('0') << setw(8) << hex << addr<<"\t" << setfill('0') << setw(8) << hex << data << "\t" << hex << opcode;
        
        //Decode the data to find out the OPCODE
        if(opcode == 0b0000000) {//--------------------------------------------------------------//inst[6:2] = 00_000
          mydecoded_out<<"\tLOAD\t-";
          op  = LOAD;
          rd  = (data >> OPCODE_WIDTH) & rd_mask;
          rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
          rs2 = 256; //Non-existent
          num_load_instr = num_load_instr + 1;
          mydecoded_out << "\t" << hex << op << "\t" << dec << rd << "\t" << rs1 << "\t" << "-";
        } else if(opcode == 0b0100000) {//-------------------------------------------------------//inst[6:2] = 01_000
          mydecoded_out<<"\tSTORE\t-";
          op  = STORE;
          rd = 256; //Non-existent
          rs1 = (data >> (OPCODE_WIDTH + IMMEDIATE_WIDTH_S1 + FUNCT3_WIDTH)) & rs1_mask;
          rs2 = (data >> (OPCODE_WIDTH + IMMEDIATE_WIDTH_S1 + FUNCT3_WIDTH + RS1_WIDTH)) & rs2_mask;
          num_store_instr = num_store_instr + 1;
          mydecoded_out << "\t" << hex << op << "\t" << "-" << "\t" << dec <<  rs1 << "\t" << rs2;
        } else if((opcode & 0b1010000) == 0b0010000) {//-----------------------------------------//inst[6:2] = 0?_10?
          mydecoded_out<<"\tALU";
          if(opcode == 0b0010000) {
            mydecoded_out<<"\tOP-IMM";
            op  = OP_IMM;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
            rs2 = 256; //Non-existent
            num_alu_instr = num_alu_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << rs1 << "\t" << "-";
          } else if(opcode == 0b0010100) {
            mydecoded_out<<"\tAUIPC";
            op = AUIPC;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = 256; //Non-existent
            rs2 = 256; //Non-existent
            num_alu_instr = num_alu_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << "-"<< "\t" << "-";
          } else if(opcode == 0b0110000) {
            mydecoded_out<<"\tOP";
            op  = OP;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
            rs2 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH + RS1_WIDTH)) & rs2_mask;
            num_alu_instr = num_alu_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << rs1 << "\t" << rs2;
          } else if(opcode == 0b0110100) {
            mydecoded_out<<"\tLUI";
            op  = LUI;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = 256; //Non-existent
            rs2 = 256; //Non-existent
            num_alu_instr = num_alu_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << "-"  << "\t" << "-";
          } else if(opcode == 0b0011000) {
            mydecoded_out<<"\tALU-IMM";
            op  = ALU_IMM;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
            rs2 = 256; //Non-existent
            num_alu_instr = num_alu_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << rs1 << "\t" << "-";
          } else if(opcode == 0b0111000) {
            mydecoded_out<<"\tALU-OP";
            op  = ALU_OP;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
            rs2 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH + RS1_WIDTH)) & rs2_mask;
            num_alu_instr = num_alu_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << rs1 << "\t" << rs2;
          } else {//--------------------------------------------//Not a known ALU instruction
            mydecoded_out<<"\tUNKNOWN";
            rs1 = 256; //Non-existent
            rs2 = 256; //Non-existent
          }
        } else if(((opcode & 0b1110000) == 0b1100000) || ((opcode & 0b1111000) == 0b1110000)){//-//inst[6:2] = 11_0?? or 11_10?
          mydecoded_out<<"\tBRANCH";
          if(opcode == 0b1100000) {
            mydecoded_out<<"\tBRANCH";
            op  = BRANCH;
            rd  = 256; //Non-existent
            rs1 = (data >> (OPCODE_WIDTH + IMMEDIATE_WIDTH_S1 + FUNCT3_WIDTH)) & rs1_mask;
            rs2 = (data >> (OPCODE_WIDTH + IMMEDIATE_WIDTH_S1 + FUNCT3_WIDTH + RS1_WIDTH)) & rs2_mask;
            num_branch_instr = num_branch_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << "-" << "\t" << dec <<  rs1 << "\t" << rs2;
          } else if (opcode == 0b1100100) {
            mydecoded_out<<"\tJALR";
            op  = JALR;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
            rs2 = 256; //Non-existent
            num_branch_instr = num_branch_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << rs1 << "\t" << "-"; 
          } else if (opcode == 0b1101000) {
            mydecoded_out<<"\tJ";
            op = J;
            rd  = 0; //Plain unconditional jumps (assembler pseudo-op J) are encoded as a JAL with rd=0.
            rs1 = 256; //Non-existent
            rs2 = 256; //Non-existent
            num_branch_instr = num_branch_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << "-"  << "\t" << "-";
          } else if (opcode == 0b1101100) {
            mydecoded_out<<"\tJAL";
            op  = JAL;
            rd  = (data >> OPCODE_WIDTH) & rd_mask;
            rs1 = 256; //Non-existent
            rs2 = 256; //Non-existent
            num_branch_instr = num_branch_instr + 1;
            mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << "-"  << "\t" << "-";
          } else if (opcode == 0b1110000 || opcode == 0b1110100) {
            if (opcode == 0b1110000) {
              mydecoded_out<<"\tSYSTEM1";
              op  = SYSTEM1;
              rd  = (data >> OPCODE_WIDTH) & rd_mask;
              rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
              rs2 = 256; //Non-existent
              num_branch_instr = num_branch_instr + 1;
              mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << rs1 << "\t" << "-";
            } else {
              mydecoded_out<<"\tSYSTEM2";
              op = SYSTEM2;
              rd  = (data >> OPCODE_WIDTH) & rd_mask;
              rs1 = (data >> (OPCODE_WIDTH + RD_WIDTH + FUNCT3_WIDTH)) & rs1_mask;
              rs2 = 256; //Non-existent
              num_branch_instr = num_branch_instr + 1;
              mydecoded_out << "\t" << hex << op << "\t" << dec <<  rd << "\t" << rs1 << "\t" << "-";
            }  
          } else {//----------------------------------------//Not SYSTEM1 or SYSTEM2
            mydecoded_out <<"\tUNKNOWN";
            op = UNKNOWN;
            rd  = 256; //Non-existent
            rs1 = 256; //Non-existent
            rs2 = 256; //Non-existent
            mydecoded_out << "\t" << "-" << "\t" << "-" << "\t" << "-"  << "\t" << "-";
          }
        } else {//----------------------------------------------------------------------------//Not a known instruction.
          mydecoded_out<<"\tUNKNOWN\t-";
          op = UNKNOWN;
          rd  = 256; //Non-existent
          rs1 = 256; //Non-existent
          rs2 = 256; //Non-existent
          mydecoded_out << "\t" << "-" << "\t" << "-" << "\t" << "-"  << "\t" << "-";
        }
        
        //Logic to find if the following address is consecutive or not.
        //Indicates if current instruction is a result of a branch.
        if(prev_addr + 0x4 != addr) {
          mydecoded_out << "\tTK_BR" << endl;
          taken_branch = 1;
        } else {
          mydecoded_out << "\t-" << endl;
          taken_branch = 0;
        }
       
        //------------------------------------------------------------------------------------//
        //Process Instruction
        mydebug_out << "*****************************************************************************************************************" << endl;
        mydebug_out << "Processing instruction with Addr = " << hex << addr << " Data = " << hex << data << " OP= " << op << " Rd= " << dec << rd << " Rs1= " << dec << rs1 << " Rs2= " << dec << rs2 << endl;
        
        
        //Instruction Fetch///////
        //Why is IF_clock_cycle incremented by 14 on a miss instead of 15? Why is there no increment when there is a hit. (lines 339 and 343/lines 356 and 360).
        //Answer: We start off with 1 as the IF clock cycle where we consider prev IF cycle + 1 or ID. So 1 cycle is already accounted for.
        mydebug_out << "***Instruction Fetch Stage.******************************\n";
        if(taken_branch == 0) {
          mydebug_out << "Consecutive.\n";
          max_arr[0] = prev_IF_clock_cycle+1;
          max_arr[1] = prev_ID_clock_cycle;
          IF_clock_cycle = max(max_arr, 2);
          mydebug_out << "IF Clock cycle: " << dec << IF_clock_cycle << endl;
          
          if(IF_clock_cycle % 128 == 55) {     //Miss rate is 0.8% (1 in 128)
            mydebug_out << "IF miss!!!\n";
            IF_clock_cycle = IF_clock_cycle + 14;
          } else {
            mydebug_out << "IF hit!!!\n";
            num_instr_cache_hits = num_instr_cache_hits + 1; //Increment the instr cache hits counter by 1.
            IF_clock_cycle = IF_clock_cycle; //Redundant but this is written for clarity. 
          }
        } else {
          //Check the branch valid clock cycle and add the diff between curr and valid clock cycle to the curr clock cycle.
          mydebug_out << "Branch.\n";
          max_arr[0] = prev_IF_clock_cycle+1;
          max_arr[1] = prev_ID_clock_cycle;
          max_arr[2] = scbd_br[0]+1;
          IF_clock_cycle = max(max_arr, 3);
          mydebug_out << "IF Clock cycle: " << dec << IF_clock_cycle << endl;
          
          if(IF_clock_cycle % 128 == 55) {     //Miss rate is 0.8% (1 in 128)
            mydebug_out << "IF miss!!!\n";
            IF_clock_cycle = IF_clock_cycle + 14;
          } else {
            mydebug_out << "IF hit!!!\n";
            num_instr_cache_hits = num_instr_cache_hits + 1; //Increment the instr cache hits counter by 1.
            IF_clock_cycle = IF_clock_cycle;
          }
          num_taken_branches = num_taken_branches + 1; //Increment the taken branch counter by 1.
        }

        //Increment the number of instruction cache access counter by 1.
        //This is essentially the same as the number of known instructions.
        //There are no unknown instructions in the instruction trace file. CHECK!!!
        num_instr_cache_acc = num_instr_cache_acc + 1; 
        
        clock_cycle = IF_clock_cycle;
        mydebug_out << "Clock cycle at the end of IF stage: " << dec << clock_cycle << endl << endl;
        ///////////////////////////




        //Instruction Decode//////
        //Register 0 is not a real register.
        //If R0 is a source register, then it reads 0. 
        //If R0 is a destination register, then the write is ignore.
        mydebug_out << "***Instruction Decode Stage.******************************\n";
        max_arr[0] = clock_cycle + 1;
        max_arr[1] = prev_ID_clock_cycle + 1;
        ID_clock_cycle = max(max_arr, 2);
        mydebug_out << "ID Clock cycle: " << dec << ID_clock_cycle << endl;
        

        if(rs1 < 32) {//Check if there is a dependency on Rs1
          mydebug_out << "\nFirst PASS....\n";
          mydebug_out << "There is a Rs1["<<dec<<rs1<<"] associated with the instr." << endl;
          mydebug_out << "Rs1["<<dec<<rs1<<"] from Scoreboard:[11,01,10,00]:\t" << dec << scbd[rs1][3] << "\t" << dec << scbd[rs1][1] << "\t" << dec << scbd[rs1][2] << "\t" << dec << scbd[rs1][0] << endl;


          if(ID_clock_cycle >= scbd[rs1][0]) { 
            mydebug_out << "ID_clock_cycle >= scbd[rs1][0] so no dependency.\n";
            rs1_stall = 0;
          } else {
            if(ID_clock_cycle <= scbd[rs1][3]) {
              if(ID_clock_cycle == scbd[rs1][3]) {
                mydebug_out << "ID_clock_cycle = scbd[rs1][3] so LD instruction\n";
                rs1_stall = rs1_stall + 1;
              } else {
                mydebug_out << "ID_clock_cycle < scbd[rs1][3]\n";
                rs1_stall = scbd[rs1][3] - ID_clock_cycle + 1;
              }
            } else if (ID_clock_cycle <= scbd[rs1][1]) {
              if(ID_clock_cycle == scbd[rs1][1]) {
                mydebug_out << "ID_clock_cycle = scbd[rs1][1] so ALU_Ex_fwd++\n";
                rs1_stall = 0;
              } else {
                rs1_stall = scbd[rs1][1] - ID_clock_cycle;
              }
            } else if (ID_clock_cycle <= scbd[rs1][2]) {
              if(ID_clock_cycle == scbd[rs1][2]) {
                mydebug_out << "ID_clock_cycle = scbd[rs1][2] so no stall\n";
                rs1_stall = 0;
              } else {
                rs1_stall = scbd[rs1][2] - ID_clock_cycle;
              }
            } else {
              mydebug_out << "ERROR\n";
            }
          }
        }


        if(rs2 < 32) {//Check if there is a dependency on rs2
          mydebug_out << "\nFirst PASS....\n";
          mydebug_out << "There is a rs2["<<dec<<rs2<<"] associated with the instr." << endl;
          mydebug_out << "rs2["<<dec<<rs2<<"] from Scoreboard:[11,01,10,00]:\t" << dec << scbd[rs2][3] << "\t" << dec << scbd[rs2][1] << "\t" << dec << scbd[rs2][2] << "\t" << dec << scbd[rs2][0] << endl;


          if(ID_clock_cycle >= scbd[rs2][0]) { 
            mydebug_out << "ID_clock_cycle >= scbd[rs2][0] so no dependency.\n";
            rs2_stall = 0;
          } else {
            if(ID_clock_cycle <= scbd[rs2][3]) {
              if(ID_clock_cycle == scbd[rs2][3]) {
                mydebug_out << "ID_clock_cycle = scbd[rs2][3] so LD instruction\n";
                rs2_stall = rs2_stall + 1;
              } else {
                mydebug_out << "ID_clock_cycle < scbd[rs2][3]\n";
                rs2_stall = scbd[rs2][3] - ID_clock_cycle + 1;
              }
            } else if (ID_clock_cycle <= scbd[rs2][1]) {
              if(ID_clock_cycle == scbd[rs2][1]) {
                mydebug_out << "ID_clock_cycle = scbd[rs2][1] so ALU_Ex_fwd++\n";
                rs2_stall = 0;
              } else {
                rs2_stall = scbd[rs2][1] - ID_clock_cycle;
              }
            } else if (ID_clock_cycle <= scbd[rs2][2]) {
              if(ID_clock_cycle == scbd[rs2][2]) {
                mydebug_out << "ID_clock_cycle = scbd[rs2][2] so no stall\n";
                rs2_stall = 0;
              } else {
                rs2_stall = scbd[rs2][2] - ID_clock_cycle;
              }
            } else {
              mydebug_out << "ERROR\n";
            }
          }
        }

        max_arr[0] = rs1_stall;
        max_arr[1] = rs2_stall;
        num_cycles_stalled_in_ID_stage = num_cycles_stalled_in_ID_stage + max(max_arr, 2); 
        ID_clock_cycle = ID_clock_cycle + max(max_arr, 2);



        if(rs1 < 32) {//Check if there is a dependency on Rs1
          mydebug_out << "\nSecond PASS....\n";
          mydebug_out << "There is a Rs1["<<dec<<rs1<<"] associated with the instr." << endl;
          mydebug_out << "Rs1["<<dec<<rs1<<"] from Scoreboard:[11,01,10,00]:\t" << dec << scbd[rs1][3] << "\t" << dec << scbd[rs1][1] << "\t" << dec << scbd[rs1][2] << "\t" << dec << scbd[rs1][0] << endl;
         

          if(ID_clock_cycle >= scbd[rs1][0]) { 
            mydebug_out << "ID_clock_cycle >= scbd[rs1][0] so no dependency.\n";
            rs1_stall = 0;
          } else {
            if(ID_clock_cycle <= scbd[rs1][3]) {
              if(ID_clock_cycle == scbd[rs1][3]) {
                mydebug_out << "ID_clock_cycle = scbd[rs1][3] so LD instruction\n";
                rs1_stall = rs1_stall + 1;
              } else {
                mydebug_out << "ID_clock_cycle < scbd[rs1][3]\n";
                rs1_stall = scbd[rs1][3] - ID_clock_cycle + 1;
              }
            } else if (ID_clock_cycle <= scbd[rs1][1]) {
              if(ID_clock_cycle == scbd[rs1][1]) {
                mydebug_out << "ID_clock_cycle = scbd[rs1][1] so ALU_Ex_fwd++\n";
                rs1_stall = 0;
                num_fwd_from_EX_stage = num_fwd_from_EX_stage + 1;
              } else {
                rs1_stall = scbd[rs1][1] - ID_clock_cycle;
              }
            } else if (ID_clock_cycle <= scbd[rs1][2]) {
              if(ID_clock_cycle == scbd[rs1][2]) {
                mydebug_out << "ID_clock_cycle = scbd[rs1][2] so no stall\n";
                rs1_stall = 0;
                if(scbd[rs1][1] != 0) {
                  mydebug_out << "LD_MEM_fwd++\n";
                } else {
                  mydebug_out << "ALU_MEM_fwd++\n";
                }
                num_fwd_from_MEM_stage = num_fwd_from_MEM_stage + 1;
              } else {
                rs1_stall = scbd[rs1][2] - ID_clock_cycle;
              }
            } else {
              mydebug_out << "ERROR\n";
            }
          }

        }

        if(rs2 < 32) {//Check if there is a dependency on rs2
          mydebug_out << "\nSecond PASS....\n";
          mydebug_out << "There is a rs2["<<dec<<rs2<<"] associated with the instr." << endl;
          mydebug_out << "rs2["<<dec<<rs2<<"] from Scoreboard:[11,01,10,00]:\t" << dec << scbd[rs2][3] << "\t" << dec << scbd[rs2][1] << "\t" << dec << scbd[rs2][2] << "\t" << dec << scbd[rs2][0] << endl;


          if(ID_clock_cycle >= scbd[rs2][0]) { 
            mydebug_out << "ID_clock_cycle >= scbd[rs2][0] so no dependency.\n";
            rs2_stall = 0;
          } else {
            if(ID_clock_cycle <= scbd[rs2][3]) {
              if(ID_clock_cycle == scbd[rs2][3]) {
                mydebug_out << "ID_clock_cycle = scbd[rs2][3] so LD instruction\n";
                rs2_stall = rs2_stall + 1;
              } else {
                mydebug_out << "ID_clock_cycle < scbd[rs2][3]\n";
                rs2_stall = scbd[rs2][3] - ID_clock_cycle + 1;
              }
            } else if (ID_clock_cycle <= scbd[rs2][1]) {
              if(ID_clock_cycle == scbd[rs2][1]) {
                mydebug_out << "ID_clock_cycle = scbd[rs2][1] so ALU_Ex_fwd++\n";
                rs2_stall = 0;
                num_fwd_from_EX_stage = num_fwd_from_EX_stage + 1;
              } else {
                rs2_stall = scbd[rs2][1] - ID_clock_cycle;
              }
            } else if (ID_clock_cycle <= scbd[rs2][2]) {
              if(ID_clock_cycle == scbd[rs2][2]) {
                mydebug_out << "ID_clock_cycle = scbd[rs2][2] so no stall\n";
                rs2_stall = 0;
                if(scbd[rs2][1] != 0) {
                  mydebug_out << "LD_MEM_fwd++\n";
                } else {
                  mydebug_out << "ALU_MEM_fwd++\n";
                }
                num_fwd_from_MEM_stage = num_fwd_from_MEM_stage + 1;
              } else {
                rs2_stall = scbd[rs2][2] - ID_clock_cycle;
              }
            } else {
              mydebug_out << "ERROR\n";
            }
          }

        }

        if((rs1_stall != 0) || (rs2_stall != 0)) {
          mydebug_out << "ERROR: Stalls have to be zero after second pass.\n";
        }

        //Update the Associated Rd in the Scoreboard.
        if((rd > 0) && (rd < 32) && (op != LOAD)) {//If there is an Rd, update the Rd state to 'b11 indicating it is busy. If LOAD then 11 is updated in MEM stage.
          mydebug_out << "Not LOAD Instr. There is a Rd associated with the instr. Updating the scoreboard column 11 with ID_clock_cycle: " << dec << ID_clock_cycle << endl;
          scbd[rd][3] = ID_clock_cycle;
        } else {
          mydebug_out << "There is no Rd associated with the instr." << endl;
        }

        clock_cycle = ID_clock_cycle;
        mydebug_out << "Clock cycle at the end of ID stage: " << dec << clock_cycle << endl << endl;
        ///////////////////////////
        





        //Execution////////////
        mydebug_out << "***Instruction Execution Stage.******************************\n";
        EX_clock_cycle = clock_cycle + 1;

        if((rd > 0) && (rd < 32)) {//If there is an Rd, update the Rd state to 'b11 indicating it is busy.
          if(op == LOAD) {//Sepcial case for LOAD instruction where 01 in scoreboard is 0.
            mydebug_out << "LOAD Instr. There is a Rd["<<dec<<rd<<"] associated with the instr. Updating Scoreboard column 01 with 0." << endl;
            scbd[rd][1] = 0;
          } else {
            mydebug_out << "There is a Rd["<<dec<<rd<<"] associated with the instr. Updating Scoreboard column 01 with EX_clock_cycle: " << dec << EX_clock_cycle << endl;
            scbd[rd][1] = EX_clock_cycle;
          }
        } else {
          mydebug_out << "There is no Rd associated with the instr." << endl;
        }

        //Branch Valid 1 Update
        if((op == BRANCH) || (op == SYSTEM1) || (op == SYSTEM2)) {
          mydebug_out << "Branch_valid_state_1 is updated with EX_clock_cycle: " << dec << EX_clock_cycle << endl;
          scbd_br[1] = EX_clock_cycle;
        }

        clock_cycle = EX_clock_cycle;
        mydebug_out << "Clock cycle at the end of EX stage: " << dec << clock_cycle << endl << endl;
        ///////////////////////////





        //Memory///////////////
        mydebug_out << "***Memory Stage.************************************************\n";
        MEM_clock_cycle = clock_cycle + 1;
        
        if(op == LOAD) {
          mydebug_out << "LOAD Instr.\n";
          num_data_cache_load_acc = num_data_cache_load_acc + 1; //Increment the data cache load access counter in the MEM stage when we see a load instr.
          if(MEM_clock_cycle % 16 == 11) {
            mydebug_out << "LOAD Instr. Data cache miss. Adding 15 clocks.\n";
            MEM_clock_cycle = MEM_clock_cycle + 14;
          } else {
            mydebug_out << "LOAD Instr. Data cache hit.\n";
            num_data_cache_hits = num_data_cache_hits + 1;
          }
        }

        if(op == STORE) {
          num_data_cache_store_acc = num_data_cache_store_acc + 1;
        }

        //Branch Valid 1 Update
        if((op == BRANCH) || (op == SYSTEM1) || (op == SYSTEM2)) {
          mydebug_out << "Branch_valid_state_0 is updated with MEM_clock_cycle: " << dec << MEM_clock_cycle << endl;
          scbd_br[0] = MEM_clock_cycle;
        }
        
        //Update the Associated Rd in the Scoreboard. Column 10 update.
        if((rd > 0) && (rd < 32)) {//If there is an Rd, update the Rd state to 'b10 indicating it is busy.
          mydebug_out << "There is a Rd["<<dec<<rd<<"] associated with the instr. Updating Scoreboard column 10 with MEM_clock_cycle: " << dec << MEM_clock_cycle << endl;
          scbd[rd][2] = MEM_clock_cycle;
        } else {
          mydebug_out << "There is no Rd associated with the instr." << endl << endl;
        }
        
        //Update the Associated Rd in the Scoreboard. Column 11 update.
        if((rd > 0) && (rd < 32) && (op == LOAD)) {//If there is an Rd, update the Rd state to 'b11 indicating it is busy. If LOAD then 11 is updated in MEM stage.
          mydebug_out << "LOAD Instr. There is a Rd["<<dec<<rd<<"] associated with the instr. Updating Scoreboard column 11 with MEM_clock_cycle-1. Does this work??????????" << endl;
          scbd[rd][3] = MEM_clock_cycle - 1;
        } else {
          mydebug_out << "There is no Rd associated with the instr." << endl;
        }

        clock_cycle = MEM_clock_cycle;
        mydebug_out << "Clock cycle at the end of MEM stage: " << dec << clock_cycle << endl;
        ///////////////////////////

        
        
        
        //Write Back//////////////
        mydebug_out << "***Write Back Stage.*********************************************\n";
        WB_clock_cycle = clock_cycle + 1;

        //Update the Associated Rd in the Scoreboard.
        if((rd > 0) && (rd < 32)) {//If there is an Rd, update the Rd state 'b00 with the WB_clock_cycle.
          mydebug_out << "There is a Rd["<<dec<<rd<<"] associated with the instr. Updating Scoreboard column 00 with WB_clock_cycle: ." << dec << WB_clock_cycle << endl;
          scbd[rd][0] = WB_clock_cycle;
        } else {
          mydebug_out << "There is no Rd associated with the instr." << endl;
        }
        
        clock_cycle = WB_clock_cycle;
        mydebug_out << "Clock cycle at the end of WB stage: " << dec << clock_cycle << endl;
        ///////////////////////////



        
        
        //Prev Clock cycle at each stage.
        prev_IF_clock_cycle = IF_clock_cycle;
        prev_ID_clock_cycle = ID_clock_cycle;
        prev_EX_clock_cycle = EX_clock_cycle;
        prev_MEM_clock_cycle = MEM_clock_cycle;
        prev_WB_clock_cycle = WB_clock_cycle;
        
        
        mydebug_out << "*****************************************************************************************************************" << endl;
        //------------------------------------------------------------------------------------//



        //Make the current addr, the prev addr.
        prev_addr = addr;
    }//while


    myaddrfile.close();
    mydatafile.close();
    mydecoded_out.close();
    mydebug_out.close();
   
    cout << dec;
    cout << "Num_data_cache_load_acc       : " << num_data_cache_load_acc << endl;
    cout << "Num_data_cache_store_acc      : " << num_data_cache_store_acc << endl;
    cout << "Num_instr_cache_acc           : " << num_instr_cache_acc << endl;
    cout << "Num_instr_cache_hits          : " << num_instr_cache_hits << endl;
    cout << "Num_data_cache_hits           : " << num_data_cache_hits << endl;
    cout << "Num_alu_instr                 : " << num_alu_instr << endl;
    cout << "Num_branch_instr              : " << num_branch_instr << endl;
    cout << "Num_load_instr                : " << num_load_instr << endl;
    cout << "Num_store_instr               : " << num_store_instr << endl;
    cout << "Num_all_other_instr           : " << num_all_other_instr << endl;
    cout << "Num_taken_branches            : " << num_taken_branches << endl;
    cout << "Num_fwd_from_EX_stage         : " << num_fwd_from_EX_stage << endl;
    cout << "Num_fwd_from_MEM_stage        : " << num_fwd_from_MEM_stage << endl;
    cout << "Num_cycles_stalled_in_ID_stage: " << num_cycles_stalled_in_ID_stage << endl;
    cout << "Total_num_of_cycles           : " << clock_cycle << endl;
    cout << "Done!" << endl;
    cout << "Calculation:" << endl;
    
    cout << fixed;
    cout << setprecision(5);

    hit_ratio = (double)num_instr_cache_hits/num_instr_cache_acc;

    data_hit_ratio = (double)num_data_cache_hits/num_data_cache_load_acc;

    ipc= (double)num_instr_cache_acc/clock_cycle;

    branchper=(double)num_taken_branches/num_branch_instr;

    Alufre= (double)num_alu_instr/num_instr_cache_acc;

    loadfre=(double)num_load_instr/num_instr_cache_acc;

    storefre=(double)num_store_instr/num_instr_cache_acc;

    branchfre=(double)num_branch_instr/num_instr_cache_acc;

    stallper=(double)num_cycles_stalled_in_ID_stage/clock_cycle;

   cout << "Instruction Hit Ratio: " << hit_ratio << endl;
   cout << "Data Cache Hit Ratio:" << data_hit_ratio << endl;
   cout << "Instruction per Clock Cycle:"<< ipc << endl;
   cout << "Percentage of taken branches over total branches:"<< branchper << endl;
   cout << "Instruction type frequency for ALU:" << Alufre << endl;
   cout <<"Instruction type frequency for load:" << loadfre << endl;
   cout <<"Instruction type frequency for store:" << storefre << endl;
   cout <<"Instruction type frequency for branch:" << branchfre << endl;
   cout <<"Percentage of stalled cycles over total cycles:"<< stallper << endl;

    
    return 0;
}
