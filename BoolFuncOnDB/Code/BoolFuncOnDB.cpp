#include "BoolFuncOnDB.h"


bool get_boolFunc(std::fstream& dataset)
{
	// ADD  HERE
	// ask user if they want to expand the dataset with those labels that came from any image annotations in the dataset
	if (image_labels)
	{
		std::cout << "Does the user want to add any attributes to the dataset? (1/0): " << std::flush;
		int c;
		std::cin >> c;

		if (c)
		{
			if (image_labels)
			{
				// parse dataset with respect to the links to the JSON of the image labels of the image that each datapoint represents (if it represents any)
				// store image labels (new, expanded attributes) in some data structure, then get that data when that datapoint is parsed in parse_dataset()
				// then, ask, if the user wants to use these new attributes in the boolean function
				// if a particular datapoint does not have these new attributes, it is still acceptable, however.
				
				std::string line, temp;
				int num_datapoint = 0;
				attribute_count = dimension;
				std::vector<std::pair<std::vector<int>, std::string>> datapoints;
				std::getline(dataset, line); // skip first line (attributes)

				while (std::getline(dataset, line))
				{
					num_datapoint++;
					std::stringstream s(line);

					for (int i = 0; std::getline(s, temp, ','); i++)
					{
						// last attribute is json location
						if (i == dimension)
						{
							std::vector<std::string> labels;

							// open json and parse it for new attributes
							std::fstream json;
							json.open(temp);

							// parse json
							std::string line2;

							while (std::getline(json, line2))
							{ 
								if (line2 == "re_features_v3: {")
								{
									std::string label = "";

									// search for all "label"s in json
									while (std::getline(json, line2))
									{
										if (line2[0] == 'l')
										{
											label = line2.substr(8, line2.size() - 1);

											// make sure there are no duplicates
											if (expanded_attribute_nums.find(label) != expanded_attribute_nums.end())
											{
												expanded_attribute_nums.insert({ label, ++attribute_count });
												labels.push_back(label);
											}
										}
									}

									// add new attributes to look up table for a particular datapoint, represented by the key
									expanded_attribute_LUT.insert({ num_datapoint, labels });

									break;
								}
							}

							// close json
							json.close();
						}
					}
				}

				// give the user all the attributes of the dataset
				std::cout << "Here are the attributes of the dataset:" << std::endl;

				for (int i = 0; i < dimension; i++)
				{
					std::cout << attributes[i] << " == x" << i + 1 << std::endl;
				}

				for (auto element : expanded_attribute_nums)
				{
					std::cout << element.first << " == " << element.second << std::endl;
				}

				dataset.clear();
				dataset.seekg(0, std::ios::beg);
			}
			else
			{
				std::cout << "There is no possibility of expansion." << std::endl;
			}
		}
	}

	// get function as a string in DNF form
	std::cout << "Enter a Monotone Boolean function in the disjunctive normal form: " << std::flush;
	std::string function;
	std::getline(std::cin, function);

	if (image_labels)
	{
		std::vector<int> clause(attribute_count);

		// put function into a matrix
		for (int i = 0; i < (int)function.size(); i++)
		{
			// if attribute
			if (function[i] == 'x')
			{
				int start = ++i;
				int temp1 = (int)function.find_first_of(" ", start);
				int temp2 = (int)function.find_first_of("x", start);
				int end = temp1 > temp2 ? temp2 : temp1;
				std::string j = function.substr(start, abs(start - end));
				int k = atoi(j.c_str()) - 1; // -1 on error either from atoi() of user entered x0

				if (-1 < k && k < attribute_count)
				{
					clause[k] = 1;

					// if the attribute is x10 or greater (index location 9 representation)
					// if the attribute is x100 or greater, this will not work---FIX
					if (k >= 9)
					{
						i++;
					}
				}
				else
				{
					std::cout << "The dimension of the Boolean Function does not match the dimension of the dataset." << std::endl;

					return false;
				}
			}
			// if new clause
			else if (function[i] == 'v')
			{
				boolFunc.push_back(clause);
				std::fill(clause.begin(), clause.end(), 0);
				i++;
			}
			else if (function[i] == ' ')
			{
				continue;
			}
			else
			{
				std::cout << "Function is not input as specified." << std::flush;

				return false;
			}
		}

		boolFunc.push_back(clause);
	}
	else
	{
		std::vector<int> clause(dimension);

		// put function into a matrix
		for (int i = 0; i < (int)function.size(); i++)
		{
			// if attribute
			if (function[i] == 'x')
			{
				int start = ++i;
				int temp1 = (int)function.find_first_of(" ", start);
				int temp2 = (int)function.find_first_of("x", start);
				int end = temp1 > temp2 ? temp2 : temp1;
				std::string j = function.substr(start, abs(start - end));
				int k = atoi(j.c_str()) - 1; // -1 on error either from atoi() of user entered x0

				if (-1 < k && k < dimension)
				{
					clause[k] = 1;

					// if the attribute is x10 or greater (index location 9 representation)
					// if the attribute is x100 or greater, this will not work---FIX
					if (k >= 9)
					{
						i++;
					}
				}
				else
				{
					std::cout << "The dimension of the Boolean Function does not match the dimension of the dataset." << std::endl;

					return false;
				}
			}
			// if new clause
			else if (function[i] == 'v')
			{
				boolFunc.push_back(clause);
				std::fill(clause.begin(), clause.end(), 0);
				i++;
			}
			else if (function[i] == ' ')
			{
				continue;
			}
			else
			{
				std::cout << "Function is not input as specified." << std::flush;

				return false;
			}
		}

		boolFunc.push_back(clause);
	}

	return true;
}


