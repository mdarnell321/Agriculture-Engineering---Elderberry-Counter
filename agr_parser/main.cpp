#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

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
	std::string dir = "";
	int dashcount = 0;
	for(int i =0 ; i < inp_path.size() - 1; ++i)
		if(inp_path[i] == '/' ||inp_path[i] == '\\')
			dashcount ++;

	int cur_dash = 0;

	for(int i =0 ; i < inp_path.size(); ++i) {
		if(inp_path[i] == '/' ||inp_path[i] == '\\') {
			if(++cur_dash == dashcount) {
				dir = inp_path.substr(0, i+1);
			}
		}
	}
	mkdir((dir + "excel").c_str(),0777);


	typedef struct {
		int frame;
		int count;
		float time_ms;
	} temp_format;

	std::vector<temp_format> data_as_time;
	{
		// convert to milliseconds
		std::ifstream isa (inp_path);

		if(file_exists(inp_path)) {
			int last_frame = -1;
			int last_count = -1;
			bool was_seeking_during_cut = false;
			unsigned int i = 0;
			int last_good_dF = 5;
			float cur_ms = 0;
			for(std::string line; getline(isa, line ); i++) {
				std::vector<std::string> internal_split;
				iter_split(internal_split, line, boost::algorithm::first_finder(": "));
				std::vector<std::string> text_frame_split;
				iter_split(text_frame_split, internal_split[0], boost::algorithm::first_finder(" "));

				int berry_count = stoi(internal_split[1]); //cur frame count
				int frame_num = stoi(text_frame_split[1]); //cur frame
				if( i > 0 && last_frame != -1 && last_count != -1) {
					if(frame_num - last_frame > 10) { // this means that this portion has been cut out
						was_seeking_during_cut = true;
					} else {
						if(was_seeking_during_cut == true) { // if we are in a section where we indentified a cut
							was_seeking_during_cut = false;
							//get last frame as well
							cur_ms += (float)((last_good_dF)*(1000))/121;
							data_as_time.push_back(temp_format{last_frame, last_count, cur_ms});
							goto skip;
						} else {
							last_good_dF = frame_num - last_frame;
						}
					}
				}
				if(was_seeking_during_cut == false) {
					cur_ms += (float)((i == 0 ? 0: (frame_num - last_frame))*(1000))/121; // if begin 0 -> else then not
					data_as_time.push_back(temp_format{frame_num, berry_count, cur_ms}); // log this current frame
				}
skip:
				last_frame = frame_num;
				last_count = berry_count;
			}
			std::cout << "Success.\n";
		}
		isa.close();
	}
	remove((dir + "frame.txt").c_str());
	remove((dir + "frame_count.txt").c_str());


	for(unsigned int i = 0; i <data_as_time.size(); ++i) {
		temp_format tf = data_as_time[i];
		temp_format last_tf = data_as_time[i == 0 ? 0 : i - 1];

		std::ofstream a(dir + "excel/frame.txt", std::ios_base::app | std::ios_base::out);
		a << tf.frame << "\n";
		std::ofstream b(dir + "excel/time.txt", std::ios_base::app | std::ios_base::out);
		b << tf.time_ms << "\n";
		std::ofstream c(dir + "excel/frame_count.txt", std::ios_base::app | std::ios_base::out);
		c << tf.count << "\n";
	}


	return 0;

}
