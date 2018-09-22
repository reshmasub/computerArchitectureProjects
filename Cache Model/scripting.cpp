#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<cmath>
#include<numeric>
#include<iomanip>

using namespace std;

//Unused - Keep for clarity!
#define NUM_OF_MEM_LINES 1743  //Not used anywhere in the program!
#define MEM_BLOCK_SIZE_IN_BYTES 16  //Not used anywhere in the program!
#define NUM_OF_HALFWORDS_PER_LINE 8  //Not used anywhere in the program!
#define BITS_PER_BYTE 8  //Not used anywhere in the program!
#define TOTAL_NUM_OF_BYTES NUM_OF_MEM_LINES*MEM_BLOCK_SIZE_IN_BYTES  //27888 bytes       //Not used anywhere in the program!
#define TOTAL_NUM_OF_HALFWORDS NUM_OF_MEM_LINES*NUM_OF_HALFWORDS_PER_LINE //13944 halfwords   //Not used anywhere in the program!

//***********************************************************************
// Defines
//***********************************************************************
//Change cache size and cache line size here. Dont change the associativity yet!!
//#define CACHE_SIZE 2048 // Cache sizes in Bytes to choose from: 1024,2048,4096,8192,16384,32758
//#define CACHE_LINE_SIZE 2 //Cache line size in Bytes to choose from: 2,4,8,16,32,64
//#define ASSOCIATIVITY 1 //1-way, 2-way, 4-way and 8-way
#define ADDR_WIDTH 32
//********************************************************************


//***********************************************************************
// Functions
//**********************************************************************
//Pass the memory vector(which is called halfword) by refetence and the const is to make sure the function doesnt override the memory.
//Takes address as input and the cache line size as input and return the string to be placed into the cache line.
vector <string> ReadMem(const vector<string> &halfword, int addr, int cache_line_size) {
    int num_of_hw_to_fetch = 0;
    int halfword_index = addr/2;
    vector <string> memory_line;
    //Vector to fetch a single cacheline from memory.
    //cout << "halfword_index: " << halfword_index << endl;
    
    num_of_hw_to_fetch = cache_line_size/2; //1,2,4,8,16,32
    //debug cout << "num_of_hw_to_fetch: " << num_of_hw_to_fetch << endl;
    int upper_lim=0,lower_lim=0;
    
    if(num_of_hw_to_fetch > 0){
        for (int i = 0; i<100000; i++) {
            if(num_of_hw_to_fetch*i > halfword_index) {
                upper_lim = num_of_hw_to_fetch*i;
                lower_lim = upper_lim - num_of_hw_to_fetch;
                //debug cout << i << endl;
                //debug cout << upper_lim << endl;
                //debug cout << lower_lim << endl;
                break;
            } else {
                
            }
        }
    } else {
//debug_l2 cout << halfword[addr/2] << endl;
    }
    
    //Add the requested data from memory to the cacheline vector.
    for (int j=lower_lim; j<upper_lim; j++) {
        memory_line.push_back(halfword[j]);
    }
    return memory_line;
}

// Function to check the cache and find out if there is a hit and if there is valid bit being set.
// For 2,4 and 8 ways the arguments hit_arr and valid_match_arr are pointing to elements in an array so as to keep a note of hit and valid
// information for all the blocks of cache. In the case of 4 way associative cache there is a hit array of 4 and valid_march_arr of 4 elements.
void check_cache(int addr_index, int addr_tag, int *hit_arr, int *valid_match_arr, const vector<int> &cache_tag, const vector<int> &cache_valid) {
    if(cache_valid[addr_index] == 0x1) {  //Valid is TRUE
        if(addr_tag == cache_tag[addr_index]) { //TAG is a MATCH
            *hit_arr = 0x1;
//debug_l2 cout << "DEBUG!!!!!!!: Inside check_cache function. For addr 0x"<< hex << addr_index << " And it is a Hit!!!" <<endl;
        } else {                    //TAG isnt a MATCH
            *hit_arr = 0x0;
            *valid_match_arr = 0x1;
//debug_l2 cout << "DEBUG!!!!!!!: Inside check_cache function. And it is a Miss!!!, But valid is " << *valid_match_arr << endl;

        }
    } else {  //Valid is FALSE
        *hit_arr = 0x0;
        *valid_match_arr = 0x0;
//debug_l2 cout << "DEBUG!!!!!!!: Inside check_cache function. And it is a Miss!!!, But valid is " << *valid_match_arr << endl;
    }
}
//**********************************************************************


