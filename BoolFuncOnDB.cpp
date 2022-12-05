#include "BoolFuncOnDB.h"


bool get_boolFunc()
{
	// get function as a string in DNF form
	std::cout << "Enter a Monotone Boolean function in the disjunctive normal form: " << std::flush;
	std::string function;
	std::getline(std::cin, function);
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

				if (k >= 9) // if the attribute is x10 or greater (index location 9 representation)
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

	return true;
}


void get_thresholds(std::fstream& dataset)
{
	std::string line, temp;
	int c = 0;
	std::vector<bool> asked(dimension, false);
	thresholds.resize(dimension);
	std::fill(thresholds.begin(), thresholds.end(), std::make_pair(INT_MIN, INT_MAX));

	// parse dataset for non-boolean attributes
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


std::vector<std::pair<std::vector<int>, std::string>> parse_dataset(std::fstream& dataset, std::fstream& results)
{
	std::string line, temp;
	int counter = 0;
	std::vector<std::pair<std::vector<int>, std::string>> datapoints;
	std::getline(dataset, line); // skip first line (attributes)

	while (std::getline(dataset, line))
	{
		counter++;
		std::vector<int> datapoint(dimension);

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
				else if (temp == "no" || temp == "n" || temp == "false" || temp == "f" || temp == "0")
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

		// check if datapoint matches a clause in the Boolean function
		// clauses represent if a clause in the Boolean function is true or false for a given datapoint
		std::vector<bool> clauses(boolFunc.size(), true); // if clause i is true, then clauses[i] = true

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
			datapoints.push_back(std::make_pair(datapoint, line + ",0,"));// index location is dimension + 1
		}
	}

	return datapoints;
}


int main()
{
	// open dataset
	std::fstream dataset;
	dataset.open("dataset.csv", std::ios::in); // read only
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

	dimension = (int)attributes.size();

	if (!get_boolFunc())
	{
		return EXIT_FAILURE;
	}

	// parse dataset
	std::cout << "For the given dataset (dataset.csv in the current directory), the class will be appended to the end of each datapoint"
		<< " by using the Boolean Function that was input." << std::endl;

	if (dataset.is_open())
	{
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