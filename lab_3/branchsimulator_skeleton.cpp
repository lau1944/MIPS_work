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

	float getAccuracy()
	{
		return float(rightCount) / float(totalCount);
	}
};

int main(int argc, char **argv)
{
	ResultTest resultTest = ResultTest();

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
	int entryIndex = 0;
	// Init saturat counters
	vector<Entry> saturatList(pow(k, m));
	// PC to entry mapping
	std::map<std::string, int> entries;
	std::string line;
	Entry *preEntry;

	while (std::getline(trace, line))
	{
		std::string pc = line.substr(0, line.size() - 3);
		if (entries.find(pc) != entries.end())
		{
			preEntry = &saturatList[entries[pc]];
		}
		else
		{
			preEntry = &saturatList[entryIndex];
			entries.insert(std::make_pair(pc, entryIndex++));
		}
		bool isTaken = line.substr(line.size() - 2, 1) == "1";
		// test prediction
		resultTest.test((*preEntry).isTaken(), isTaken);
		// update entry
		(*preEntry).update(isTaken);
	}

	// write in file
	for (int i = 0; i < saturatList.size(); i++)
	{
		out << saturatList[i].isTaken() << "\n";
	}

	cout << resultTest.getAccuracy() * 100. << "%" << endl;



	// from m = 10 to 20, output loss rate
	for (int m = 10; m <= 20; m++)
	{
		int mIndex = 0;
		// Init saturat counters
		vector<Entry> saturatList(pow(k, m));
		// PC to entry mapping
		std::map<std::string, int> entries;
		std::string line;
		Entry *preEntry;

		while (std::getline(trace, line))
		{
			std::string pc = line.substr(0, line.size() - 3);
			if (entries.find(pc) != entries.end())
			{
				preEntry = &saturatList[entries[pc]];
			}
			else
			{
				preEntry = &saturatList[mIndex];
				entries.insert(std::make_pair(pc, mIndex++));
			}
			bool isTaken = line.substr(line.size() - 2, 1) == "1";
			// test prediction
			resultTest.test((*preEntry).isTaken(), isTaken);
			// update entry
			(*preEntry).update(isTaken);
		}
		loss_out << m << ":" << 1 - resultTest.getAccuracy() << "\n";
	}

	preEntry = NULL;

	loss_out.close();
	out.close();
	trace.close();
}
