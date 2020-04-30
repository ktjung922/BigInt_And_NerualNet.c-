#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <time.h>
#include <fstream>

#define MAXBUF 512 // fstream buffer 크기

using namespace std;

/*
#define MAXINT 8
#define LayerNum 6
int LayerNodeNum[LayerNum] = {3, 20, 50, 30, 70, 10};
unsigned int weightKey[LayerNum - 1] = { 12, 20, 25, 35, 40 };
int input[3] = { 123456,324563, 323453 };
*/

#define MAXINT 8 // 해당 값으로 BigInt 총 크기 조절

/* 전역 변수값들 */
int LayerNum; 
int LayerNodeNum[MAXBUF]; 
unsigned int weightKey[MAXBUF]; 
int input[MAXBUF]; 

/* BigInt Class (set, get, print) */
class BigInt {
private:
	unsigned int intNode[MAXINT];
	bool overflow;
public:
	BigInt() {
		overflow = 0;
		for (int i = 0; i < MAXINT; i++) {
			intNode[i] = 0;
		}
	}
	BigInt(unsigned int a) {
		overflow = 0;
		for (int i = 0; i < MAXINT; i++) {
			intNode[i] = a;
		}
	}
	void setIntNode(int num, unsigned int key) {
		intNode[num] = key;
	}
	unsigned int getIntNode(unsigned int num) {
		return intNode[num];
	}
	string print() {
		unsigned int tempNode[MAXINT + 1];
		int flag = 0;
		ostringstream sout;
		for (int i = 0; i < MAXINT + 1; i++) {
			tempNode[i] = 0;
		}
		for (int j = MAXINT - 1; j > -1; j--){
			for (int i = 31; i > -1; i--) {
				for (int k = MAXINT; k > -1; k--) {
					if (tempNode[k] >= 500000000) {
						tempNode[k] += 1647483648;
					}
					if (k != MAXINT) {
						if ((tempNode[k] & 0x80000000) != 0) {
							tempNode[k + 1] |= 0x00000001;
						}
					}
					tempNode[k] <<= 1;
				}
				if ((intNode[j] & (1 << i)) != 0) {
					tempNode[0] |= 0x00000001;
				}
			}
		}

		for (int i = MAXINT; i > -1; i--) {
			if (flag == 1) {
				sout << setfill('0') << setw(9) << to_string(tempNode[i]);
			}
			if (tempNode[i] != 0) {
				sout << to_string(tempNode[i]);
				flag = 1;
			}
		}
		return sout.str();
	}
	string print32() { // 2^32형태로 출력
		int flag = 0;
		ostringstream sout;
		for (int i = MAXINT - 1; i > -1; i--) {
			if (intNode[i] != 0) {
				flag = 1;
			}
			if (flag == 1) {
				sout << setfill('0') << setw(10) << to_string(intNode[i]);
			}
		}
		return sout.str();
	}
};
/* pluse 함수 */
void plusBigInt(BigInt a, BigInt b, BigInt& result) {
	bool checkA = 0;
	bool checkB = 0;
	bool overFlow = 0;
	bool prevOverFlow = 0;
	unsigned int checkNum1;
	unsigned int checkNum2;
	unsigned int checkBit = 0x80000000;

	for (int intnum = 0; intnum < MAXINT; intnum++) {
		checkNum1 = a.getIntNode(intnum);
		checkNum2 = b.getIntNode(intnum);
		for (int i = 0; i < 32; i++) {
			if (i != 0) {
				if (checkA == 1) {
					checkNum1 -= checkBit;
				}
				if (checkB == 1) {
					checkNum2 -= checkBit;
				}
				checkBit >>= 1;
			}

			if ((checkNum1 / checkBit) == 1) {
				checkA = 1;
			}
			else checkA = 0;

			if ((checkNum2 / checkBit) == 1) {
				checkB = 1;
			}
			else checkB = 0;

			if (checkA == 1 && checkB == 1) {
				overFlow = 1;
				break;
			}
			else if (checkA == 0 && checkB == 0) {
				overFlow = 0;
				break;
			}
			if (i == 31) {
				if ((checkA && prevOverFlow) || (checkB && prevOverFlow)) {
					overFlow = 1;
				}
			}
		}
		result.setIntNode(intnum, (a.getIntNode(intnum) + b.getIntNode(intnum) + prevOverFlow));
		prevOverFlow = overFlow;
		overFlow = 0;
	}
}
/* multi할때 오버플로가 발생했는지 확인해주는 함수 */
bool overflowCheck(unsigned int checkNum1, unsigned int checkNum2) {
	bool checkA = 0;
	bool checkB = 0;
	bool overFlow = 0;
	unsigned int checkBit = 0x80000000;

	for (int i = 0; i < 32; i++) {
		if (i != 0) {
			if (checkA == 1) {
				checkNum1 -= checkBit;
			}
			if (checkB == 1) {
				checkNum2 -= checkBit;
			}
			checkBit >>= 1;
		}

		if ((checkNum1 / checkBit) == 1) {
			checkA = 1;
		}
		else checkA = 0;

		if ((checkNum2 / checkBit) == 1) {
			checkB = 1;
		}
		else checkB = 0;

		if (checkA == 1 && checkB == 1) {
			overFlow = 1;
			break;
		}
		else if (checkA == 0 && checkB == 0) {
			overFlow = 0;
			break;
		}
	}
	return overFlow;
}
/* / % 구할때 필요해서 만든 minus 함수 */
void minusBigInt(BigInt a, BigInt b, BigInt& result) {
	BigInt* newBigint = new BigInt(0);
	BigInt* tempBigint = new BigInt(0);
	tempBigint->setIntNode(0, 1);
	bool checkA = 0;
	bool checkB = 0;
	bool overFlow = 0;
	bool prevOverFlow = 0;
	unsigned int checkNum1;
	unsigned int checkNum2;
	unsigned int checkBit = 0x80000000;

	int alen = -1;
	int blen = -1;
	int bitlen = -1;

	for (int find = MAXINT - 1; find > -1; find--) {
		if ((b.getIntNode(find) != 0) && (blen == -1)) {
			blen = find + 1;
		}
		if ((a.getIntNode(find) != 0) && (alen == -1)) {
			alen = find + 1;
		}
	}
	for (int i = blen-1; i > -1; i--) {
		newBigint->setIntNode(i, ~b.getIntNode(i));
	}
	
	plusBigInt(*newBigint, *tempBigint, *newBigint);

	for (int intnum = 0; intnum < MAXINT; intnum++) {
		checkNum1 = a.getIntNode(intnum);
		checkNum2 = newBigint->getIntNode(intnum);

		checkNum1 -= prevOverFlow;
		if (intnum < blen) {
			if (checkNum1 + checkNum2 > checkNum1 ) {
				overFlow = 1;
			}
		}
		result.setIntNode(intnum, checkNum1 + checkNum2);
		prevOverFlow = overFlow;
		overFlow = 0;
	}
}
/* multi 함수 */
void multiBigInt(BigInt a, BigInt b, BigInt& result) {
	BigInt* newBigint = new BigInt(0);
	BigInt* resultBigint = new BigInt(0);

	bool checkA = 0;
	bool checkB = 0;
	bool overFlowHigh = 0;
	bool prevOverFlow = 0;
	unsigned int checkNum1;
	unsigned int checkNum2;
	unsigned int checkBit = 0x00000001;

	unsigned int TempLw = 0;
	unsigned int TempHi = 0;

	int alen = -1;
	int blen = -1;
	for (int find = MAXINT - 1; find > -1; find--) {
		if ((a.getIntNode(find) != 0) && (alen == -1)) {
			alen = find+1;
		}
		if ((b.getIntNode(find) != 0) && (blen == -1)) {
			blen = find+1;
		}
	}

	for (int mulnum = 0; mulnum < blen; mulnum++) {
		checkNum2 = b.getIntNode(mulnum);
		for (int intnum = 0; intnum < alen; intnum++) {
			checkNum1 = a.getIntNode(intnum);

			for (int i = 0; i < 32; i++) {
				if (i != 0) {
					checkBit <<= 1;
				}
				if ((checkNum2 & checkBit) != 0) {
					if (overflowCheck(TempLw, (checkNum1 << i))) {
						TempLw += (checkNum1 << i);
						if (i != 0) {
							TempHi += (checkNum1 >> 32 - i);
						}
						TempHi++;
					}
					else {
						TempLw += (checkNum1 << i);
						if (i != 0) {
							TempHi += (checkNum1 >> 32 - i);
						}
					}
				}
			}
			newBigint->setIntNode(mulnum + intnum, TempLw);
			newBigint->setIntNode(mulnum + intnum + 1, TempHi);
			plusBigInt(*resultBigint, *newBigint, *resultBigint);

			for (int i = 0; i < MAXINT; i++) {
				newBigint->setIntNode(i, 0);
			}
			TempLw = 0;
			TempHi = 0;
			checkBit = 0x00000001;
		}
	}
	
	result = *resultBigint;
}
/* Div가 가능한지 확인해주는 함수 */
int divCheck(BigInt a, BigInt b, int& shiftlen) {
	int alen = -1;
	int blen = -1;
	int abitlen = -1;
	int bbitlen = -1;
	unsigned int checkNum1;
	unsigned int checkNum2;
	int check = 0;

	for (int find = MAXINT - 1; find > -1; find--) {
		if ((a.getIntNode(find) != 0) && (alen == -1)) {
			alen = find + 1;
			for (int bitfind = 31; bitfind > -1; bitfind--) {
				if ((a.getIntNode(find) & (1 << bitfind)) != 0) {
					abitlen = bitfind;
					break;
				}
			}
		}
		if ((b.getIntNode(find) != 0) && (blen == -1)) {
			blen = find + 1;
			for (int bitfind = 31; bitfind > -1; bitfind--) {
				if ((b.getIntNode(find) & (1 << bitfind)) != 0) {
					bbitlen = bitfind;
					break;
				}
			}
		}
	}
	if (blen > alen) {
		return 0;
	}
	else if (blen == 0) {
		return 0;
	}
	else if (blen == alen) {
		for (int i = alen -1 ; i > -1; i--) {
			checkNum1 = a.getIntNode(i);
			checkNum2 = b.getIntNode(i);
			if (checkNum2 > checkNum1) {
				return 0;
			}
			else if (checkNum2 < checkNum1) {
				shiftlen = ((alen - blen) * 32) + (abitlen - bbitlen);
				return 1;
			}
		}
		shiftlen = ((alen - blen) * 32) + (abitlen - bbitlen);
		return 1;
	}
	else {
		shiftlen = ((alen - blen) * 32) + (abitlen - bbitlen);
		return 1;
	}
}
/* div 함수 및 남은 값을 moduler값으로 사용*/
void divBigInt(BigInt a, BigInt b, BigInt& result , BigInt& moduler) {
	BigInt* resultBigint = new BigInt(0);
	unsigned int checkNum1;
	unsigned int checkNum2;
	int looplen = 0;

	int shiftlen;
	if (divCheck(a, b, shiftlen)) {
		looplen = shiftlen;
	}
	for (int i = 0; i < shiftlen; i++) {
		for (int k = MAXINT; k > -1; k--) {
			if (k != MAXINT) {
				if ((b.getIntNode(k) & 0x80000000) != 0) {
					b.setIntNode(k + 1, b.getIntNode(k + 1) | 0x00000001);
				}
			}
			b.setIntNode(k, b.getIntNode(k) << 1);
		}
	}

	for (int j = 0; j < looplen + 1; j++) {
		if (divCheck(a, b, shiftlen)) {
			
			minusBigInt(a, b, a);
			resultBigint->setIntNode(((looplen - j) / 32),  resultBigint->getIntNode((looplen - j) / 32) | (1 << (looplen - j) % 32));
		}
		for (int k = 0; k < MAXINT; k++) {
			if (k != 0) {
				if ((b.getIntNode(k) & 0x00000001) != 0) {
					b.setIntNode(k - 1, b.getIntNode(k - 1) | 0x80000000);
				}
			}
			b.setIntNode(k, b.getIntNode(k) >> 1);
		}

	}
	moduler = a;
	result = *resultBigint;
}
/* Test1 */
void BigIntegerTest1(int op1, int op2) {
	BigInt* newBigInt1 = new BigInt();
	BigInt* newBigInt2 = new BigInt();
	

	newBigInt1->setIntNode(0, (unsigned)op1);
	newBigInt2->setIntNode(0, (unsigned)op2);

	BigInt* result = new BigInt();
	plusBigInt(*newBigInt1, *newBigInt2, *result);
	cout << op1 << " + " << op2 << " : " << result->print() << endl;
	delete(result);


	result = new BigInt();
	multiBigInt(*newBigInt1, *newBigInt2, *result);
	cout << op1 << " * " << op2 << " : " << result->print() << endl;
	delete(result);


	BigInt* result1 = new BigInt();
	BigInt* result2 = new BigInt();
	divBigInt(*newBigInt1, *newBigInt2, *result1, *result2);
	cout << op1 << " / " << op2 << " : " << result1->print() << endl;
	cout << op1 << " % " << op2 << " : " << result2->print() << endl;
	delete(result1);
	delete(result2);

}
/* Test2 */
void BigIntegerTest2(string textfile) {
	char inputstring[MAXBUF];
	char* tok = NULL;
	char* p_next = NULL;
	int wArr[MAXBUF] = { 0, };
	int xArr[MAXBUF] = { 0, };
	int num = 0;
	ifstream in(textfile);
	for (int i = 0; i < 2; i++) {
		in.getline(inputstring, MAXBUF);
		tok = strtok(inputstring, " ");
		while (tok != NULL) {
			if (i == 0)
				wArr[num] = stoi(tok);
			else
				xArr[num] = stoi(tok);
			num++;
			tok = strtok(NULL, " ");
		}
		num = 0;
	}

	BigInt* newBigint = new BigInt(0);
	BigInt* newBigintTemp = new BigInt(0);
	while (wArr[num] != 0) {
		BigInt* wBigint = new BigInt(0);
		BigInt* xBigint = new BigInt(0);
			
		wBigint->setIntNode(0, wArr[num]);
		xBigint->setIntNode(0, xArr[num]);

		multiBigInt(*wBigint, *xBigint, *newBigintTemp);
		plusBigInt(*newBigint, *newBigintTemp, *newBigint);

		num++;
	}
	cout <<"sigma Wi * Xi : " <<newBigint->print() << endl;
}


