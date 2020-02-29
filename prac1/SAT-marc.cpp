#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;

vector<pair<vector<int>,vector<int>>> occurLists;

vector<int> numConflict;
int timeConflict = 0;

const int time_reset = 1000;



void readClauses( ){
	// Skip comments
	char c = cin.get();
	while (c == 'c') {
		while (c != '\n') c = cin.get();
		c = cin.get();
	}
	// Read "cnf numVars numClauses"
	string aux;
	cin >> aux >> numVars >> numClauses;
	clauses.resize(numClauses);
	occurLists.resize(numVars+1);

	// Read clauses
	for (uint i = 0; i < numClauses; ++i) {
		int lit;
		while (cin >> lit and lit != 0) {
			clauses[i].push_back(lit);

			if (lit < 0) {
				occurLists[-lit].first.push_back(i);
			}
			else {
				occurLists[lit].second.push_back(i);
			}

		}
	}

	// new heuristic
	numConflict.resize(numVars+1,0);


	for (uint i = 1; i <= numVars; ++i) {
		//cout << i << endl;
		numConflict[i] = min(occurLists[i].first.size(),occurLists[i].second.size());
	}

}



int currentValueInModel(int lit){   //DONT TOUCH
	if (lit >= 0) return model[lit];
	else {
		if (model[-lit] == UNDEF) return UNDEF; //se mira desde la vista del negado
		else return 1 - model[-lit];  //opuesto a su negativo
	}
}


void setLiteralToTrue(int lit){   //DONT TOUCH
	modelStack.push_back(lit);
	if (lit > 0) model[lit] = TRUE;
	else model[-lit] = FALSE;
}


bool propagateGivesConflict ( ) {
	while ( indexOfNextLitToPropagate < modelStack.size() ) {

		int topStack = modelStack[indexOfNextLitToPropagate];
		vector <int> *topClauses;
		if (topStack > 0) topClauses = &occurLists[topStack].first;
		else topClauses = &occurLists[-topStack].second;

		++indexOfNextLitToPropagate;

		for (uint j = 0; j < topClauses->size(); ++j) { //itermaos solo en las listas del firstPila...
			int i = topClauses->at(j);

			bool someLitTrue = false;
			int numUndefs = 0;
			int lastLitUndef = 0;

			for (uint k = 0; not someLitTrue and k < clauses[i].size(); ++k){
				int val = currentValueInModel(clauses[i][k]);
				if (val == TRUE) someLitTrue = true;
				else if (val == UNDEF){ ++numUndefs; lastLitUndef = clauses[i][k]; }
			}
			if (not someLitTrue and numUndefs == 0){
				if (timeConflict==time_reset) {
					for (uint z = 1; z <= numVars; ++z){
						numConflict[z] /= 2;
					}
					timeConflict = 0;
				}
				++timeConflict;
				for (uint k = 0; k < clauses[i].size(); ++k){
					++numConflict[abs(clauses[i][k])];

				}

				return true; // conflict! all lits false
			}
			else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);
		}
	}
	return false;
}


void backtrack(){
	uint i = modelStack.size() -1;
	int lit = 0;
	while (modelStack[i] != 0){ // 0 is the DL mark
		lit = modelStack[i];
		model[abs(lit)] = UNDEF;
		modelStack.pop_back();
		--i;
	}
	// at this point, lit is the last decision
	modelStack.pop_back(); // remove the DL mark
	--decisionLevel;
	indexOfNextLitToPropagate = modelStack.size();
	setLiteralToTrue(-lit);  // reverse last decision
}


// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
	int maxSize = -1;
	int maxVar = 0;
	for (uint i = 1; i <= numVars; ++i) {
		if (model[i] == UNDEF and numConflict[i] > maxSize) {
			maxSize = numConflict[i]; 
			maxVar = i;
		}
	}
	return maxVar; // reurns 0 when all literals are defined
}

void checkmodel(){
	for (uint i = 0; i < numClauses; ++i){
		bool someTrue = false;
		for (uint j = 0; not someTrue and j < clauses[i].size(); ++j)
			someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
		if (not someTrue) {
			cout << "Error in model, clause is not satisfied:";
			for (uint j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
			cout << endl;
			exit(1);
		}
	}
}

int main(){
	readClauses(); // reads numVars, numClauses and clauses
	model.resize(numVars+1,UNDEF);
	indexOfNextLitToPropagate = 0;
	decisionLevel = 0;

	// Take care of initial unit clauses, if any
	for (uint i = 0; i < numClauses; ++i)
		if (clauses[i].size() == 1) {
			int lit = clauses[i][0];
			int val = currentValueInModel(lit);
			if (val == FALSE) {cout << "UNSATISFIABLE" << endl; return 10;}
			else if (val == UNDEF) setLiteralToTrue(lit);
		}

	// DPLL algorithm
	while (true) {
		while ( propagateGivesConflict() ) {
			if ( decisionLevel == 0) { cout << "UNSATISFIABLE" << endl; return 10; }
			backtrack();
		}
		int decisionLit = getNextDecisionLiteral();
		if (decisionLit == 0) { checkmodel(); cout << "SATISFIABLE" << endl; return 20; }
		// start new decision level:
		modelStack.push_back(0);  // push mark indicating new DL
		++indexOfNextLitToPropagate;
		++decisionLevel;
		setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
	}
}