//**********************************************************************
// Main Function
//**********************************************************************
int main (int argc, char *argv[]) {
    
    int CACHE_SIZE = 0;
    int CACHE_LINE_SIZE = 0;
    int ASSOCIATIVITY = 0;
    
    if (argc != 4) {
        cout << "ERROR: Incorrect Number of Arguments!!!." << endl; 
        cout << "Please enter the 3 arguments in the following order." << endl;
        cout << "Cache Size, Cache Line Size and Associativity. " << endl;
        return 0;
    } else {
        CACHE_SIZE = atoi(argv[1]);
        CACHE_LINE_SIZE = atoi(argv[2]);
        ASSOCIATIVITY = atoi(argv[3]);
        
        cout << "Executing for the following Cache Configuration:";
        cout << " CACHE_SIZE: " << CACHE_SIZE;
        cout << " CACHE_LINE_SIZE: " << CACHE_LINE_SIZE;
        cout << " ASSOCIATIVITY: " << ASSOCIATIVITY << endl;
    }
    
    //Memory vectors
    vector <string> halfword; //This is the Memory vector
    vector <string> cacheline; //Variable to receive the cacheline vector from the ReadMem function.
    
    //Cache vectors
    //Note: The number of lines in the cache stays the same irrespective of the ASSOCIATIVITY value.
    //      This is because we virtually split the cache by manipulating the address instead of having to physically split the cache.
    vector <vector <string> > cache_data (CACHE_SIZE/(CACHE_LINE_SIZE));  //Cache data vector
    vector <int> cache_tag (CACHE_SIZE/(CACHE_LINE_SIZE),0); //Cache tag vector
    vector <int> cache_valid (CACHE_SIZE/(CACHE_LINE_SIZE),0); //Cache valid vector
    
    int i=0;  //Index into the memory
    
    //Code to read the memory file.
    ifstream myReadFile;  //File pointer
    myReadFile.open("inst_mem_hex_16byte_wide.txt");    //Open the mem trace file.

    string line;     //Temporary String to get each line from the mem hex file.
    
    if (myReadFile.is_open()) {
        
        while (getline(myReadFile,line)) {
            //Read each line into a string.
            string upper_8bytes = line.substr(0, line.length()/2);
            string lower_8bytes = line.substr(line.length()/2);
            
            string upper_2byte_3 = upper_8bytes.substr(0,4);
            string upper_2byte_2 = upper_8bytes.substr(4,4);
            string upper_2byte_1 = upper_8bytes.substr(8,4);
            string upper_2byte_0 = upper_8bytes.substr(12,4);
            
            string lower_2byte_3 = lower_8bytes.substr(0,4);
            string lower_2byte_2 = lower_8bytes.substr(4,4);
            string lower_2byte_1 = lower_8bytes.substr(8,4);
            string lower_2byte_0 = lower_8bytes.substr(12,4);
            
            //Push it into a vector halfword by halfword.
            halfword.push_back(lower_2byte_0);
            halfword.push_back(lower_2byte_1);
            halfword.push_back(lower_2byte_2);
            halfword.push_back(lower_2byte_3);
            
            halfword.push_back(upper_2byte_0);
            halfword.push_back(upper_2byte_1);
            halfword.push_back(upper_2byte_2);
            halfword.push_back(upper_2byte_3);
            
            //Increment the memory index.
        } //while loop
    } else { //if(*.isopen())
        std::cout << "Error opening file";
    } //else
    myReadFile.close();


    //Variables
    int addr_index = 0;
    int addr_tag = 0;
    int addr_byte_offset = 0;
    int index_mask = 0;
    int tag_mask = 0;
    int byte_offset_mask = 0;
    int addr = 0;
    int data_size_per_address = 0;
    
    //For associativity
    int hit_arr[8];
    int valid_match_arr[8];
    int rand_set_to_write = 0;
    int hit_found = 0;
    int hit_in_set = 0;
    int valid_zero_found = 0;
    int valid_zero_index = 0;
    
    
    int CACHE_BYTE_OFFSET_WIDTH = 0;
    int CACHE_INDEX_WIDTH       = 0;
    int CACHE_TAG_WIDTH         = 0;
    int CACHE_INDEX_DEPTH       = 0;
    
    CACHE_BYTE_OFFSET_WIDTH = log2(CACHE_LINE_SIZE);
    CACHE_INDEX_WIDTH       = log2(CACHE_SIZE/(CACHE_LINE_SIZE*ASSOCIATIVITY));
    CACHE_TAG_WIDTH         = ADDR_WIDTH - CACHE_INDEX_WIDTH - CACHE_BYTE_OFFSET_WIDTH;
    CACHE_INDEX_DEPTH       = CACHE_SIZE/(CACHE_LINE_SIZE*ASSOCIATIVITY);
    //debug_l2 cout << "CACHE_BYTE_OFFSET_WIDTH: " << CACHE_BYTE_OFFSET_WIDTH << endl;
    //debug_l2 cout << "CACHE_INDEX_WIDTH:       " << CACHE_INDEX_WIDTH << endl;
    //debug_l2 cout << "CACHE_TAG_WIDTH:         " << CACHE_TAG_WIDTH << endl << endl;
    
    //******************* Calculate the index, tag, byte offset mask based on config ******************//
    byte_offset_mask = (pow(2,CACHE_BYTE_OFFSET_WIDTH) - 1);
    index_mask = (pow(2,CACHE_INDEX_WIDTH) - 1);
    tag_mask = (pow(2,CACHE_TAG_WIDTH) - 1);
    
    //debug_l2 cout << "Index Mask:       " << hex << index_mask << endl;
    //debug_l2 cout << "Tag Mask:         " << hex << tag_mask << endl;
    //debug_l2 cout << "Byte offset Mask: " << hex << byte_offset_mask << endl;
    //*************************************************************************************************************//

    
    //Variables to be used for cache.
    int hit = 0;
    int miss = 0;
    int prev_addr_index = 0;
    int local_addr = 0;
    int num_of_clock_cycles = 0;
    string data_output;
    
    std::fstream myaddrfile("inst_addr_trace_hex_project_1.txt", std::ios_base::in);
    std::fstream mydatasizefile("inst_data_size_project_1.txt", std::ios_base::in);
    std::fstream mydataoutputfile("inst_data_trace_hex_project_1_result.txt", std::ios_base::out);
    
    while (myaddrfile >> hex >> addr && mydatasizefile >> data_size_per_address) {
        //debug_l2 cout << "*******************************************************************************************************************" << endl;
        //debug_l2 cout << "Read from the addr_trace and data_size files. INSTRUCTION ADDR in hex: 0x" << hex << addr << endl;
        //debug_l2 cout <<" Data size to be read from instruction cache in decimal: " << dec << data_size_per_address << endl;
        
        for(int num_acc_per_addr = 0; num_acc_per_addr < (data_size_per_address/4); num_acc_per_addr++) {
            local_addr = addr + 2*num_acc_per_addr; //Incrementing the address.
            
            addr_index       = (local_addr >> CACHE_BYTE_OFFSET_WIDTH) & index_mask;
            addr_tag         = (local_addr >> (CACHE_BYTE_OFFSET_WIDTH+CACHE_INDEX_WIDTH) & tag_mask);
            addr_byte_offset = (local_addr & byte_offset_mask);
            //debug_l2 cout << "ADDR: halfword addr = 0x" << hex << local_addr << endl;
            //debug_l2 cout << "ADDR: index = 0x" << hex << addr_index << endl;
            //debug_l2 cout << "ADDR: tag = 0x" << hex << addr_tag << endl;
            //debug_l2 cout << "ADDR: byte_offset = 0x" << hex << addr_byte_offset << endl;           
            //debug_l1 cout << "Trying in cache for address: 0x" << hex << local_addr << endl;

            for(int assoc=0; assoc < ASSOCIATIVITY; assoc ++) {
                //Check the cache by giving the addr_index to see if there is a hit and also check for valid being 0 or 1.
                //We need to check the valid bit to decide on our replacement strategy. If one of the valids is 0, just use that particular cache line for replacement. Else choose a random line.
                check_cache((addr_index+assoc*CACHE_INDEX_DEPTH), addr_tag, &hit_arr[assoc], &valid_match_arr[assoc], cache_tag, cache_valid);
            }
            
            //Code to parse the hit array to see if there was a hit in one of the Ways of associative set of cache.
            for(int hit_index=0; hit_index < ASSOCIATIVITY; hit_index++) {
                if(hit_arr[hit_index] == 0x1) {
                    hit_found = 0x1;
                    hit_in_set = hit_index;
                    //debug_l2 cout << "Parsing the hit array and found the hit_index: " << hit_index << endl;
                    break; //exit the for loop once a hit is found!!!!
                } else {
                    hit_found = 0x0;
                    hit_in_set = 0x0;
                }
            } // for loop to parse the hit array.
            
            //Code to parse the valid array to see if one of the valids is zero. If it is then replace the line in that particular set.
            for(int valid_index=0; valid_index < ASSOCIATIVITY; valid_index++) {
                if(valid_match_arr[valid_index] == 0x0) {
                    valid_zero_found = 0x1;
                    valid_zero_index = valid_index;
                    //debug_l2 cout << "Parsing the valid array and found the valid_zero_index: " << valid_zero_index << endl;
                    break; //exit the for loop once a hit for zero valid is found.
                } else {
                    valid_zero_found = 0x0;
                    valid_zero_index = 0x0;
                }
            } //for loop to parse the valid match array.
            
            if(hit_found == 0x1) { //HIT is 1
                //cout << "The cache access for addr: 0x" << hex << local_addr << " is a HIT!!!" << endl;
                if(prev_addr_index != addr_index) {
                    hit = hit + 1;
                    num_of_clock_cycles = num_of_clock_cycles + 1;
                } else {
                    //debug_l2 cout << "Hit found but the prev addr index was the same as addr index" << endl;
                }
                //debug_l2 cout << "DEBUG: " << cache_data[addr_index+hit_in_set*CACHE_INDEX_DEPTH][addr_byte_offset/2] << endl;
                data_output = cache_data[addr_index+hit_in_set*CACHE_INDEX_DEPTH][addr_byte_offset/2]+data_output;
            } else {
                if(valid_zero_found == 0x1) { //HIT is zero but VALID is 1
                    //debug_l2 cout << "Valid zero found. Associativity is " << ASSOCIATIVITY << endl;
                    cache_tag[addr_index+valid_zero_index*CACHE_INDEX_DEPTH] = addr_tag;
                    cache_valid[addr_index+valid_zero_index*CACHE_INDEX_DEPTH] = 0x1;
                    cacheline = ReadMem(halfword, local_addr, CACHE_LINE_SIZE);
                    cache_data[addr_index+valid_zero_index*CACHE_INDEX_DEPTH] = cacheline;
                    
                    miss = miss + 1;
                    num_of_clock_cycles  = num_of_clock_cycles + 15;
                    
                    //debug_l2 cout << "Block/line read from memory: " << accumulate(cacheline.begin(),cacheline.end(),string(""))<<endl;
                    //debug_l2 cout << "DEBUG: " << cache_data[addr_index+valid_zero_index*CACHE_INDEX_DEPTH][addr_byte_offset/2] << endl;
                    data_output = cache_data[addr_index+valid_zero_index*CACHE_INDEX_DEPTH][addr_byte_offset/2]+data_output;
                } else { //HIT is zero and VALID is zero.
                    //debug_l2 cout << "Valid zero not found." << endl;
                    rand_set_to_write = num_of_clock_cycles % ASSOCIATIVITY;
                    //debug_l2 cout << "Random set to write to: " << rand_set_to_write << ". Associativity is " << dec << ASSOCIATIVITY << endl;
                    cache_tag[addr_index+rand_set_to_write*CACHE_INDEX_DEPTH] = addr_tag;
                    cache_valid[addr_index+rand_set_to_write*CACHE_INDEX_DEPTH] = 0x1;
                    cacheline = ReadMem(halfword, local_addr, CACHE_LINE_SIZE);
                    cache_data[addr_index+rand_set_to_write*CACHE_INDEX_DEPTH] = cacheline;
                    
                    miss = miss + 1;
                    num_of_clock_cycles  = num_of_clock_cycles + 15;
                    
                    //debug_l2 cout << "Block/line read from memory: " << accumulate(cacheline.begin(),cacheline.end(),string(""))<<endl;
                    //debug_l2 cout << "DEBUG: " << cache_data[addr_index+rand_set_to_write*CACHE_INDEX_DEPTH][addr_byte_offset/2] << endl;
                    data_output = cache_data[addr_index+rand_set_to_write*CACHE_INDEX_DEPTH][addr_byte_offset/2]+data_output;
                }
            }
            
           // cout << "DEBUG:!!!!!!!!! Outside the check_cache function. hit_arr[0] = " << hit_arr[0] << endl;
           // cout << "DEBUG:!!!!!!!!! Outside the check_cache function. valid_match_arr[0] = " << valid_match_arr[0] << endl;
            prev_addr_index = addr_index;
        } //for(int num_acc_per_addr loop
        
        //debug_l2 cout << "Data Result: 0x" << data_output << " for Addr: 0x" << hex <<addr << endl << endl;
        mydataoutputfile << data_output << endl;
        data_output = "";
        //debug_l2 cout << "Number of clock cycles: " << dec << num_of_clock_cycles << endl;
        //debug_l2 cout << "Number of Hits: " << dec << hit << endl;
        //debug_l2 cout << "Number of Misses: " << dec << miss << endl;
    
            i++;
    }//while(myaddrfile >> hex >> addr && mydatasizefile >> data_size_per_address)
    
    myaddrfile.close();
    mydatasizefile.close();
    mydataoutputfile.close();
    
    //Configuration data output
    double hit_ratio;
    int num_accesses = 0;
    double ipc;
    
    ipc = (double)i/num_of_clock_cycles;
    num_accesses = hit+miss;
    hit_ratio = (double)hit/num_accesses;

    //Open the output data file to append.
    std::fstream myconfigdataoutfile("config_data_out.txt", std::ios_base::app);    
    
    myconfigdataoutfile << "****************************************************" << endl;
    myconfigdataoutfile << "Cache Size: " << dec << CACHE_SIZE << " Cache Line Size: " << dec << CACHE_LINE_SIZE << " Associativity: " << dec << ASSOCIATIVITY << endl;
    myconfigdataoutfile << "Total Number of Cache Accesses: " << dec << num_accesses << endl;
    myconfigdataoutfile << "Number of Hits:                 " << dec << hit << endl;
    myconfigdataoutfile << "Number of Misses:               " << dec << miss << endl;
    myconfigdataoutfile << "Number of clock cycles:         " << dec << num_of_clock_cycles << endl;
    myconfigdataoutfile << "Number of Instructions Fetched: " << dec << i << endl;
    myconfigdataoutfile << setprecision(5) << fixed;
    myconfigdataoutfile << "Hit Ratio:                      " << hit_ratio << endl;
    myconfigdataoutfile << "IPC:                            " << ipc << endl;
    myconfigdataoutfile << "****************************************************" << endl;

    myconfigdataoutfile.close();
    
    return 0;
}