BigInt outputBigint[10]; // 전역 output
/* BigIntNerualNet 클래스 */
class BigIntNerualNet {
private:
	BigInt** Layer;
	BigInt*** weight;
public:
	BigIntNerualNet() {
		Layer = NULL;
		weight = NULL;

	}
	BigIntNerualNet(string path) {
		Layer = NULL;
		weight = NULL;
		ifstream in(path);
		char inputstring[MAXBUF];
		char* tok = NULL;
		char* p_next = NULL;
		int lineNum = 0;

		bool bRandom = false;
		while (!in.eof()) {
			switch (lineNum)
			{
			case 0:
				in.getline(inputstring, MAXBUF);
				tok = strtok(inputstring, " ");

				if (!strcmp(tok, "0")) {
					bRandom = true;
					srand(time(NULL));
				}
				break;
			case 1:
				in.getline(inputstring, MAXBUF);
				tok = strtok(inputstring, " ");
				LayerNum = stoi(tok);
				break;
			case 2:
				in.getline(inputstring, MAXBUF);
				tok = strtok(inputstring, " ");
				for (int i = 0; i < LayerNum; i++) {
					LayerNodeNum[i] = stoi(tok);
					tok = strtok(NULL, " ");
				}
				break;
			case 3:
				if (bRandom == false) {
					in.getline(inputstring, MAXBUF);
					tok = strtok(inputstring, " ");
					for (int i = 0; i < LayerNum - 1; i++) {
						weightKey[i] = stoi(tok);
						tok = strtok(NULL, " ");
					}
				}
				else {
					for (int i = 0; i < LayerNum - 1; i++) {
						weightKey[i] = (rand() % 10000);
					}
				}
				break;
			case 4:
				in.getline(inputstring, MAXBUF);
				tok = strtok(inputstring, " ");
				for (int i = 0; i < LayerNodeNum[0]; i++) {
					input[i] = stoi(tok);
					tok = strtok(NULL, " ");
				}
				break;
			}
			lineNum++;
		}
	}

