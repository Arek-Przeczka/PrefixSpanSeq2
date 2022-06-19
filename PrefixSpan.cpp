#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "PrefixSpan.h"

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define BIT_SET(var,pos) ((var) |= (1ULL<<(pos)))
#define BIT_CLEAR(var,pos) ((var) &= ~(1ULL<<(pos)))


PrefixSpan::PrefixSpan(std::vector<std::string> input_database, char* output_file_name) {
	output.open(output_file_name);
	database = input_database;
	wanted_seq_ap.push_back(0x7fffffff);
	wanted_seq_as.push_back(0);
	last_element.push_back(0);
	seq.resize(1);
	seq_val.resize(1);
	database_idx.resize(1);
	idx_offset.resize(1);
	idx_offset[0].resize(input_database.size(), 0);
	for (int i = 0; i < input_database.size(); i++) {
		database_idx[0].push_back(i);
	}
	for (int i = 0; i < 31; i++) {
		seq[0].push_back({char(97 + i) });
		seq_val[0].push_back(0);
	}

}

void PrefixSpan::firstScan() {
	unsigned int seq_candidate;
	unsigned int already_found_seq = 0x0;
	for (int i = 0; i < database.size(); i++) { //for each database row
		already_found_seq = 0x0;
		for (int j = 0; j < database[i].size(); j++) { //for each char in given database row
			if (database[i][j] != ',') {
				seq_candidate = 1 << (database[i][j] - 97); 
				seq_candidate = already_found_seq | seq_candidate;
				if (seq_candidate != already_found_seq) {
					seq_val[0][database[i][j] - 97]++;
					already_found_seq = seq_candidate;
				}
			}
		}
	}
}

void PrefixSpan::scanDatabase() {
	unsigned int seq_candidate;
	unsigned int already_found_seq = 0x0;
	unsigned int items_in_current_element = 0x0;
	int assemblage_offset = 0;
	bool comma_found = false;
	for (int i = 0; i < wanted_seq_ap.size(); i++) {//for each database
		for (int j = 0; j < database_idx[i].size(); j++) { //for each row in given database
			already_found_seq = 0x0;
			comma_found = false;
			for (int k = idx_offset[i][j]; k < database[database_idx[i][j]].size(); k++) {//for each item in given row find appends
				if (database[database_idx[i][j]][k] == ',') {
					comma_found = true;
				}
				else if (comma_found == true) {
					seq_candidate = CHECK_BIT(wanted_seq_ap[i], database[database_idx[i][j]][k] - 97);
					seq_candidate = already_found_seq | seq_candidate;
					if (seq_candidate != already_found_seq) {
						seq_val[i][__popcnt((wanted_seq_ap[i]) & (0x7fffffff >> (32 - database[database_idx[i][j]][k] + 96)))]++;
						already_found_seq = seq_candidate;
					}
				}
			}
			items_in_current_element = last_element[i];
			already_found_seq = 0x0;
			comma_found = false;
			assemblage_offset = __popcnt(wanted_seq_ap[i]);
			for (int k = idx_offset[i][j]; k < database[database_idx[i][j]].size(); k++) {//for each item in given row find assemblages
				if (database[database_idx[i][j]][k] == ',') {
					items_in_current_element = 0x0;
				}
				else if (items_in_current_element != last_element[i]) {
					items_in_current_element = items_in_current_element | CHECK_BIT(last_element[i], database[database_idx[i][j]][k] - 97);
				}
				else {
					seq_candidate = CHECK_BIT(wanted_seq_as[i], database[database_idx[i][j]][k] - 97);
					seq_candidate = already_found_seq | seq_candidate;
					if (seq_candidate != already_found_seq) {
						seq_val[i][__popcnt((wanted_seq_as[i]) & (0x7fffffff >> (32 - database[database_idx[i][j]][k] + 96))) + assemblage_offset]++;
						already_found_seq = seq_candidate;
					}
				}			
			}
		}
	}
}

