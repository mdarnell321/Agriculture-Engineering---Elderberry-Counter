#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>

bool file_exists (std::string p)
{
	struct stat info;
	return stat(p.c_str(), &info) == 0;
}

int main(int argc, char **argv)
{
	int at_frame = -1;
	NewResp:
	std::string resp;
	std::cout << "Type 'yes' if this is for zeroing, or 'no' if its appending after a frame. (Dont type the apostraphes)\n";
	std::cin >> resp;
	if(resp == "yes")
	{
		
	}
	else if(resp == "no")
	{
		std::cout << "Which frame shall we start appending at?\n";
		std::cin >> at_frame;
	}
	else
	{
		std::cout << "Invalid Response!\n";
		goto NewResp;
	}
	int the_amount= 0;
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
	std::cout << "What is the amount." << std::endl;
	std::cin >> the_amount;

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
	
	
	std::ifstream is (inp_path);
	if(file_exists(inp_path)) {
		std::ofstream log(path_dir + (at_frame == -1 ? "count_zeroed(" : "count_appended(")  + std::to_string(the_amount) + ").txt", std::ios_base::out);
		for(std::string line; getline(is, line );) {
			std::vector<std::string> arr;
			iter_split(arr, line, boost::algorithm::first_finder(": "));
			std::vector<std::string> text_frame_split;
			iter_split(text_frame_split, arr[0], boost::algorithm::first_finder(" "));
			int new_amount = stoi(arr[1]) + (at_frame == -1 || at_frame != -1 && stoi(text_frame_split[1]) >= at_frame ? the_amount : 0);
			std::string transformed = arr[0] + ": " + std::to_string(new_amount);
			log << transformed << "\n";
		}
		std::cout << "Success.\n";
	}
	is.close();
	return 0;
}
