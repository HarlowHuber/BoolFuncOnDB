#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>


// the file that serves as the csv dataset for this program. change as needed.
const std::string filename = "kv_test.csv";


// named attributes of dataset, not including expanded attributes
std::vector<std::string> attributes;


// true if k-values are used
bool kv_used = false;


// the size is the dimension. 0 == '=' // 1 == '<' // 2 == '>' // 3 == '<=' // 4 == '>='
// this data structure represents what each k-value attributes is. e.g. x1>1 & x2<3 is [2, 1] for 
std::vector<std::vector<int>> kv_order;


// the k-value of each attribute, if applicable
std::vector<int> kv_attributes;


// the number of attributes. equal to the dimension if there are no expanded attributes.
int attribute_count;


// look-up table for the attributes. the key is the datapoint which corresponds to a certain vector of expanded attributes
std::unordered_map<int, std::vector<std::string>> expanded_attribute_LUT;


// store what expanded attribute has what abbreviation (x10, x11, etc.).  the key is the attribute, and the value is the abbreviation
std::unordered_map<std::string, int> expanded_attribute_nums;


// true if the dataset the labels of annotated images
bool image_labels = false;


// the dimension, which is the number of attributes, not including the expanded ones (if any)
int dimension;


// vector representation of the Boolean function (a row is a clause; e.g. x1x2 w/ 4 dimensions is a row of { 1, 1, 0, 0 }
std::vector<std::vector<int>> dbFunc;


// pair->first is bottom of range (min threshold) and pair->second is top of range (max threshold)
// these thresholds also define the lower and upper numeric data limit of for k-value
// the outer vector represents the threhold for each attribute. the inner pair is the pair of thresholds.
std::vector<std::pair<int, int>> thresholds(dimension, std::make_pair(INT_MIN, INT_MAX));


// these define thresholds for k-value. these specific thresholds are between k-values, so size is max k-value - 1
// the other thresholds data structure can store an upper and lower bound, however
// the outer vector represents the list of thresholds for each k-value attribute. each inner vector has a size of k-value - 1. 
// the upper and lower limits for the thresholds for a specific attribute is defined by the thresholds data structure above.
std::vector<std::vector<int>> kv_thresholds;


// retrieve the Boolean function
bool get_dbFunc(std::fstream& dataset);


// retrieve the thresholds if there is any non-Boolean data
void get_thresholds(std::fstream& dataset);


// write the Boolean function and thresholds to the results file
void write_func_and_thresholds(std::fstream& results);


// parse the dataset and store the results of the Boolean function
std::vector<std::pair<std::vector<int>, std::string>> parse_dataset(std::fstream& dataset, std::fstream& results);
