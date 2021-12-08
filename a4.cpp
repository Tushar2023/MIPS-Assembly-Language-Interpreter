#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <iomanip>
using namespace std;

class Waiting_queue
{
public:
	string instruction;
	string register_name;
	int memory_address;
	Waiting_queue* next = NULL;
	
};

Waiting_queue* head = NULL;

vector<string> insertOrder;
map<string, int> RegisterMap;
map<string, int> label_map;
map<string, int> Register_in_use{
		{"zero",0},
		{"at",0},
		{"v0",0},
		{"v1",0},
		{"a0",0},
		{"a1",0},
		{"a2",0},
		{"a3",0},
		{"t0",0},
		{"t1",0},
		{"t2",0},
		{"t3",0},
		{"t4",0},
		{"t5",0},
		{"t6",0},
		{"t7",0},
		{"s0",0},
		{"s1",0},
		{"s2",0},
		{"s3",0},
		{"s4",0},
		{"s5",0},
		{"s6",0},
		{"s7",0},
		{"t8",0},
		{"t9",0},
		{"k0",0},
		{"k1",0},
		{"gp",0},
		{"sp",0},
		{"s8",0},
		{"ra",0},
	};;
vector<string> InCode;
int CurrentLineNo = 1, TotalInstructions = 0, TotalLines = 0, clk_cycle = 1;
map<string, int> InstructionMap={
		{"add",0},
		{"sub",0},
		{"mul",0},
		{"beq",0},
		{"bne",0},
		{"slt",0},
		{"j",0},
		{"lw",0},
		{"sw",0},
		{"addi",0}
	};
vector<string> insertOrder2 = {"add","sub","mul","beq","bne","slt","j","lw","sw","addi"};
map<int, int> memory;
int ROW_ACCESS_DELAY, COL_ACCESS_DELAY;
int DRAM[1024][1024] = {0};
int row_buffer[1025] = {-1};
bool continue_parallel = true;
int buffer_updates = 0;

void rm_space(string &str)
{
	int32_t j=0;
	while(j<str.size() && (str[j]==' ' || str[j]=='\t'))
	{
		j++;
	}
	str=str.substr(j);
}

void lable_check(string str, int h){
	if(str.find(":")!=-1) //if label encountered
	{
		label_map.insert({str.substr(0,str.find(":")+1), h});
	}
	return;
}

