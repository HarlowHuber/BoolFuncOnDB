#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>


// named attributes of dataset
std::vector<std::string> attributes;


// the dimension
int dimension;


// vector representation of the Boolean function (a row is a clause; e.g. x1x2 w/ 4 dimensions is a row of { 1, 1, 0, 0 }
std::vector<std::vector<int>> boolFunc;


// pair->first is bottom of range (min threshold) and pair->second is top of range (max threshold)
std::vector<std::pair<int, int>> thresholds(dimension, std::make_pair(INT_MIN, INT_MAX));


// retrieve the Boolean function
bool get_boolFunc();


// retrieve the thresholds if there is any non-Boolean data
void get_thresholds(std::fstream& dataset);


// write the Boolean function and thresholds to the results file
void write_func_and_thresholds(std::fstream& results);


// parse the dataset and store the results of the Boolean function
std::vector<std::pair<std::vector<int>, std::string>> parse_dataset(std::fstream& dataset, std::fstream& results);
