#include<bits/stdc++.h>
using namespace std;
typedef long long int ll;
#define pb push_back
#define ff first
#define ss second
#define fast ios_base::sync_with_stdio(false), cin.tie(NULL), cout.tie(NULL);
#define INF (ll) (1e9)
#define mod (ll) (1e9 + 7)
#define endl "\n"


// const int ROW_ACCESS_DELAY = 10;
// const int COL_ACCESS_DELAY = 2;

int master_cycle;
int num_files;
int MRM_count;
int row_buffer_updates;
int ROW_ACCESS_DELAY;
int COL_ACCESS_DELAY;
// vector<int> master_dependencies(32, 0);
vector<vector<int>> memory(1024, vector<int>(1024, 0));
int num_rows;
int busy_till = -1;

void my_insert(vector<int> instruction);
void reorder_MRM(int box_num);

vector<int> ROW_BUFFER(1024, 0);
int ACTIVE_ROW;
void insert(vector<int>a);


// MRM: ---
    // queue (vector<vector<instruction)
    // current running instruction
    // row buffer mein konsi row
    // insert karne ka function

    // 0 -- file number
    // 1 -- current row/ address row
    // 2 -- lw = 1, sw = 0
    // 3 -- register number
    // 4 -- address
    // 5 -- register number 2

vector<vector<vector<int>>>MRM;
// MRM <--dabba <--instruct

// order of boxes
map<int, int> box_index;

struct store_label {
    string label;
    int address;
};

class my_simulator {
public:
        // int row_buffer_updates;
        int new_curr_line;
        my_simulator(string filename, int r, int c);
        void display_curr();
        void display_memory_diff();
        void run();
        void update_memory(); // CHANGE THIS
        int num_instructions = 0;
        vector<int> lw_dependencies;
        vector<int> sw_dependencies;
        int box_no;
        string register_names[32];
        int registers[32];
        int halt;

private:
    // $z0 is zero
    // int ROW_ACCESS_DELAY;
    // int COL_ACCESS_DELAY;
    vector<bool>done;
    set<pair<int, int>>changed;
    int max_allowed;
    string instructs[10] = {"add",
                           "sub",
                           "mul",
                           "beq",
                           "bne",
                           "slt",
                           "j",
                           "lw",
                           "sw",
                           "addi" };
    int curr_instruct;
    int cycle = 0;
    string curr_instruction;
    // int curr_line = 0;
    vector<string>in_program;
    vector<struct store_label> all_labels;

    // vector<vector<int>>store;
    // all initialized with 0s
    // vector<int>ROW_BUFFER;
    // int ACTIVE_ROW = -1;

    void error(int l);
    void string_error(string s);
    void remove_comments(string &s);
    void look_ahead(int line, int r1, int r2, int num_cycles_left); // change
    void add();
    void sub();
    void mul();
    void addi();
    void beq();
    void bne();
    void slt();
    void j();
    void sw();
    void lw();
    int what_instruction();

    int get_register(string &s);
    void remove_comma(string &s);
};

vector<my_simulator*> simulators(32);
void lw_run(vector<int>a, vector<my_simulator*> &simulators);
void sw_run(vector<int>a, vector<my_simulator*> &simulators);


void ltrim(string &s) {
    int n = s.size();
    int i=0;
    while(i<n && (s[i] == ' ' || s[i] == '\t')) i++;
    s = s.substr(i);
}

void rtrim(string &s) {
    int n = s.size();
    int r=n-1;
    while(s[r] == ' ' || s[r] == '\t') {
        r--;
    }
    s = s.substr(0, r+1);
}

int my_simulator::get_register(string &s) {
    ltrim(s);
    if(s[0] != '$') {
        cout<<"error: register name wrong. \n";
        cout<<s<<endl;
        error(new_curr_line);
    }
    s = s.substr(1);
    string r_name = s.substr(0, 2);

    if(r_name == "ze") {
        string rest = s.substr(2, 2);
        if(rest != "ro") {
            cout << "Error: invalid register name. \n";
            cout<<s<<endl;
            error(new_curr_line);
        } else {
            s = s.substr(4);
            ltrim(s);
            return 0;
        }
    }
    for(int i=1; i<32; i++) {
        if(r_name == register_names[i]) {
            s = s.substr(2);
            ltrim(s);
            return i;
        }
    }
    cout<<"error: register name wrong. \n";
    cout<<s<<endl;
    error(new_curr_line);
    return -1;
}

void my_simulator::remove_comma(string &s) {
    ltrim(s);
    if(s[0] != ',') {
        cout<<"Syntax error: missing comma. \n";
        error(new_curr_line);
    }
    s = s.substr(1);
    ltrim(s);
}

