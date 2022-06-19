// PrefixSpanSeq2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <math.h>

#include "PrefixSpan.h"

//function for tracking time of given operations
void trackTime(int operation_type) {
	static std::chrono::time_point<std::chrono::steady_clock> start;
	static std::chrono::time_point<std::chrono::steady_clock> stop;
	static std::chrono::microseconds duration[6];
	static std::ofstream timefile("time.txt");
	stop = std::chrono::high_resolution_clock::now();
	switch (operation_type) {
		//load data from disc to RAM
	case 0:
		duration[0] += std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		break;
		//scan database
	case 1:
		duration[1] += std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		break;
		//trim vector
	case 2:
		duration[2] += std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		break;
		//build database
	case 3:
		duration[3] += std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		break;
		//build vector
	case 4:
		duration[4] += std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		break;
		//save data on disc
	case 5:
		duration[5] += std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		break;
		//write time measurements to file
	case 6:
		if (timefile.is_open()) {
			for (int i = 0; i < 6/*sizeof(duration) / sizeof(duration[0])*/; i++) {
				timefile << duration[i].count() << "\n";
			}
			timefile.close();
		}
		else {
			std::cout << "Unable to open time file";
			exit(1);
		}
		break;
		//time tracking initialization - do nothing 
	case 7:
		break;

	default:
		std::cout << "Invalid value passed to trackTime function";
		exit(1);
		break;
	}
	start = std::chrono::high_resolution_clock::now();
}


int main(int argc, char** argv)
{
    auto start = std::chrono::high_resolution_clock::now();

	//enum for measuring exec time of different parts of program
	/*enum TimeMeasure
	{
		disk_to_RAM, scan_db, trim_vector, build_database, build_vector, RAM_to_disk, write_results, init
	};
	trackTime(TimeMeasure::init);*/

    //check argv
    if (argc != 4) {
        std::cout << "Incorrect number of input arguments\n";
        return 0;
    }

    float float_min_sup = std::stof(argv[3]); //minimum support
    if (float_min_sup > 1 || float_min_sup <= 0) {
        std::cout << "Incorrect minimum support value\n";
        return 0;
    }

    std::ifstream file(argv[1]);
    //std::ofstream output(argv[2]);

    //open file
    if (file.is_open()) {
        std::string line;
        std::vector<std::string> database;
        
        //read file
        while (std::getline(file, line)) {
            line.pop_back();
            database.push_back(line);
            //printf("%s\n", line.c_str());
        }
        file.close();

		//trackTime(TimeMeasure::disk_to_RAM);

        const int min_sup = ceil(database.size() * float_min_sup); //minimum support expressed as number of rows in database
        
        PrefixSpan sp(database, argv[2]);

        sp.firstScan();
		//trackTime(TimeMeasure::scan_db);
        sp.trimVector2(min_sup);
		//trackTime(TimeMeasure::trim_vector);
        sp.printVector2();
		//trackTime(TimeMeasure::RAM_to_disk);
        sp.buildFirstDatabases();
		//trackTime(TimeMeasure::build_database);
        sp.buildFirstVector();
		//trackTime(TimeMeasure::build_vector);
        

        while (sp.isEmpty() == false) {
            sp.scanDatabase();
			//trackTime(TimeMeasure::scan_db);
            sp.trimVector2(min_sup);
			//trackTime(TimeMeasure::trim_vector);
            sp.printVector2();
			//trackTime(TimeMeasure::RAM_to_disk);
            sp.buildDatabases();
			//trackTime(TimeMeasure::build_database);
            sp.buildVector();
			//trackTime(TimeMeasure::build_vector);
        }

		//trackTime(TimeMeasure::write_results);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Time taken by function: " << duration.count() << " microseconds\n";
    }

    else {
        std::cout << "Unable to open file";
        return 0;
    }
}