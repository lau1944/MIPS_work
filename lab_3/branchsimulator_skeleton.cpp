#include <iostream>
#include <fstream>
#include <math.h>
#include <map>
#include <vector>
#include <bitset>
#include <math.h>
#define StrongTaken 3
#define WeakTaken 2
#define WeakNtaken 1
#define StrongNtaken 0

using namespace std;

class Entry
{
private:
	int state;
	/**
	 * 状态机
	 */
	int updateState(bool isTaken)
	{
		switch (state)
		{
		case StrongTaken:
		{
			if (!isTaken)
				this->state = WeakTaken;
			break;
		}
		case WeakTaken:
		{
			if (isTaken)
				this->state = StrongTaken;
			else
				this->state = WeakNtaken;
			break;
		}
		case WeakNtaken:
		{
			if (isTaken)
				this->state = WeakTaken;
			else
				this->state = StrongNtaken;
			break;
		}
		case StrongNtaken:
		{
			if (isTaken)
				this->state = WeakNtaken;
			break;
		}
		default:
			break;
		}
		return state;
	}

public:
	Entry()
	{
		this->state = StrongTaken;
	}
	bool isTaken()
	{
		return this->state == StrongTaken || this->state == WeakTaken;
	}
	int update(bool isT)
	{
		return updateState(isT);
	}
	int getState()
	{
		return state;
	}
};

class ResultTest
{
private:
	int totalCount;
	int rightCount;

public:
	ResultTest()
	{
	}

	bool test(bool predi, bool result)
	{
		totalCount += 1;
		if (predi == result)
		{
			rightCount += 1;
		}
		return predi == result;
	}

	void restart() {
		rightCount = 0;
		totalCount = 0;
	}

	float getAccuracy()
	{
		return float(rightCount) / float(totalCount);
	}
};

// hex to bits
const char* hex_char_to_bin(char c)
{
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
    }
}

std::string hex_str_to_bin_str(const std::string& hex)
{
    std::string bin;
    for(unsigned i = 0; i != hex.length(); ++i)
       bin += hex_char_to_bin(hex[i]);
    return bin;
}

int main(int argc, char **argv)
{
	ResultTest resultTest = ResultTest();

	vector<string> pcs;

	// read config file
	ifstream config;
	config.open("config.txt");
	int m, k;
	config >> m >> k;
	config.close();

	// open trace output file
	ofstream out;
	string out_file_name = string("trace.txt") + ".out";
	out.open(out_file_name.c_str());

	// loss rate output file
	ofstream loss_out;
	string loss_rate_file = string("loss_rate.txt");
	loss_out.open(loss_rate_file.c_str());

	// read trace file
	std::ifstream trace;
	trace.open("trace.txt");
	// Init saturat counters
	vector<Entry> saturatList(pow(k, m));
	string line;

	while (getline(trace, line))
	{
		pcs.push_back(line);
	}

	for (int l = 0; l < pcs.size(); l++) {
		string line = pcs[l];
		string pc_in_bits = hex_str_to_bin_str(line.substr(0, line.size() - 3));
		long pc_index = bitset<32>(pc_in_bits.substr(32 - m, m)).to_ullong();	

		Entry* entry = &saturatList[pc_index];
		bool isTaken = line.substr(line.size() - 2, 1) == "1";
		resultTest.test((*entry).isTaken(), isTaken);
		(*entry).update(isTaken);
	}


	// write in file
	for (int i = 0; i < saturatList.size(); i++)
	{
		out << (saturatList[i]).isTaken() << "\n";
	}

	cout << "Accuracy: " << resultTest.getAccuracy() * 100. << "%" << endl;
	resultTest.restart();


	// from m = 10 to 20, output loss rate
	for (int m = 10; m <= 20; m++)
	{
		// Init saturat counters
		vector<Entry> saturatList(pow(k, m));

		for (int l = 0; l < pcs.size(); l++) {
			string line = pcs[l];
			string pc_in_bits = hex_str_to_bin_str(line.substr(0, line.size() - 3));
			long pc_index = bitset<32>(pc_in_bits.substr(32 - m, m)).to_ullong();	
			Entry* entry = &saturatList[pc_index];

			bool isTaken = line.substr(line.size() - 2, 1) == "1";
			resultTest.test((*entry).isTaken(), isTaken);
			(*entry).update(isTaken);
		}
		loss_out << m << ":" << 1 - resultTest.getAccuracy() << "\n";
		resultTest.restart();
	}

	loss_out.close();
	out.close();
	trace.close();

}