void get_thresholds(std::fstream& dataset)
{
	std::string line, temp;
	int c = 0; // count each attribute checked
	std::vector<bool> asked(dimension, false);
	thresholds.resize(dimension);
	std::fill(thresholds.begin(), thresholds.end(), std::make_pair(INT_MIN, INT_MAX));

	// parse dataset for non-boolean attributes
	// while loop goes until all attributes are non-Boolean (c == dimension) or when all datapoints are parsed
	while (std::getline(dataset, line)) 
	{
		// retrieve datapoint
		std::stringstream s(line);

		for (int i = 0; std::getline(s, temp, ','); i++)
		{
			if (temp != "yes" && temp != "y" && temp != "true" && temp != "t" && temp != "1" &&
				temp != "no" && temp != "n" && temp != "false" && temp != "f" && temp != "0" && !asked[i]) 
			{
				asked[i] = true;
				c++;
				int range, threshold;
				std::cout << "Is the threshold for attribute " << attributes[i] << " a range? (1/0): " << std::flush;
				std::cin >> range;

				if (range)
				{
					std::cout << "Please enter the min threshold: " << std::flush;
					std::cin >> threshold;
					thresholds[i].first = threshold;

					std::cout << "Please enter the max threshold: " << std::flush;
					std::cin >> threshold;
					thresholds[i].second = threshold;
				}
				else
				{
					std::cout << "Please enter a threshold for this attribute: " << std::flush;
					std::cin >> threshold;
					std::cout << "Is this threshold a max threshold or min threshold? (1/0): " << std::flush;
					int max;
					std::cin >> max;

					if (max)
					{
						thresholds[i].second = threshold;
					}
					else
					{
						thresholds[i].first = threshold;
					}
				}
			}
		}

		if (c == dimension)
		{
			dataset.clear();
			dataset.seekg(0, std::ios::beg);

			return;
		}
	}

	dataset.clear();
	dataset.seekg(0, std::ios::beg);
}


