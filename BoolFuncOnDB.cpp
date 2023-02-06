#include "boolFuncOnDB.h"


bool get_dbFunc(std::fstream& dataset)
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
				std::vector<std::pair<std::vector<int>, std::string>> datapoints;

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
											label = line2.substr(8);
											label = label.substr(0, label.size() - 1);

											// make sure there are no duplicates by checking to see if the label is not in the
											if (expanded_attribute_nums.find(label) == expanded_attribute_nums.end())
											{
												expanded_attribute_nums.insert({ label, attribute_count++ });
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
					std::cout << element.first << " == x" << element.second + 1 << std::endl;
				}

				dataset.clear();
				dataset.seekg(0, std::ios::beg);
			}
			else
			{
				attribute_count = dimension;
				std::cout << "There is no possibility of expansion." << std::endl;
			}
		}
		else
		{
			attribute_count = dimension;
			image_labels = false;
		}
	}

	// get function as a string in DNF form
	std::cout << "Enter a Monotone Boolean function in the disjunctive normal form.\n" 
		<< "No parenthesis and spaces between clauses are a must; e.g. \"x1x2 v x3\": " << std::flush;
	std::string function;
	std::cin.clear();
	std::getline(std::cin, function);
	std::vector<int> clause(attribute_count, -1);
	std::vector<int> kv_order_temp(dimension, -1);

	if (image_labels)
	{
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
				dbFunc.push_back(clause);
				std::fill(clause.begin(), clause.end(), -1);
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

		dbFunc.push_back(clause);
	}
	else
	{
		int k = -1;

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
				k = atoi(j.c_str()) - 1; // -1 on error either from atoi() of user entered x0

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
				kv_order.push_back(kv_order_temp);
				dbFunc.push_back(clause);
				std::fill(clause.begin(), clause.end(), -1);
				std::fill(kv_order_temp.begin(), kv_order_temp.end(), -1);
				i++;
			}
			else if (function[i] == ' ')
			{
				continue;
			}
			else if (function[i] == '>')
			{
				// check if >=
				if (i + 2 < (int)function.size() && function[i + 1] == '=')
				{
					kv_order_temp[k] = 4;
					i += 2;
				}
				else if (i + 1 < (int)function.size())
				{
					kv_order_temp[k] = 2;
					i++;
				}
				// else error
				else
				{
					std::cout << "Function is not input as specified." << std::flush;

					return false;
				}

				clause[k] = function[i] - 48;
			}
			else if (function[i] == '<')
			{
				// check if <=
				if (i + 2 < (int)function.size() && function[i + 1] == '=')
				{
					kv_order_temp[k] = 3;
					i += 2;
				}
				else if (i + 1 < (int)function.size())
				{
					kv_order_temp[k] = 1;
					i++;
				}
				// else error
				else
				{
					std::cout << "Function is not input as specified." << std::flush;

					return false;
				}

				clause[k] = function[i] - 48;
			}
			else if (function[i] == '=')
			{
				if (i + 1 < (int)function.size())
				{
					kv_order_temp[k] = 0;
					i++;
				}
				// else error
				else
				{
					std::cout << "Function is not input as specified." << std::flush;

					return false;
				}

				clause[k] = function[i] - 48;
			}
			else if (function[i] == '&')
			{
				continue;
			}
			else
			{
				std::cout << "Function is not input as specified." << std::flush;

				return false;
			}
		}

		kv_order.push_back(kv_order_temp);
		dbFunc.push_back(clause);
	}

	return true;
}


