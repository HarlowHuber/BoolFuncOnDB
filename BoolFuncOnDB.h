#include <vector>


/// @brief the dimension of the dataset
int dimension;


/// @brief attributes of dataset
std::vector<std::string> attributes;


std::vector<std::vector<int>> boolFunc;



/// @brief parses a Boolean function into a matrix where each row is a clause of that function
/// @param function a Boolean function in DNF form as a string
/// @param dimension dimension of the dataset in question
/// @return a pointer to a vector of vector<int> that represents the Boolean function
bool parse_boolFunc(std::string function)
{
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

			return false;
		}
	}

	boolFunc.push_back(clause);

	return true;
}


/// @brief 
std::vector<std::pair<int, int>> get_thresholds()
{
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

	return thresholds;
}