my_simulator::my_simulator(string path, int r, int c) {
    new_curr_line = 0;
    // cycle = 0;
    num_instructions = 0;
    halt = 0;
    // row_buffer_updates = 0;
    // max_allowed = 196608;
    // ACTIVE_ROW = -1;
    // ROW_BUFFER.assign(1024, 0);
    // store.assign(1024, vector<int>(1024, 0));
    lw_dependencies.assign(32, 0);
    sw_dependencies.assign(32, 0);
    for(int i=0; i<32; i++) {
        registers[i] = 0;
        lw_dependencies[i] = 0;
        sw_dependencies[i] = 0;
    }
    // string temp_register_names[32] = {"z0", "at", "v0", "v1",
    //                                   "a0", "a1", "a2", "a3",
    //                                   "t0", "t1", "t2", "t3",
    //                                   "t4", "t5", "t6", "t7",
    //                                   "s0", "s1", "s2", "s3",
    //                                   "s4", "s5", "s6", "s7",
    //                                   "t8", "t9", "a4", "a5",
    //                                   "a6", "a7", "a8", "a9"}; // last six not conventional ones

    string temp_register_names[32] = {"zero", "at", "v0", "v1",
                                      "a0", "a1", "a2", "a3",
                                      "t0", "t1", "t2", "t3",
                                      "t4", "t5", "t6", "t7",
                                      "s0", "s1", "s2", "s3",
                                      "s4", "s5", "s6", "s7",
                                      "t8", "t9", "k0", "k1", // chnaged register names
                                      "gp", "sp", "fp", "ra"};
    for(int i=0; i<32; i++) {
        register_names[i] = temp_register_names[i];
    }
    ifstream in_file(path);
    if(!in_file.is_open()) {
        cout<<"Could not open the input.\n";
        exit(1);
    }
    string temp;
    vector <int> prev_for_forwarding (2, -1);
    string prev_sw;
    bool prev_is_lw_sw = 0;
    // (lw/sw), (register number/address)
    //   1/0
    bool done = false;
    int lline = -1;
    while(getline(in_file, temp)) {
        done = false;
        if(temp == "" || temp == "\t") continue;

        remove_comments(temp);
        string temp_copy = temp;
        ltrim(temp_copy);
        rtrim(temp_copy);

        if(temp_copy[0] == 'l' && temp_copy[1] == 'w') {
            if(prev_is_lw_sw) {
                if(prev_for_forwarding[0] == 1) {
                    temp_copy = temp_copy.substr(2);
                    ltrim(temp_copy);
                    int curr_reg = get_register(temp_copy);
                    if(curr_reg == prev_for_forwarding[1]) {
                        // remove last pushed instruction
                        // and add current
                        int size = in_program.size();
                        in_program[size-1] = temp;
                        done = true;
                        // update prev_for_forwarding
                        // no need, it is same
                    } else { // both lw but regs did not match
                        prev_for_forwarding[1] = curr_reg;
                    }
                } else { // prev was sw, curr is lw
                    prev_for_forwarding[0] = 1;
                    temp_copy = temp_copy.substr(2);
                    ltrim(temp_copy);
                    int curr_reg = get_register(temp_copy);
                    prev_for_forwarding[1] = curr_reg;
                }
            } else {
                // set prev_is_lw_sw and prev_for_forwarding
                prev_is_lw_sw = 1;
                prev_for_forwarding[0] = 1;
                temp_copy = temp_copy.substr(2);
                ltrim(temp_copy);
                int curr_reg = get_register(temp_copy);
                prev_for_forwarding[1] = curr_reg;
            }
        } else if(temp_copy[0] == 's' && temp_copy[1] == 'w') {
            temp_copy = temp_copy.substr(2);
            // cout << temp_copy << endl;
            vector<int>r_ind(3, -1);
            ltrim(temp_copy);
            r_ind[0] = get_register(temp_copy);

            ltrim(temp_copy);
            remove_comma(temp_copy);
            ltrim(temp_copy);

            rtrim(temp_copy);

            if(prev_is_lw_sw) {
                if(prev_for_forwarding[0] == 0) { // sw - sw
                    if(temp_copy == prev_sw) {
                        // remove last pushed instruction
                        // and add current
                        
                        int size = in_program.size();
                        // cout << prev_sw << " Hello before updating " <<in_program[size-1] << endl;
                        in_program[size-1] = temp;
                        // cout << temp_copy<< "Hello after updating " <<in_program[size-1] << endl;
                        done = true;
                        // update prev_for_forwarding
                        // no need, it is same
                    } else { // both sw but addresss did not match
                        prev_sw = temp_copy;
                    }
                } else { // prev was lw, curr is sw
                    prev_for_forwarding[0] = 0;
                    prev_sw = temp_copy;
                }
            } else {
                // set prev_is_lw_sw and prev_for_forwarding
                prev_is_lw_sw = 1;
                prev_for_forwarding[0] = 0;
                prev_sw = temp_copy;
            }
        } else {
            prev_is_lw_sw = 0;
            prev_for_forwarding[0] = -1;
            prev_for_forwarding[1] = -1;
            prev_sw = "";
        }

        if(!done) {
            remove_comments(temp);
            if(temp == "") continue;
            int v = temp.find(":");
            if(v < 0) {
                // cout << num_instructions << " " << temp << endl;
                num_instructions++;
                in_program.pb(temp);

                // cout << "Hello cout in not done " << temp << endl;
                continue;
            } // no label found
            // boost::algorithm::trim(temp);
            int n = temp.size();
            int i, r;
            for(i=v-1; i>=0; i--) {
                if(temp[i] != '\t' || temp[i] != ' ') break;
            }
            r = i;
            int l=0;
            for( ; i>=0; i--) {
                if(temp[i] == '\t' || temp[i] == ' ') break;
            }
            l = i+1;
            for( ; i>=0; i--) {
                if(temp[i] != '\t' || temp[i] != ' ') {
                    cout << "error: invalid label name. \n";
                    exit(EXIT_FAILURE);
                }
            }
            string labeln = temp.substr(l, r-l+1);
            store_label lab;
            // lab.address = new_curr_line;
            lab.address = num_instructions;
            // cout << lab.address << endl;
            lab.label = labeln;
            all_labels.pb(lab);

        }
        // if(num_instructions > max_allowed) {
        //  cout << "Error: out of memory, decrease the number of instructions.\n";
        //  error();
        // }

    }

    in_file.close();
    cout<<endl<<"File read and closed. Executing/interpreting now.\n";


    cout<<"Number of non-empty lines: "<<num_instructions<<endl;
    // ROW_ACCESS_DELAY = r;
    // COL_ACCESS_DELAY = c;
}

void my_simulator::run() {
    

    // do new_curr_line instr
    curr_instruction = in_program[new_curr_line];
    remove_comments(curr_instruction); // and set curr_instruction
    ltrim(curr_instruction);
    if(curr_instruction == "" || curr_instruction == "\t") {
        new_curr_line++;
        return;
    }
    cout<<endl<<"Core "<<box_no<<": Current instruction: "<<in_program[new_curr_line]<<endl;
    int instruct = what_instruction();
    if(instruct != 6 && instruct != 3 && instruct != 4) {
        new_curr_line++;
    }


}

