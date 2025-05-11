#include "group.h"  // include your header file
#include "node.h"   // if you need node class visibility

group::group (node* members[9], bool isBox, bool isRow) {
    this->isBox = isBox;
    this->isRow = isRow;
    for (int i = 0; i < 9; i++) {
        cont[i] = members[i];
    }
}

void group::sendCrit() {
    for (int i = 0; i < 9; i++) {
        if (cont[i] != nullptr) {
            node* critValues[20];
            int index = 0;

            for (int j = 0; j < 9; j++) {
                if (j != i && cont[j] != nullptr) {
                    critValues[index++] = cont[j];
                }
            }

            while (index < 20) {
                critValues[index++] = nullptr;
            }

            cont[i]->addCrit(critValues);
        }
    }
}

int group::findHiddenSingles() {

    int changes = 0;
    vector<pair<int, int>> population; // Use pair to store {value, count}
    for (node* node : cont) {
        if (node->getPopulation().size() != 1) {
            std::vector<int> currPop = node->getPopulation();
            if (population.empty()) {
                for (int viable : currPop) {
                    population.push_back({ viable, 1 }); // Initialize with value and count 1
                }
            }
            else {
                for (int i = 0; i < currPop.size(); i++) {
                    bool found = false;
                    for (auto& pop : population) {
                        if (pop.first == currPop[i]) {
                            pop.second++; // Increment count
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        population.push_back({ currPop[i], 1 }); // Add new value with count 1
                    }
                }
            }
        }
    }

    for (const auto& pop : population) {
        if (pop.second == 1) { // If count is 1, it's a hidden single
            for (node* node : cont) {
                if (node->isViable(pop.first)) {
                    node->numberSet(pop.first);
                    changes++;
                }
            }
        }
    }
    return changes;
}

vector<pair<pair<bool, int>, pair<bool, int>>> group::findPointingPairs(bool RowCheck) {
    vector<pair<pair<bool, int>, pair<bool, int>>> groups;

    // Pointing pairs only apply to box groups
    if (!isBox) {
        return groups; // Return empty vector if not a box
    }

    // Iterate through each possible candidate (1–9)
    for (int candidate = 1; candidate <= 9; ++candidate) {
        // Store indices of cells where the candidate is viable
        vector<int> cell_indices;
        for (int i = 0; i < 9; ++i) {
            if (cont[i] != nullptr && cont[i]->isViable(candidate)) {
                cell_indices.push_back(i);
            }
        }

        // Check for pointing pair (2 cells) or triple (3 cells)
        if (cell_indices.size() == 2 || cell_indices.size() == 3) {
            if (RowCheck) {
                // Check row alignment
                bool same_row = false;
                int row_index = -1;
                if (cell_indices.size() == 2) {
                    if ((cell_indices[0] / 3) == (cell_indices[1] / 3)) {
                        same_row = true;
                        row_index = cell_indices[0] / 3; // Box-local row (0, 1, or 2)
                    }
                }
                else { // Size == 3 (triple)
                    if ((cell_indices[0] / 3) == (cell_indices[1] / 3) &&
                        (cell_indices[1] / 3) == (cell_indices[2] / 3)) {
                        same_row = true;
                        row_index = cell_indices[0] / 3;
                    }
                }
                if (same_row) {
                    groups.push_back({ {true, candidate}, {true, row_index} });
                }
            }
            else {
                // Check column alignment
                bool same_column = false;
                int col_index = -1;
                if (cell_indices.size() == 2) {
                    if ((cell_indices[0] % 3) == (cell_indices[1] % 3)) {
                        same_column = true;
                        col_index = cell_indices[0] % 3; // Box-local column (0, 1, or 2)
                    }
                }
                else {
                    if ((cell_indices[0] % 3) == (cell_indices[1] % 3) &&
                        (cell_indices[1] % 3) == (cell_indices[2] % 3)) {
                        same_column = true;
                        col_index = cell_indices[0] % 3;
                    }
                }
                if (same_column) {
                    groups.push_back({ {true, candidate}, {false, col_index} });
                }
            }
        }
    }

    return groups; // Return detected groups based on RowCheck
}

    // No pointing pair or triple found for any candidate.

// Removes a candidate from cells in this group (row or column) that are outside the specified box.
// Used for pointing pairs/triples, where a candidate in 2–3 cells in a box (boxIndex) aligned
// in a row/column is eliminated from other cells in that row/column outside the box.
// Parameters:
// - boxIndex: Index of the box (0–8) whose cells should be skipped
// - candidate: The candidate number (1–9) to remove
// - gridIndex: The grid row (if isRow=true) or column (if isRow=false) index (0–8)
int group::removeIfPointingPairs(int boxIndex, int candidate, int gridIndex) {
    int changes = 0; // Track number of candidate removals

    // Validate inputs to prevent out-of-bounds errors
    if (boxIndex < 0 || boxIndex > 8 || candidate < 1 || candidate > 9 || gridIndex < 0 || gridIndex > 8) {
        return 0; // No changes if inputs are invalid
    }

    // Compute the box’s grid coordinates to identify cells to skip
    // - boxRow: Box’s row in the 3x3 box grid (0, 1, or 2)
    // - boxCol: Box’s column in the 3x3 box grid (0, 1, or 2)
    // - startRow, startCol: Top-left grid coordinates of the box (e.g., box 3 ? rows 3–5, cols 0–2)
    int boxRow = boxIndex / 3; // e.g., box 3 ? boxRow = 1
    int boxCol = boxIndex % 3; // e.g., box 3 ? boxCol = 0
    int startRow = boxRow * 3; // e.g., boxRow 1 ? startRow = 3
    int startCol = boxCol * 3; // e.g., boxCol 0 ? startCol = 0

    // Iterate through the 9 cells in this group (row or column)
    for (int i = 0; i < 9; ++i) {
        if (cont[i] != nullptr) {
            // Determine the cell’s grid coordinates
            // - Row group: row = gridIndex (fixed), col = i (0–8)
            // - Column group: row = i (0–8), col = gridIndex (fixed)
            int cellRow = isRow ? gridIndex : i;
            int cellCol = isRow ? i : gridIndex;

            // Check if the cell is in the box (to skip it)
            // - A cell is in the box if both its row and column are within the box’s 3x3 region
            bool inBox = (cellRow >= startRow && cellRow < startRow + 3 &&
                cellCol >= startCol && cellCol < startCol + 3);

            // Remove the candidate from cells outside the box if viable
            if (!inBox && cont[i]->isViable(candidate)) {
                // Assumes node::updateElims(int) removes the candidate and returns true if successful
                if (cont[i]->updateElims(candidate)) {
                    changes++;
                }
            }
        }
    }

    // Return the total number of candidates removed for integration with board::updatePointingPairs
    return changes;
}