void PrefixSpan::trimVector(int min_sup) {
	for (int i = seq_val.size() - 1; i > -1; i--) {
		for (int j = seq_val[i].size() - 1; j > -1; j--) {
			if (seq_val[i][j] < min_sup) {
				if (seq[i][j].size() > 1) {
					if (seq[i][j][seq[i][j].size() - 2] == ',') {
						BIT_CLEAR(wanted_seq_ap[i], seq[i][j].back() - 97);
					}
					else {
						BIT_CLEAR(wanted_seq_as[i], seq[i][j].back() - 97);
					}
				}
				else {			
					BIT_CLEAR(wanted_seq_ap[i], seq[i][j].back() - 97);
				}
				seq_val[i].erase(seq_val[i].begin() + j);
				seq[i].erase(seq[i].begin() + j);
			}
		}
		if (seq_val[i].empty()) {
			seq_val.erase(seq_val.begin() + i);
			seq.erase(seq.begin() + i);
			//sup_databases.erase(sup_databases.begin() + i);
		}
	}
}

void PrefixSpan::trimVector2(int min_sup) {
	std::vector<std::vector<int>> seq_val_temp;
	std::vector<std::vector<std::string>> seq_temp;
	bool new_sub_database;
	int iterator = -1;
	for (int i = 0; i < seq_val.size(); i++) {
		new_sub_database = true;
		for (int j = 0; j < seq_val[i].size(); j++) {
			if (seq_val[i][j] < min_sup) {
				if (seq[i][j].size() > 1) {
					if (seq[i][j][seq[i][j].size() - 2] == ',') {
						BIT_CLEAR(wanted_seq_ap[i], seq[i][j].back() - 97);
					}
					else {
						BIT_CLEAR(wanted_seq_as[i], seq[i][j].back() - 97);
					}
				}
				else {
					BIT_CLEAR(wanted_seq_ap[i], seq[i][j].back() - 97);
				}
			}
			else {
				if (new_sub_database) {
					seq_val_temp.push_back({});
					seq_temp.push_back({});
					new_sub_database = false;
					iterator++;
				}
				seq_val_temp[iterator].push_back(seq_val[i][j]);
				seq_temp[iterator].push_back(seq[i][j]);
			}
		}
	}
	seq_val = std::move(seq_val_temp);
	seq = std::move(seq_temp);
}


void PrefixSpan::printVector() {
	for (int i = 0; i < seq_val.size(); i++) {
		for (int j = 0; j < seq_val[i].size(); j++) {
			std::cout << seq[i][j] << " " << seq_val[i][j] << "\n";
		}
		std::cout << "\n";
	}
}

void PrefixSpan::printVector2() {
	if (output.is_open()) {
		for (int i = 0; i < seq_val.size(); i++) {
			for (int j = 0; j < seq_val[i].size(); j++) {
				output << seq[i][j] << " " << seq_val[i][j] << "\n";
			}
		}
	}
	else {
		std::cout << "Unable to open output file";
		exit(0);
	}
}

void PrefixSpan::buildFirstDatabases() {
	unsigned int seq_candidate;
	unsigned int already_found_seq = 0x0;

	new_database_idx.resize(seq[0].size());
	new_idx_offset.resize(seq[0].size());

	for (int i = 0; i < database.size(); i++) { //for each database row
		already_found_seq = 0x0;
		for (int j = 0; j < database[i].size(); j++) { //for each char in given database row
			if (database[i][j] != ',') {
				seq_candidate = CHECK_BIT(wanted_seq_ap[0], database[i][j] - 97);
				seq_candidate = already_found_seq | seq_candidate;
				if (seq_candidate != already_found_seq) {
					new_database_idx[__popcnt((wanted_seq_ap[0]) & (0x7fffffff >> (32 - database[i][j] + 96)))].push_back(database_idx[0][i]);
					new_idx_offset[__popcnt((wanted_seq_ap[0]) & (0x7fffffff >> (32 - database[i][j] + 96)))].push_back(j + 1);
					already_found_seq = seq_candidate;
				}
			}
		}
	}
}