	/* initNN() */
	void InitNN() {
		BigInt** newlayer = new BigInt *[LayerNum];
		Layer = newlayer;

		for (int i = 0; i < LayerNum; i++) {
			BigInt* newNode = new BigInt[LayerNodeNum[i]];
			Layer[i] = newNode;
		}

		BigInt*** newweight = new BigInt **[LayerNum - 1];
		weight = newweight;
		for (int i = 0; i < LayerNum - 1; i++) {
			BigInt** newweightnum_hi = new BigInt * [LayerNodeNum[i + 1]];
			weight[i] = newweightnum_hi;
			for (int j = 0; j < LayerNodeNum[i + 1]; j++) {
				BigInt* newweightnum_lo = new BigInt[LayerNodeNum[i]];
				weight[i][j] = newweightnum_lo;
			}
		}

		for (int i = 0; i < LayerNum - 1; i++) {
			for (int j = 0; j < LayerNodeNum[i + 1]; j++) {
				for (int k = 0; k < LayerNodeNum[i]; k++) {
					weight[i][j][k].setIntNode(0, weightKey[i]);
				}
			}
		}
		
		ostringstream sout;
		if (LayerNum > 1) {
			for (int i = LayerNum - 1; i >= 0; i--) {
				if (i == LayerNum - 1) {
					sout << "Output : " << LayerNodeNum[i] << "\n" << "  => w : " << weightKey[i - 1] << endl;
				}
				else if (i == 0) {
					sout << "Input : " << LayerNodeNum[i] << endl;
				}
				else {
					sout << "H" << i << " : " << LayerNodeNum[i] << "\n" << "  => w : " << weightKey[i - 1] << endl;
				}
			}
		}
		cout << sout.str()<<endl;
	}
	