void write_func_and_thresholds(std::fstream& results)
{
	// write function and thresholds (if any)
	std::string boolFuncStr = "";

	for (int i = 0; i < (int)boolFunc.size(); i++)
	{
		std::string temp = "";

		for (int j = 0; j < dimension; j++)
		{
			if (boolFunc[i][j])
			{
				temp += "x" + std::to_string(j + 1);
			}
		}

		if (!temp.empty() && i > 0)
		{
			boolFuncStr += " v " + temp;
		}
		else if (!temp.empty())
		{
			boolFuncStr += temp;
		}
	}

	results << boolFuncStr + "\n";

	for (int i = 0; i < (int)thresholds.size(); i++)
	{
		if (thresholds[i] != std::pair<int, int>(INT_MIN, INT_MAX)) // if not Boolean
		{
			if (thresholds[i].first == INT_MIN)
			{
				results << "x" << i + 1 << " [-infinity;" << thresholds[i].second << "]\n";
			}
			else if (thresholds[i].second == INT_MAX)
			{
				results << "x" << i + 1 << " [" << thresholds[i].first << ";infinity]\n";
			}
			else
			{
				results << "x" << i + 1 << " [" << thresholds[i].first << ";" << thresholds[i].second << "]\n";
			}
		}
	}

	results << "\n";

	for (auto a : attributes)
	{
		results << a << ",";
	}

	results << "class,,Boolean representation" << "\n";
}