void RemoveComma(string &current_instruction)
{
	if(current_instruction.size()<2 || current_instruction[0]!=',') //check that first element exists and is a comma
	{
		cout<<"Error: Comma expected"<<endl;
		exit(1);
	}
	current_instruction=current_instruction.substr(1); //remove it
}
bool check_comment(string x){
	string y;
	if(x.find("#")!=-1)
	{
		y = x.substr(0,x.find("#"));
	}else{
		y =x;
	}
	
	//remove spaces
	rm_space(y);
	if(y=="") //if there was only comment
	{
		return true;
	}
	return false;

}
string findRegister(string &str, map<string, int> registers)
{

	if(str[0]!='$' || str.size()<2) //find '$' sign
	{
		cout<<"Error: Register expected here"<<endl;
		exit(1);
	}
	bool flag = true;
	str = str.substr(1); //remove '$' sign
	string registerID; //next two characters to match

	if(str.size()>=4){
		if(str.substr(0,4) == "zero"){
			registerID = "zero";
			str = str.substr(4);
			flag = false;}
	}
	if(flag){
		registerID = str.substr(0,2);
		auto itr = registers.find(registerID);
		if(itr == registers.end()){
			cout<<"Error: Invalid register"<<endl;
			exit(1);
		}
		str = str.substr(2);

	}
	return registerID;
	
}
string findLabel(string z)
{
	rm_space(z);
	bool f= false, d= false;
	string tmpStr="";
	for(int i=0;i<z.size();i++)
	{
		if(f==true && d==false && (z[i]==' '||z[i]=='\t')){
				d = true;
		}
		else if(f==true && d==true && !(z[i]==' '||z[i]=='\t')){
				cout<<"Error: Unexpected text after value"<<endl;
				exit(1);
		}
		else if(f==false && !(z[i]==' '||z[i]=='\t')){
				f=true;
				tmpStr+=z[i];	
		}
		else if(f==true && !(z[i]==' '||z[i]=='\t')){
			tmpStr+=z[i];
		}
	}
	return tmpStr;
}
void number_verify(string z)
{
	int start=0;
	char character = z[0];
	if (character=='-') start++;
	for(int i=start;i<z.size();i++)
	{
		if(z[i]<48 || z[i]>57)
		{
			cout<<"Error: Given value is not a valid number"<<endl;
			exit(1);
		}
	}
	if(character!='-'){ 
		if(z.size()>10 || (z.size()==10 && z>"2147483647")){
			cout<<"Error: Out of Range Integer"<<endl;
			exit(1);
		}
	}
	else if(z.size()>11 || (z.size()==11 && z>"-2147483648")){
		cout<<"Error: Out of Range Integer"<<endl;
		exit(0);
	}
}
void add(map<string, int> &RegisterMap, string str1, string str2, string str3)
{
	long long int answer = long(RegisterMap.find(str2)->second)+long(RegisterMap.find(str3)->second);
	if(answer<-2147483648 || answer>2147483647){
		cout<<"Addition Overflow"<<endl;
		exit(1);
	}
	InstructionMap.find("add")->second++;
	RegisterMap[str1]=RegisterMap.find(str2)->second+RegisterMap.find(str3)->second;
	CurrentLineNo++;
	cout<<"cycle "<<clk_cycle<<": $"<<str1<<" = ";
	printf("%X\n", RegisterMap.find(str1)->second);
	clk_cycle++;
}
void sub(map<string, int> &RegisterMap, string str1, string str2, string str3)
{
	long long int answer = long(RegisterMap.find(str2)->second)-long(RegisterMap.find(str3)->second);
	if(answer<-2147483648 || answer>2147483647){
		cout<<"Subtraction Overflow"<<endl;
		exit(1);
	}
	InstructionMap.find("sub")->second++;
	RegisterMap[str1]=RegisterMap.find(str2)->second-RegisterMap.find(str3)->second;
	CurrentLineNo++;
	cout<<"cycle "<<clk_cycle<<": $"<<str1<<" = ";
	printf("%X\n", RegisterMap.find(str1)->second);
	clk_cycle++;
}
void mul(map<string, int> &RegisterMap, string str1, string str2, string str3)
{
	InstructionMap.find("mul")->second++;
	RegisterMap[str1]=RegisterMap.find(str2)->second*RegisterMap.find(str3)->second;
	CurrentLineNo++;
	cout<<"cycle "<<clk_cycle<<": $"<<str1<<" = ";
	printf("%X\n", RegisterMap.find(str1)->second);
	clk_cycle++;
}
void slt(map<string, int> &RegisterMap, string str1, string str2, string str3)
{
	InstructionMap.find("slt")->second++;
	if(RegisterMap.find(str2)->second<RegisterMap.find(str3)->second){
		RegisterMap[str1]=1;
		CurrentLineNo++;

		cout<<"cycle "<<clk_cycle<<": $"<<str1<<" = 1"<<"\n";
		clk_cycle++;
	}
	else{
		RegisterMap[str1]=0;
		CurrentLineNo++;
		cout<<"cycle "<<clk_cycle<<": $"<<str1<<" = 0"<<"\n";
		clk_cycle++;
	}
}
void beq(map<string, int> &RegisterMap, string str1, string str2, string str3)
{
	if(label_map.find(str3) == label_map.end()){
		cout<<"Lable Not Found"<<endl;
		exit(1);
	}
	InstructionMap.find("beq")->second++;
	if(RegisterMap.find(str1)->second==RegisterMap.find(str2)->second){
		CurrentLineNo = label_map.find(str3)->second;
	}
	else{
		CurrentLineNo++;
	}
	cout<<"cycle "<<clk_cycle<<": beq executed"<<"\n";
	clk_cycle++;
}
void bne(map<string, int> &RegisterMap, string str1, string str2, string str3)
{
	if(label_map.find(str3) == label_map.end()){
		cout<<"Lable Not Found"<<endl;
		exit(1);
	}
	InstructionMap.find("bne")->second++;
	if(RegisterMap.find(str1)->second!=RegisterMap.find(str2)->second){
		CurrentLineNo = label_map.find(str3)->second;
	}
	else{
		CurrentLineNo++;
	}
	cout<<"cycle "<<clk_cycle<<": bne executed"<<"\n";
	clk_cycle++;
}
void j(map<string, int> &RegisterMap, string str1)
{
	if(label_map.find(str1) == label_map.end()){
		cout<<"label Not Found"<<endl;
		exit(1);
	}
	InstructionMap.find("j")->second++;
	CurrentLineNo = label_map.find(str1)->second;

	cout<<"cycle "<<clk_cycle<<": j executed"<<"\n";
	clk_cycle++;
}
void addi(map<string, int> &RegisterMap, string str1, string str2, int str3)
{
	long long int answer = long(RegisterMap.find(str2)->second)+long(str3);
	if(answer<-2147483648 || answer>2147483647){
		cout<<"Addition Overflow"<<endl;
		exit(1);
	}
	InstructionMap.find("addi")->second++;
	RegisterMap[str1]=RegisterMap.find(str2)->second+str3;
	CurrentLineNo++;

	cout<<"cycle "<<clk_cycle<<": $"<<str1<<" = ";
	printf("%X\n", RegisterMap.find(str1)->second);
	clk_cycle++;
}
/* ********************************************************************************************************************************* */
/* *********************************** ASSIGNMENT 4 CHNAGES HERE!! ***************************************************************** */
void parallel_executer_lw(map<string, int> &registers, int line_number, vector<string> lines){
	if(check_comment(lines.at(line_number))){
		CurrentLineNo++;
		return;
	}
	string l = lines.at(line_number);

	//remove comments
	if(l.find("#")!=-1) //remove comments
	{
		l = l.substr(0,l.find("#"));
	}
	
	//remove spaces
	rm_space(l);
	if(l=="") //if there was only comment
	{
		return;
	}

	int32_t no_of_register=0,k=0;

	if(l.find(":")!=-1) //if label encountered
	{
		label_map.insert({l.substr(0,l.find(":")+1), CurrentLineNo});
		CurrentLineNo++;
		return;
	}
	if((l.size()<4&&l[0]!='j') || (l[0] == 'j' && l.size()<3)) //no valid instruction is this small
	{
		cout<<"Error: Invalid operation at line:"<<line_number<<endl;
		exit(1);
	}
	for(k=0;k<4;k++) //find length of operation
	{
		if(l[k]==' ' || l[k]=='\t')
		{
			break;
		}
	}
	string operation = l.substr(0,k); //cut the operation out

	if(l.size()>0 && k<l.size()-1)
	{
		l = l.substr(k+1);
	}

	if(operation != "add" && operation != "sub" && operation != "mul" && operation != "slt" && operation != "addi" && operation != "lw" && operation != "sw" && operation != "beq" && operation != "bne" && operation != "j") //if not valid
	{
		cout<<"Error: Invalid operation at line:"<<line_number<<endl;
		exit(1);
	}

	int32_t foundOp=0; //whether valid operation or not
	string register1, register2, register3;
	int imme_value;
	int address;
	
	if(operation == "add" || operation == "sub" || operation == "mul" || operation == "slt"){

		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register3 = findRegister(l, registers);
		rm_space(l);
		
		no_of_register = 3;

		if(l!="") //if something more found
		{
			cout<<"Error: More arguments provided than required at line:"<<line_number<<endl;
			exit(1);
		}
		if(Register_in_use.find(register1)->second == 0 && Register_in_use.find(register2)->second == 0 && Register_in_use.find(register3)->second == 0){
			if(operation == "add"){
				add(registers,register1,register2,register3);
			}else if(operation == "sub"){
				sub(registers,register1,register2,register3);
			}else if(operation == "mul"){
				mul(registers,register1,register2,register3);
			}else{
				slt(registers,register1,register2,register3);
			}
		}else{
			continue_parallel = false;
		}
		
	}else if(operation == "addi"){

		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);

		string tempString = findLabel(l); //find third argument, a number
		number_verify(tempString); //check validity
		imme_value=stoi(tempString); //convert and store
		no_of_register = 2;

		if(Register_in_use.find(register1)->second == 0 && Register_in_use.find(register2)->second == 0){
			addi(registers,register1,register2,imme_value);
		}else{
			continue_parallel = false;
		}
		

	}else if(operation == "lw" || operation == "sw"){

		string tempString="";
		rm_space(l);
		register1 = findRegister(l, registers); //find source/destination register
		rm_space(l);
		RemoveComma(l); //find comma, ignoring extra spaces
		rm_space(l);
		tempString = findLabel(l);
		string temp1,temp2;
		temp1 = tempString.substr(0,tempString.find("("));
		temp2 = tempString.substr(tempString.find("$")+1);
		temp2 = temp2.substr(0,temp2.find(")"));
		if(RegisterMap.find(temp2) == RegisterMap.end()){
			cout<<"Incorrect Register"<<endl;
			exit(1);
		}
		address = stoi(temp1) + RegisterMap.find(temp2)->second;
		no_of_register = 1;


		if(Register_in_use.find(register1)->second == 0){

			// add to queue whether dependent or not????????????????? to be continued from here,,,,,changes in sw have not been made yet so be careful
			Waiting_queue* temp_i = new Waiting_queue();
			if(operation == "lw"){
				temp_i->instruction = "lw";
			}else{
				temp_i->instruction = "sw";
			}
			temp_i->register_name = register1;
			temp_i->memory_address = address;
			if(head->next == NULL){
				head->next = temp_i;
			}else{
				bool flag = false;
				Waiting_queue* temp_node = head;
				while(temp_node->next != NULL){
					temp_node = temp_node->next;
					if(temp_node->memory_address/1024 == temp_i->memory_address/1024){
						flag = true;
						while(temp_node->next != NULL && temp_node->next->memory_address/1024 == temp_i->memory_address/1024){
							temp_node = temp_node->next;
						}
						Waiting_queue* y = temp_node->next;
						temp_node->next = temp_i;
						temp_i->next = y;
					}
					if(flag){
						break;
					}
				}
				if(!flag){
					if(row_buffer[0] == temp_i->memory_address/1024){
						Waiting_queue* x = head->next;
						head->next = temp_i;
						temp_i->next = x;
					}else{
						temp_node->next = temp_i;
					}
				}
			}
			Register_in_use[register1] = 1;
			CurrentLineNo++;
		}else{
			continue_parallel = false;
		}
	}else if(operation == "beq" || operation == "bne"){
		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);

		string tempString="";
		tempString = findLabel(l);
		tempString = tempString + ":";
		//address = stoi(tempString);
		no_of_register = 2;
		if(Register_in_use.find(register1)->second == 0 && Register_in_use.find(register2)->second == 0){
			if(operation == "beq"){
				beq(registers,register1,register2,tempString);
			}else{
				bne(registers,register1,register2,tempString);
			}
		}else{
			continue_parallel = false;
		}
	}else if(operation == "j"){
		
		string tempString="";
		rm_space(l);
		tempString = findLabel(l);
		tempString = tempString + ":";
		//address = stoi(tempString);
		no_of_register = 1;

		j(registers, tempString);
	}else{
		continue_parallel = false;
	}
	return;
}
void parallel_executer_sw(map<string, int> &registers, int line_number, vector<string> lines){
	if(check_comment(lines.at(line_number))){
		CurrentLineNo++;
		return;
	}
	string l = lines.at(line_number);

	//remove comments
	if(l.find("#")!=-1) //remove comments
	{
		l = l.substr(0,l.find("#"));
	}
	
	//remove spaces
	rm_space(l);
	if(l=="") //if there was only comment
	{
		return;
	}

	int32_t no_of_register=0,k=0;

	if(l.find(":")!=-1) //if label encountered
	{
		label_map.insert({l.substr(0,l.find(":")+1), CurrentLineNo});
		CurrentLineNo++;
		return;
	}
	if((l.size()<4&&l[0]!='j') || (l[0] == 'j' && l.size()<3)) //no valid instruction is this small
	{
		cout<<"Error: Invalid operation at line:"<<line_number<<endl;
		exit(1);
	}
	for(k=0;k<4;k++) //find length of operation
	{
		if(l[k]==' ' || l[k]=='\t')
		{
			break;
		}
	}
	string operation = l.substr(0,k); //cut the operation out

	if(l.size()>0 && k<l.size()-1)
	{
		l = l.substr(k+1);
	}

	if(operation != "add" && operation != "sub" && operation != "mul" && operation != "slt" && operation != "addi" && operation != "lw" && operation != "sw" && operation != "beq" && operation != "bne" && operation != "j") //if not valid
	{
		cout<<"Error: Invalid operation at line:"<<line_number<<endl;
		exit(1);
	}

	int32_t foundOp=0; //whether valid operation or not
	string register1, register2, register3;
	int imme_value;
	int address;
	
	if(operation == "add" || operation == "sub" || operation == "mul" || operation == "slt"){

		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register3 = findRegister(l, registers);
		rm_space(l);
		
		no_of_register = 3;

		if(l!="") //if something more found
		{
			cout<<"Error: More arguments provided than required at line:"<<line_number<<endl;
			exit(1);
		}
		if(operation == "add"){
			add(registers,register1,register2,register3);
		}else if(operation == "sub"){
			sub(registers,register1,register2,register3);
		}else if(operation == "mul"){
			mul(registers,register1,register2,register3);
		}else{
			slt(registers,register1,register2,register3);
		}
		
	}else if(operation == "addi"){

		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);

		string tempString = findLabel(l); //find third argument, a number
		number_verify(tempString); //check validity
		imme_value=stoi(tempString); //convert and store
		no_of_register = 2;

		addi(registers,register1,register2,imme_value);
		

	}else if(operation == "lw" || operation == "sw"){

		string tempString="";
		rm_space(l);
		register1 = findRegister(l, registers); //find source/destination register
		rm_space(l);
		RemoveComma(l); //find comma, ignoring extra spaces
		rm_space(l);
		tempString = findLabel(l);
		string temp1,temp2;
		temp1 = tempString.substr(0,tempString.find("("));
		temp2 = tempString.substr(tempString.find("$")+1);
		temp2 = temp2.substr(0,temp2.find(")"));
		if(RegisterMap.find(temp2) == RegisterMap.end()){
			cout<<"Incorrect Register"<<endl;
			exit(1);
		}
		address = stoi(temp1) + RegisterMap.find(temp2)->second;
		no_of_register = 1;


		if(Register_in_use.find(register1)->second == 0){

			// add to queue whether dependent or not????????????????? to be continued from here,,,,,changes in sw have not been made yet so be careful
			Waiting_queue* temp_i = new Waiting_queue();
			if(operation == "lw"){
				temp_i->instruction = "lw";
			}else{
				temp_i->instruction = "sw";
			}
			temp_i->register_name = register1;
			temp_i->memory_address = address;
			if(head->next == NULL){
				head->next = temp_i;
			}else{
				bool flag = false;
				Waiting_queue* temp_node = head;
				while(temp_node->next != NULL){
					temp_node = temp_node->next;
					if(temp_node->memory_address/1024 == temp_i->memory_address/1024){
						flag = true;
						while(temp_node->next != NULL && temp_node->next->memory_address/1024 == temp_i->memory_address/1024){
							temp_node = temp_node->next;
						}
						Waiting_queue* y = temp_node->next;
						temp_node->next = temp_i;
						temp_i->next = y;
					}
					if(flag){
						break;
					}
				}
				if(!flag){
					if(row_buffer[0] == temp_i->memory_address/1024){
						Waiting_queue* x = head->next;
						head->next = temp_i;
						temp_i->next = x;
					}else{
						temp_node->next = temp_i;
					}
				}
			}
			Register_in_use[register1] = 1;
			CurrentLineNo++;
		}else{
			continue_parallel = false;
		}
	}else if(operation == "beq" || operation == "bne"){
		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);

		string tempString="";
		tempString = findLabel(l);
		tempString = tempString + ":";
		//address = stoi(tempString);
		no_of_register = 2;

		if(operation == "beq"){
			beq(registers,register1,register2,tempString);
		}else{
			bne(registers,register1,register2,tempString);
		}
	}else if(operation == "j"){
		
		string tempString="";
		rm_space(l);
		tempString = findLabel(l);
		tempString = tempString + ":";
		//address = stoi(tempString);
		no_of_register = 1;

		j(registers, tempString);
	}else{
		continue_parallel = false;
	}
	return;
}
void lw(map<string, int> &RegisterMap,string str1, int str2){
	if(str2 > 1048576 || str2%4!=0){
		cout<<"Invalid Memory address"<<"\n";
		exit(1);
	}
	InstructionMap.find("lw")->second++;

	int intial_clk_cycle,final_clk_cycle, intial_current_line;
	intial_clk_cycle = clk_cycle;
	cout<<"cycle "<<clk_cycle<<": DRAM request issued"<<"\n";
	Register_in_use[str1] = 1;

	if(row_buffer[0] == -1){

		row_buffer[0] = str2/1024;
		for (int i = 0; i < 1024; ++i)
		{
			row_buffer[i+1] = DRAM[str2/1024][i];
		}
		RegisterMap[str1] = DRAM[str2/1024][str2%1024];
		buffer_updates++;

		final_clk_cycle = clk_cycle + COL_ACCESS_DELAY + ROW_ACCESS_DELAY;
		intial_current_line = CurrentLineNo;

		continue_parallel = true;

		clk_cycle++;
		int no_of_task_performed = 0;
		while(continue_parallel && no_of_task_performed < COL_ACCESS_DELAY + ROW_ACCESS_DELAY && CurrentLineNo < TotalLines){

			parallel_executer_lw(RegisterMap, CurrentLineNo, InCode);
			no_of_task_performed++;
		}
		clk_cycle = final_clk_cycle;
		
	}else if(row_buffer[0] == str2/1024){
		RegisterMap[str1] = row_buffer[(str2%1024)+1];

		final_clk_cycle = clk_cycle + COL_ACCESS_DELAY;
		intial_current_line = CurrentLineNo;
		continue_parallel = true;

		clk_cycle++;
		int no_of_task_performed = 0;
		while(continue_parallel && no_of_task_performed < COL_ACCESS_DELAY && CurrentLineNo < TotalLines){

			parallel_executer_lw(RegisterMap, CurrentLineNo, InCode);
			no_of_task_performed++;
		}
		clk_cycle = final_clk_cycle;
	}else{
		for (int i = 0; i < 1024; ++i)
		{
			DRAM[row_buffer[0]][i] = row_buffer[i+1];
		}

		row_buffer[0] = str2/1024;
		for (int i = 0; i < 1024; ++i)
		{
			row_buffer[i+1] = DRAM[str2/1024][i];
		}
		RegisterMap[str1] = row_buffer[(str2%1024)+1];
		buffer_updates++;

		final_clk_cycle = clk_cycle + COL_ACCESS_DELAY + ROW_ACCESS_DELAY + ROW_ACCESS_DELAY;
		intial_current_line = CurrentLineNo;

		continue_parallel = true;

		clk_cycle++;
		int no_of_task_performed = 0;
		while(continue_parallel && no_of_task_performed < COL_ACCESS_DELAY + ROW_ACCESS_DELAY + ROW_ACCESS_DELAY && CurrentLineNo < TotalLines){

			parallel_executer_lw(RegisterMap, CurrentLineNo, InCode);
			no_of_task_performed++;
		}
		clk_cycle = final_clk_cycle;
	}
	cout<<"cycle "<<intial_clk_cycle+1<<"-"<<final_clk_cycle<<": $"<<str1<<" = "<<row_buffer[(str2%1024)+1]<<"\n";
	CurrentLineNo++;
	clk_cycle++;
	Register_in_use[str1] = 0;
}
void sw(map<string, int> &RegisterMap,string str1, int str2){
	if(str2 > 1048576 || str2%4!=0){
		cout<<"Invalid Memory address"<<"\n";
		exit(1);
	}
	InstructionMap.find("sw")->second++;

	int final_clk_cycle, intial_current_line,intial_clk_cycle;
	intial_clk_cycle = clk_cycle;
	cout<<"cycle "<<clk_cycle<<": DRAM request issued"<<"\n";
	Register_in_use[str1] = 1;

	if (row_buffer[0] == str2/1024)
	{
		row_buffer[(str2%1024) + 1] = RegisterMap.find(str1)->second;
		buffer_updates++;

		final_clk_cycle = clk_cycle + COL_ACCESS_DELAY;
		intial_current_line = CurrentLineNo;


		continue_parallel = true;

		clk_cycle++;
		int no_of_task_performed = 0;
		while(continue_parallel && no_of_task_performed < COL_ACCESS_DELAY && CurrentLineNo < TotalLines){

			parallel_executer_sw(RegisterMap, CurrentLineNo, InCode);
			no_of_task_performed++;
		}
		clk_cycle = final_clk_cycle;

	}else if(row_buffer[0] == -1){
		row_buffer[0] = str2/1024;
		for (int i = 0; i < 1024; ++i)
		{
			row_buffer[i+1] = DRAM[str2/1024][i];
		}
		buffer_updates++;

		row_buffer[(str2%1024) + 1] = RegisterMap.find(str1)->second;
		buffer_updates++;
		
		final_clk_cycle = clk_cycle + COL_ACCESS_DELAY + ROW_ACCESS_DELAY;
		intial_current_line = CurrentLineNo;

		continue_parallel = true;

		clk_cycle++;
		int no_of_task_performed = 0;
		while(continue_parallel && no_of_task_performed < COL_ACCESS_DELAY + ROW_ACCESS_DELAY && CurrentLineNo < TotalLines){

			parallel_executer_sw(RegisterMap, CurrentLineNo, InCode);
			no_of_task_performed++;
		}
		clk_cycle = final_clk_cycle;

	}else{
		for (int i = 0; i < 1024; ++i)
		{
			DRAM[row_buffer[0]][i] = row_buffer[i+1];
		}

		row_buffer[0] = str2/1024;
		for (int i = 0; i < 1024; ++i)
		{
			row_buffer[i+1] = DRAM[str2/1024][i];
		}
		buffer_updates++;

		row_buffer[(str2%1024) + 1] = RegisterMap.find(str1)->second;
		buffer_updates++;

		final_clk_cycle = clk_cycle + COL_ACCESS_DELAY + ROW_ACCESS_DELAY + ROW_ACCESS_DELAY;
		intial_current_line = CurrentLineNo;

		continue_parallel = true;

		clk_cycle++;
		int no_of_task_performed = 0;
		while(continue_parallel && CurrentLineNo < intial_current_line + COL_ACCESS_DELAY + ROW_ACCESS_DELAY+ ROW_ACCESS_DELAY && CurrentLineNo < TotalLines){

			parallel_executer_sw(RegisterMap, CurrentLineNo, InCode);
			no_of_task_performed++;
		}
		clk_cycle = final_clk_cycle;
	}
	cout<<"cycle "<<intial_clk_cycle+1<<"-"<<final_clk_cycle<<": memory address "<<str2<<"-"<<str2+3<<" = "<<row_buffer[(str2%1024) +1]<<"\n";
	CurrentLineNo++;
	clk_cycle++;
	Register_in_use[str1] = 0;
}
void displayInstructions (map<string, int> &InstructionMap,vector<string> insertOrder){
	// Iterate over the map using iterator
	cout<<"Instruction Name\tInstruction Count"<<endl;
	// Print the final results.
	for (int i = 0; i < insertOrder.size(); ++i)
	{
    		const string &s = insertOrder[i];
    		cout<<s<<"\t\t\t"<<InstructionMap[s]<<'\n';
	}
	cout<<'\n';
}
void displayRegisters (map<string, int> &RegisterMap,vector<string> insertOrder){
	// Iterate over the map using iterator
	cout<<"Register Name\tRegister Value"<<endl;
	// Print the final results.
	for (int i = 0; i < insertOrder.size(); ++i)
	{
    		const string &s = insertOrder[i];
    		cout<<s<<"\t\t";
    		printf("%X\n", RegisterMap[s]);
	}
	cout<<'\n';
}
void instruction_executer(map<string, int> &registers, int line_number, vector<string> lines){
	string l = lines.at(line_number);

	//remove comments
	if(l.find("#")!=-1) //remove comments
	{
		l = l.substr(0,l.find("#"));
	}
	
	//remove spaces
	rm_space(l);
	if(l=="") //if there was only comment
	{
		return;
	}

	int32_t no_of_register=0,k=0;

	if(l.find(":")!=-1) //if label encountered
	{
		label_map.insert({l.substr(0,l.find(":")+1), CurrentLineNo});
		CurrentLineNo++;
		return;
	}
	if((l.size()<4&&l[0]!='j') || (l[0] == 'j' && l.size()<3)) //no valid instruction is this small
	{
		cout<<"Error: Invalid operation"<<endl;
		exit(1);
	}
	for(k=0;k<4;k++) //find length of operation
	{
		if(l[k]==' ' || l[k]=='\t')
		{
			break;
		}
	}
	string operation = l.substr(0,k); //cut the operation out

	if(l.size()>0 && k<l.size()-1)
	{
		l = l.substr(k+1);
	}

	if(operation != "add" && operation != "sub" && operation != "mul" && operation != "slt" && operation != "addi" && operation != "lw" && operation != "sw" && operation != "beq" && operation != "bne" && operation != "j") //if not valid
	{
		cout<<"Error: Invalid operation"<<endl;
		exit(1);
	}

	int32_t foundOp=0; //whether valid operation or not
	string register1, register2, register3;
	int imme_value;
	int address;
	
	if(operation == "add" || operation == "sub" || operation == "mul" || operation == "slt"){

		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register3 = findRegister(l, registers);
		rm_space(l);
		
		no_of_register = 3;

		if(l!="") //if something more found
		{
			cout<<"Error: More arguments provided than required"<<endl;
			exit(1);
		}
		if(operation == "add"){
			add(registers,register1,register2,register3);
		}else if(operation == "sub"){
			sub(registers,register1,register2,register3);
		}else if(operation == "mul"){
			mul(registers,register1,register2,register3);
		}else{
			slt(registers,register1,register2,register3);
		}
	}else if(operation == "addi"){

		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);

		string tempString = findLabel(l); //find third argument, a number
		number_verify(tempString); //check validity
		imme_value=stoi(tempString); //convert and store
		no_of_register = 2;
		addi(registers,register1,register2,imme_value);

	}else if(operation == "lw" || operation == "sw"){

		string tempString="";
		rm_space(l);
		register1 = findRegister(l, registers); //find source/destination register
		rm_space(l);
		RemoveComma(l); //find comma, ignoring extra spaces
		rm_space(l);
		tempString = findLabel(l);
		string temp1,temp2;
		temp1 = tempString.substr(0,tempString.find("("));
		temp2 = tempString.substr(tempString.find("$")+1);
		temp2 = temp2.substr(0,temp2.find(")"));

		if(RegisterMap.find(temp2) == RegisterMap.end()){
			cout<<"Incorrect Register"<<endl;
			exit(1);
		}
		address = stoi(temp1) + RegisterMap.find(temp2)->second;
		no_of_register = 1;

		if(operation == "lw"){
			lw(registers,register1,address);
		}else{
			sw(registers,register1,address);
		}
	}else if(operation == "beq" || operation == "bne"){
		rm_space(l);
		register1 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);
		register2 = findRegister(l, registers);
		rm_space(l);
		RemoveComma(l);

		rm_space(l);

		string tempString="";
		tempString = findLabel(l);
		tempString = tempString + ":";
		//address = stoi(tempString);
		no_of_register = 2;

		if(operation == "beq"){
			beq(registers,register1,register2,tempString);
		}else{
			bne(registers,register1,register2,tempString);
		}
	}else if(operation == "j"){
		
		string tempString="";
		rm_space(l);
		tempString = findLabel(l);
		tempString = tempString + ":";
		//address = stoi(tempString);
		no_of_register = 1;

		j(registers, tempString);
	}
	return;

}

