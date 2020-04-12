#include <string.h>
#include <vector> 
#include <map> 
#include <fstream>
#include <iostream>

using namespace std;

void defineRegisters(); // initialize registers
void parseInput(ifstream & inFile); // take the file and parse it to tokens vector
bool readTokensAndErrorChecking(); // read all tokens one by one and check that if there is any error about formatting

template <class E>
bool parseNumber(E & result, string numb); // takes a string and parse it to a number after, convert to decimal
bool isBinaryNumber(const string & numb); // check for if it is binary number
bool isHexNumber(const string & numb);// check for if it is hexadecimal number
bool isDecNumber(const string & numb); // check for if it is decimal number

bool prepareToOperation(string & operand1, string & operand2, int & address1, int & address2, string & dest, string & source, bool & isOffset2); 
// it takes operands and firstly arrange it according to format. After, finds the address of operands with helps of regMap16bit, regMap8bit, wordVariables and byteVariables vectors
template <class E>
bool handleOperation(E type, const vector<string> & token, const string & operand2, int address1, int address2, const string & dest, const string & source, bool isOffset2);
// it takes necessery arguments to check that operands are legal to move forward step according to HYP86 rules
bool operationFunc(const vector<string> & token); // takes token and calls prepareToOperation func and handleOperation func and return success or not
bool getAddress(int & address, string name, string operand); // get address of the operand with helps of regMap16bit, regMap8bit, wordVariables and byteVariables vectors
bool isContainSquareBracket(string & operand, bool & isByte); // // check for if it contains any square bracket to detect if it is address operand
bool isOffset(string & operand); // check for if it is a offset operand or not

template <class E>
void movFunc(const string & type, const int & address, const E & value); // MOV instruction
template <class E>
void addFunc(const string & type, const int & address, const E & value); // ADD instruction
template <class E>
void subFunc(const string & type, const int & address, const E & value); // SUB instruction
template <class E>
void mulFunc(const string & type, const int & address, const E & value); // MUL instruction
template <class E>
bool divFunc(const string & type, const int & address, const E & value); // DIV instruction
template <class E>
void orFunc(const string & type, const int & address, const E & value); // OR instruction
template <class E>
void andFunc(const string & type, const int & address, const E & value); // AND instruction
template <class E>
void xorFunc(const string & type, const int & address, const E & value); // XOR instruction
template <class E>
void notFunc(const string & type, const int & address, const E & value); // NOT instruction
template <class E>
void rclFunc(const string & type, const int & address, const E & value); // RCL instruction
template <class E>
void rcrFunc(const string & type, const int & address, const E & value); // RCR instruction
template <class E>
void shlFunc(const string & type, const int & address, const E & value); // SHL instruction
template <class E>
void shrFunc(const string & type, const int & address, const E & value); // SHR instruction
template <class E>
void pushFunc(const string & type, const int & address, const E & value); // PUSH instruction
template <class E>
void popFunc(const string & type, const int & address, const E & value); // POP instruction
template <class E>
void cmpFunc(const string & type, const int & address, const E & value); // CMP instruction
bool intFunc(const string & operand); // INT instruction

bool jumpFunc(const vector<string> & token); // JUMP instructions

template <class E>
void changeZF(const E & result); // change Zero Flag
template <class E>
void changeSF(const E & result); // change Sign Flag
template <class E>
void changeCF(const E & first, const E & second, bool isAddition); // change Carry Flag
template <class E>
void changeOF(E first, E second, bool isAddition); // change Overflow Flag
template <class E>
void changeAF(E first, E second, bool isAddition); // change Auxiliary Flag

bool giveError(); // give error and return false

// global variables ( memory, registers and flags )
unsigned char memory[2 << 15]; // 64K memory
vector<unsigned short> reg16bit {0, 0, 0, 0, 0, 0, 0, (2<<15)-2}; // according to id which given by regMap16bit vector puts value of 16 bit registers
vector<unsigned char *> reg8bit {((unsigned char *)&reg16bit[0]), (((unsigned char *)&reg16bit[0]) + 1), // according to id which given by regMap8bit vector puts value of 8 bit registers to a pointer according to value of 16 bit registers
                                  ((unsigned char *)&reg16bit[1]), (((unsigned char *)&reg16bit[1]) + 1),
                                  ((unsigned char *)&reg16bit[2]), (((unsigned char *)&reg16bit[2]) + 1),
                                  ((unsigned char *)&reg16bit[3]), (((unsigned char *)&reg16bit[3]) + 1)};


vector<vector<string>> tokens; // contains tokens which gained from file and first dimension for line number, second dimension for opcode, operand1, operand2 by that order
map<string, int> labels; // key is label name and value is line number of that label
map<string, int> byteVariables; // key is the name of byte variables and value is location of that variable in the memory
map<string, int> wordVariables; // key is the name of word variables and value is location of that variable in the memory
map<string, int> regMap16bit; // key is name of 16 bit registers ex: AX, BX.. and value is id of that register
map<string, int> regMap8bit; // key is name of 8 bit registers ex: AL, AH.. and value is id of that register

