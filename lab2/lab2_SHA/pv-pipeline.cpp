#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

enum StallType
{
    Id,
    If,
    Ex,
    Mem,
    Wb,
    None
};

/**
 * Stall 命令管理类
 */
class StallTemplate
{
public:

    StallTemplate() {
        this->type = None;
    };

    StallTemplate(StallType type) {
        this->type = type;
    }

    /**
     * 是否需要 stall
     */
    bool needStall() {
        return type != None;
    }

    /**
     * 设置需要 stall 命令种类
     */
    void setStall(StallType stallType) {
        type = stallType;
    }

    StallType getStallType() {
        return type;
    }

    /**
     * 清除 stall 命令
     */
    void clear() {
        type = None;
    }
private:
    StallType type;
};

struct IFStruct
{
    bitset<32> PC;
    bool nop;
};

struct IDStruct
{
    bitset<32> Instr;
    bool nop;
};

struct EXStruct
{
    bitset<64> Read_data1;
    bitset<64> Read_data2;
    bitset<64> Imm;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool is_I_type;
    bool rd_mem;
    bool wrt_mem;
    bitset<3> alu_op; //001 for addu, lw, sw, 000 for subu , 010 for and, 011 for or, 100 for xor
    bool wrt_enable;
    bool nop;
};

struct MEMStruct
{
    bitset<64> ALUresult;
    bitset<64> Store_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool rd_mem;
    bool wrt_mem;
    bool wrt_enable;
    bool nop;
};

struct WBStruct
{
    bitset<64> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;
    bool nop;
};

struct stateStruct
{
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
};

class RF
{
public:
    bitset<64> Reg_data;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<64>(0);
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
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open())
        {
            rfout << "State of RF:\t" << endl;
            for (int j = 0; j < 32; j++)
            {
                rfout << Registers[j] << endl;
            }
        }
        else
            cout << "Unable to open file";
        rfout.close();
    }

