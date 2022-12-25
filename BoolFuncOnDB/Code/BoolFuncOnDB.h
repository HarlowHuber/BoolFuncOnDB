#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>


// named attributes of dataset, not including expanded attributes
std::vector<std::string> attributes;


// the number of attributes. equal to the dimension if there are no expanded attributes.
int attribute_count;


// look-up table for the attributes. the key is the datapoint which corresponds to a certain vector of expanded attributes
std::unordered_map<int, std::vector<std::string>> expanded_attribute_LUT;


// store what expanded attribute has what abbreviation (x10, x11, etc.).  the key is the attribute, and the value is the abbreviation
std::unordered_map<std::string, int> expanded_attribute_nums;


// true if the dataset the labels of annotated images
bool image_labels = false;


// the dimension, which is th number of attributes, not including the expanded ones (if any)
int dimension;


// vector representation of the Boolean function (a row is a clause; e.g. x1x2 w/ 4 dimensions is a row of { 1, 1, 0, 0 }
std::vector<std::vector<int>> boolFunc;


// pair->first is bottom of range (min threshold) and pair->second is top of range (max threshold)
std::vector<std::pair<int, int>> thresholds(dimension, std::make_pair(INT_MIN, INT_MAX));


// retrieve the Boolean function
bool get_boolFunc(std::fstream& dataset);


// retrieve the thresholds if there is any non-Boolean data
void get_thresholds(std::fstream& dataset);


// write the Boolean function and thresholds to the results file
void write_func_and_thresholds(std::fstream& results);


// parse the dataset and store the results of the Boolean function
std::vector<std::pair<std::vector<int>, std::string>> parse_dataset(std::fstream& dataset, std::fstream& results);
