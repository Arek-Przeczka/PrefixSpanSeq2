#ifndef PREFIXSPAN_H
#define PREFIXSPAN_H

class PrefixSpan {
private:
	std::vector<std::string> database; //input database as vector of strings. Each line of database is a single string
	std::vector<std::vector<int>> database_idx;
	std::vector<std::vector<int>> idx_offset;
	std::vector<std::vector<int>> new_database_idx;
	std::vector<std::vector<int>> new_idx_offset;
	std::vector<unsigned int> wanted_seq_ap;
	std::vector<unsigned int> wanted_seq_as;
	std::vector<std::vector<std::string>> seq;
	std::vector<std::vector<std::string>> new_seq;
	std::vector<std::vector<int>> seq_val;
	std::vector<unsigned int> last_element;
	std::vector<std::vector<std::string>> sup_databases;
	std::ofstream output;

public:
	PrefixSpan(std::vector<std::string> input_database, char* output_file_name);

	void firstScan();
	void scanDatabase();
	void trimVector(int min_sup);
	void trimVector2(int min_sup);
	void printVector();
	void printVector2();
	void buildFirstDatabases();
	void buildDatabases();
	void buildFirstVector();
	void buildVector();
	
	bool isEmpty();

};


#endif