unsigned short PC = 0; // program counter

bool ZF = false; // zero flag
bool CF = false; // carry flag
bool AF = false; // auxillary flag
bool SF = false; // sign flag
bool OF = false; // overflow flag

int main(int argc, char* argv[]) {
   defineRegisters();

   ifstream inFile(argv[1]);
   parseInput(inFile);
   bool success = readTokensAndErrorChecking();
   if(!success) return -1;

   while(!(tokens[PC][0] == "INT" && tokens[PC][1] == "20H")) {
      const vector<string> & token = tokens[PC];
      const string & opcode = token[0];

      if(opcode == "MOV" || opcode == "ADD" || opcode == "SUB" || opcode == "MUL" ||
         opcode == "DIV" || opcode == "RCL" || opcode == "RCR" || opcode == "SHL" ||
         opcode == "SHR" || opcode == "PUSH" || opcode == "POP" || opcode == "OR" ||
         opcode == "AND" || opcode == "XOR" || opcode == "NOT" || opcode == "CMP") { 
         if(!operationFunc(token)) return -1;
      }
            
      else if(opcode == "JZ" || opcode == "JNZ" || opcode == "JE" || opcode == "JNE" || 
         opcode == "JA" || opcode == "JAE" || opcode == "JB" || opcode == "JBE" ||
         opcode == "JNAE" || opcode == "JNB" || opcode == "JNBE" || opcode == "JNC" || opcode == "JC") {
         if(!jumpFunc(token)) return -1;
      }

      else if(opcode == "INT" && intFunc(token[1])) return 0;

      PC++;
   }
   return 0;
}


bool prepareToOperation(string & operand1, string & operand2, int & address1, int & address2, string & dest, string & source, bool & isOffset2) {
   bool isAddress1, isByte = false; 
   isAddress1 = isContainSquareBracket(operand1, isByte);

   if(getAddress(address1, "reg16bit", operand1)) {
      dest = "reg16bit";
      if(isAddress1) {
         if(isByte) dest = "byteVariables";
         else dest = "wordVariables";
         address1 = reg16bit[address1];
      }
   }
   if(getAddress(address1, "reg8bit", operand1)) dest = "reg8bit";
   if(getAddress(address1, "wordVariables", operand1)) dest = "wordVariables";
   if(getAddress(address1, "byteVariables", operand1)) dest = "byteVariables";
   if(isOffset(operand1)) dest = "immediate";
   if(isAddress1 && ((operand1[0] >= '0' && operand1[0] <= '9') || (operand2[0] == '\'' && operand2[2] == '\''))) {
      if(isByte) {
            dest = "byteVariables";
            unsigned short number;
            if(!parseNumber(number, operand1)) return false;
            address1 = number;
         }
         else {
            dest = "wordVariables";
            unsigned short number;
            if(!parseNumber(number, operand1)) return false;
            address1 = number;
         }
   }


   isByte = false;
   bool isAddress2 = isContainSquareBracket(operand2, isByte);
   isOffset2 = isOffset(operand2);
   
   if(getAddress(address2, "reg16bit", operand2)) {
      source = "reg16bit";
      if(isAddress2) {
         if(isByte) source = "byteVariables";
         else source = "wordVariables";
         address2 = reg16bit[address2];
      }
   }
   if(getAddress(address2, "reg8bit", operand2)) source = "reg8bit";
   if(getAddress(address2, "wordVariables", operand2)) source = "wordVariables";
   if(getAddress(address2, "byteVariables", operand2)) {
      source = "byteVariables";
   }
   if((operand2[0] >= '0' && operand2[0] <= '9') || (operand2[0] == '\'' && operand2[2] == '\'')) {
      if(isAddress2) {
         if(isByte) {
            source = "byteVariables";
            unsigned short number;
            if(!parseNumber(number, operand2)) return false;
            address2 = number;
         }
         else {
            source = "wordVariables";
            unsigned short number;
            if(!parseNumber(number, operand2)) return false;
            address2 = number;
         }
      }
      else source = "immediate";
   }
   if(isOffset2) source = "immediate";

   if(isAddress1 && isAddress2) {dest = "wordVariables";source = "wordVariables";}
   return true;
}

