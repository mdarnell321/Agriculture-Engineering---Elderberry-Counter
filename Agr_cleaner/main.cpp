#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>
#include <cmath>

bool file_exists (std::string p)
{
	struct stat info;
	return stat(p.c_str(), &info) == 0;
}
int main(int argc, char **argv)
{
	std::string inp_path =
	    ""
	    ;

	std::cout << "What is the file path.\n";
	std::cin.ignore();
	getline(std::cin, inp_path, '\n');
	if(inp_path[0] != '/') {
		inp_path = "/" + inp_path;
	}
	if(file_exists(inp_path) == false) {

		std::cout << "This file does not exist\n";
		return 0;
	}
	std::string path_dir = "";
	int dashcount = 0;
	for(int i =0 ; i < inp_path.size() - 1; ++i)
		if(inp_path[i] == '/' ||inp_path[i] == '\\')
			dashcount ++;

	int cur_dash = 0;

	for(int i =0 ; i < inp_path.size(); ++i) {
		if(inp_path[i] == '/' ||inp_path[i] == '\\') {
			if(++cur_dash == dashcount) {
				path_dir = inp_path.substr(0, i+1);
			}
		}
	}
	int steps = 0;
	std::cout << "By how many steps would you like to get the mean?" << std::endl;
	std::cin >> steps;

	std::ifstream is (inp_path);
	char i = 0;
	unsigned int total = 0;
	if(file_exists(inp_path)) {
		std::ofstream log(path_dir + "count_averaged(" + std::to_string(steps) + ").txt", std::ios_base::out);
		for(std::string line; getline(is, line ); ++i) {

			std::vector<std::string> arr;
			iter_split(arr, line, boost::algorithm::first_finder(": "));
			unsigned int amount = stoi(arr[1]);
			total += amount;
			if(i == steps - 1) {
				int avg = (int)round((float)total/steps);
				std::string transformed = arr[0] + ": " + std::to_string(avg);
				i = -1;

				
				log << transformed << "\n";
				
				total = 0;
			}
		}
		std::cout << "Success.\n";
	}
	is.close();
	return 0;
}