int main(int argc, char* argv[]){
	insertOrder = {"zero","at","v0","v1","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","s8","ra"};
    RegisterMap={
		{"zero",0},
		{"at",0},
		{"v0",0},
		{"v1",0},
		{"a0",0},
		{"a1",0},
		{"a2",0},
		{"a3",0},
		{"t0",0},
		{"t1",0},
		{"t2",0},
		{"t3",0},
		{"t4",0},
		{"t5",0},
		{"t6",0},
		{"t7",0},
		{"s0",0},
		{"s1",0},
		{"s2",0},
		{"s3",0},
		{"s4",0},
		{"s5",0},
		{"s6",0},
		{"s7",0},
		{"t8",0},
		{"t9",0},
		{"k0",0},
		{"k1",0},
		{"gp",0},
		{"sp",0},
		{"s8",0},
		{"ra",0},
	};
	head = new Waiting_queue();
	head->instruction = "";
	head->register_name = "";
	head->memory_address = -1;
	head->next = NULL;
	ifstream InputStream;
	ROW_ACCESS_DELAY = atoi(argv[2]);
	COL_ACCESS_DELAY = atoi(argv[3]);
	InputStream.open(argv[1],ios::in);
	if(!InputStream)
	{
		cout<<"Error: File does not exist or could not be opened"<<endl;
		exit(1);
	}
	string tmpStr;
	int label_line = 0;
	while(getline(InputStream,tmpStr))
	{
		TotalLines++;
		if(!check_comment(tmpStr)){TotalInstructions++;}
		InCode.push_back(tmpStr);
		lable_check(tmpStr, label_line);
		label_line++;
	}
	InputStream.close();
	while(CurrentLineNo<=TotalLines){
		if(!check_comment(InCode[CurrentLineNo-1])){
			TotalInstructions++;
			//cout<< "Current Line Number: " << to_string(CurrentLineNo) << " " << InCode[CurrentLineNo-1] << "\n";
			instruction_executer(RegisterMap, CurrentLineNo-1, InCode);
			//displayRegisters(RegisterMap,insertOrder);
			
			Waiting_queue* Temp = head;
			if(Temp->next != NULL){CurrentLineNo = CurrentLineNo-1;}
			while(Temp->next != NULL){
				Temp = Temp->next;
				if(Temp->instruction == "lw"){
					lw(RegisterMap, Temp->register_name, Temp->memory_address);
				}else{
					sw(RegisterMap, Temp->register_name, Temp->memory_address);
				}
			}
			head->next = NULL;
		}else{
			CurrentLineNo++;
		}
	}
	cout << "Total number of clock cycles: " << clk_cycle-1 << "\n";
	cout << "Total number of row buffer updates: " << buffer_updates << "\n";
	displayInstructions(InstructionMap,insertOrder2);
}
