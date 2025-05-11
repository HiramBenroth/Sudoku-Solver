#pragma once
#include "node.h"
#include <vector>
#include <utility>

using std::vector;
using std::pair;

class group
{
		
	private:
		bool isBox;
		bool isRow;
	public:
		node* cont[9];
		group(node* members[9], bool isBox = false, bool isRow = false);
		void sendCrit();
		int findHiddenSingles();
		vector<pair<pair<bool, int>, pair<bool, int>>> findPointingPairs(bool RowCheck);
		int removeIfPointingPairs(int b, int num, int gridIndex);
};