bool operationFunc(const vector<string> & token) {
   const string & opcode = token[0]; 
   string operand1 = token[1], operand2;
   int address1, address2;
   string dest, source;
   bool isOffset2;

   if(opcode == "DIV" || opcode == "MUL" || opcode == "NOT" || opcode == "PUSH" || opcode == "POP") operand2 = operand1;
   else operand2 = token[2];

   if(!prepareToOperation(operand1, operand2, address1, address2, dest, source, isOffset2))
      return giveError();

   if(opcode == "MUL" || opcode == "DIV") {
      if(source == "wordVariables") dest = "reg16bit";
      if(source == "byteVariables") dest = "reg8bit";
      if(source == "immediate" || isOffset2) return giveError();
   }
   if(opcode == "NOT" && (source == "immediate" || isOffset2)) return giveError();
   if(opcode == "PUSH" && (source == "byteVariables" && source == "reg8bit")) {
      if(source == "immediate") dest = "reg16bit";
      return giveError();
   }

   bool success;
   if(dest == "reg16bit" || dest == "wordVariables")
      success = handleOperation((unsigned short)0, token, operand2, address1, address2, dest, source, isOffset2);   
   if(dest == "reg8bit" || dest == "byteVariables")
      success = handleOperation((unsigned char)0, token, operand2, address1, address2, dest, source, isOffset2);
   
   if(!success) return false;
   return true;
}

template <class E>
bool handleOperation(E type, const vector<string> & token, const string & operand2, int address1, int address2, const string & dest, const string & source, bool isOffset2) {
   const string & opcode = token[0];
   E value;

   if(source == "reg16bit") value = reg16bit[address2];
   else if(source == "reg8bit") value = *reg8bit[address2];
   else if(source == "wordVariables") value = memory[address2] + memory[address2 + 1] * 256;
   else if(source == "byteVariables") value = memory[address2];
   else if(source == "immediate") {
      int temp;
      if((dest == "reg8bit" || dest == "byteVariables") && parseNumber(temp, operand2) && temp > (2 << 7))
         return giveError();   
      if((dest == "reg16bit" || dest == "wordVariables") && parseNumber(temp, operand2) && temp > (2 << 15))
         return giveError();   

      if(isOffset2) value = address2;
      else if(!parseNumber(value, operand2)) return giveError();

      if(token[1].find("[") != string::npos && !(token[1][0] == 'B' || token[1][0] == 'W')) 
         return giveError();         
   }

   if(((dest == "reg16bit" || dest == "wordVariables") && source == "reg16bit") ||
      (dest == "reg16bit" && source == "wordVariables") ||
      ((dest == "reg8bit" || dest == "byteVariables") && source == "reg8bit") ||
      (dest == "reg8bit" && source == "byteVariables") ||
      (source == "immediate" && (dest == "wordVariables" || dest == "byteVariables" || dest == "reg16bit" || dest == "reg8bit"))) {

      if(opcode == "MOV") 
         {movFunc(dest, address1, value); return true;}
      else if(opcode == "ADD")
         {addFunc(dest, address1, value); return true;}
      else if(opcode == "SUB")
         {subFunc(dest, address1, value); return true;}
      else if(opcode == "MUL")
         {mulFunc(dest, address1, value); return true;}
      else if(opcode == "DIV") 
         {if(divFunc(dest, address1, value)) return true;}
      else if(opcode == "AND")
         {andFunc(dest, address1, value); return true;}
      else if(opcode == "OR")
         {orFunc(dest, address1, value); return true;}
      else if(opcode == "XOR")
         {xorFunc(dest, address1, value); return true;}
      else if(opcode == "NOT")
         {notFunc(dest, address1, value); return true;}
      else if(opcode == "RCL")
         {rclFunc(dest, address1, value); return true;}
      else if(opcode == "RCR")
         {rcrFunc(dest, address1, value); return true;}
      else if(opcode == "SHL")
         {shlFunc(dest, address1, value); return true;}
      else if(opcode == "SHR")
         {shrFunc(dest, address1, value); return true;}
      else if(opcode == "CMP")
         {cmpFunc(dest, address1, value); return true;}   
   }
   if(dest == "reg16bit" || dest == "wordVariables") {
      if(opcode == "PUSH")
         {pushFunc(dest, address1, value); return true;}
      else if(opcode == "POP")
         {popFunc(dest, address1, value); return true;}
   }
   return giveError();
}

bool getAddress(int & address, string name, string operand) {
   if(name == "reg16bit") {
      if(regMap16bit.find(operand) != regMap16bit.end()) {
         map<string, int>::iterator iter = regMap16bit.find(operand);
         address = iter->second;
         return true;
      }
      return false;
   }
   if(name == "reg8bit") {
      if(regMap8bit.find(operand) != regMap8bit.end()) {
         map<string, int>::iterator iter = regMap8bit.find(operand);
         address = iter->second;
         return true;       
      }
      return false;
   }
   if(name == "wordVariables") {
      if(wordVariables.find(operand) != wordVariables.end()) {
         map<string, int>::iterator iter = wordVariables.find(operand);
         address = iter->second;
         return true;
      }
      return false;
   }
   if(name == "byteVariables") {
      if(byteVariables.find(operand) != byteVariables.end()) {
         map<string, int>::iterator iter = byteVariables.find(operand);
         address = iter->second;
         return true;
      }
      return false;
   }
}