int my_simulator::what_instruction() {
    int v = curr_instruction.find(':');
    if(v > 0) {
        cout << "Label.\n";
        // cycle++;
        return -1;
    }
    // label hai

    ltrim(curr_instruction);
    int l=0;
    for(l=0; l<=4; l++) {
        if(curr_instruction[l] == '\t' || curr_instruction[l] == ' ') break;
    }
    string op = curr_instruction.substr(0, l);
    curr_instruction = curr_instruction.substr(l);
    int id =-2;
    for(int i=0; i<10; i++) {
        if(op == instructs[i]) {
            id = i;
            break;
        }
    }
    if(id == -2) {
        // error
        cout<<"error: operation not supported."<<endl;
        cout<<"Input operation: "<<op<<"--> "<<curr_instruction<<endl;
    }
    switch(id) {
        case 0:
            add();
            // display_curr();
            return 0;
        case 1:
            sub();
            // display_curr();
            return 1;
        case 2:
            mul();
            // display_curr();
            return 2;
        case 3:
            beq();
            // display_curr();
            return 3;
        case 4:
            bne();
            // display_curr();
            return 4;
        case 5:
            slt();
            // display_curr();
            return 5;
        case 6:
            j();
            // display_curr();
            return 6;
        case 7:
            lw();
            return 7;
        case 8:
            sw();
            return 8;
        case 9:
            addi();
            // display_curr();
            return 9;
    }
    return id; // reduntant, only to suppress warnings
}

void my_simulator::remove_comments(string &curr_instruction) {
    // curr_instruction = in_program[l];
    int comment = curr_instruction.find("#");
    if(comment>=0) {
        curr_instruction = curr_instruction.substr(0, comment);
    }
    // curr_line = l;
}

void my_simulator::add(){
    // three registers necessary
    ltrim(curr_instruction);
    vector<int>r_ind(3, -1);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[2] = get_register(curr_instruction);


    if(lw_dependencies[r_ind[0]] != 0 ||
        lw_dependencies[r_ind[1]] != 0 ||
        lw_dependencies[r_ind[2]] != 0) {
        // dependent hai // halt karo // chNGE priority
        new_curr_line--;
        halt = 1;
        reorder_MRM(box_no);
    } else {
        if(r_ind[0] != 0) {
            // r[0], r[1], r[2]
            // do r[0] = r[1] + r[2]
            // cout<<registers[r_ind[2]]<<" in add\n";
            halt = 0;
            registers[r_ind[0]] = registers[r_ind[1]] + registers[r_ind[2]];
            cout << "File: " << box_no << " - $" << register_names[r_ind[0]] << " = " << registers[r_ind[0]] << endl;
            // cehck overflow
        } else {
            // modifying zero register error
            cout<<"Cannot modify $z0 register. \n";
            error(new_curr_line);
        }
    }


}

