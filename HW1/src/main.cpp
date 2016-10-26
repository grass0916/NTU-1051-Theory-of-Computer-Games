#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

#include "elapsed_time.h"


/*
   Tuple means the clues of Nonogram each row / column.
    > Tuple unit A is [1, 2, 1] and tuple unit B is [2, 3].
    > Tuples are ["1,2,1", "2,3"], totally 2 tuples.
*/
#define TupleUnit std::vector<int>
#define Tuple     std::string
#define Tuples    std::vector<Tuple>

/*
   State means the allowed case with the tuple unit.
   If boundary is 5.
     a) Tuple is [1, 2].
     b) States are [10110, 10011, 01011], totally 3 states.
     c) State set is { first: "1,2" and second: [10110, 10011, 01011] }.
     d) State sets are multi state sets.
*/
#define State     std::string
#define States    std::vector<State>
#define StateSet  std::pair<std::string, States>
#define StateSets std::vector<StateSet>

class ClueTable {
	public:
		ClueTable();
		ClueTable(int columns, int rows);		

		void addClue(std::string vectors, TupleUnit tuple);
		Tuple getTuple(std::string vectors, int number);
		States getClue(Tuple tuple);
		void showContent();

	private:
		// Amount of column and row.
		int columns, rows;
		// Pure clue information in 2-d array.
		Tuples columnTuples, rowTuples;
		// 'list' is combination of column and row, then unique them.
		StateSets stateSets;
		// 
		void searchAllClues(StateSet& states, State current, TupleUnit tuple, int boundary);
};

void ClueTable::searchAllClues(StateSet& states, State current, TupleUnit tuple, int boundary) {
	// Get the first number then erase it. 
	const int number = tuple.at(0);
	tuple.erase(tuple.begin());

	// Starting to determine the every allowed conditions of the tuple.
	for (int padding = (current.length() ? 1 : 0); current.length() + padding + number <= boundary; padding++) {
		State tempState = current + std::string(padding, '0') + std::string(number, '1');

		// If the tuple is empty, padding the remaining cells and store it.
		if (tuple.empty()) {
			tempState += std::string(boundary - tempState.length(), '0');
			states.second.push_back(tempState);	
		}
		// If there is any element in tuple, continue the recursion.
		else {
			this->searchAllClues(states, tempState, tuple, boundary);
		}
	}
}

// Add a new clue into set of states.
void ClueTable::addClue(std::string vectors, TupleUnit tuple) {
	// Boundary is state container size of vectors.
	int boundary   = vectors == "column" ? this->rows : this->columns;
	// Counts is the tuple amount of vectors.
	int counts     = vectors == "column" ? this->columns : this->rows;
	Tuples& tuples = vectors == "column" ? this->columnTuples : this->rowTuples;

	// Convert the tuple to string.
	// Tuple [1, 2, 5] with boundary 10 => String "10-1,2,5".
	std::ostringstream tupleStringstream;
	tupleStringstream << boundary << "-";
	std::copy(tuple.begin(), tuple.end()-1,
		std::ostream_iterator<int>(tupleStringstream, ",")
	);
	tupleStringstream << tuple.back();

	// Using the string of tuple to search is it already exist in table or not.
	const std::string tupleString = tupleStringstream.str(); 
	auto it = std::find_if(this->stateSets.begin(), this->stateSets.end(),
		[tupleString] (const StateSet& set) -> bool {
			return set.first == tupleString;
		}
	);
	// If not found it, store it into table.
	if (it == this->stateSets.end()) {
		// 'states' is the set of all allowed clues.
		StateSet states;
		states.first = tupleString;
		this->searchAllClues(states, "", tuple, boundary);
		this->stateSets.push_back(states);
		// Ordering by tuple.
		std::sort(this->stateSets.begin(), this->stateSets.end());
	}

	// Store it into tuple set.
	if (tuples.size() < counts) {
		tuples.push_back(tupleString);
	} else {
		std::cout << tuples.size();
		throw "ERR_VECTORS_OF_CLUE";
	}

	return;
}

// Get the specific tuple with column or row number.
Tuple ClueTable::getTuple(std::string vectors, int number) {
	Tuples& tuples = vectors == "column" ? this->columnTuples : this->rowTuples;
	if (number >= 0 && number < tuples.size()) {
		return tuples[number];
	} else {
		throw "ERR_OUT_OF_TUPLES";
	}
}

// Get the specific clue with tuple.
States ClueTable::getClue(Tuple tuple) {
	auto it = std::find_if(this->stateSets.begin(), this->stateSets.end(),
		[tuple] (const StateSet& set) -> bool {
			return set.first == tuple;
		}
	);
	// If found it, return it.
	if (it != this->stateSets.end()) {
		return (*it).second;
	} else {
		throw "ERR_WRONG_TUPLE";
	}
}



// [Debug mode]: Show the current table content of clues.
void ClueTable::showContent() {
	std::cout << "Show the content of clue table.\n\n";
	// 'ss' is states; 's' is state. 
	for (const auto& ss: this->stateSets) {
		std::cout << "\nTuple [" << ss.first << "] with " << ss.second.size() << " states.\n";
		/*
		for (const auto& s: ss.second) {
			std::cout << (s == ss.second[0] ? "": ", ") << s;
		}
		std::cout << "]\n\n============================\n";
		*/
	}
}

