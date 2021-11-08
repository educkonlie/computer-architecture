#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 65536 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.


#define FORWARD_UNIT_SUPPORT

/*********************************

these are stage registers. they are the input of this stage, and the output of previous one.
the whole pipeline could look like a series of  producer-consumer mode.

**********************************/

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
	bitset<5>   Rs1;
	bitset<5>   Rs2;
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
	bitset<5>   Rs1;
	bitset<5>   Rs2;    
	bitset<5>   Wrt_reg_addr;
	bool        rd_mem;
	bool        wrt_mem; 
	bool        wrt_enable;    
	bool        nop;
};

struct WBStruct {
	bitset<64>  Wrt_data;
	bitset<5>   Rs1;
	bitset<5>   Rs2;     
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
    bitset<64> ReadData1, ReadData2;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<64>(0);
	/// for test purpose
	///Registers[3] = bitset<64>("01010001000011110101000100001111");
	///Registers[4] = bitset<64>("10100000111111111111111111110001");
	/*
	Registers[1] = bitset<64>(15);
	Registers[3] = bitset<64>(13);
	Registers[5] = bitset<64>(5);
	Registers[6] = bitset<64>(6);
	Registers[15] = bitset<64>(15);
	Registers[4] = bitset<64>(20);
	*/
	///Registers[20] = bitset<64>(20);
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<64> WrtData, bool WrtEnable) //WrtEnable 0 is read else is read and write 
    {
cout << "rs1 " << RdReg1 << " rs2 " << RdReg2 << " rd " << WrtReg << endl;
        // TODO: implement!
        ReadData1 = Registers[RdReg1.to_ulong()];
        ReadData2 = Registers[RdReg2.to_ulong()];
        
        if (WrtEnable)
            Registers[WrtReg.to_ulong()] = WrtData;
        Registers[0] = bitset<64>(0);
    }

    void OutputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open())
        {
            rfout << "A state of RF:" << endl;
            for (int j = 0; j < 32; j++)
            {
                rfout << Registers[j] << endl;
            }

        }
        else cout << "Unable to open file RFresult.txt";
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
if (line[0] == '#')
	continue;
                IMem[i] = bitset<8>(line.substr(0, 8));
cout << IMem[i] << endl;
                i++;
            }

        }
        else cout << "Unable to open file imem.txt";
        imem.close();

    }

    bitset<32> ReadInstr(bitset<32> ReadAddress)
    {
        Instruction = bitset<32>(IMem[ReadAddress.to_ulong()].to_string()+IMem[ReadAddress.to_ulong()+1].to_string()+ \
                    IMem[ReadAddress.to_ulong()+2].to_string()+IMem[ReadAddress.to_ulong()+3].to_string());
        return Instruction;
    }

private:
    vector<bitset<8> > IMem;

};