void my_simulator::sub(){
    vector<int>r_ind(3, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[2] = get_register(curr_instruction);

    // r[0], r[1], r[2]
    // do r[0] = r[1] + r[2]
    if(lw_dependencies[r_ind[0]] != 0 ||
        lw_dependencies[r_ind[1]] != 0 ||
        lw_dependencies[r_ind[2]] != 0) {
        // dependent hai // halt karo // chNGE priority
        new_curr_line--;
        halt = 1;
        reorder_MRM(box_no);
    } else {
        if(r_ind[0] != 0) {
            halt = 0;
            registers[r_ind[0]] = registers[r_ind[1]] - registers[r_ind[2]];
            cout << "File: " << box_no << " - $" << register_names[r_ind[0]] << " = " << registers[r_ind[0]] << endl;
            // cehck overflow
        } else {
            // modifying zero register error
            cout<<"Cannot modify $z0 register. \n";
            error(new_curr_line);
        }
    }
}

void my_simulator::mul(){
    vector<int>r_ind(3, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[2] = get_register(curr_instruction);
    // r[0], r[1], r[2]
    // do r[0] = r[1] + r[2]
    if(lw_dependencies[r_ind[0]] != 0 ||
        lw_dependencies[r_ind[1]] != 0 ||
        lw_dependencies[r_ind[2]] != 0) {
        // dependent hai // halt karo // chNGE priority
        new_curr_line--;
        halt = 1;
        reorder_MRM(box_no);
    } else {
        if(r_ind[0] != 0) {
            halt = 0;
            registers[r_ind[0]] = registers[r_ind[1]] * registers[r_ind[2]];
            cout << "File: " << box_no << " - $" << register_names[r_ind[0]] << " = " << registers[r_ind[0]] << endl;
            // cehck overflow
        } else {
            // modifying zero register error
            cout<<"Cannot modify $z0 register. \n";
            error(new_curr_line);
        }
    }
}

void my_simulator::slt(){
    // three registers necessary
    vector<int>r_ind(3, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[2] = get_register(curr_instruction);
    // r[0], r[1], r[2]
    if(lw_dependencies[r_ind[0]] != 0 ||
        lw_dependencies[r_ind[1]] != 0 ||
        lw_dependencies[r_ind[2]] != 0) {
        // dependent hai // halt karo // chNGE priority
        new_curr_line--;
        halt = 1;
        // cout << "F\n";
        reorder_MRM(box_no);
    } else {
        if(r_ind[0] != 0) {
            halt = 0;
            // cout << "registers[r_ind[0]]: " << registers[r_ind[0]] << endl;
            // cout << "registers[r_ind[1]]: " << registers[r_ind[1]] << endl;
            // cout << "registers[r_ind[2]]: " << registers[r_ind[2]] << endl;
            registers[r_ind[0]] = (registers[r_ind[1]] < registers[r_ind[2]]);
            cout << "File: " << box_no << " - $" << register_names[r_ind[0]] << " = " << registers[r_ind[0]] << endl;
        } else {
            // modifying zero register error
            cout<<"Cannot modify $z0 register. \n";
            error(new_curr_line);
        }
    }
}

void my_simulator::addi() {
    vector<int>r_ind(2, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    rtrim(curr_instruction);
    int n = curr_instruction.size();
    if(curr_instruction[0] == '-') {
        // negative int
        for(int i=1; i<n; i++) {
            if(!isdigit(curr_instruction[i])) {
                cout << "Syntax error: Integer value is required with I-format instructions. \n";
                error(new_curr_line);
            }
        }
    } else {
        for(int i=0; i<n; i++) {
            if(!isdigit(curr_instruction[i])) {
                cout << "Syntax error: Integer value is required with I-format instructions. \n";
                error(new_curr_line);
            }
        }
    }
    int val = stoi(curr_instruction); // works with negative as well
    // cout << val << endl;
    if(lw_dependencies[r_ind[0]] != 0 || lw_dependencies[r_ind[1]] != 0) {
        // dependent hai // halt karo // chNGE priority
        new_curr_line--;
        halt = 1;
        reorder_MRM(box_no);
    } else {
        if(r_ind[0] != 0) {
            halt = 0;
            registers[r_ind[0]] = registers[r_ind[1]] + val;
            cout << "File: " << box_no << " - $" << register_names[r_ind[0]] << " = " << registers[r_ind[0]] << endl;
            // cehck overflow
            // cout<<registers[r_ind[0]]<<" in addi\n";
        } else {
            // modifying zero register error
            cout<<"Cannot modify $z0 register. \n";
            error(new_curr_line);
        }
    }
}

void my_simulator::beq() {
    vector<int>r_ind(3, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    rtrim(curr_instruction);
    int n = curr_instruction.size();
    for(int i=0; i<n; i++) {
        if(curr_instruction[i] == ' ' || curr_instruction[i] == '\t') {
            cout<<"Syntax error: 3rd argument should be label name only \n Extra arguments given. \n";
            error(new_curr_line);
        }
    }
    string lab = curr_instruction;
    bool lab_found = 0;
    for(auto label : all_labels) {
        if(lab == label.label) {
            lab_found = 1;
            r_ind[2] = label.address;
            break;
        }
    }
    if(!lab_found) {
        cout << "Invalid label name. \n";
        error(new_curr_line);
    }
    if(lw_dependencies[r_ind[0]] != 0 || lw_dependencies[r_ind[1]] != 0) {
        // dependent hai // halt karo // chNGE priority
        new_curr_line--;
        halt = 1;
        reorder_MRM(box_no);
    } else {
        if(registers[r_ind[0]] == registers[r_ind[1]]) {
            halt = 0;
            new_curr_line = r_ind[2];
            cout << "beq executed." << endl;
        }else{
            new_curr_line++;
        }
    }
    //  else {
    //  curr_line++;
    // }
}

void my_simulator::bne() {
    vector<int>r_ind(3, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);
    rtrim(curr_instruction);
    int n = curr_instruction.size();
    for(int i=0; i<n; i++) {
        if(curr_instruction[i] == ' ' || curr_instruction[i] == '\t') {
            cout<<"Syntax error: 3rd argument should be label name only \n Extra arguments given. \n";
            error(new_curr_line);
        }
    }
    string lab = curr_instruction;
    // cout << "lab in bne: " << lab << endl;
    bool lab_found = 0;
    for(auto label : all_labels) {
        if(lab == label.label) {
            lab_found = 1;
            r_ind[2] = label.address;
            break;
        }
    }
    if(!lab_found) {
        cout << "Invalid label name. \n";
        error(new_curr_line);
    }
    if(lw_dependencies[r_ind[0]] != 0 || lw_dependencies[r_ind[1]] != 0) {
        // dependent hai // halt karo // chNGE priority
        new_curr_line--;
        halt = 1;
        reorder_MRM(box_no);
    } else {
        if(registers[r_ind[0]] != registers[r_ind[1]]) {
            halt = 0;
            new_curr_line = r_ind[2];
            // cout << new_curr_line << endl;
            cout << "bne executed" << endl;
        }else{
            new_curr_line++;
        }
    }
    //  else {
    //  curr_line++;
    // }
}

void my_simulator::lw() {
    vector<int>r_ind(3, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);

    int n = curr_instruction.size();
    int l=0;
    while(l < n && curr_instruction[l] != ' ' && curr_instruction[l] != '\t') {
        if(curr_instruction[l] == '(') break;
        l++;
    }
    if(l == n) {
        cout<<"Expected register after offset. \n";
        error(new_curr_line);
    }
    string st_offset = curr_instruction.substr(0, l);
    int offset = stoi(st_offset);
    curr_instruction = curr_instruction.substr(l);
    // ltrim(curr_instruction);
    if(curr_instruction[0] != '(') {
        cout<<"Expected register after offset. \n";
        error(new_curr_line);
    }
    curr_instruction = curr_instruction.substr(1);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    if(curr_instruction[0] != ')') {
        cout<<"Syntax error: close parenthesis \n";
        error(new_curr_line);
    }
    r_ind[2] = offset;
    int user_addr = registers[r_ind[1]] + r_ind[2];
    int addr = box_no * (1024 / num_files) * 1024 + user_addr;

    // ---------------------------// change below this |||||

    int ROW_INDEX = 0;
    int COL_INDEX = 0;
    ROW_INDEX = addr/1024;
    COL_INDEX = addr%1024;
    // changed.insert({ROW_INDEX, COL_INDEX});
    // if(addr%4 != 0 || addr <= 786432 || addr >= 1048575) {
        // cout << "error: accessing memory which is out of bounds. \n";
        // error(new_curr_line);
    // }
    if(ROW_INDEX < box_no * (1024 / num_files) || ROW_INDEX > (box_no +1) * (1024 / num_files) || COL_INDEX < 0 || COL_INDEX > 1023) {
        cout << "error: accessing memory which is out of bounds. \n";
     error(new_curr_line);
    }
    if(COL_INDEX%4 != 0) {
        cout << "Address not multiple of 4.\n";
        error(new_curr_line);
    }
    // cout<<"Cycle number: "<<cycle<<": "<<"DRAM request issued.\n";
    cout<<"Cycle number: "<<master_cycle<<": "<<"Request queued into MRM.\n";

    // 0 -- file number
    // 1 -- current row/ address row
    // 2 -- lw = 1, sw = 0
    // 3 -- address
    // 4 -- register number
    // 5 -- register number 2
    vector<int> instruction;
    instruction.pb(box_no);
    instruction.pb(ROW_INDEX);
    instruction.pb(1);
    instruction.pb(addr);
    instruction.pb(r_ind[0]);
    instruction.pb(r_ind[1]);
    instruction.pb(registers[r_ind[0]]);
    // check lw dependencies and sw dependencies
    if(lw_dependencies[r_ind[0]] != 0 || sw_dependencies[r_ind[0]] != 0) {
        new_curr_line--; //
        halt = 1;
        reorder_MRM(box_no);
    } else {
        if( MRM_count == 32) {
            new_curr_line--;
            halt = 1;
            reorder_MRM(box_no);
        } else {
            cout << "Request inserted in to MRM" << endl;
            halt = 0;
            lw_dependencies[r_ind[0]]++;
            my_insert(instruction);
            // call insert
            MRM_count++;
        }
    }
}

void my_simulator::sw() {
    vector<int>r_ind(3, -1);
    ltrim(curr_instruction);
    r_ind[0] = get_register(curr_instruction);

    ltrim(curr_instruction);
    remove_comma(curr_instruction);
    ltrim(curr_instruction);

    int n = curr_instruction.size();
    int l=0;
    while(l < n && curr_instruction[l] != ' ' && curr_instruction[l] != '\t') {
        if(curr_instruction[l] == '(') break;
        l++;
    }
    if(l == n) {
        cout<<"Expected register after offset. \n";
        error(new_curr_line);
    }
    string st_offset = curr_instruction.substr(0, l);
    int offset = stoi(st_offset);
    curr_instruction = curr_instruction.substr(l);
    ltrim(curr_instruction);
    if(curr_instruction[0] != '(') {
        cout<<"Expected register after offset. \n";
        error(new_curr_line);
    }
    curr_instruction = curr_instruction.substr(1);
    ltrim(curr_instruction);
    r_ind[1] = get_register(curr_instruction);

    ltrim(curr_instruction);
    if(curr_instruction[0] != ')') {
        cout<<"Syntax error: close parenthesis \n";
        error(new_curr_line);
    }
    r_ind[2] = offset;
    int user_addr = registers[r_ind[1]] + r_ind[2];
    int addr = box_no * (1024 / num_files) * 1024 + user_addr;
    // cout << user_addr << endl;
    // cout << addr << endl;


    // -------------------------CHANGE BELOW THIS

    // 0 - 1023
    // 1024 - 2047
    // 2048 - 3071
    // ..
    // ..
    int ROW_INDEX = 0;
    int COL_INDEX = 0;
    ROW_INDEX = addr/1024;
    COL_INDEX = addr%1024;
    
    if(ROW_INDEX < box_no * (1024 / num_files) || ROW_INDEX > (box_no +1) * (1024 / num_files) || COL_INDEX < 0 || COL_INDEX > 1023) {
        cout << "error: accessing memory which is out of bounds. \n";
     error(new_curr_line);
    }
    if(COL_INDEX%4 != 0) {
        cout << "Address not multiple of 4.\n";
        error(new_curr_line);
    }
    cout<<"Cycle number: "<<master_cycle<<": "<<"Request queued into MRM.\n";

    // 0 -- file number
    // 1 -- current row/ address row
    // 2 -- lw = 1, sw = 0
    // 3 -- address
    // 4 -- register number
    // 5 -- register number 2
    vector<int> instruction;
    instruction.pb(box_no);
    instruction.pb(ROW_INDEX);
    instruction.pb(0);
    instruction.pb(addr);
    instruction.pb(r_ind[0]);
    // cout << r_ind[0] << endl;
    instruction.pb(r_ind[1]);
    instruction.pb(registers[r_ind[0]]);
    // check lw dependencies and sw dependencies
    if(lw_dependencies[r_ind[0]] != 0 || sw_dependencies[r_ind[0]] != 0) {
        new_curr_line--; //
        halt = 1;
        // cout << "call reordering\n";
        reorder_MRM(box_no);
        // cout << "done reordering\n";
    } else {
        if( MRM_count == 32) {
            new_curr_line--;
            halt = 1;
            // cout << "call reordering\n";
            reorder_MRM(box_no);
            // cout << "done reordering\n";
        } else {

            sw_dependencies[r_ind[0]]++;
            // cout << "called myinsert\n";
            my_insert(instruction);
            cout << "Request inserted in to MRM" << endl;
            halt = 0;
            // cout << "done myinsert\n";
            // call insert
            MRM_count++;
        }
    }
}

void my_simulator::j() {
    ltrim(curr_instruction);
    rtrim(curr_instruction);
    int n = curr_instruction.size();
    for(int i=0; i<n; i++) {
        if(curr_instruction[i] == ' ') {
            cout << "Invalid label name. \n";
            error(new_curr_line);
        }
    }
    string label = curr_instruction;
    bool found = false;
    int new_line = 0;
    for(int i=0; i<all_labels.size(); i++) {
        if(label == all_labels[i].label) {
            new_line = all_labels[i].address;
            found = true;
            break;
        }
    }
    if(!found) {
        cout << "Invalid label name. \n";
        error(new_curr_line);
    }
    new_curr_line = new_line; // jump
    cout << "j executed" << endl;
}

void my_simulator::display_curr() { // referred stack_overflow fro formatting the print on cmd and printing out hexadecimal
    // if(curr_line <= num_instructions) //display current instruction
    // {
    //  cout<<endl<<"Executing instruction: "<<in_program[curr_line-1]<<endl;
    // }
    cout<<"Cycle number: "<<cycle<<endl;
    // cout<<"Program Counter: "<<(4*curr_line)<<endl<<endl; //display curr_line
    cout<<"Registers:"<<endl<<endl;
    printf("%11s%12s\t\t%10s%12s\n","Register","Value","Register","Value");
    for(int i=0;i<16;i++) //display registers
    {
        printf("%6s[%2d]:%12x\t\t%5s[%2d]:%12x\n",register_names[i].c_str(),i,registers[i],register_names[i+16].c_str(),i+16,registers[i+16]);
    }
    cout<<endl<<endl;
}

void my_simulator::string_error(string s) {
    cout << "Error: In instruction/string: " << s << endl;
    exit(1);
}

void my_simulator::error(int l) {
    // cout<<"Error at line:"<<(l+1)<<":  "<<in_program[l]<<endl<<endl;
    // // display_curr();
    // exit(1);
    if(l == -1) {
        cout<<"Error: Not at specific line, but as a whole. \n";
        exit(1);
    }
    cout<<"Error at line:"<<(l+1)<<":  "<<in_program[l]<<endl<<endl;
    // display_curr();
    exit(1);
}

void my_simulator::display_memory_diff() {
    // cout<<"Memory content at the end of execution: \n";
    // for(auto x : changed) {
    //     int r = x.ff;
    //     int c = x.ss;
    //     int addr = 1024*r + c;
    //     cout<<addr<<"-"<<addr+3<<": "<<store[r][c]<<endl;
    // }
}

void my_simulator::update_memory() {
    // for(int i=0; i<1024; i++) {
    //     store[ACTIVE_ROW][i] = ROW_BUFFER[i];
    // }
    // row_buffer_updates++;
}

// 0 -- file number
// 1 -- current row/ address row
// 2 -- lw = 1, sw = 0
// 3 -- address
// 4 -- register number
// 5 -- register number 2

void sw_run(vector<int> instruction, vector<my_simulator*> &simulators) {
    cout << "Cycle: " << master_cycle <<" Core "<<instruction[0] << ": DRAM request issued. \n";
    // cout << " in sw_run\n";
    int ROW_INDEX = instruction[1];
    // cout << ROW_INDEX << endl;
    int COL_INDEX = instruction[3]%1024;
    int reg = instruction[4];
    // cout << reg << endl;
    if(ACTIVE_ROW == -1) {
        // cout << "line 1239\n";
        for(int i=0; i<1024; i++) {
            ROW_BUFFER[i] = memory[ROW_INDEX][i];
        }
        row_buffer_updates++;
        // cout<<"Cycle number: "<<cycle+1<<"-"<<cycle+ROW_ACCESS_DELAY+COL_ACCESS_DELAY<<": memory address "<<addr<<"-"<<addr+3<<"= "<<registers[r_ind[0]]<<endl;
        // look_ahead(curr_line+1, r_ind[0], r_ind[1], ROW_ACCESS_DELAY+COL_ACCESS_DELAY);
        busy_till = master_cycle + (ROW_ACCESS_DELAY);
        
        ROW_BUFFER[COL_INDEX] = instruction[6];

        busy_till += (COL_ACCESS_DELAY);
        // ACTIVE_ROW = ROW_INDEX;
    } else {
        if(ACTIVE_ROW == ROW_INDEX) {
            // cout<<"Cycle number: "<<cycle+1<<"-"<<cycle+COL_ACCESS_DELAY<<": memory address "<<addr<<"-"<<addr+3<<"= "<<registers[reg]<<endl;
            // look_ahead(curr_line+1, reg, r_ind[1], COL_ACCESS_DELAY);
            ROW_BUFFER[COL_INDEX] = instruction[6];
            busy_till = master_cycle + (COL_ACCESS_DELAY);
        } else {
            for(int i=0; i<1024; i++) {
                memory[ACTIVE_ROW][i] = ROW_BUFFER[i];
                // cout << memory[ACTIVE_ROW][i];
                // cout << ROW_BUFFER[i];
            }
            // cout << endl;
            row_buffer_updates++;
            // ACTIVE_ROW = ROW_INDEX;
            for(int i=0; i<1024; i++) {
                ROW_BUFFER[i] = memory[ROW_INDEX][i];
            }

            row_buffer_updates++;
            // cout<<"Cycle number: "<<cycle+1<<"-"<<cycle+ 2*ROW_ACCESS_DELAY +COL_ACCESS_DELAY<<": memory address "<<addr<<"-"<<addr+3<<"= "<<registers[reg]<<endl;
            // look_ahead(curr_line+1, reg, r_ind[1], 2*ROW_ACCESS_DELAY+COL_ACCESS_DELAY);
            busy_till = master_cycle + (ROW_ACCESS_DELAY);
            busy_till += (ROW_ACCESS_DELAY);
            ROW_BUFFER[COL_INDEX] = instruction[6];
            busy_till += (COL_ACCESS_DELAY);
        }
    }

    ACTIVE_ROW = instruction[1];
    // cout << "active row: " << ACTIVE_ROW << endl;
    // cout << "sw_run finished.\n";
}

void lw_run(vector<int> instruction, vector<my_simulator*> &simulators) {
    cout << "Cycle: " << master_cycle <<" Core "<<instruction[0] << ": DRAM request issued. \n";
    // cout << "in lw\n";
    int ROW_INDEX = instruction[1];
    int COL_INDEX = instruction[3]%1024;
    int reg = instruction[4];
    if(ACTIVE_ROW == -1) {
        for(int i=0; i<1024; i++) {
            ROW_BUFFER[i] = memory[ROW_INDEX][i];
        }
        row_buffer_updates++;
        // cout<<"Cycle number: "<<cycle+1<<"-"<<cycle+ROW_ACCESS_DELAY+COL_ACCESS_DELAY<<": memory address "<<addr<<"-"<<addr+3<<"= "<<ROW_BUFFER[COL_INDEX]<<endl;
        // look_ahead(curr_line+1, r_ind[0], r_ind[1], ROW_ACCESS_DELAY+COL_ACCESS_DELAY);
        busy_till = master_cycle + (ROW_ACCESS_DELAY);
        busy_till += (COL_ACCESS_DELAY);
        simulators[instruction[0]]->registers[reg] = ROW_BUFFER[COL_INDEX];
    } else {
        if(ACTIVE_ROW == ROW_INDEX) {
            // cout<<"Cycle number: "<<cycle+1<<"-"<<cycle+COL_ACCESS_DELAY<<": memory address "<<addr<<"-"<<addr+3<<"= "<<ROW_BUFFER[COL_INDEX]<<endl;
            simulators[instruction[0]]->registers[reg] = ROW_BUFFER[COL_INDEX];
            // look_ahead(curr_line+1, reg, r_ind[1], COL_ACCESS_DELAY);
            busy_till = master_cycle + (COL_ACCESS_DELAY);
        } else {
            for(int i=0; i<1024; i++) {
                memory[ACTIVE_ROW][i] = ROW_BUFFER[i];
            }
            row_buffer_updates++;
            // ACTIVE_ROW = ROW_INDEX;
            for(int i=0; i<1024; i++) {
                ROW_BUFFER[i] = memory[ROW_INDEX][i];
            }
            row_buffer_updates++;
            // cout<<"Cycle number: "<<cycle+1<<"-"<<cycle+2*ROW_ACCESS_DELAY+COL_ACCESS_DELAY<<": memory address "<<addr<<"-"<<addr+3<<"= "<<ROW_BUFFER[COL_INDEX]<<endl;
            // look_ahead(curr_line+1, reg, r_ind[1], 2*ROW_ACCESS_DELAY+COL_ACCESS_DELAY);
            busy_till = master_cycle + (ROW_ACCESS_DELAY);
            busy_till += (ROW_ACCESS_DELAY);
            simulators[instruction[0]]->registers[reg] = ROW_BUFFER[COL_INDEX];
            busy_till += (COL_ACCESS_DELAY);
        }
    }

    // cout << "active row:/ " << ACTIVE_ROW << endl;

    ACTIVE_ROW = instruction[1];
    // cout << "sw_run finished.\n";
}

// 0 -- file number
// 1 -- current row/ address row
// 2 -- lw = 1, sw = 0
// 3 -- address
// 4 -- register number
// 5 -- register number 2

void my_insert(vector<int> instruction) {
    // cout << "in my insert: line 1330\n";
    // cout << instruction.size() << ": this is arg size\n";
    // for(auto v: instruction) {
    //     cout << v << " ";
    // }
    // cout << endl;
    int f_num = instruction[0];
    int ROW_INDEX = instruction[1];
    int addr = instruction[3];
    int box_ind = box_index[f_num];
    // cout << "box_ind: " << box_ind << "\n";
    // cout << "f_num: " << f_num << "\n";

    if(box_ind == -1) {
        //
    }

    // cout << "MRM[box_ind].size(): " << MRM[f_num].size() << "\n";
    // find place to insert in box_ind
    if(MRM[f_num].size() == 0) {
        MRM[f_num].pb(instruction);
        // reorder karna bacha hai bc
        int low_curr_pri = -1;
        for(auto box : box_index) {
            low_curr_pri = max(low_curr_pri, box.ss);
        }
        box_index[f_num] = low_curr_pri + 1;
    } else {
        int n = MRM[f_num].size();
        int ind=-1;
        for(int i=0; i<n; i++) {
            if(MRM[f_num][i][1] == instruction[1]) {
                ind = i;
            }
        }
        if(ind == -1) {
            if(ACTIVE_ROW == ROW_INDEX) {
                MRM[f_num].insert(MRM[f_num].begin(), instruction);
            } else {
                MRM[f_num].pb(instruction);
            }
        } else {
            // isnert after ind
            MRM[f_num].insert(MRM[f_num].begin() + ind + 1, instruction);
        }
    }
}

void reorder_MRM(int box_num) {
    // when delete  -- do after while
    // when halt

    //arguments - halt in which file,
    int k = 1024/num_files;
    int curr_request_file = ACTIVE_ROW/k;

    int zero_priority_box;
    for(auto boxes : box_index) {
        if(boxes.ss == 0) {
            zero_priority_box = boxes.ff;
        }
    }
    if(curr_request_file == zero_priority_box && simulators[zero_priority_box]->halt ==1){
        if (box_num == zero_priority_box)
        {
            int curr_prior = box_index[box_num];

            map<int, int>::iterator it;

            for (it = box_index.begin(); it != box_index.end(); it++)
            {
                if(it->ss != -1) {
                    if (it->ss < curr_prior)
                    {
                        int x = it->ss;
                        x += 1;
                        it->ss = x;
                    }
                }
            }
            box_index[box_num] = 0;
        }else{
            int curr_prior = box_index[box_num];

            map<int, int>::iterator it;

            for (it = box_index.begin(); it != box_index.end(); it++)
            {
                if(it->ss != -1) {
                    if (it->ss < curr_prior && it->ss != 0)
                    {
                        int x = it->ss;
                        x += 1;
                        it->ss = x;
                    }
                }
            }
            box_index[box_num] = 1;
        }
        
        // for(auto box : box_index) {
        //     if(box.ss != -1) {
        //         if(box.ss < curr_prior && box.ss != 0) {
        //             box.ss++;
        //         }
        //     }
        // }
        

    }else{
        int curr_prior = box_index[box_num];

        map<int, int>::iterator it;

        for (it = box_index.begin(); it != box_index.end(); it++)
        {
            if(it->ss != -1) {
                if (it->ss < curr_prior)
                {
                    int x = it->ss;
                    x += 1;
                    it->ss = x;
                }
            }
        }
        // for(auto box : box_index) {
        //     if(box.ss != -1) {
        //         if(box.ss < curr_prior) {
        //             box.ss++;
        //         }
        //     }
        // }
        box_index[box_num] = 0;
    }

}

int main() {
    string path;
    vector<int> do_instruction(7, -1);
    MRM_count = 0;
    ACTIVE_ROW = -1;
    int m, r, c; // num_files = n
    cout<<endl<<"MIPS Simulator"<<endl<<endl;
    cout<<"Enter ROW_ACCESS_DELAY: \n";
    cin>>r;
    ROW_ACCESS_DELAY = r;
    cout<<"Enter COL_ACCESS_DELAY: \n";
    cin>>c;
    COL_ACCESS_DELAY = c;
    cout<<"Enter number of files: \n";
    cin>>num_files;
    cout<<"Enter M: \n";
    cin>>m;


    // MRM.assign(vector<vector<vector<int>>>(num_files, vector<vector<int>>));
    for(int i=0; i<num_files; i++) {
        MRM.pb(vector<vector<int>>());
    }
    busy_till = -1;


    num_rows = 1024/num_files;
    // last file might have extra (<num_rows) rows // 1024%num_files

    vector<string> filenames(num_files);
    for(int i=0; i<num_files; i++) {
        cout<<"Enter file "<<i<<" name: ";
        cin >> filenames[i];
    }

    // my_simulator* simulators = new my_simulator[num_files];
    // vector<my_simulator*> simulators(num_files);

    for (int i = 0; i < num_files; i++) {
        // cout << i << "\n";
        simulators[i] = new my_simulator(filenames[i], r, c);
        // box_index[i] = i;
        box_index[i] = -1;
        // my_simulator temp = new my_simulator(filenames[i], r, c);
        simulators[i]->box_no = i;
        // simulators.pb(temp);
    }

    // for(int i=0; i < num_files; i++) {
    //     cout << simulators[i].in_program[1]<<endl;
    // }



    master_cycle = 1;
    while(master_cycle <= m) { // when m > num of lines in a file // incorporate
        cout <<endl<< "Cycle: " << master_cycle << "\n";
        // cout <<"Mrm count " <<MRM_count << endl;
        // cout << "busy till " << busy_till << endl;
        int file_being_written = do_instruction[0];
        for(int file_num = 0; file_num < num_files; file_num++) {
            // simulators[file_num].run();
            if(simulators[file_num]->num_instructions <= simulators[file_num]->new_curr_line) {
                //
            } else {
                if(master_cycle == busy_till) {
                    // cout << file_num << " " << fil
                    if(file_num != file_being_written) {
                        simulators[file_num]->run();
                        if(simulators[file_num]->halt == 1){
                            cout << "File: " << file_num << " halted!" << endl;
                        }
                    }
                } else {
                    simulators[file_num]->run();
                    if(simulators[file_num]->halt == 1){
                        cout << "File: " << file_num << " halted!" << endl;
                    }
                }
            }
        }

        // loop over all files
            // run one instruction
                // if independent --> execute
                // if dependent

        if(master_cycle == busy_till) {
            // cout << "in if(master_cycle == busy_till) { \n";
            cout << "Cycle: " << master_cycle << ": DRAM request executed." << endl;
            int f_num = do_instruction[0];
            int addr = do_instruction[3];
            int user_addr = addr - f_num * 1024 * (1024/num_files);
            if(do_instruction[2]) { // lw
                cout << simulators[f_num]->register_names[do_instruction[4]] << " Register value changed to: " <<
                simulators[f_num]->registers[do_instruction[4]] << endl;
            } else { // sw
                // cout << addr << endl;
                cout << "Memory address " << user_addr << " - " << user_addr+3 << ": " <<  ROW_BUFFER[addr%1024] << endl;
            }
            // remove lw_dependencies , sw_dependencies --------done
            // do_instruction is jo chal rahi hai
            int register1 = do_instruction[4];
            // int register2 = do_instruction[];
            if(do_instruction[2]) { // lw
                simulators[f_num]->lw_dependencies[register1]--;
            } else { // sw
                simulators[f_num]->sw_dependencies[register1]--;
            }

            MRM[do_instruction[0]].erase(find(MRM[do_instruction[0]].begin(), MRM[do_instruction[0]].end(), do_instruction));
            // MRM[do_instruction[0]].erase(MRM[do_instruction[0]].begin());
            MRM_count--;
        }
        // cout << "after::: if(master_cycle == busy_till) { \n";
        if((master_cycle > busy_till && busy_till > 0) ||
            (master_cycle > busy_till && busy_till == -1 && MRM_count != 0)) {
            // execute dram req
            // update busy_till
            // cout << "in if((master_cycle > busy_till && busy_till > 0) || (master_cycle > busy_till && busy_till == -1 && MRM_count != 0)) { \n";
            int highest_priority_box;
            
            for(auto boxes : box_index) {
                
                if(boxes.ss == 0) {
                    highest_priority_box = boxes.ff;
                }
            }
            if(MRM[highest_priority_box].size() == 0) {
                
                map<int, int>::iterator it;

                for (it = box_index.begin(); it != box_index.end(); it++)
                {
                    if(it->ss != -1) {
                        int x = it->ss;
                        x -= 1;
                        it->ss = x;
                    }
                }
            }

            for(auto boxes : box_index) {
                if(boxes.ss == 0) {
                    highest_priority_box = boxes.ff;
                }
            }

            int flag = 1;
            if(MRM[highest_priority_box].size() != 0){
                do_instruction = MRM[highest_priority_box][0]; // argument for lw_run/sw_run
            } else {
                flag = 0;
            }

            // 0 -- file number
            // 1 -- current row/ address row
            // 2 -- lw = 1, sw = 0
            // 3 -- address
            // 4 -- register number
            // 5 -- register number 2
            if(flag == 1){
                if(do_instruction[2]) {
                // cout << "before lw_run \n";
                    lw_run(do_instruction, simulators);
                } else {
                    // cout << "before sw_run \n";
                    sw_run(do_instruction, simulators);
                }
            } else {
                // don't have to do anything
            }
        } else {
            // cant do anything
        }

        master_cycle++;
    }
    return 0;
}
