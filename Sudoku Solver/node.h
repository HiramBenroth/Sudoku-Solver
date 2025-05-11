#pragma once
#include <vector>

using std::vector;

class node
{
private:
	int number;
	vector<int> viable;
	vector<int> eliminated;
	vector<node*> critical_squares;

public:
	node(int num);
	vector<int> getPopulation();
	int getNum();
	void addCrit(node * addr[20]);
	void updateViables(int num); // to add a possibility from outside source
	bool updateElims(int num); // to add an Eliminated value from outside source
	bool checkCrit(void); 
	void checkQuick(void);
	void numberSet(int num); 
	bool isViable(int num);
};