// FIX FOR JSONS!!!
// if datapoint does not contain an attribute which was expanded, then it can still match with boolean function
std::vector<std::pair<std::vector<int>, std::string>> parse_dataset(std::fstream& dataset, std::fstream& results)
{
	std::string line, temp;
	int counter = 0;
	std::vector<std::pair<std::vector<int>, std::string>> datapoints;
	std::getline(dataset, line); // skip first line (attributes)

	while (std::getline(dataset, line))
	{
		counter++;

		// ADD HERE
		// check if datapoint is in look up table
		// if it is, then the datapoint is the size of attribute_count, not the dimension.
		// moreover, those attributes that are the value in the look up table will be immedietely assigned as 1 in the datapoint 
		std::vector<int> datapoint;
		auto datapoint_search = expanded_attribute_LUT.find(counter);

		if (datapoint_search != expanded_attribute_LUT.end())
		{
			datapoint.resize(attribute_count);

			// assign 1s for those attributes that are in search.second
			for (auto attribute : datapoint_search->second)
			{
				auto num = expanded_attribute_nums.find(attribute);

				if (num != expanded_attribute_nums.end())
				{
					datapoint[num->second - 1] = 1;
				}
			}
		}
		else
		{
			datapoint.resize(dimension);
		}


		// retrieve datapoint
		std::stringstream s(line);

		for (int i = 0; std::getline(s, temp, ','); i++)
		{
			for (char& c : temp)
			{
				c = std::tolower(c);
			}
			
			// if boolean
			if (thresholds[i] == std::pair<int, int>(INT_MIN, INT_MAX))
			{
				if (temp == "yes" || temp == "y" || temp == "true" || temp == "t" || temp == "1")
				{
					datapoint[i] = 1;
				}
				else if (temp == "no" || temp == "n" || temp == "false" || temp == "f" || temp == "0" || temp == "") // if temp is nothing, then its 0. FIX???
				{
					datapoint[i] = 0;
				}
			}
			else
			{
				// else if datapoint is not Boolean
				auto threshold = thresholds[i];
				int num = stoi(temp);

				if (threshold.first <= num && num <= threshold.second) // in range
				{
					datapoint[i] = 1;
				}
				else // not in range
				{
					datapoint[i] = 0;
				}
			}
		}

		// ADD HERE
		// IF IMAGE LABELS, then check to see if a particular datapoint is in the look-up table
		// if it is, then the inner for loop is attribute_count, not the dimension
		// otherwise the double for loop works the same.
		// 
		// 
		// 
		// check if datapoint matches a clause in the Boolean function
		// clauses represent if a clause in the Boolean function is true or false for a given datapoint
		std::vector<bool> clauses(boolFunc.size(), true); // if clause i is true, then clauses[i] = true

		if (image_labels)
		{
			for (int i = 0; i < (int)boolFunc.size(); i++)
			{
				if (expanded_attribute_LUT.find(counter) != expanded_attribute_LUT.end())
				{
					for (int j = 0; j < dimension; j++)
					{
						// if boolean function at one point is true and the datapoint at that point is not true, then datapoint is a class of 0
						// otherwise, if clause of function is true and datapoint is also true for those attributes, then it passes and is a class of 1
						if (boolFunc[i][j] && !datapoint[j])
						{
							clauses[i] = false;
							break; // try next clause
						}
					}
				}
				else
				{
					for (int j = 0; j < attribute_count; j++)
					{
						// if boolean function at one point is true and the datapoint at that point is not true, then datapoint is a class of 0
						// otherwise, if clause of function is true and datapoint is also true for those attributes, then it passes and is a class of 1
						if (boolFunc[i][j] && !datapoint[j])
						{
							clauses[i] = false;
							break; // try next clause
						}
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < (int)boolFunc.size(); i++)
			{
				for (int j = 0; j < dimension; j++)
				{
					// if boolean function at one point is true and the datapoint at that point is not true, then datapoint is a class of 0
					// otherwise, if clause of function is true and datapoint is also true for those attributes, then it passes and is a class of 1
					if (boolFunc[i][j] && !datapoint[j])
					{
						clauses[i] = false;
						break; // try next clause
					}
				}
			}
		}

		// determine class
		bool wrote_1 = false;

		for (bool clause : clauses)
		{
			if (clause) // as long as one clause is true, then class is 1
			{
				datapoint.push_back(1);
				datapoints.push_back(std::make_pair(datapoint, line + ",1,")); // index location of class is dimension + 1
				wrote_1 = true;
				break;
			}
		}

		if (!wrote_1) // if no clause is true, then class is 0
		{
			datapoint.push_back(0);
			datapoints.push_back(std::make_pair(datapoint, line + ",0,")); // index location is dimension + 1
		}
	}

	return datapoints;
}


int main()
{
	// open dataset
	std::fstream dataset;
	//dataset.open("dataset.csv", std::ios::in); // read only
	dataset.open("new_houses_json_loc.csv", std::ios::in);
	std::string line, temp;

	if (!std::getline(dataset, line))
	{
		std::cout << "Nothing in dataset." << std::endl;

		return EXIT_FAILURE;
	}

	// get attributes at top of dataset
	std::stringstream s(line);

	while (std::getline(s, temp, ','))
	{
		attributes.push_back(temp);
	}

	// if the dataset has a json of image annotations as an attribute
	if (attributes[attributes.size() - 1] == "json_loc") // location of json on computer
	{
		dimension = (int)attributes.size() - 1;
		image_labels = true;
	}
	else
	{
		dimension = (int)attributes.size();
	}

	// parse dataset
	std::cout << "For the given dataset (dataset.csv in the current directory), the class will be appended to the end of each datapoint"
		<< " by using the Boolean Function that was input." << std::endl;

	if (dataset.is_open())
	{
		if (!get_boolFunc(dataset))
		{
			return EXIT_FAILURE;
		}

		// if there are any non-Boolean attributes, get the thresholds for them
		get_thresholds(dataset);

		// open results file with classes (dataset file is untouched)
		std::fstream results;
		results.open("results.csv", std::ios::out | std::ios::app);
		write_func_and_thresholds(results);

		// parse dataset and store results
		auto datapoints = parse_dataset(dataset, results);

		// sort datapoints by true (1) first, then print results to file
		std::sort(datapoints.begin(), datapoints.end(), [](std::pair<std::vector<int>, std::string> a, std::pair<std::vector<int>, std::string> b)
			{
				return a.first[a.first.size() - 1] > b.first[b.first.size() - 1];
			});

		// write results to file
		for (auto datapoint : datapoints)
		{
			results << datapoint.second << ",=";

			for (auto element : datapoint.first)
			{
				results << element << ",";
			}

			results << "\n";
		}

		results.close();
	}
	else
	{
		std::cout << "file not found";

		return EXIT_FAILURE;
	}

	dataset.close();

	return EXIT_SUCCESS;
}