void PrefixSpan::buildDatabases() {
	unsigned int already_found_seq = 0x0;
	unsigned int seq_candidate;
	int prepared_databases = 0;
	int assemblage_offset = 0;
	bool comma_found = false;
	unsigned int items_in_current_element = 0x0;
	//wanted_seq_ap.clear();
	new_database_idx.clear();
	new_idx_offset.clear();
	for (int i = 0; i < seq.size(); i++) {
		//sub_database_size = 0;
		//wanted_seq_ap.push_back(0x0);
		
		for (int j = 0; j < seq[i].size(); j++) {
			//wanted_seq_ap[i] = BIT_SET(wanted_seq_ap[i], seq[i][j].back() - 97);
			//sub_database_size += seq_val[i][j];
			new_database_idx.push_back({});
			new_idx_offset.push_back({});
			//new_database_idx[j].resize(seq_val[i][j]);
			//new_idx_offset[j].resize(seq_val[i][j]);
		}
	}

	for (int i = 0; i < wanted_seq_ap.size(); i++) {//for each database
		assemblage_offset = __popcnt(wanted_seq_ap[i]);
		for (int j = 0; j < database_idx[i].size(); j++) { //for each row in given database
			already_found_seq = 0x0;
			comma_found = false;
			for (int k = idx_offset[i][j]; k < database[database_idx[i][j]].size(); k++) {//for each item in given row
				if (database[database_idx[i][j]][k] == ',') {
					comma_found = true;
				}
				else if(comma_found == true){
					seq_candidate = CHECK_BIT(wanted_seq_ap[i], database[database_idx[i][j]][k] - 97);
					seq_candidate = already_found_seq | seq_candidate;
					if (seq_candidate != already_found_seq) {
						//shifting by 32 bits can result in undefined behavior, so most significant bit is set to zero
						new_database_idx[prepared_databases + __popcnt((wanted_seq_ap[i]) & (0x7fffffff >> (32 - database[database_idx[i][j]][k] + 96)))].push_back(database_idx[i][j]);
						new_idx_offset[prepared_databases + __popcnt((wanted_seq_ap[i]) & (0x7fffffff >> (32 - database[database_idx[i][j]][k] + 96)))].push_back(k + 1);
						already_found_seq = seq_candidate;
					}
				}
			}
			already_found_seq = 0x0;
			comma_found = false;
			items_in_current_element = last_element[i];
			for (int k = idx_offset[i][j]; k < database[database_idx[i][j]].size(); k++) {//for each item in given row find assemblages
				if (database[database_idx[i][j]][k] == ',') {
					items_in_current_element = 0x0;
				}
				else if (items_in_current_element != last_element[i]) {
					items_in_current_element = items_in_current_element | CHECK_BIT(last_element[i], database[database_idx[i][j]][k] - 97);
				}
				else {
					seq_candidate = CHECK_BIT(wanted_seq_as[i], database[database_idx[i][j]][k] - 97);
					seq_candidate = already_found_seq | seq_candidate;
					if (seq_candidate != already_found_seq) {
						new_database_idx[prepared_databases + assemblage_offset + __popcnt((wanted_seq_as[i]) & (0x7fffffff >> (32 - database[database_idx[i][j]][k] + 96)))].push_back(database_idx[i][j]);
						new_idx_offset[prepared_databases + assemblage_offset + __popcnt((wanted_seq_as[i]) & (0x7fffffff >> (32 - database[database_idx[i][j]][k] + 96)))].push_back(k + 1);
						already_found_seq = seq_candidate;
					}
				}
			}
		}
		prepared_databases += assemblage_offset + __popcnt(wanted_seq_as[i]);
	}
}

void PrefixSpan::buildFirstVector() {
	new_seq.clear();
	wanted_seq_ap.clear();
	wanted_seq_as.clear();
	seq_val.clear();
	last_element.clear();
	int seq_iterator = 0;
	for (int i = 0; i < seq.size(); i++) {//for each database
		for (int j = 0; j < seq[i].size(); j++) {//for each sequence in given database
			new_seq.push_back({});
			wanted_seq_ap.push_back(0x0);
			wanted_seq_as.push_back(0x0);
			seq_val.push_back({});

			for (int k = 0; k < seq[i].size(); k++) {//for each sequence in given database create appends
				new_seq[seq_iterator].push_back(seq[i][j]);
				new_seq[seq_iterator][k].push_back(',');
				new_seq[seq_iterator][k].push_back(seq[i][k].back());
				wanted_seq_ap[seq_iterator] = BIT_SET(wanted_seq_ap[seq_iterator], seq[i][k].back() - 97);
				seq_val[seq_iterator].push_back(0);
			}

			last_element.push_back(0x0);
			for (int k = seq[i][j].size(); k > -1; k--) {
				if (seq[i][j][k] == ',') break;
				last_element[last_element.size() - 1] = BIT_SET(last_element[last_element.size() - 1], seq[i][j][k] - 97);
			}


			for (int k = j + 1; k < seq[i].size(); k++) {//for each sequence in given database create assemblages
				new_seq[seq_iterator].push_back(seq[i][j]);
				new_seq[seq_iterator][k - (j + 1) + seq[i].size()].push_back(seq[i][k].back());
				wanted_seq_as[seq_iterator] = BIT_SET(wanted_seq_as[seq_iterator], seq[i][k].back() - 97);
				seq_val[seq_iterator].push_back(0);
			}

			seq_iterator++;
		}	
	}

	seq = std::move(new_seq);
	database_idx = std::move(new_database_idx);
	idx_offset = std::move(new_idx_offset);
}