class DataMem
{
public:
    bitset<64> readdata;
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
				//if (line[0] == '#')
					//continue;
                DMem[i] = bitset<8>(line.substr(0, 8));
                i++;
            }
        }
        else cout << "Unable to open file dmem.txt";
        dmem.close();

    }
    bitset<64> MemoryAccess(bitset<64> Address, bitset<64> WriteData, bool readmem, bool writemem)
    {
        // TODO: implement!
        if(readmem)
        {
            readdata = bitset<64>(DMem[Address.to_ulong()].to_string() + 
									DMem[Address.to_ulong()+1].to_string() + 
									DMem[Address.to_ulong()+2].to_string() + 
									DMem[Address.to_ulong()+3].to_string() +
                                    DMem[Address.to_ulong()+4].to_string() + 
									DMem[Address.to_ulong()+5].to_string() + 
									DMem[Address.to_ulong()+6].to_string() + 
									DMem[Address.to_ulong()+7].to_string());
        }
        else if(writemem)
        {
            readdata = WriteData;
            DMem[Address.to_ulong()] = bitset<8>(readdata.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(readdata.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(readdata.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(readdata.to_string().substr(24,8));
            DMem[Address.to_ulong()+4] = bitset<8>(readdata.to_string().substr(32,8));
            DMem[Address.to_ulong()+5] = bitset<8>(readdata.to_string().substr(40,8));
            DMem[Address.to_ulong()+6] = bitset<8>(readdata.to_string().substr(48,8));
            DMem[Address.to_ulong()+7] = bitset<8>(readdata.to_string().substr(56,8));
        }
        else
        {
            readdata = Address;
        }
        return readdata;
    }

    void OutputDataMem()
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
        else cout << "Unable to open file dmemresult.txt";
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

		if (state.IF.nop == false) {
			printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
			printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
		} else {
			printstate<<"IF is not working......." <<endl;
		}

		if (state.ID.nop == false) {
			printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
			printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
		} else {
			printstate<<"ID is not working......" <<endl;
		}
		if (state.EX.nop == false) {
			printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
			printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
			printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
			printstate<<"EX.Rs1:\t"<<state.EX.Rs1<<endl;
			printstate<<"EX.Rs2:\t"<<state.EX.Rs2<<endl;
			printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
			printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
			printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
			printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
			printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
			printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
			printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        
		} else {
			printstate<<"EX is not working......" <<endl;
		}

		if (state.MEM.nop == false) {
			printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
			printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
			printstate<<"MEM.Rs1:\t"<<state.MEM.Rs1<<endl;
			printstate<<"MEM.Rs2:\t"<<state.MEM.Rs2<<endl;   
			printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
			printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
			printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
			printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
			printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        
		} else {
			printstate<<"MEM is not working......" <<endl;
		}

		if (state.WB.nop == false) {
			printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
			printstate<<"WB.Rs1:\t"<<state.WB.Rs1<<endl;
			printstate<<"WB.Rs2:\t"<<state.WB.Rs2<<endl;        
			printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
			printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
			printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
		} else {
			printstate<<"WB is not working......" <<endl;
		}
	}
	else cout<<"Unable to open file";
	printstate.close();
}


class ALU
{
public:
    bitset<64> ALUresult;
    bitset<64> ALUOperation(bool ALUOP, bitset<64> oprand1, bitset<64> oprand2)
    {
	cout << "op1: " << oprand1.to_ulong() << " op2: " << oprand2.to_ulong() << endl;
        // TODO: implement!
		if (ALUOP) {
			ALUresult = bitset<64>(oprand1.to_ulong() + oprand2.to_ulong());
			cout << "ALU result(add) is" << ALUresult.to_ulong() << endl;
		} else {
			ALUresult = bitset<64>(oprand1.to_ulong() - oprand2.to_ulong());
			cout << "ALU result(sub) is" << ALUresult.to_ulong() << endl;
		}
	return ALUresult;
    }
};

bitset<64> imm_gen(bitset<32> instruction, bool isItype)
{
	//// imm_gen can generate imm from instruction in one cycle, the combined logic is not well known.

	bitset<64> tmp = bitset<64>(0);
	if (isItype) {
		//imm[11:0]
		tmp = bitset<64>(instruction.to_string().substr(0,12)); // if positive, 0 padded
		if (instruction[31] == true) {
			tmp = bitset<64>(string(52, '1') + tmp.to_string().substr(52, 12));
		}
	} else {             ///isStore[0] == 1
		//mm[11:5] rs2 rs1 010 imm[4:0]
		// if positive, 0 padded
		tmp = bitset<64>(instruction.to_string().substr(0, 7) + instruction.to_string().substr(20, 5)); 
		if (instruction[31] == true) {
			tmp = bitset<64>(string(52, '1') + tmp.to_string().substr(52, 12));
		}
	}

	cout << "tmp is " << tmp << endl;
	return tmp;
}

bitset<32> address_extend_gen(bitset<32> instruction)
{
	bitset<32> addressExtend;  
	//imm[13|11:6] rs2 rs1 000 imm(20)[5:2|12]
	if (instruction[31] == true)
		addressExtend = bitset<32>(string(18, '1') + 
				instruction.to_string().substr(0, 1) + 
				instruction.to_string().substr(24, 1) + 
				instruction.to_string().substr(2, 6)  +
				instruction.to_string().substr(20, 4) + string("00"));
	else
		addressExtend = bitset<32>(string(18, '0') + 
				instruction.to_string().substr(0, 1) + 
				instruction.to_string().substr(24, 1) + 
				instruction.to_string().substr(2, 6)  +
				instruction.to_string().substr(20, 4) + string("00"));
	return addressExtend;
}
/************************************
 *  IF ID EX MEM WB
 *     IF ID EX  MEM WB
 *        IF ID  EX  MEM WB
 *           IF  ID  EX  MEM WB
 *               IF  ID  EX  MEM WB
 *                   IF  ID  EX  MEM WB  
 *                       if  id  ex  mem wb
 *                           if  id  ex  mem
 *                               if  id  ex
 *                                   if  id
 *                                       if
 *                                       exit
 * one while === one circle == one column
 * if means state.IF.nop == true
 * 
 *                   
 * **********************************/

int main()
{

	RF myRF;
	INSMem myInsMem;
	DataMem myDataMem;
	ALU myALU;

	struct stateStruct state;
	state.IF.nop = false;
	state.ID.nop = true;
	state.EX.nop = true;
	state.MEM.nop = true;
	state.WB.nop = true;
	state.EX.alu_op = true;

	bitset<32> addressExtend = bitset<32>(0);
	int cycle = 0;
	state.IF.PC = bitset<32>(0);
	bool is_beq = false;

#ifdef FORWARD_UNIT_SUPPORT
	bitset<2> ForwardA = bitset<2>("00");
	bitset<2> ForwardB = bitset<2>("00");
#endif
	bitset<64> alu_oprand1, alu_oprand2;

	while (1) {

		/* --------------------- WB stage --------------------- */
		// 5. Register File Update(Write Back)
		if (state.WB.nop == false) {
			cout << "WB stage" << endl;
			myRF.ReadWrite(state.WB.Rs1, state.WB.Rs2,
					state.WB.Wrt_reg_addr,
					state.WB.Wrt_data, state.WB.wrt_enable);

		} else {
			;//flush
		}

		/* --------------------- MEM stage --------------------- */
		if (state.MEM.nop == false) {
			cout << "MEM stage" << endl;
			state.WB.Wrt_data = myDataMem.MemoryAccess(state.MEM.ALUresult, state.MEM.Store_data, 
											state.MEM.rd_mem, state.MEM.wrt_mem);

			state.WB.Rs1 = state.MEM.Rs1;
			state.WB.Rs2 = state.MEM.Rs2;
			state.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			state.WB.wrt_enable = state.MEM.wrt_enable;

#ifdef FORWARD_UNIT_SUPPORT
			/* forward unit, detection unit II */
			if (state.WB.wrt_enable && (state.WB.Wrt_reg_addr != bitset<5>(0))) {
				if ((state.WB.Wrt_reg_addr == state.EX.Rs1) && 
						(state.MEM.Wrt_reg_addr != state.EX.Rs1))
					ForwardA = bitset<2>("01");
				if ((state.EX.is_I_type == false) && (state.WB.Wrt_reg_addr == state.EX.Rs2) &&
						(state.MEM.Wrt_reg_addr != state.EX.Rs2))
					ForwardB = bitset<2>("01");
			}
#endif
		} else {
			;// flush
		}
		state.WB.nop = state.MEM.nop;


		/* --------------------- EX stage --------------------- */
		if (state.EX.nop == false/* && (state.EX.bubble == false)*/) {

			cout << "EX stage" << endl;

#ifdef FORWARD_UNIT_SUPPORT
			/* Forward unit, excute */
		alu_oprand1 = state.EX.Read_data1;
		alu_oprand2 = (state.EX.rd_mem || state.EX.wrt_mem || state.EX.is_I_type) ?
						state.EX.Imm : state.EX.Read_data2;
		if (ForwardA == bitset<2>("10"))
			alu_oprand1 = state.MEM.ALUresult;
		if (ForwardB == bitset<2>("10"))
			alu_oprand2 = state.MEM.ALUresult;
		if (ForwardA == bitset<2>("01"))
			alu_oprand1 = state.WB.Wrt_data;
		if (ForwardB == bitset<2>("01"))
			alu_oprand2 = state.WB.Wrt_data;
		cout << "ForwardA: " << ForwardA << endl;
		cout << "ForwardB: " << ForwardB << endl;
		ForwardA = ForwardB = bitset<2>("00");
#endif
		
			state.MEM.ALUresult = myALU.ALUOperation(state.EX.alu_op, alu_oprand1, alu_oprand2);
			

			state.MEM.Rs1 = state.EX.Rs1;
			state.MEM.Rs2 = state.EX.Rs2;
			state.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
			state.MEM.wrt_enable = state.EX.wrt_enable;
			state.MEM.Store_data = state.EX.Read_data2;
			state.MEM.rd_mem = state.EX.rd_mem;
			state.MEM.wrt_mem = state.EX.wrt_mem;


#ifdef FORWARD_UNIT_SUPPORT
			/* forward unit, detection unit I */
			if (state.MEM.wrt_enable && (state.MEM.Wrt_reg_addr != bitset<5>(0))) {
				if (state.MEM.Wrt_reg_addr == state.EX.Rs1)
					ForwardA = bitset<2>("10");
				if ((state.EX.is_I_type == false) && (state.MEM.Wrt_reg_addr == state.EX.Rs2))
					ForwardB = bitset<2>("10");
			}
#endif
		} else {
			;// flush
		}
		state.MEM.nop = state.EX.nop; 


		/* --------------------- ID stage --------------------- */

		if (state.ID.nop == false) {
			if (state.ID.Instr.to_ulong() != 0xffffffff) {
				cout << "ID stage" << endl;
				cout << state.ID.Instr.to_string().substr(25, 7) << endl;

				state.EX.alu_op = true; /// default value
				state.EX.rd_mem = state.ID.Instr.to_string().substr(25, 7) == string("0000011");
				state.EX.wrt_mem = state.ID.Instr.to_string().substr(25, 7) == string("0100011");

				is_beq = state.ID.Instr.to_string().substr(25, 7) == string("1100011");

				if (state.EX.rd_mem || state.EX.wrt_mem)
					state.EX.alu_op = true;
				state.EX.is_I_type = state.ID.Instr.to_string().substr(25, 7) == string("0000011") || ///ld
					state.ID.Instr.to_string().substr(25, 7) == string("0010011"); ///addi

				state.EX.wrt_enable = !(state.EX.wrt_mem); 
				if (state.ID.Instr.to_string().substr(25, 7) == string("1100011"))
					state.EX.wrt_enable = false;
				///if (isRType[0] == 1)
				if (state.ID.Instr.to_string().substr(25, 7) == string("0110011")) {
					if(state.ID.Instr.to_string().substr(17, 3) == string("000")) {
						if(state.ID.Instr.to_string().substr(0, 7) == string("0000000"))
							//add
							state.EX.alu_op = true;
						else if(state.ID.Instr.to_string().substr(0, 7) == string("0100000"))
							///sub
							state.EX.alu_op = false;
					}
				} else if (state.ID.Instr.to_string().substr(25, 7) == string("0010011")) {
					//addi
					state.EX.alu_op = true;
				} else {
					;
				}

				cout << "aluOp: " << state.EX.alu_op << endl;

				state.EX.Rs1 = bitset<5>(state.ID.Instr.to_string().substr(12, 5));
				state.EX.Rs2 = bitset<5>(state.ID.Instr.to_string().substr(7, 5));
				state.EX.Wrt_reg_addr  = bitset<5>(state.ID.Instr.to_string().substr(20, 5));
				myRF.ReadWrite(
						state.EX.Rs1, 
						state.EX.Rs2,
						state.EX.Wrt_reg_addr,
						bitset<64>(0), false);
				state.EX.Read_data1 = myRF.ReadData1;
				state.EX.Read_data2 = myRF.ReadData2;
				state.EX.Imm = imm_gen(state.ID.Instr, state.EX.is_I_type);

				addressExtend = address_extend_gen(state.ID.Instr);

				cout << "ReadData1 is " << myRF.ReadData1 << endl;
				cout << "ReadData2 is " << myRF.ReadData2 << endl;
				cout << "2 done      " << endl;
			} else {
				state.IF.nop = true;
			}
		} else {
			//// flush state.EX.
			//// rd_mem should be flushed, otherwise the hazard mechanism will be failed
			state.EX.rd_mem = false;
			state.EX.Read_data1 = state.EX.Read_data2 = bitset<64>(0);
		}
		state.EX.nop = state.ID.nop;

		/* --------------------- IF stage --------------------- */

		if (state.IF.nop == false) {
			cout << "IF stage" << endl;
			cout << "PC: " << state.IF.PC << endl;
			state.ID.Instr = myInsMem.ReadInstr(state.IF.PC);
			cout << "instruction: " << state.ID.Instr << endl;
		}
		state.ID.nop = state.IF.nop;


		/* --------------------- Stall unit--------------------- */

			/*** hazard detection unit in ID stage of ld instruction ***/
			// ld x2, 20(x1)   IF1
			// add x4, x2, x5  IF2
			/* load & use case: reload IF2; adding bubble like below 
			
			IF ID  EX   MEM WB
			   IF1 ID1. EX  MEM WB
			       IF2  O   O   O
				        IF2 .........
			**************************************/
		if (state.EX.rd_mem &&
			 ((state.EX.Wrt_reg_addr == bitset<5>(state.ID.Instr.to_string().substr(12, 5)))
					|| (state.EX.Wrt_reg_addr == bitset<5>(state.ID.Instr.to_string().substr(7, 5))))) {
			state.ID.nop = true; /// insert bubble here
			cout << "load & use case: insert bubble here" << endl;
		/************* beq is very like the same as load & use case
		 * IFb means beq,  IFn means next, IFt means target
		 * 
		 * IF ID   EX   MEM  WB
		 *    IFb  IDb. EX   MEM WB
		 *         IFn  o    o   o    o
		 *              IFt  IDt EXt MEMt WBt
		 * IF ID1.   EX   MEM  WB
		 *    IFb    o    o    o   o
		 *           IFb  IDb. EX  MEM  WB 
		 *                IFn  o   o    o  o
		 *               
		 *	NOTE: 
		 *        beq is exactly branch-if-not-eq, 
		 *		  and address extention should be 2bits in this lab.
		 * **************************************/
		} else if (is_beq &&
				(state.EX.Read_data1 != state.EX.Read_data2)) {
			state.IF.PC = bitset<32>(state.IF.PC.to_ulong() + addressExtend.to_ulong());
			cout << "PC jumps to " << state.IF.PC << endl;
			state.ID.nop = true;
			cout << "branch-not-eq case: insert bubble here" << endl;
		} else {
			state.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
		}

		printState(state, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 

		if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
			break;

				cycle += 1;
		/////state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

	}

	myRF.OutputRF(); // dump RF;	
	myDataMem.OutputDataMem(); // dump data mem 

	return 0;

}


