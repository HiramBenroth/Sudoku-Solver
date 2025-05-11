#pragma once

#include "node.h"
#include "group.h"
#include <string>
#include <vector>
#include <stdexcept>

using std::string;
using std::vector;

class board
{
private:
	group *rows[9];
	group *columns[9];
	group *boxes[9];
	node *nodes[9][9];
	int stepState = 1;
	int boxChecked = 0;
		
public:
	board( string line[9]);
	vector<int> step();
	vector<string> getBoard(void);
	bool checkSolution();
	int updatePointingPairs();
	~board();
};