ClueTable::ClueTable() {
	this->rows = this->columns = 0;
}

ClueTable::ClueTable(int columns, int rows) {
	this->rows = rows;
	this->columns = columns;
}



#define Board std::string

class Nonogram {
	public:
		// Constructor.
		Nonogram();
		Nonogram(std::string inputFile, int columns, int rows);
		// Return the empty board.
		Board initialBoard();
		// First iteration for DFS.
		void DFS();

	private:
		int rows, columns;
		std::string id;
		// Store the clues.
		ClueTable* clue;
		// Function overloading of DFS for recursing.
		void DFS(Board current, int row, int stateIndex);
};

// 
void Nonogram::DFS() {
	this->clue->showContent();
	// Initial board.
	Board board = this->initialBoard();
	this->DFS(board, 0, 0);
}

// 
void Nonogram::DFS(Board current, int row, int stateIndex) {
	// Get the relative information about states of row.
	Tuple tupleRow = this->clue->getTuple("row", row);
	States states = this->clue->getClue(tupleRow);
	// Replace the state by row.
	current.replace(row * this->columns, this->columns, states[stateIndex]);

	bool isAllowed = true;
	// Check the board is legal or not when it is completed.
	for (int column = 0; column < this->columns; column++) {
		// Get the relative information about states of column.
		Tuple tupleColumn = this->clue->getTuple("column", column);
		States states = this->clue->getClue(tupleColumn);
		// Check the column state is exist in clue table or not.
		State columnState;
		std::copy_if(current.begin(), current.end(),
			std::back_inserter(columnState),
			[iter = current.begin(), begin = current.begin(), columns = this->columns, column = column] (char c) mutable {
				return (std::distance(begin, iter++) % columns == column);
			}
		);	

		// If
		auto findIter = std::find_if(states.begin(), states.end(),
			[columnState] (const State& s) {
				State trimA = columnState.substr(0, columnState.find_first_of("-"));
				State trimB = s.substr(0, trimA.size());
				return trimA == trimB;
			}
		); 

		// Have not found the matched state.
		if (findIter == states.end()) {
			isAllowed = false;
			// [Optimize I] This Line is the optimized condition.
			break;
		}
	}

	if (isAllowed && row+1 == this->rows) {
		std::stringstream ss;
		for (int i = 0; i < current.size(); i++) {
			ss << current[i];
			if ((i+1) % this->columns == 0) {
				ss << "\n";
			}
		}
		std::cout << ss.str() << "\n\n";
	}

	// Next recursion for depth.
	if (isAllowed && row+1 < this->rows) {
		this->DFS(current, row+1, 0);
	}
	// Next recursion for breadth.
	if (stateIndex+1 < states.size()) {
		this->DFS(current, row, stateIndex+1);
	}
	return;
}

Board Nonogram::initialBoard() {
	return (std::string(this->columns * this->rows, '-'));
}

// This is the constructor of Nonogram.
// Set the clues information for the table.
Nonogram::Nonogram() {}
Nonogram::Nonogram(std::string inputFile, int columns, int rows) {
	// Read text file
	std::string inputFilePath = "./data/" + std::string(inputFile);
	std::cout << inputFilePath << std::endl;
	// Read sub-numbers each line.
	std::ifstream file(inputFilePath.c_str());
	std::string line;


	// If this text file was opened fail.
	if (! file.is_open()) {
		std::cout << "Unable to open the file." << std::endl;
		return;
	}

	// Store some information and create a prototype of clue table.
	this->columns = columns, this->rows = rows;
	this->clue = new ClueTable(columns, rows);

	// First line is problem number.
	std::getline(file, line);
	this->id = std::string(line);

	// Read and write into data structure each line.
	for (int i = 0, times = columns + rows; i < times; i++) {
		std::getline(file, line);
		std::istringstream iss(line);
		// Start from second line, these are the tuple of clues.
		int number;
		TupleUnit tuple;
		while (iss >> number) {
			tuple.push_back(number);
		}

		// Determine to push into columns or rows.
		this->clue->addClue((i < columns ? "column" : "row"), tuple);
	}

	// Close the file.
	file.close();
}

int main (int argc, char* args[]) {
	try {
		if (argc <= 4) {
			throw "ERR_NOT_ENOUGH_ARGS";
		}

		std::string fileName(args[1]);
		int columns = atoi(args[2]), rows = atoi(args[3]);
		std::string method(args[4]);

		// Create a nonogram
		Nonogram nonogram(fileName, columns, rows);

		// Calculating the elapsed time.
		ElapsedTime time;
		time.setStart();

		nonogram.DFS();

		// Print the elapsed time.
		time.setEnd();	
		std::cout << "Elapsed time is " << time.getElapsedTime() << " microseconds.\n";

	} catch (const char* e) {
		std::cout << e << std::endl;
	}

	return 0;
}
