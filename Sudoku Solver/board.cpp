#include "board.h"

board::board(string line[9])
{
	// Initialize all nodes and rows
	for (int r = 0; r < 9; r++) {
        string cline = line[r];
        if (cline.length() < 9) {
            throw std::invalid_argument("Row " + std::to_string(r) + " is too short");
        }
        node* RowNodes[9];
        for (int c = 0; c < 9; c++) {
            char ch = cline[c];
            if (ch < '0' || ch > '9') {
                throw std::invalid_argument("Invalid character in row " + std::to_string(r) + ", column " + std::to_string(c));
            }
            int value = ch - '0'; // Convert '0'-'9' to 0-9
            node* newN = new node(value);
            nodes[r][c] = newN;
            RowNodes[c] = newN;
        }
        rows[r] = new group(RowNodes, false, true);
    }



	// Intialize collumns

	for (int c = 0; c < 9; c++) {
		node* ColNodes[9];
		for (int r = 0; r < 9; r++) {
			ColNodes[r] = nodes[r][c];
		}
		columns[c] = new group(ColNodes);
	}

	// Initialize Bozes
	for (int b = 0; b < 9; b++) {
		int rInit = (b / 3) * 3;
		int cInit = (b % 3) * 3;
		node* boxNodes[9];
		for (int n = 0; n < 9; n++){
			int r = rInit + (n / 3);
			int c = cInit + (n % 3);
			boxNodes[n] = nodes[r][c];
		}
		boxes[b] = new group(boxNodes, true);
	}

	// Get Critical Squartes and Send to node
	// By doing sendCrit in each Row, Column and Box Group

	for (int i = 0; i < 9; i++) {
		rows[i]->sendCrit();
		columns[i]->sendCrit();
		boxes[i]->sendCrit();
	}



}

board::~board() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			delete nodes[i][j];
		}
		delete rows[i];
		delete columns[i];
		delete boxes[i];
	}
}

vector<int> board::step() {
	int changes = 0; // Track total modifications made
	int step_type = 0; // Track the technique applied

	switch (stepState) {
	case 1: // Naked Singles
		for (int r = 0; r < 9; r++) {
			for (int c = 0; c < 9; c++) {
				if (nodes[r][c]->checkCrit()) {
					changes++;
				}
			}
		}
		if (changes > 0) {
			step_type = 1;
		}
		break;
	case 2: // Hidden Singles
		for (int i = 0; i < 9; i++) {
			// Check rows 
			
			int rc = rows[i]->findHiddenSingles();
			changes += rc;
			if (changes) break;
			// Check columns
			int cc = columns[i]->findHiddenSingles();
			changes += cc;
			if (changes) break;

			// Check boxes
			int bc = boxes[i]->findHiddenSingles();
			changes += bc;
			if (changes) break;

		}
		if (changes > 0) {
			step_type = 2;
		}
		break;
	case 3: // Pointing Pairs
		changes = updatePointingPairs();
		if (changes > 0) {
			step_type = 3;
		}
		break;
	default:
		break;
	}

	// Update stepState for the next step
	// - If changes were made, reset to Naked Singles (1) to prioritize simpler techniques
	// - If no changes, cycle to the next technique (1, 2, 3, then back to 1)
	stepState = (changes > 0) ? 1 : (stepState >= 3 ? 1 : stepState + 1);

	// Return changes and the technique applied (not the next state)
	return { changes, stepState };
}

vector<string> board::getBoard(void)
{
	vector<string> BoardData;
	for (int r = 0; r < 9; r++) {
		string row;
		for (int c = 0; c < 9; c++) {
			vector<int> nodeNum = nodes[r][c]->getPopulation();
			string num;
			for (int element : nodeNum) {
				num = num + std::to_string(element);
			}
			row += "{" + num + "}";
		}
		BoardData.push_back(row);
	}
	return BoardData; 
}

bool board::checkSolution() {
	// Step 1: Check that every cell has exactly one number
	for (int r = 0; r < 9; r++) {
		for (int c = 0; c < 9; c++) {
			std::vector<int> population = nodes[r][c]->getPopulation();
			if (population.size() != 1) {
				return false; // Cell is either empty or has multiple candidates
			}
			int value = population[0];
			if (value < 1 || value > 9) {
				return false; // Value is out of valid range
			}
		}
	}

	// Step 2: Check each row, column, and box for valid numbers (1-9, no duplicates)
	for (int i = 0; i < 9; i++) {
		// Check rows
		std::vector<bool> row_seen(10, false); // Index 0 unused, 1-9 for values
		for (node* node : rows[i]->cont) {
			int value = node->getPopulation()[0];
			if (row_seen[value]) {
				return false; // Duplicate found
			}
			row_seen[value] = true;
		}
		// Verify all numbers 1-9 are present
		for (int v = 1; v <= 9; v++) {
			if (!row_seen[v]) {
				return false; // Missing number
			}
		}

		// Check columns
		std::vector<bool> col_seen(10, false);
		for (node* node : columns[i]->cont) {
			int value = node->getPopulation()[0];
			if (col_seen[value]) {
				return false; // Duplicate found
			}
			col_seen[value] = true;
		}
		for (int v = 1; v <= 9; v++) {
			if (!col_seen[v]) {
				return false; // Missing number
			}
		}

		// Check boxes
		std::vector<bool> box_seen(10, false);
		for (node* node : boxes[i]->cont) {
			int value = node->getPopulation()[0];
			if (box_seen[value]) {
				return false; // Duplicate found
			}
			box_seen[value] = true;
		}
		for (int v = 1; v <= 9; v++) {
			if (!box_seen[v]) {
				return false; // Missing number
			}
		}
	}

	// All checks passed: each cell has one number, and each unit has 1-9 exactly once
	return true;
}

int board::updatePointingPairs() {
	int changes = 0; // Track total candidate removals

	// Iterate through all 9 boxes (0–8)
	for (int b = 0; b < 9; b++) {
		// Find all pointing groups in the box
		vector<pair<pair<bool, int>, pair<bool, int>>> rowGroups = boxes[b]->findPointingPairs(true);
		vector<pair<pair<bool, int>, pair<bool, int>>> colGroups = boxes[b]->findPointingPairs(false);

		// Process row-aligned groups
		for (const auto& group : rowGroups) {
			int candidate = group.first.second;
			if (candidate >= 1 && candidate <= 9 && group.second.second >= 0 && group.second.second <= 2) {
				// Map box-local row index to grid row
				int Row = (b / 3) * 3 + group.second.second;
				if (Row >= 0 && Row < 9) {
					// Remove candidate from row cells outside box b
					changes += rows[Row]->removeIfPointingPairs(b, candidate, Row);
				}
			}
		}

		// Process column-aligned groups
		for (const auto& group : colGroups) {
			int candidate = group.first.second;
			if (candidate >= 1 && candidate <= 9 && group.second.second >= 0 && group.second.second <= 2) {
				// Map box-local column index to grid column
				int Col = (b % 3) * 3 + group.second.second;
				if (Col >= 0 && Col < 9) {
					// Remove candidate from column cells outside box b
					changes += columns[Col]->removeIfPointingPairs(b, candidate, Col);
				}
			}
		}
	}

	// Return total changes for integration with board::step
	return changes;
}