void PrefixSpan::buildVector() {
	new_seq.clear();
	wanted_seq_ap.clear();
	wanted_seq_as.clear();
	seq_val.clear();
	last_element.clear();
	int assemblage_offset;
	int seq_iterator = 0;
	for (int i = 0; i < seq.size(); i++) {//for each database
		assemblage_offset = seq[i].size();
		for (int j = 0; j < seq[i].size(); j++) {
			if (seq[i][j][seq[i][j].size() - 2] != ',') {
				assemblage_offset = j;
				break;
			}
		}
		for (int j = 0; j < seq[i].size(); j++) {//for each sequence in given database
			new_seq.push_back({});
			wanted_seq_ap.push_back(0x0);
			wanted_seq_as.push_back(0x0);
			seq_val.push_back({});
			last_element.push_back(0x0);
			
			for (int k = 0; k < assemblage_offset; k++) {//create appends from appends
				new_seq[seq_iterator].push_back(seq[i][j]);
				new_seq[seq_iterator][k].push_back(',');
				new_seq[seq_iterator][k].push_back(seq[i][k].back());
				wanted_seq_ap[seq_iterator] = BIT_SET(wanted_seq_ap[seq_iterator], seq[i][k].back() - 97);
				seq_val[seq_iterator].push_back(0);
			}

			for (int k = seq[i][j].size() - 1; k > -1; k--) {
				if (seq[i][j][k] == ',') break;
				last_element[last_element.size() - 1] = BIT_SET(last_element[last_element.size() - 1], seq[i][j][k] - 97);
			}


			for (int k = j + 1; k < assemblage_offset; k++) {//create assemblages from appends
				new_seq[seq_iterator].push_back(seq[i][j]);
				new_seq[seq_iterator][new_seq[seq_iterator].size() - 1].push_back(seq[i][k].back());
				wanted_seq_as[seq_iterator] = BIT_SET(wanted_seq_as[seq_iterator], seq[i][k].back() - 97);
				seq_val[seq_iterator].push_back(0);
			}

			if (j >= assemblage_offset) {//create assemblages from assemblages
				for (int k = j + 1; k < seq[i].size(); k++){
					new_seq[seq_iterator].push_back(seq[i][j]);
					new_seq[seq_iterator][new_seq[seq_iterator].size() - 1].push_back(seq[i][k].back());
					wanted_seq_as[seq_iterator] = BIT_SET(wanted_seq_as[seq_iterator], seq[i][k].back() - 97);
					seq_val[seq_iterator].push_back(0);
				}
			}

			seq_iterator++;
		}
		/*for (int j = assemblage_offset; j < seq[i].size(); j++) {
			new_seq.push_back({});
			wanted_seq_ap.push_back(0x0);
			wanted_seq_as.push_back(0x0);
			seq_val.push_back({});
			prefix.push_back(0x0);

			for (int k = seq[i][j].size(); k > -1; k--) {
				if (seq[i][j][k] == ',') break;
				prefix[prefix.size() - 1] = BIT_SET(prefix[prefix.size() - 1], seq[i][j][k] - 97);
			}
			for (int k = assemblage_offset + 1; k < seq[i].size(); k++) {
				new_seq[seq_iterator].push_back(seq[i][j]);
				new_seq[seq_iterator][new_seq[seq_iterator].size() - 1].push_back(seq[i][k].back());
				wanted_seq_as[seq_iterator] = BIT_SET(wanted_seq_as[seq_iterator], seq[i][k].back() - 97);
				seq_val[seq_iterator].push_back(0);
			}
		}*/
	}
	seq = std::move(new_seq);
	database_idx = std::move(new_database_idx);
	idx_offset = std::move(new_idx_offset);
}


bool PrefixSpan::isEmpty() {
	if (database_idx.empty() == true) { return true; }
	return false;
}