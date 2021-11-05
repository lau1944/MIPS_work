#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<64>  Read_data1;
    bitset<64>  Read_data2;
    bitset<64>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<64>  ALUresult;
    bitset<64>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<64>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<64> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<64> (0);  
        }
	
        bitset<64> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<64> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<64> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};

class ALU
{
    public:
        bitset<64> ALUresult;
        bitset<64> ALUOperation(bitset<1> ALUOP, bitset<64> oprand1, bitset<64> oprand2)
        {
            string op = ALUOP.to_string();
            if (op == "0") {
                ALUresult = bitset<64>(oprand1.to_ulong() + oprand2.to_ulong());
            } 
            
            if (op == "1") {
                ALUresult = bitset<64>(oprand1.to_ulong() - oprand2.to_ulong());
            } 
            return ALUresult;
        }
};
      
class DataMem    
{
    public:
        bitset<64> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<64> readDataMem(bitset<64> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            datamem.append(DMem[Address.to_ulong()+4].to_string());
            datamem.append(DMem[Address.to_ulong()+5].to_string());
            datamem.append(DMem[Address.to_ulong()+6].to_string());
            datamem.append(DMem[Address.to_ulong()+7].to_string());            
            ReadData = bitset<64>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<64> Address, bitset<64> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
	        DMem[Address.to_ulong()+4] = bitset<8>(WriteData.to_string().substr(32,8));
            DMem[Address.to_ulong()+5] = bitset<8>(WriteData.to_string().substr(40,8));
            DMem[Address.to_ulong()+6] = bitset<8>(WriteData.to_string().substr(48,8));
            DMem[Address.to_ulong()+7] = bitset<8>(WriteData.to_string().substr(56,8));   
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 

int main()
{
    RF myRF;
    ALU alu;
    INSMem myInsMem;
    DataMem myDataMem;
    struct stateStruct state = {};
    state.IF.nop = false;
    state.ID.nop = true;
    state.EX.nop = true;
    state.MEM.nop = true;
    state.WB.nop = true;
    state.EX.alu_op = true;
    int cycle = 0;
			
             
    while (1) {
        struct stateStruct newState = {};
        struct WBStruct pre_wbState     = state.WB;
        struct MEMStruct pre_memState   = state.MEM;
        struct EXStruct pre_exState     = state.EX;
        struct IDStruct pre_idState     = state.ID;
        struct IFStruct pre_ifState     = state.IF;

        /* --------------------- WB stage --------------------- */
        if (!pre_wbState.nop && pre_wbState.wrt_enable) {
            myRF.writeRF(pre_wbState.Rs, pre_wbState.Wrt_data);
        }

        /* --------------------- MEM stage --------------------- */
        struct WBStruct memState = { .nop = pre_exState.nop };
        if (!pre_memState.nop) {
            if (pre_memState.wrt_mem && pre_memState.wrt_enable) 
                myDataMem.writeDataMem(pre_memState.ALUresult, pre_memState.Store_data);
            else if (pre_memState.rd_mem) 
                bitset<64> readData = myDataMem.readDataMem(pre_memState.ALUresult);

        }

        /* --------------------- EX stage --------------------- */
        if (!pre_exState.nop) {
            if (!pre_exState.wrt_mem && !pre_exState.rd_mem) {
                bitset<64> results = alu.ALUOperation(pre_exState.alu_op, pre_exState.Read_data1, pre_exState.Read_data2);
            }
        }

        /* --------------------- ID stage --------------------- */
        bitset<1> aluOp;
        if (!pre_idState.nop) {
            // decode(Read RF)
            // Decoder
            bitset<32> instruction = pre_idState.Instr;
            string instr_str = instruction.to_string();
            bitset<1> isLoad = instr_str.substr(25, 7) == string("0000011");
            bitset<1> isStore = instr_str.substr(25, 7) == string("0100011");
            bitset<1> isRType = instr_str.substr(25, 7) == string("0110011");
            bitset<1> isBranch = instr_str.substr(25, 7) == string("1100011");
            bitset<1> isIType = instr_str.substr(25, 5) == string("00100") ||
                instr_str.substr(25, 5) == string("11000");
            bitset<1> wrtEnable = !(isStore.to_ulong() || isBranch.to_ulong()); 

            if (isRType[0] == 1) {
                if(instr_str.substr(17, 3) == string("000")) {
                    if(instr_str.substr(0, 7) == string("0000000"))
                        aluOp = bitset<1>(0);  //add
                    else if(instr_str.substr(0, 7) == string("0100000"))
                        aluOp = bitset<1>(1);  //sub
                }
            } else if (isStore[0] == 1 || isLoad[0] == 1) {
                aluOp = bitset<1>(0); //sw or lw
            }

            if (wrtEnable.to_string() == "1") {
                // write back

            } else {
                if (isLoad.to_string() == "1") {
                    bitset<64> data = myRF.readRF(bitset<5>(instr_str.substr(12,5)));
                }
                else if (isRType.to_string() == "1") {
                    bitset<64> read_data1 = myRF.readRF(bitset<5>(instr_str.substr(7,5)));
                    bitset<64> read_data2 = myRF.readRF(bitset<5>(instr_str.substr(12,5)));
                }
            }

            // 3. Execuete alu operation
            bitset<64> tmp;
            if(isLoad[0] == 1 || isIType[0] == 1) {
                //imm[11:0]
                tmp = bitset<64>(instr_str.substr(0,12)); // if positive, 0 padded
                if (tmp[20] == true) {
                    tmp = bitset<64>(string(52, '1') + tmp.to_string().substr(20, 12));
                }
            }
            else if(isStore[0] == 1)
            {
                //mm[11:5] rs2 rs1 010 imm[4:0]
                tmp = bitset<64>(instr_str.substr(0, 7) + instr_str.substr(20, 5)); 
                if (tmp[20] == true) {
                    tmp = bitset<64>(string(52, '1') + tmp.to_string().substr(20, 12));
                }
            } else if(isBranch[0] == 1) {
                bitset<64> read_data1 = myRF.readRF(bitset<5>(instr_str.substr(7,5)));
                bitset<64> read_data2 = myRF.readRF(bitset<5>(instr_str.substr(12,5)));

                // work as bne
                if (instr_str.substr(17,3) == "000" && read_data1 == read_data2) {
                    bitset<13> offset = bitset<13>(instr_str.substr(0, 1) 
                        + instr_str.substr(24,1) + instr_str.substr(30, 5) + instr_str.substr(20, 4) + "0");
                    // update PC, stall

                }
            }
        }


        /* --------------------- IF stage --------------------- */
        if (!pre_ifState.nop) {
            bitset<32> pc = pre_ifState.PC;
            bitset<32> address_offset;
            bitset<32> instr = myInsMem
                .readInstr(bitset<32>(pc.to_ulong() + address_offset.to_ulong()));

            if (instr == 0xffffffff) {
                break;
            }    
        }

        /* --------------------- Stall unit--------------------- */





        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;

        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 

        cycle += 1;
        state = newState; /* The end of the cycle and updates the current state with the values calculated in this cycle */

    }

    myRF.outputRF(); // dump RF;	
    myDataMem.outputDataMem(); // dump data mem 

    return 0;
}
