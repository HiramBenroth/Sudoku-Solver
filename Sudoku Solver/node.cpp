#include "node.h"

node::node(int num) {
	number = num;

	if (number == 0) {
		for (int i = 1; i <= 9; ++i) {
			viable.push_back(i);
		}
		eliminated.clear(); // Optional; default-initialized empty
	}
	else {
		viable.clear();     // No options, fixed number
		eliminated.clear(); // Optional
	}
}

vector<int> node::getPopulation()
{
	vector<int> numbers;
	if (number != 0) {
		numbers.push_back(number);
	}
	else {
		numbers = viable;
	}
	return numbers;

}

void node::addCrit(node* addr[20])
{
    for (int newC = 0; newC < 20; newC++) {
        if (addr[newC] != nullptr) {
            bool alreadyExists = false;
            for (node* critS : critical_squares) {
                if (critS == addr[newC]) {
                    alreadyExists = true;
                    break;
                }
            }
            if (!alreadyExists) {
                critical_squares.push_back(addr[newC]);
            }
        }
    }
}


void node::updateViables(int num)
{
    viable.erase(std::remove(viable.begin(), viable.end(), num), viable.end());
    if (viable.size() == 1) {
        numberSet(viable[0]);
    }
}

bool node::updateElims(int num)
{
    for (int elim : eliminated) {
        if (num == elim) {
            return false;
        }
    }
    eliminated.push_back(num);
    this->updateViables(num);
    return true;
}

bool node::checkCrit(void)
{
    /* Returns true if there was any change, via eliminations*/
    bool haschange = 0;
    if (number == 0) {
        for (node* critN : critical_squares) {
            int nodeNum = critN->getNum();
            if (nodeNum != 0) {
                if (updateElims(nodeNum)) {
                    haschange = 1;
                }
            }
        }
    }
    return haschange;
}

void node::checkQuick(void)
{

}

void node::numberSet(int num)
{
    if (this->isViable(num)) {
        number = num;
        for (int newElim : viable) {
            if (newElim != num) {
                eliminated.push_back(newElim);
            }
        }
        viable.clear();
    }

}

bool node::isViable(int num) {
    for (int via : viable) {
        if (num == via) {
            return true;
        }
    }
    return false;
}

int node::getNum() {
    return number;
}