bool isContainSquareBracket(string & operand, bool & isByte) {
   if(operand.find("[") != string::npos && operand[operand.size() - 1] == ']') {
      if(operand[0] == 'B') {
         operand = operand.substr(1, operand.size());
         isByte = true;
      }
      if(operand[0] == 'W') {
         operand = operand.substr(1, operand.size());
         isByte = false;
      }
      operand = operand.substr(1, operand.size() - 2);
      return true;
   }
   return false;
}
bool isOffset(string & operand) {
   if(operand.substr(0,6) == "OFFSET") {
      operand = operand.substr(7, operand.size());
      return true;
   }
   return false;
}

template <class E>
void movFunc(const string & type, const int & address, const E & value) {
   if(type == "reg16bit") 
      reg16bit[address] = value;
   else if(type == "reg8bit")
      *reg8bit[address] = value;
   else if(type == "wordVariables") {
      memory[address] = (unsigned char)(value % 256);
      memory[address + 1] = (unsigned char)(value / 256);
   }
   else if(type == "byteVariables")
      memory[address] = value;  
}
template <class E>
void addFunc(const string & type, const int & address, const E & value) {
   E result;
   if(type == "reg16bit") {
      result = reg16bit[address] + value;
      reg16bit[address] = result; 
   }
   else if(type == "reg8bit") {
      result = *reg8bit[address] + value;
      *reg8bit[address] = result;
   }
   else if(type == "wordVariables") {
      result = memory[address] + memory[address + 1] * 256 + value;
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   else if(type == "byteVariables") {
      result = memory[address] + value;
      memory[address] = result;
   }

   changeCF((E)(result - value), value, true);
   changeOF((E)(result - value), value, true);
   changeAF((E)(result - value), value, true);
   changeSF(result);
   changeZF(result);
}
template <class E>
void subFunc(const string & type, const int & address, const E & value) {
   E result;
   if(type == "reg16bit") {
      result = reg16bit[address] - value;
      reg16bit[address] = result; 
   }
   else if(type == "reg8bit") {
      result = *reg8bit[address] - value;
      *reg8bit[address] = result;
   }
   else if(type == "wordVariables") {
      result = memory[address] + memory[address + 1] * 256 - value;
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   else if(type == "byteVariables") {
      result = memory[address] - value;
      memory[address] = result;
   }

   changeCF((E)(result + value), value, false);
   changeOF((E)(result + value), value, false);
   changeOF((E)(result + value), value, false);   
   changeSF(result);
   changeZF(result);
}
template <class E>
void mulFunc(const string & type, const int & address, const E & value) {
   if(type == "reg16bit") {
      unsigned long int result = reg16bit[regMap16bit["AX"]] * value;
      reg16bit[regMap16bit["AX"]] = (unsigned int)(result % (2 << 15));
      reg16bit[regMap16bit["DX"]] = (unsigned int)(result / (2 << 15)); 
      
      OF = reg16bit[regMap16bit["DX"]] == 0 ? 1 : 0;
      CF = reg16bit[regMap16bit["DX"]] == 0 ? 1 : 0;
   }
   else if(type == "reg8bit") {
      reg16bit[regMap16bit["AX"]] = *reg8bit[regMap8bit["AL"]] * value;
      OF = *reg8bit[regMap8bit["AH"]] == 0 ? 1 : 0;
      CF = *reg8bit[regMap8bit["AH"]] == 0 ? 1 : 0;   
   }
}
template <class E>
bool divFunc(const string & type, const int & address, const E & value) {
   if(value == 0) return false;

   unsigned short ax = reg16bit[regMap16bit["AX"]];

   if(type == "reg16bit") {
      unsigned int result = ((reg16bit[regMap16bit["DX"]] * (2 << 15)) + reg16bit[regMap16bit["AX"]]) / value;
      if(result > (2 << 15)) return false;
      reg16bit[regMap16bit["AX"]] = (unsigned short)result;
      reg16bit[regMap16bit["DX"]] = ((reg16bit[regMap16bit["DX"]] * (2 << 15)) + ax) % value;
   }
   else if(type == "reg8bit") {
      unsigned short result = reg16bit[regMap16bit["AX"]] / value;
      if(result > (2 << 7)) return false;
      *reg8bit[regMap8bit["AL"]] = (unsigned char)result;
      *reg8bit[regMap8bit["AH"]] = ax % value;
   }
   return true;
}
template <class E>
void orFunc(const string & type, const int & address, const E & value) {
   E result;
   if(type == "reg16bit") {
      result = reg16bit[address] | value;
      reg16bit[address] = result; 
   }
   else if(type == "reg8bit") {
      result = *reg8bit[address] | value;
      *reg8bit[address] = result;
   }
   else if(type == "wordVariables") {
      result = memory[address] + memory[address + 1] * 256 | value;
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   else if(type == "byteVariables") {
      result = memory[address] | value;
      memory[address] = result;
   }

   OF = false;CF = false;AF = false;
   changeSF(result);
   changeZF(result);
}
template <class E>
void andFunc(const string & type, const int & address, const E & value) {
   E result;
   if(type == "reg16bit") {
      result = reg16bit[address] & value;
      reg16bit[address] = result; 
   }
   else if(type == "reg8bit") {
      result = *reg8bit[address] & value;
      *reg8bit[address] = result;
   }
   else if(type == "wordVariables") {
      result = memory[address] + memory[address + 1] * 256 & value;
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   else if(type == "byteVariables") {
      result = memory[address] & value;
      memory[address] = result;
   }

   OF = false;CF = false;AF = false;
   changeSF(result);
   changeZF(result);
}
template <class E>
void xorFunc(const string & type, const int & address, const E & value) {
   E result;
   if(type == "reg16bit") {
      result = reg16bit[address] ^ value;
      reg16bit[address] = result; 
   }
   else if(type == "reg8bit") {
      result = *reg8bit[address] ^ value;
      *reg8bit[address] = result;
   }
   else if(type == "wordVariables") {
      result = memory[address] + memory[address + 1] * 256 ^ value;
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   else if(type == "byteVariables") {
      result = memory[address] ^ value;
      memory[address] = result;
   }

   OF = false;CF = false;AF = false;
   changeSF(result);
   changeZF(result);
}
template <class E>
void notFunc(const string & type, const int & address, const E & value) {
   if(type == "reg16bit") 
      reg16bit[address] = ~reg16bit[address];
   else if(type == "reg8bit") *reg8bit[address] = ~(*reg8bit[address]);
   else if(type == "wordVariables") {
      E temp = ~(memory[address] + memory[address + 1] * 256);
      memory[address] = (unsigned char)(temp % 256);
      memory[address + 1] = (unsigned char)(temp / 256);
   }
   else if(type == "byteVariables") 
      memory[address] = ~memory[address];
}
template <class E>
void rclFunc(const string & type, const int & address, const E & value) {
   E result;
   bool isByte = false;
   unsigned int count = (value % 32);

   if(type == "reg16bit" || type == "wordVariables") {
      result = type == "reg16bit" ? reg16bit[address] : (memory[address] + memory[address + 1] * 256);
      count %= 17;
   }
   else if(type == "reg8bit" || type == "byteVariables") {
      isByte = true;
      result = type == "reg8bit" ? *reg8bit[address] : memory[address];
      count %= 9;
   }

   while(count != 0) {
      bool temp = isByte ? result >> 7 : result >> 15;
      result = (result << 1) + CF;
      CF = temp;
      count--;
   }

   OF = value == 1 ? (isByte ? result >> 7 : result >> 15) ^ CF : (value == 0 ? value : false);

   if(type == "reg16bit") reg16bit[address] = result;
   if(type == "reg8bit") *reg8bit[address] = result;
   if(type == "wordVariables") {
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   if(type == "byteVariables") memory[address] = result;
}
template <class E>
void rcrFunc(const string & type, const int & address, const E & value) {
   E result;
   bool isByte = false;
   unsigned int count = (value % 32);

   if(type == "reg16bit" || type == "wordVariables") {
      result = type == "reg16bit" ? reg16bit[address] : (memory[address] + memory[address + 1] * 256);
      count %= 17;
   }
   else if(type == "reg8bit" || type == "byteVariables") {
      isByte = true;
      result = type == "reg8bit" ? *reg8bit[address] : memory[address];
      count %= 9;
   }

   OF = value == 1 ? (isByte ? result >> 7 : result >> 15) ^ CF : (value == 0 ? value : false);

   while(count != 0) {
      bool temp = result % 2;
      result = (result >> 1) + (isByte ? CF << 7 : CF << 15);
      CF = temp;
      count--;
   }

   if(type == "reg16bit") reg16bit[address] = result;
   if(type == "reg8bit") *reg8bit[address] = result;
   if(type == "wordVariables") {
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   if(type == "byteVariables") memory[address] = result;
}
template <class E>
void shlFunc(const string & type, const int & address, const E & value) {
   E result;
   bool isByte = false;
   unsigned int count = (value % 32);

   if(type == "reg16bit" || type == "wordVariables") {
      result = type == "reg16bit" ? reg16bit[address] : (memory[address] + memory[address + 1] * 256);
      count %= 17;
   }
   else if(type == "reg8bit" || type == "byteVariables") {
      isByte = true;
      result = type == "reg8bit" ? *reg8bit[address] : memory[address];
      count %= 9;
   }

   while(count != 0) {
      CF = isByte ? result >> 7 : result >> 15;
      result = result << 1;
      count--;
   }

   OF = value == 1 ? (isByte ? result >> 7 : result >> 15) ^ CF : (value == 0 ? value : false);
   AF = false;
   changeSF(result);
   changeZF(result);

   if(type == "reg16bit") reg16bit[address] = result;
   if(type == "reg8bit") *reg8bit[address] = result;
   if(type == "wordVariables") {
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   if(type == "byteVariables") memory[address] = result;
}
template <class E>
void shrFunc(const string & type, const int & address, const E & value) {
   E result;
   bool isByte = false;
   unsigned int count = (value % 32);

   if(type == "reg16bit" || type == "wordVariables") {
      result = type == "reg16bit" ? reg16bit[address] : (memory[address] + memory[address + 1] * 256);
      count %= 17;
   }
   else if(type == "reg8bit" || type == "byteVariables") {
      isByte = true;
      result = type == "reg8bit" ? *reg8bit[address] : memory[address];
      count %= 9;
   }

   OF = value == 1 ? (isByte ? result >> 7 : result >> 15) ^ CF : (value == 0 ? value : false);
   
   while(count != 0) {
      CF = result % 2;
      result = result >> 1;
      count--;
   }

   AF = false;
   changeSF(result);
   changeZF(result);

   if(type == "reg16bit") reg16bit[address] = result;
   if(type == "reg8bit") *reg8bit[address] = result;
   if(type == "wordVariables") {
      memory[address] = (unsigned char)(result % 256);
      memory[address + 1] = (unsigned char)(result / 256);
   }
   if(type == "byteVariables") memory[address] = result;
}
template <class E>
void pushFunc(const string & type, const int & address, const E & value) {
   if(type == "reg16bit" || type == "wordVariables") {
      memory[reg16bit[regMap16bit["SP"]]] = (unsigned char)(value % 256);
      memory[reg16bit[regMap16bit["SP"]] + 1] = (unsigned char)(value / 256);
      reg16bit[regMap16bit["SP"]] = reg16bit[regMap16bit["SP"]] - 2;
   }
}
template <class E>
void popFunc(const string & type, const int & address, const E & value) {
   if(type == "reg16bit") {
      reg16bit[address] = memory[reg16bit[regMap16bit["SP"]] + 2] + memory[reg16bit[regMap16bit["SP"]] + 3] * 256;
      reg16bit[regMap16bit["SP"]] = reg16bit[regMap16bit["SP"]] + 2;
   }
   if(type == "wordVariables") {
      memory[address] = memory[reg16bit[regMap16bit["SP"]] + 2];
      memory[address + 1] = memory[reg16bit[regMap16bit["SP"]] + 3];
      reg16bit[regMap16bit["SP"]] = reg16bit[regMap16bit["SP"]] + 2;
   }
}
template <class E>
void cmpFunc(const string & type, const int & address, const E & value) {
   E result;
   if(type == "reg16bit") result = reg16bit[address] - value;
   else if(type == "reg8bit") result = *reg8bit[address] - value;
   else if(type == "wordVariables") result = memory[address] + memory[address + 1] * 256 - value;
   else if(type == "byteVariables") result = memory[address] - value;
   
   changeCF((E)(result + value), value, false);
   changeOF((E)(result + value), value, false);
   changeAF((E)(result + value), value, false);
   changeSF(result);
   changeZF(result);
}
bool intFunc(const string & operand) {
   unsigned int number;
   parseNumber(number, operand);
   if(number == 33) { // 21h
      if(*reg8bit[regMap8bit["AH"]] == 1) {
         string input;
         getline(cin, input);
         *reg8bit[regMap8bit["AL"]] = input[0];
      }
      if(*reg8bit[regMap8bit["AH"]] == 2) {
         cout << *reg8bit[regMap8bit["DL"]];
      }
   }
   if(number == 32) return true; // 20h
   return false;
}

bool jumpFunc(const vector<string> & token) {
   const string & opcode = token[0];
   const string & label = token[1]; 

   if(labels.find(label) == labels.end()) return giveError();

   if(((opcode == "JZ" || opcode == "JE") && ZF == true) || 
      ((opcode == "JNZ" || opcode == "JNE") && ZF == false) ||
      (opcode == "JA" && CF == false && ZF == false) ||
      (opcode == "JAE" && (CF == false || ZF == true)) ||
      (opcode == "JB" && CF == true) ||
      (opcode == "JBE" && (CF == true || ZF == true)) ||
      (opcode == "JNAE" && CF == true) ||
      (opcode == "JNB" && CF == false) ||
      (opcode == "JNBE" && CF == false && ZF == false) ||
      (opcode == "JNC" && CF == false) ||
      (opcode == "JC" && CF == true)) {
         
      PC = labels[label];
   }
   return true;
}

template <class E>
void changeZF(const E & result) {
   if(result == 0) ZF = true;
   else ZF = false;
}
template <class E>
void changeSF(const E & result) {
   SF = *typeid(E).name() == 'h' ? result << 7 : (*typeid(E).name() == 't' ? result << 15 : result << 31);
}
template <class E>
void changeCF(const E & first, const E & second, bool isAddition) {
   unsigned int result = isAddition ? first + second : first - second;
   CF = *typeid(E).name() == 'h' ? result > (2 << 7) : (*typeid(E).name() == 't' ? result > (2 << 15) : false);
}
template <class E>
void changeOF(E first, E second, bool isAddition) {
   first = *typeid(E).name() == 'h' ? first % (2 << 6) : (*typeid(E).name() == 't' ? first % (2 << 14) : false);
   second = *typeid(E).name() == 'h' ? second % (2 << 6) : (*typeid(E).name() == 't' ? second % (2 << 14) : false);
   unsigned int result = isAddition ? first + second : first - second;
   OF = *typeid(E).name() == 'h' ? result > (2 << 6) : (*typeid(E).name() == 't' ? result > (2 << 14) : false);
}
template <class E>
void changeAF(E first, E second, bool isAddition) {
   first = first % (2 << 3);
   second = second % (2 << 3);
   unsigned int result = isAddition ? first + second : first - second;
   AF = result > (2 << 3);
}



void defineRegisters() {
   regMap16bit.insert(pair<string, int>("AX", 0));
   regMap16bit.insert(pair<string, int>("BX", 1));
   regMap16bit.insert(pair<string, int>("CX", 2));
   regMap16bit.insert(pair<string, int>("DX", 3));
   regMap16bit.insert(pair<string, int>("DI", 4));
   regMap16bit.insert(pair<string, int>("SI", 5));
   regMap16bit.insert(pair<string, int>("BP", 6));
   regMap16bit.insert(pair<string, int>("SP", 7));
   
   regMap8bit.insert(pair<string, int>("AL", 0));
   regMap8bit.insert(pair<string, int>("AH", 1));
   regMap8bit.insert(pair<string, int>("BL", 2));
   regMap8bit.insert(pair<string, int>("BH", 3));
   regMap8bit.insert(pair<string, int>("CL", 4));
   regMap8bit.insert(pair<string, int>("CH", 5));
   regMap8bit.insert(pair<string, int>("DL", 6));
   regMap8bit.insert(pair<string, int>("DH", 7));
}

void parseInput(ifstream & inFile) {
   vector<string> blocks;
   string line;

   while(getline(inFile, line)) { 
      if(line.substr(line.size() - 1, line.size()) == "\r") 
         line = line.substr(0, line.size() - 1);
      for (auto & c: line) c = toupper(c);

      blocks.push_back(line);
   }   
   inFile.close();

   tokens.resize(blocks.size() - 2);

   for(int i = 1; i < blocks.size() - 1; i++) {
      string item = blocks[i];
      bool isLabel = item.find(":") != string::npos; // whether contain any ":" charachter
      char * token = strtok( &item[0], " "); // tokenize item
      
      int lastIndex;
      if(isLabel && ((string)token).find(":") == string::npos) {
         tokens[i - 1].push_back((string)token + ":");
         lastIndex = blocks[i].find(":") + 1;
      }
      else {
         tokens[i - 1].push_back(token); // push instruction to array
         lastIndex = blocks[i].find(token) + ((string)token).size(); // find last index of instruction
      }
      item = blocks[i].substr(lastIndex, blocks[i].size()); // remove instruction from line
      token = strtok(&item[0], ",");
      vector<string> tempTokens;
      while (token != NULL) {
         tempTokens.push_back(token);
         token = strtok(NULL, ",");
      }
      for(string tempToken: tempTokens) {
         char * innerToken = strtok(&tempToken[0], " ");
         bool isAdressOperator = tempToken.find("[") != string::npos; // whether contain any '[' charachter
         string resultToken = "";
         while(innerToken != NULL) {
            resultToken += innerToken;
            innerToken = strtok(NULL, " ");
            if(!isAdressOperator && innerToken != NULL) resultToken += " ";
         }
         tokens[i - 1].push_back(resultToken);
      }
   }
}

bool readTokensAndErrorChecking() {
   int memoryCounter = 0;
   for(int i = 0; i < tokens.size(); i++) {
      vector<string> & token = tokens[i];
      string & opcode = token[0];
      if(opcode.find(":") != string::npos) {
         labels.insert(pair<string, int>(opcode.substr(0,opcode.size() - 1), i));
      }
      else if(opcode == "MOV" || opcode == "ADD" || opcode == "SUB" ||  opcode == "XOR" || 
              opcode == "OR" || opcode == "AND" || opcode == "RCL" || opcode == "RCR" ||
              opcode == "SHL" || opcode == "SHR" || opcode == "CMP") {
         if(token.size() != 3) return giveError(); 
         memoryCounter += 6;
      }
      else if(opcode == "MUL" || opcode == "DIV" || opcode == "NOT" || opcode == "PUSH" || 
              opcode == "POP" || opcode == "JZ" || opcode == "JNZ" || opcode == "JE" ||
              opcode == "JNE" || opcode == "JA" || opcode == "JAE" || opcode == "JB" ||
              opcode == "JBE" || opcode == "JNAE" || opcode == "JNB" || opcode == "JNBE" ||
              opcode == "JNC" || opcode == "JC" || opcode == "INT") {
         if(token.size() != 2) return giveError();
         memoryCounter += 6;
      }
      else if(opcode == "NOP") {
         if(token.size() != 1) return giveError();
         memoryCounter += 6;
      }
      else if(token[1].substr(0,2) == "DW" || token[1].substr(0,2) == "DB") {
         if(token.size() != 2) return giveError();
         
         if(token[1].substr(0,2) == "DB") {
            int number;
            if(!parseNumber(number, token[1].substr(3, token[1].size()))) return giveError();
            
            if(number > (2 << 7)) return giveError();
            
            memory[memoryCounter] = (unsigned char)number;
            byteVariables.insert(pair<string,int>(token[0], memoryCounter));
            memoryCounter++;
         }
         if(token[1].substr(0,2) == "DW") {
            int number;
            if(!parseNumber(number, token[1].substr(3, token[1].size()))) return giveError();
            
            if(number > (2 << 15)) return giveError();

            memory[memoryCounter] = (unsigned char)(number % 256);
            memory[memoryCounter + 1] = (unsigned char)(number / 256);         
            wordVariables.insert(pair<string,int>(token[0], memoryCounter));
            memoryCounter += 2;
         }
      }
      else {
         return giveError();
      }
   }
   return true;
}

template <class E>
bool parseNumber(E & result, string numb) {
   int size = numb.size();
   unsigned int control = *typeid(E).name() == 'h' ? 2 << 7 : (*typeid(E).name() == 't' ? 2 << 15 : __INT_MAX__);

   if(numb[0] == '\'' && numb[2] == '\'') {
      result = numb[1];
      return true;
   }

   for(char c : numb) {
      if(!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || c == 'H' || (c == 'X' && c == numb[size - 2]))) return false; // give an error
   }

   if(numb[size - 2] == 'X') {
      if(numb[size - 1] == 'D') {
         if(!isDecNumber(numb.substr(0, size - 2)) || std::stoi(numb.substr(0, size - 2), 0, 10) > control) return false;
         result = std::stoi(numb.substr(0, size - 2), 0, 10);
         return true;
      }
      if(numb[size - 1] == 'B') {
         if(!isBinaryNumber(numb.substr(0, size - 2)) || std::stoi(numb.substr(0, size - 2), 0, 2) > control) return false;
         result = std::stoi(numb.substr(0, size - 2), 0, 2);
         return true;
      }
   }

   if(numb[0] == '0') {
      if(numb[size - 1] == 'H') numb = numb.substr(0, size - 1);
      if(!isHexNumber(numb) || std::stoi(numb, 0, 16) > control) return false;
      
      result = std::stoi(numb, 0, 16);
      return true;
   }
   else if(numb[0] >= '1' && numb[0] <='9') {
      if(numb[size - 1] == 'H') {
        if(!isHexNumber(numb.substr(0, size - 1)) || std::stoi(numb.substr(0, size - 1), 0, 16) > control) return false;
        result = std::stoi(numb.substr(0, size - 1), 0, 16);
        return true;
      }
      
      if(numb[size - 1] == 'B') {
        if(!isBinaryNumber(numb.substr(0, size - 1)) || std::stoi(numb.substr(0, size - 1), 0, 2) > control) return false;
        result = std::stoi(numb.substr(0, size - 1), 0, 2);
        return true;
      } 

      if(numb[size - 1] == 'D') numb = numb.substr(0, size - 1);
      if(!isDecNumber(numb) || std::stoi(numb, 0, 10) > control) return false;
      result = std::stoi(numb, 0, 10);
      return true;
   }
   else {
      return false; // give an error
   }
}
bool isBinaryNumber(const string & numb) {
  int size = numb.size();
  for(char c : numb) {
      if(!((c >= '0' && c <= '1'))) return false; // give an error
   }
   return true;
}
bool isHexNumber(const string & numb) {
   int size = numb.size();
   for(char c : numb) {
      if(!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return false; // give an error
   }
   return true;
}
bool isDecNumber(const string & numb) {
  int size = numb.size();
  for(char c : numb) {
    if(!(c >= '0' && c <= '9')) return false; // give an error
  }
  return true;
}

bool giveError() {
   cout << "Error";
   return false;
}