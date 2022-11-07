#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

int main()
{
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
	std::vector<std::string> attributes;

	while (std::getline(s, temp, ','))
	{
		attributes.push_back(temp);
	}

	int dimension = (int)attributes.size();

	// get function as a string in DNF form
	std::cout << "Enter a Monotone Boolean function in the disjunctive normal form: " << std::flush;
	std::string function;
	std::getline(std::cin, function);
	std::vector<std::vector<int>> boolFunc;
	std::vector<int> clause(dimension);

	// put function into a matrix
	for (int i = 0; i < (int)function.size(); i++)
	{
		// if attribute
		if (function[i] == 'x')
		{
			int j = function[++i] - 49; // -48 because ASCII value, and another -1 because x1 corresponds to clause[0]
			clause[j] = 1;
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

			return EXIT_FAILURE;
		}
	}

	boolFunc.push_back(clause);

	// pair->first is bottom of range (min threshold) and pair->second is top of range (max threshold)
	std::vector<std::pair<int, int>> thresholds(dimension, std::make_pair(INT_MIN, INT_MAX));

	// non-Boolean attributes
	for (int i = 0; i < dimension; i++)
	{
		std::cout << "Is attribute " << attributes[i] << " Boolean? (1/0): " << std::flush;
		int boolean;
		std::cin >> boolean;

		if (!boolean)
		{
			int range, threshold;
			std::cout << "Is the threshold a range? (1/0): " << std::flush;
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
				std::cout << "Is this threshold a max threshold (1) or min threshold (0)?" << std::flush;
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

	// parse dataset
	std::cout << "For the given dataset (dataset.csv in the current directory), the class will be appended to the end of each datapoint"
		<< " by using the Boolean Function that was input.";

	if (dataset.is_open())
	{
		// open results file with classes (dataset file is untouched)
		std::fstream results;
		results.open("results.csv", std::ios::out | std::ios::app);

		// write function and thresholds (if any)
		std::string boolFuncStr = "";

		for (int i = 0; i < (int)boolFunc.size(); i++)
		{
			std::string temp = "";

			for (int j = 0; j < dimension; j++)
			{
				if (boolFunc[i][j]) temp += "x" + std::to_string(j + 1);
			}

			if (!temp.empty() && i > 0) boolFuncStr += " v " + temp;
			else if (!temp.empty()) boolFuncStr += temp;
		}

		results << boolFuncStr + "\n";

		for (int i = 0; i < (int)thresholds.size(); i++)
		{
			if (thresholds[i] != std::pair<int, int>(INT_MIN, INT_MAX)) // if not Boolean
			{
				results << "x" << i + 1 << " [" << thresholds[i].first << ";" << thresholds[i].second << "]\n";
			}
		}

		results << "\n";

		for (auto a : attributes)
		{
			results << a << ",";
		}

		results << "class" << "\n";
		std::string line;
		int counter = 0;

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
					results << line << ",1\n";
					wrote_1 = true;
					break;
				}
			}

			if (!wrote_1) // if no clause is true, then class is 0
			{
				results << line << ",0\n";
			}
		}
	}
	else
	{
		std::cout << "file not found";

		return EXIT_FAILURE;
	}

	dataset.close();

	return EXIT_SUCCESS;
}