void get_thresholds(std::fstream& dataset)
{
	int use_kv_thresholds;
	std::cout << "Does the user want to use ordinal (ordered from least to greatest) k-values for numeric attributes? (1/0):" << std::flush;
	std::cin >> use_kv_thresholds;

	std::string line, temp;
	int c = 0; // count each attribute checked
	std::vector<bool> asked(dimension, false);
	thresholds.resize(dimension);
	std::fill(thresholds.begin(), thresholds.end(), std::make_pair(INT_MIN, INT_MAX));
	std::vector<bool> kv(dimension, false); // true if there are assigned kv attributes

	if (use_kv_thresholds)
	{
		kv_thresholds.resize(dimension);
		kv_used = true;
	}

	for (int i = 0; i < dimension; i++)
	{
		std::string kv_str = attributes[i];

		try
		{
			if (kv_str.substr(kv_str.length() - 3) == "_kv")
			{
				kv[i] = true;
			}
		}
		catch (std::exception){}

	}

	// parse dataset for non-boolean attributes
	// while loop goes until all attributes are non-Boolean (c == dimension) or when all datapoints are parsed
	while (std::getline(dataset, line)) 
	{
		// retrieve datapoint
		std::stringstream s(line);

		for (int i = 0; std::getline(s, temp, ',') && i < dimension; i++)
		{
			if (temp != "yes" && temp != "y" && temp != "true" && temp != "t" && temp != "1" &&
				temp != "no" && temp != "n" && temp != "false" && temp != "f" && temp != "0" && !asked[i] && kv[i] != true)
			{
				asked[i] = true;
				c++;

				if (use_kv_thresholds)
				{
					// get thresholds for numeric values. k-value => n - 1 thresholds minimum and n + 1 thresholds maximum
					std::cout << "What is the k-value for this attribute '" << attributes[i] << "' (must be integers greater than 1)? Enter: " << std::flush;

					try
					{
						std::cin >> kv_attributes[i];
					}
					catch (std::exception& e) 
					{
						std::cerr << e.what() << std::endl;
						std::cout << "Value  not convertable from string to number (k-value)." << std::endl;

						exit(EXIT_FAILURE);
					}

					// lower bound
					std::cout << "Is there a lower bound for this k-valued attribute? (1/0): " << std::flush;
					int y = 0;
					std::cin >> y;

					if (y)
					{
						std::cout << "Please enter the lower bound: " << std::flush;
								
						try
						{
							std::cin >> thresholds[i].first;
						}
						catch (std::exception& e)
						{
							std::cerr << e.what() << std::endl;
							std::cout << "Value not convertable from string to number (k-value)." << std::endl;
						}
					}
					
					// thresholds between k-values
					std::vector<int> kv_threshold_temp(kv_attributes[i] - 1);
					kv_thresholds.resize(dimension);

					for (int j = 0; j < kv_attributes[i] - 1; j++)
					{
						std::cout << "What is the threshold for between the k-values " << j << " and " << j + 1 << " of attribute x"
							<< i + 1 << "(" << attributes[i] << ")?: " << std::flush;

						try
						{
							std::cin >> kv_threshold_temp[j];
						}
						catch (std::exception& e) 
						{
							std::cerr << e.what() << std::endl;
							std::cout << "Value not convertable from string to number (k-value)." << std::endl;

							exit(EXIT_FAILURE);
						}
					}

					kv_thresholds[i] = kv_threshold_temp;

					// upper bound
					std::cout << "Is there an upper bound for this k-valued attribute? (1/0): " << std::flush;
					std::cin >> y;

					if (y)
					{
						std::cout << "Please enter the upper bound: " << std::flush;

						try
						{
							std::cin >> thresholds[i].second;
						}
						catch (std::exception& e)
						{
							std::cerr << e.what() << std::endl;
							std::cout << "Value not convertable from string to number (k-value)." << std::endl;
						}
					}
				}
				else
				{
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
			else if (kv[i] == true) // retrieve k-value 
			{
				asked[i] = true;
				c++;
				kv_used = true;

				int kv_a = -1;
				
				try
				{
					kv_a = stoi(temp);
				}
				catch (std::exception& e)
				{
					std::cerr << e.what() << std::endl;
					std::cout << "Value in dataset not convertable from string to number (k-value)." << std::endl;

					exit(EXIT_FAILURE);
				}

				if (kv_a >= kv_attributes[i])
				{
					kv_attributes[i] = kv_a + 1;
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
	std::string dbFuncStr = "";

	for (int i = 0; i < (int)dbFunc.size(); i++)
	{
		std::string temp = "";

		for (int j = 0; j < (int)dbFunc[i].size(); j++)
		{
			if (dbFunc[i][j] > 0 || kv_used && !dbFunc[i][j])
			{
				if (!temp.empty())
				{
					temp += " & ";
				}

				temp += "x" + std::to_string(j + 1);
					
				if (kv_used)
				{
					if (kv_order[i][j] == 0)
					{
						temp += "=" + std::to_string(dbFunc[i][j]);
					}
					else if (kv_order[i][j] == 1)
					{
						temp += "<" + std::to_string(dbFunc[i][j]);
					}
					else if (kv_order[i][j] == 2)
					{
						temp += ">" + std::to_string(dbFunc[i][j]);
					}
					else if (kv_order[i][j] == 3)
					{
						temp += "<=" + std::to_string(dbFunc[i][j]);
					}
					else if (kv_order[i][j] == 4)
					{
						temp += ">=" + std::to_string(dbFunc[i][j]);
					}
				}
			}
		}

		if (!temp.empty() && i > 0)
		{
			dbFuncStr += " v " + temp;
		}
		else if (!temp.empty())
		{
			dbFuncStr += temp;
		}
	}

	results << dbFuncStr + "\n";

	for (int i = 0; i < (int)thresholds.size(); i++)
	{
		if (thresholds[i] != std::pair<int, int>(INT_MIN, INT_MAX)) // if not Boolean
		{
			if (kv_used)
			{
				results << "x" << i + 1 << "; k = " << kv_attributes[i] << ";";

				if (thresholds[i].first == INT_MIN)
				{
					results << " lower/upper bound [-infinity;";
				}
				else
				{
					results << " lower/upper bound [" << thresholds[i].first << ";";
				}

				if (thresholds[i].second == INT_MAX)
				{
					results << "infinity]; ";
				}
				else
				{
					results << thresholds[i].second << "]; ";
				}

				results << "kv_thresholds [";

				if (i < kv_thresholds.size())
				{
					for (int j = 0; j < kv_thresholds[i].size(); j++)
					{
						results << kv_thresholds[i][j] << ";";
					}
				}

				results << "]\n";
			}
			else
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
		else
		{
			if (kv_used)
			{
				results << "x" << i + 1 << "; k = " << kv_attributes[i] << ";";

				if (thresholds[i].first == INT_MIN)
				{
					results << " lower/upper bound [-infinity;";
				}
				else
				{
					results << " lower/upper bound [" << thresholds[i].first << ";";
				}

				if (thresholds[i].second == INT_MAX)
				{
					results << "infinity] ";
				}
				else
				{
					results << thresholds[i].second << "] ";
				}

				results << "kv_thresholds [";

				if (i < kv_thresholds.size())
				{
					for (int j = 0; j < kv_thresholds[i].size(); j++)
					{
						results << kv_thresholds[i][j] << ";";
					}
				}

				results << "]\n";
			}
		}
	}

	results << "original data";

	for (int i = 0; i < dimension + 2; i++)
	{
		results << ",";
	}

	if (kv_used)
	{
		results << "k-value representation (can differ if there are numeric attributes)\n";
	}
	else
	{
		results << "Boolean representation (can differ if there are numeric attributes)\n";
	}
	

	for (auto a : attributes)
	{
		results << a << ",";
	}

	results << "function value";

	// print attributes for Boolean represenation section
	std::vector<std::string> temp_vec(attribute_count - dimension);

	for (auto element : expanded_attribute_nums)
	{
		temp_vec[element.second - dimension] = element.first;
	}

	results << ",,";

	for (int i = 0; i < dimension; i++)
	{
		results << attributes[i] << ",";
	}


	for (auto element : temp_vec)
	{
		results << element << ",";
	}

	results << "function value\n";
}


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

		// check if datapoint is in look up table
		// if it is, then the datapoint is the size of attribute_count, not the dimension.
		// moreover, those attributes that are the value in the look up table will be immedietely assigned as 1 in the datapoint 
		std::vector<int> datapoint(attribute_count, 0);
		auto datapoint_search = expanded_attribute_LUT.find(counter);

		if (datapoint_search != expanded_attribute_LUT.end())
		{
			//datapoint.resize(attribute_count);

			// assign 1s for those attributes that are in search.second
			for (auto attribute : datapoint_search->second)
			{
				auto num = expanded_attribute_nums.find(attribute);

				if (num != expanded_attribute_nums.end())
				{
					datapoint[num->second] = 1;
				}
			}
		}

		// retrieve datapoint
		std::stringstream s(line);

		for (int i = 0; std::getline(s, temp, ',') && i < dimension; i++)
		{
			for (char& c : temp)
			{
				c = std::tolower(c);
			}
			
			// if boolean or k-value
			if (thresholds[i] == std::pair<int, int>(INT_MIN, INT_MAX))
			{
				if (kv_used)
				{
					// kv_thresholds for numeric data
					if (kv_thresholds[i].size() > 0)
					{
						int num = -1;
						int bound;

						try
						{
							num = stoi(temp);
						}
						catch (std::exception& e)
						{
							std::cerr << e.what() << std::endl;

							exit(EXIT_FAILURE);
						}

						// for each k-value 
						for (int j = 0; j < kv_attributes[i]; j++)
						{
							// first kv can have a lower bound
							if (j == 0)
							{
								bound = thresholds[i].first;

								if (bound < num && num <= kv_thresholds[i][j])
								{
									datapoint[i] = j;
									break;
								}
							}
							// last kv can have upper bound
							else if (j == kv_attributes[i] - 1)
							{
								bound = thresholds[i].second;

								if (kv_thresholds[i][j - 1] < num && num <= bound)
								{
									datapoint[i] = j;
									break;
								}
							}
							// else the threshold is in the middle of the k-va
							else
							{
								if (kv_thresholds[i][j - 1] < num && num <= kv_thresholds[i][j])
								{
									datapoint[i] = j;
									break;
								}
							}
						}
					}
					// else already kv
					else
					{
						if (isdigit(temp[0])) // else kv; plus, check if number is convertable to an int before putting it in datapoint
						{
							try
							{
								datapoint[i] = stoi(temp);
							}
							catch (std::exception& e)
							{
								std::cerr << e.what() << std::endl;
								std::cout << "Value in dataset not convertable from string to number (k-value)." << std::endl;

								exit(EXIT_FAILURE);
							}
						}
					}
				}
				else if (temp == "yes" || temp == "y" || temp == "true" || temp == "t" || temp == "1")
				{
					datapoint[i] = 1;
				}
				else if (temp == "no" || temp == "n" || temp == "false" || temp == "f" || temp == "0" || temp == "") // if temp is nothing, then its 0. FIX???
				{
					datapoint[i] = 0;
				}
			}
			else if (kv_used)
			{
				// kv_thresholds for numeric data
				if (kv_thresholds[i].size() > 0)
				{
					int num = -1;
					int bound;

					try
					{
						num = stoi(temp);
					}
					catch (std::exception& e)
					{
						std::cerr << e.what() << std::endl;

						exit(EXIT_FAILURE);
					}

					// for each k-value 
					for (int j = 0; j < kv_attributes[i]; j++)
					{
						// first kv can have a lower bound
						if (j == 0)
						{
							bound = thresholds[i].first;

							if (bound <= num && num <= kv_thresholds[i][j])
							{
								datapoint[i] = j;
								break;
							}
						}
						// last kv can have upper bound
						else if (j == kv_attributes[i] - 1)
						{
							bound = thresholds[i].second;

							if (kv_thresholds[i][j - 1] <= num && num <= bound)
							{
								datapoint[i] = j;
								break;
							}
						}
						// else the threshold is in the middle of the k-va
						else 
						{
							if (kv_thresholds[i][j - 1] <= num && num <= kv_thresholds[i][j])
							{
								datapoint[i] = j;
								break;
							}
						}
					}
				}
				// else already kv
				else
				{
					if (isdigit(temp[0])) // else kv; plus, check if number is convertable to an int before putting it in datapoint
					{
						try
						{
							datapoint[i] = stoi(temp);
						}
						catch (std::exception& e)
						{
							std::cerr << e.what() << std::endl;
							std::cout << "Value in dataset not convertable from string to number (k-value)." << std::endl;

							exit(EXIT_FAILURE);
						}
					}
				}
			}
			else
			{
				// else if datapoint is not Boolean
				auto threshold = thresholds[i];
				int num = -1;

				try
				{
					num = stoi(temp);
				}
				catch (std::exception& e)
				{
					std::cerr << e.what() << std::endl;

					exit(EXIT_FAILURE);
				}

				if (threshold.first < num && num <= threshold.second) // in range
				{
					datapoint[i] = 1;
				}
				else // not in range
				{
					datapoint[i] = 0;
				}
			}
		}

		// IF IMAGE LABELS, then check to see if a particular datapoint is in the look-up table
		// if it is, then the inner for loop is attribute_count, not the dimension
		// otherwise the double for loop works the same.
		// 
		// 
		// 
		// check if datapoint matches a clause in the Boolean function
		// clauses represent if a clause in the Boolean function is true or false for a given datapoint
		std::vector<bool> clauses(dbFunc.size(), true); // if clause i is true, then clauses[i] = true

		if (image_labels && !kv_used)
		{
			for (int i = 0; i < (int)dbFunc.size(); i++)
			{
				if (expanded_attribute_LUT.find(counter) == expanded_attribute_LUT.end())
				{
					bool stop = false;

					for (int j = 0; j < dimension; j++)
					{
						// if boolean function at one point is true and the datapoint at that point is not true, then datapoint is a class of 0
						// otherwise, if clause of function is true and datapoint is also true for those attributes, then it passes and is a class of 1
						if (dbFunc[i][j] && !datapoint[j])
						{
							clauses[i] = false;
							stop = true;
							break; // try next clause
						} 
						else if (dbFunc[i][j] && datapoint[j]) 
						{
							stop = true;
						}
					}

					// make sure that a clause is false:
					//  if a clause is all zeroes until an expanded attribute, 
					// it will be falsely marked as true becauses clauses is constructed with all values as true
					if (!stop)
					{
						for (int j = dimension; j < attribute_count; j++)
						{
							if (dbFunc[i][j])
							{
								clauses[i] = false;
							}
						}
					}
				}
				else
				{
					for (int j = 0; j < attribute_count; j++)
					{
						// if boolean function at one point is true and the datapoint at that point is not true, then datapoint is a class of 0
						// otherwise, if clause of function is true and datapoint is also true for those attributes, then it passes and is a class of 1
						if (dbFunc[i][j] && !datapoint[j])
						{
							clauses[i] = false;
							break; // try next clause
						}
					}
				}
			}
		}
		else if (!kv_used)
		{
			for (int i = 0; i < (int)dbFunc.size(); i++)
			{
				for (int j = 0; j < dimension; j++)
				{
					// if boolean function at one point is true and the datapoint at that point is not true, then datapoint is a class of 0
					// otherwise, if clause of function is true and datapoint is also true for those attributes, then it passes and is a class of 1
					if (dbFunc[i][j] && !datapoint[j])
					{
						clauses[i] = false;
						break; // try next clause
					}
				}
			}
		}
		else if (image_labels && kv_used)
		{
			// ADD HERE
		}
		else if (kv_used)
		{
			for (int i = 0; i < (int)dbFunc.size(); i++)
			{
				for (int j = 0; j < dimension; j++)
				{
					if (kv_order[i][j] == 0) // if dbFunc is true, then datapoint must be true
					{
						if (dbFunc[i][j] != -1 && datapoint[j] != dbFunc[i][j]) // check if false
						{
							clauses[i] = false;
							break; // try next clause
						}
					}
					else if (kv_order[i][j] == 1) // datapoint[j] must be less than dbFunc to be true
					{
						if (dbFunc[i][j] != -1 && datapoint[j] >= dbFunc[i][j])  // check if false
						{
							clauses[i] = false;
							break; // try next clause
						}
					}
					else if (kv_order[i][j] == 2) // datapoint[j] must be greater than the dbFunc to be true in this case because function was input with >
					{
						if (dbFunc[i][j] != -1 && datapoint[j] <= dbFunc[i][j]) // dbFunc[i] [0, 1, 0, 0]]; datapoint [0, 2, 0, 0]; datapoint has class of true
						{
							clauses[i] = false;
							break; // try next clause
						}
					}
					else if (kv_order[i][j] == 3) // datapoint[j] must be <= to be true
					{
						if (dbFunc[i][j] != -1 && datapoint[j] > dbFunc[i][j])
						{
							clauses[i] = false;
							break; // try next clause
						}
					}
					else if (kv_order[i][j] == 4) // datapoint[j] must be >= to be true
					{
						if (dbFunc[i][j] != -1 && datapoint[j] < dbFunc[i][j])
						{
							clauses[i] = false;
							break; // try next clause
						}
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
	dataset.open(filename, std::ios::in);
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

	attribute_count = dimension;
	kv_attributes.resize(dimension);
	std::fill(kv_attributes.begin(), kv_attributes.end(), 2);

	// parse dataset
	std::cout << "For the given dataset (dataset.csv in the current directory), the class will be appended to the end of each datapoint"
		<< " by using the Boolean Function that was input." << std::endl;

	if (dataset.is_open())
	{
		if (!get_dbFunc(dataset))
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

			for (int i = 0; i < attribute_count; i++)
			{
				if (datapoint.first[i])
				{
					results << datapoint.first[i] << ",";
				}
				else if (i < dimension)
				{
					results << datapoint.first[i] << ",";
				}
				else
				{
					results << ",";
				}
			}

			results << datapoint.first[attribute_count] << "\n"; // datapoint is + 1 because of CLASS/value
		}

		results << "\n";
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