	/* FeedForward() */
	void FeedForward() {
		for (int i = 0; i < LayerNodeNum[0]; i++) {
			Layer[0][i].setIntNode(0, input[i]);
		}

		for (int k = 0; k < LayerNum - 1; k++) {
			int lowNum;
			int highNum;
			lowNum = LayerNodeNum[k];
			highNum = LayerNodeNum[k + 1];
			for (int j = 0; j < highNum; j++) {
				BigInt* newBigint = new BigInt(0);
				BigInt* newBigintTemp = new BigInt(0);
				for (int i = 0; i < lowNum; i++) {
					multiBigInt(Layer[k][i], weight[k][j][i], *newBigintTemp);
					plusBigInt(*newBigint, *newBigintTemp, *newBigint);
				}
				plusBigInt(Layer[k + 1][j], *newBigint, Layer[k + 1][j]);
				delete newBigint;
				delete newBigintTemp;
			}
		}

		BigInt* newBigint = new BigInt(0);
		for (int i = 0; i < LayerNodeNum[LayerNum - 1]; i++) {
			plusBigInt(*newBigint, Layer[LayerNum - 1][i], outputBigint[i]);
		}
		delete newBigint;
	}
	/* ShowResult() */
	void ShowResult() {
		for (int i = 0; i < LayerNodeNum[LayerNum - 1]; i++) {
			cout <<"OutNode"<<i+1<<" : "<<outputBigint[i].print() << endl;
		}
	}
};

/* TestFFalgrotithm */
void TestFFalgrotithm(string path)
{
	BigIntNerualNet nn(path);
	nn.InitNN();
	nn.FeedForward();
	nn.ShowResult();
}

int main() {


	/* BigInt가 잘 작동하는지 확인해보는 코드 */
	/*
	int op1 = 403456789;
	int op2 = 12345122;
	string testfile = "BigIntegerTestFile.txt";
	BigIntegerTest1(op1, op2);
	BigIntegerTest2(testfile); // 명세표에 sigma(i)(Xi * Wi) 형태여서 그렇게 구현함 FeedFoward에서는 sigma(Xi * Wij)임
	*/

	/* BigIntNerualNet가 잘 작동하는지 확인해보는 코드 */
	TestFFalgrotithm("FFtestFile.txt"); // FFtestFile.txt 과 FFtestFile_Ran.txt파일의 형태에 유의해야함
	//TestFFalgrotithm("FFtestFile_Ran.txt");


	return 0;
}