private:
    vector<bitset<64> > Registers;
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
        int i = 0;
        imem.open("imem.txt");
        if (imem.is_open())
        {
            while (getline(imem, line))
            {
                IMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open file";
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress)
    {
        string insmem;
        insmem.append(IMem[ReadAddress.to_ulong()].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
        Instruction = bitset<32>(insmem); //read instruction memory
        return Instruction;
    }

private:
    vector<bitset<8> > IMem;
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
        int i = 0;
        dmem.open("dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem, line))
            {
                DMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open file";
        dmem.close();
    }

    bitset<64> readDataMem(bitset<32> Address)
    {
        string datamem;
        datamem.append(DMem[Address.to_ulong()].to_string());
        datamem.append(DMem[Address.to_ulong() + 1].to_string());
        datamem.append(DMem[Address.to_ulong() + 2].to_string());
        datamem.append(DMem[Address.to_ulong() + 3].to_string());
        datamem.append(DMem[Address.to_ulong() + 4].to_string());
        datamem.append(DMem[Address.to_ulong() + 5].to_string());
        datamem.append(DMem[Address.to_ulong() + 6].to_string());
        datamem.append(DMem[Address.to_ulong() + 7].to_string());
        ReadData = bitset<64>(datamem); //read data memory
        return ReadData;
    }

    void writeDataMem(bitset<32> Address, bitset<64> WriteData)
    {
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
        DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
        DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
        DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
        DMem[Address.to_ulong() + 4] = bitset<8>(WriteData.to_string().substr(32, 8));
        DMem[Address.to_ulong() + 5] = bitset<8>(WriteData.to_string().substr(40, 8));
        DMem[Address.to_ulong() + 6] = bitset<8>(WriteData.to_string().substr(48, 8));
        DMem[Address.to_ulong() + 7] = bitset<8>(WriteData.to_string().substr(56, 8));
    }

    void outputDataMem()
    {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open())
        {
            for (int j = 0; j < 1000; j++)
            {
                dmemout << DMem[j] << endl;
            }
        }
        else
            cout << "Unable to open file";
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
        printstate << "State after executing cycle:\t" << cycle << endl;

        printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
        printstate << "IF.nop:\t" << state.IF.nop << endl;

        printstate << "ID.Instr:\t" << state.ID.Instr << endl;
        printstate << "ID.nop:\t" << state.ID.nop << endl;

        printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
        printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
        printstate << "EX.Imm:\t" << state.EX.Imm << endl;
        printstate << "EX.Rs:\t" << state.EX.Rs << endl;
        printstate << "EX.Rt:\t" << state.EX.Rt << endl;
        printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
        printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
        printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
        printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
        printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
        printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
        printstate << "EX.nop:\t" << state.EX.nop << endl;

        printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
        printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
        printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
        printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
        printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
        printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
        printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
        printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
        printstate << "MEM.nop:\t" << state.MEM.nop << endl;

        printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
        printstate << "WB.Rs:\t" << state.WB.Rs << endl;
        printstate << "WB.Rt:\t" << state.WB.Rt << endl;
        printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
        printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
        printstate << "WB.nop:\t" << state.WB.nop << endl;
    }
    else
        cout << "Unable to open file";
    printstate.close();
}

int main()
{

    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    struct stateStruct state;
    state.IF.nop = false;
    state.ID.nop = true;
    state.EX.nop = true;
    state.MEM.nop = true;
    state.WB.nop = true;
    state.EX.alu_op = true;
    int cycle = 0;

    // bool PCwrite = true;
    // bool IFIDwrite = true;
    // bool branchTaken = false;
    StallTemplate stallTem = StallTemplate();
    bool end = false;

    while (1)
    {
        struct stateStruct newState;
        /* --------------------- WB stage --------------------- */
        if (state.WB.nop == false)
        {
            // 写回寄存器
            if (state.WB.wrt_enable)
            {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
            newState.WB.nop = state.MEM.nop;
        }
        else
        {
            newState.WB.nop = state.MEM.nop;
        }

        /* --------------------- MEM stage --------------------- */
        if (state.MEM.nop == false)
        {
            bitset<32> address = bitset<32>(state.MEM.ALUresult.to_string().substr(32, 32));

            if (state.MEM.wrt_mem)
            {
                myDataMem.writeDataMem(address, state.MEM.Store_data);
            }
            if (state.MEM.rd_mem)
            {
                newState.WB.Wrt_data = myDataMem.readDataMem(address);
            }

            if (!state.MEM.rd_mem)
            {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;
            newState.WB.nop = state.MEM.nop;
        }
        else
        {
            newState.WB.nop = state.MEM.nop;
        }

        /* --------------------- EX stage --------------------- */
        if (state.EX.nop == false)
        {
            // forward unit --> for sd & R
            // 次相邻
            if (!state.WB.nop && state.WB.wrt_enable && (int)(state.WB.Wrt_reg_addr.to_ulong()) != 0)
            {
                if (state.WB.Wrt_reg_addr == state.EX.Rs)
                {
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }
                if (state.WB.Wrt_reg_addr == state.EX.Rt)
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                }
            }
            // 相邻
            if (!state.MEM.nop && state.MEM.wrt_enable && (int)(state.MEM.Wrt_reg_addr.to_ulong()) != 0)
            {
                if (state.MEM.Wrt_reg_addr == state.EX.Rs)
                {
                    state.EX.Read_data1 = state.MEM.ALUresult;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rt)
                {
                    state.EX.Read_data2 = state.MEM.ALUresult;
                }
            }

            if (state.EX.is_I_type)
            {
                long int tmp = 0;
                tmp = (long int)(state.EX.Imm.to_ulong()) + (long int)(state.EX.Read_data1.to_ulong());
                newState.MEM.ALUresult = bitset<64>(tmp);
            } // sub
            else if (state.EX.alu_op.to_string() == string("000"))
            {
                long int tmp = 0;
                tmp = (long int)(state.EX.Read_data1.to_ulong()) - (long int)(state.EX.Read_data2.to_ulong());
                newState.MEM.ALUresult = bitset<64>(tmp);
            } // add
            else if (state.EX.alu_op.to_string() == string("001"))
            {
                long int tmp = 0;
                tmp = (long int)(state.EX.Read_data1.to_ulong()) + (long int)(state.EX.Read_data2.to_ulong());
                newState.MEM.ALUresult = bitset<64>(tmp);
            } // and
            else if (state.EX.alu_op.to_string() == string("010"))
            {
                newState.MEM.ALUresult = state.EX.Read_data1 & state.EX.Read_data2;
            } // or
            else if (state.EX.alu_op.to_string() == string("011"))
            {
                newState.MEM.ALUresult = state.EX.Read_data1 | state.EX.Read_data2;
            } // xor
            else if (state.EX.alu_op.to_string() == string("100"))
            {
                newState.MEM.ALUresult = state.EX.Read_data1 ^ state.EX.Read_data2;
            }

            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            newState.MEM.Store_data = state.EX.Read_data2;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.nop = state.EX.nop;
            // ld+sd
            if (state.MEM.rd_mem && state.EX.wrt_mem)
            {
                if (state.MEM.Wrt_reg_addr == state.EX.Rt)
                {
                    newState.MEM.Store_data = newState.WB.Wrt_data;
                }
            }
        }
        else
        {
            newState.MEM.nop = state.EX.nop;
        }

        /* --------------------- ID stage --------------------- */
        bitset<3> aluOp;
        if (state.ID.nop == false)
        {
            // decode
            string ins = state.ID.Instr.to_string();
            string opcode = ins.substr(25, 7);
            string func3 = ins.substr(17, 3);
            string func7 = ins.substr(0, 7);
            bitset<5> tmpRs = bitset<5>(ins.substr(12, 5));
            bitset<5> tmpRt = bitset<5>(ins.substr(7, 5));

            if (opcode == "1111111")
            {
                end = true;
            }

            newState.EX.Rs = bitset<5>(ins.substr(12, 5));
            newState.EX.Rt = bitset<5>(ins.substr(7, 5));
            newState.EX.Wrt_reg_addr = bitset<5>(ins.substr(20, 5));

            // decode之后，读取RF
            newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
            newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);

            // set Imm
            bitset<12> Imm = bitset<12>(ins.substr(0, 12));
            if (ins[0] == '1')
            {
                newState.EX.Imm = bitset<64>(string(52, '1') + Imm.to_string());
            }
            else
            {
                newState.EX.Imm = bitset<64>(string(52, '0') + Imm.to_string());
            }
            // sd
            if (opcode == string("0100011"))
            {
                string tmp = ins.substr(0, 7) + ins.substr(20, 5);
                if (ins[0] == '1')
                {
                    newState.EX.Imm = bitset<64>(string(52, '1') + tmp);
                }
                else
                {
                    newState.EX.Imm = bitset<64>(string(52, '0') + tmp);
                }
            }
            else if (opcode == string("1100011"))
            { // beq
                string tmp = ins.substr(0, 1) + ins.substr(24, 1) + ins.substr(1, 6) + ins.substr(20, 4) /*+ string("0")*/;
                if (ins[0] == '1')
                {
                    newState.EX.Imm = bitset<64>(string(52, '1') + tmp);
                }
                else
                {
                    newState.EX.Imm = bitset<64>(string(52, '0') + tmp);
                }
            }
            else if (opcode == string("1101111"))
            { // jal
                string tmp = ins.substr(0, 1) + ins.substr(12, 8) + ins.substr(11, 1) + ins.substr(1, 10) /*+ string("0")*/;
                if (ins[0] == '1')
                {
                    newState.EX.Imm = bitset<64>(string(44, '1') + tmp);
                }
                else
                {
                    newState.EX.Imm = bitset<64>(string(44, '0') + tmp);
                }
            }

            // set I_type
            newState.EX.is_I_type = false; // addi, ld, sd
            if (opcode == string("0010011") || opcode == string("0000011") || opcode == string("0100011"))
            {
                newState.EX.is_I_type = true;
            }

            // ld
            newState.EX.rd_mem = false;
            if (opcode == string("0000011"))
            {
                newState.EX.rd_mem = true;
            }

            // sd
            newState.EX.wrt_mem = false;
            if (opcode == string("0100011"))
            {
                newState.EX.wrt_mem = true;
            }

            // set wrt_enable
            // beq, jal, sd do not write register
            newState.EX.wrt_enable = true;
            if (opcode == string("1100011") || opcode == string("1101111") || opcode == string("0100011"))
            {
                newState.EX.wrt_enable = false;
            }

            // set alu_op
            if (opcode == string("0110011"))
            {
                // add
                if (func3 == string("000") && func7 == string("0000000"))
                {
                    newState.EX.alu_op = bitset<3>(string("001"));
                }
                // sub
                else if (func3 == string("000") && func7 == string("0100000"))
                {
                    newState.EX.alu_op = bitset<3>(string("000"));
                }
                // and
                else if (func3 == string("111"))
                {
                    newState.EX.alu_op = bitset<3>(string("010"));
                }
                // or
                else if (func3 == string("110"))
                {
                    newState.EX.alu_op = bitset<3>(string("011"));
                }
                // xor
                else if (func3 == string("100"))
                {
                    newState.EX.alu_op = bitset<3>(string("100"));
                }
            }

            // control hazard module
            // branch
            if (opcode == string("1100011"))
            {
                if (newState.EX.Read_data1 != newState.EX.Read_data2)
                {
                    int offset = (int)(newState.EX.Imm.to_ulong());
                    int curPC = (int)(newState.IF.PC.to_ulong()) - 4;
                    int tmp = offset + curPC;
                    newState.IF.PC = bitset<32>(tmp);
                    stallTem.setStall(Id);
                }
            }
            // jal
            else if (opcode == string("1101111"))
            {
                // branchTaken = true;
                stallTem.setStall(Id);
            }

            // LOAD - ADD Hazard
            if (!state.EX.nop && (newState.EX.Rs == state.EX.Wrt_reg_addr || newState.EX.Rt == state.EX.Wrt_reg_addr))
            {
                if (aluOp.to_string() == "001" || state.EX.is_I_type)
                {
                    stallTem.setStall(Ex);
                }
            }

            newState.EX.nop = state.ID.nop;
        }
        else
        {
            newState.EX.nop = state.ID.nop;
        }

        /* --------------------- IF stage --------------------- */
        if (!state.IF.nop)
        {
            if (!stallTem.needStall())
            {
                newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
                // 根据PC取指令
                // PC update module
                int tmp = (int)(state.IF.PC.to_ulong()) + 4;
                newState.IF.PC = bitset<32>(tmp);
                newState.ID.nop = state.IF.nop;
            }
        }
        else
        {
            newState.ID.nop = state.IF.nop;
        }

        /* --------------------- Stall unit--------------------- */

        switch (stallTem.getStallType()) {
            case If: 
                newState.IF.nop = true; 
                break;
            case Id: 
                newState.ID.nop = true;
                newState.IF = state.IF;
                break;
            case Ex: 
                newState.IF = state.IF;
                newState.ID = state.ID;
                newState.EX.nop = true; 
                break;
            case Mem: 
                newState.IF = state.IF;
                newState.ID = state.ID;
                newState.MEM = state.MEM;
                newState.MEM.nop = true; 
                break;
            case Wb: 
                newState.IF = state.IF;
                newState.ID = state.ID;
                newState.MEM = state.MEM;
                newState.WB = state.WB;
                newState.WB.nop = true; 
                break;
            default: 
                break;
        }
        stallTem.clear();
        /*
        if (needStall)
        {
            newState.EX.nop = true;
            needStall = false;
        }
        if (!IFIDwrite)
        {
            newState.ID = state.ID;
            IFIDwrite = true;
        }
        if (!PCwrite)
        {
            newState.IF = state.IF;
            PCwrite = true;
        } 

        if (branchTaken)
        {
            newState.ID.nop = true;
            branchTaken = false;
        }
        */
        if (end)
        {
            newState.ID.nop = true;
            newState.IF.nop = true;
            newState.EX.nop = true;
            end = false;
        }

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;

        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...

        cycle += 1;
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */
    }

    myRF.outputRF();           // dump RF;
    myDataMem.outputDataMem(); // dump data mem

    return 0;
}
