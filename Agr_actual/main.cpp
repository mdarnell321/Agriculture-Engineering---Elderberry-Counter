#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <cstring>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace cv;
Mat brightness_contrast(Mat, double, int);
#define Vector3 Vec3i
#define endl std::endl;


struct cutouts {
	Point a;
	Point b;
};

//global
int thresh = 40, thresh_exp = 7, brightness = 90, contrast = 100/*double*/, min_dist = 10, sens = 15, min_r = 8, max_r = 17;
bool ready_to_write = false;
int start_frame = 1;
int clicking = 0;

Point cur_cutout;
std::vector<struct cutouts> ignore_areas;

std::string path_dir_global = "";

void save_params()
{
	std::ofstream log(path_dir_global + "params.txt", std::ios_base::out);
	log << thresh << "\n" <<thresh_exp << "\n" <<brightness << "\n" <<contrast << "\n" <<min_dist << "\n" <<sens << "\n" <<min_r << "\n" <<max_r;
}
void console_inp()
{

	while(1) {
		std::cout << "\nType which frame you would like to start from in the console to start processing video\n";
		std::string input;
		std::cin >> input;
		if(input.find_first_not_of("-0123456789") == std::string::npos) {
			int temp = stoi(input);
			if(temp <= 0) {
				std::cout << "Integers from 1-infinite are only allowed\n";
				continue;
			}

			std::cout << "Processing is beggining...\n";
			start_frame = temp;
			break;
		} else {
			std::cout << "This isnt a number\n";
			continue;
		}

	}
	ready_to_write = true;
	save_params();
	destroyAllWindows();

}

bool file_exists (std::string p)
{
	struct stat info;
	return stat(p.c_str(), &info) == 0;
}

void load_params()
{
	std::string str = path_dir_global + "params.txt";
	std::ifstream is (str);
	if(file_exists(str)) {
		int i =0;
		for(std::string line; getline(is, line ); ++i) {
			switch(i) {
			case 0:
				thresh = stoi(line);
				break;
			case 1:
				thresh_exp = stoi(line);
				break;
			case 2:
				brightness = stoi(line);
				break;
			case 3:
				contrast = stoi(line);
				break;
			case 4:
				min_dist = stoi(line);
				break;
			case 5:
				sens = stoi(line);
				break;
			case 6:
				min_r = stoi(line);
				break;
			case 7:
				max_r = stoi(line);
				break;
			default:
				break;
			}
		}
	}
}
void update_save_bounds()
{
	std::ofstream log(path_dir_global + "ignore_areas.txt", std::ios_base::out);
	for(int i = 0; i < ignore_areas.size(); ++i) {
		log << ignore_areas[i].a.x << "/" << ignore_areas[i].a.y << "|" << ignore_areas[i].b.x << "/" << ignore_areas[i].b.y << "\n";
	}
}

void mouse(int event, int x, int y, int, void*)
{
	switch (event) {
	case CV_EVENT_RBUTTONDOWN:
		if(ignore_areas.size() > 0) {
			ignore_areas.erase(ignore_areas.end());
			update_save_bounds();
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		if(++clicking >= 3)
			clicking = 0;
		else {
			if(clicking == 1) {
				std::cout << "Point A has been marked\n";
				cur_cutout = Point(x,y);
			}

			else if(clicking == 2) {
				std::cout << "Ignore area has been created\n";
				struct cutouts c {
					cur_cutout, Point(x,y)
				};
				ignore_areas.push_back(c);
				clicking = 0;
				update_save_bounds();
			}
		}
		break;
	}
}
int main(int argc, char **argv)
{

	std::cout << "What is the base frame file. Please use file names with ascending numbers (Ex: frame1, frame2, etc...)?\n";
	std::string input_path;
	while(1) {
		input_path = "";
		std::cin.ignore();
		getline(std::cin, input_path, '\n');
		if(input_path[0] != '/') {
			input_path = "/" + input_path;
		}
		if(file_exists(input_path) == false) {

			std::cout << "This file does not exist\n";
			continue;
		}
		break;
	}
	Mat disp;
	std::string file_base = input_path;
	std::string path_dir = "";
	std::string file_base_name = "";
	std::string file_base_ext = "";
	{
		//73
		//parsing
		int dashcount = 0;
		for(int i =0 ; i < file_base.size() - 1; ++i)
			if(file_base[i] == '/' ||file_base[i] == '\\')
				dashcount ++;

		int cur_dash = 0;

		for(int i =0 ; i < file_base.size(); ++i) {
			if(file_base[i] == '/' ||file_base[i] == '\\') {
				if(++cur_dash == dashcount) {
					path_dir = file_base.substr(0, i+1);
					path_dir_global = path_dir;
					file_base_name = file_base.substr(i+1, (file_base.size() - i+1));
					file_base_ext = "";
					for(int ii = 0; ii < file_base_name.size() ; ++ii) {
						if(file_base_name[ii] == '.') {
							//split @ period
							file_base_ext = file_base_name.substr(ii+1, (file_base_name.size() - ii+1));
							file_base_name = file_base_name.substr(0, ii);

							int cur = -1;
							for(int iii = 0; iii < file_base_name.size(); ++iii) {
								char cur_char = file_base_name[iii];
								if(isdigit(cur_char)) {
									if(cur == -1)
										cur = iii;
								} else
									cur = -1;
							}
							if(cur != -1) {
								start_frame = stoi(file_base_name.substr(cur,file_base_name.size() - cur ));
								std::cout << start_frame << endl;
								file_base_name = file_base_name.substr(0,cur);
							}

							break;
						}
					}
					if(file_base_ext != "")
						break;
				}
			}
		}
		std::cout << file_base_name << endl;
		std::cout << file_base_ext <<endl;
	}
//
	remove((path_dir + "count.txt").c_str());
	std::cout << "Please calibrate the model before processing all the rest of the frames.\n\nNote: All frames need to have roughly the same lighting and positioning conditions.\n";

	new std::thread(&console_inp);
	load_params();
	namedWindow("Trackbars", (640, 200));
	createTrackbar("HSV Value Max", "Trackbars", &thresh, 255);
	createTrackbar("Dilation", "Trackbars", &thresh_exp, 16);
	createTrackbar("Brightness", "Trackbars", &brightness, 100);
	createTrackbar("Contrast", "Trackbars", &contrast, 100);

	createTrackbar("Berry Minimum Distance", "Trackbars", &min_dist, 100);
	createTrackbar("Berry Gradient Sensitivity", "Trackbars", &sens, 50);
	createTrackbar("Berry Minimum Radius", "Trackbars", &min_r, 50);
	createTrackbar("Berry Maximum Radius", "Trackbars", &max_r, 50);

	setTrackbarMin("HSV Value Max", "Trackbars", 1);
	setTrackbarMin("Dilation", "Trackbars", 1);
	setTrackbarMin("Brightness", "Trackbars", 1);
	setTrackbarMin("Contrast", "Trackbars", 1);

	setTrackbarMin("Berry Minimum Distance", "Trackbars", 1);
	setTrackbarMin("Berry Gradient Sensitivity", "Trackbars", 1);
	setTrackbarMin("Berry Minimum Radius", "Trackbars", 1);
	setTrackbarMin("Berry Maximum Radius", "Trackbars", 1);

	std::string str = path_dir_global + "ignore_areas.txt";
	std::ifstream is (str);
	if(file_exists(str)) {
		for(std::string line; getline(is, line );) {
			std::vector<std::string> arr;
			boost::split(arr, line, boost::is_any_of("|"));
			{
				std::vector<std::string> a_arr;
				boost::split(a_arr, arr[0], boost::is_any_of("/"));

				std::vector<std::string> b_arr;
				boost::split(b_arr, arr[1], boost::is_any_of("/"));


				Point a = Point(stoi(a_arr[0]),stoi(a_arr[1]));
				Point b = Point(stoi(b_arr[0]),stoi(b_arr[1]));
				cutouts c {a,b};
				ignore_areas.push_back(c);
			}
		}
	}

	while(1) {

		Mat img = imread(path_dir + file_base_name + std::to_string(start_frame) + "." + file_base_ext);

		Mat img_copy = img.clone();
		Mat hsv;
		cvtColor(img_copy,hsv, COLOR_BGR2HSV);


		//select only the berries
		int hmin = 0,smin = 0, vmin = 0;
		int hmax = 255,smax = 255, vmax = thresh;
		Scalar lower(hmin, smin, vmin);
		Scalar upper(hmax, smax, vmax);
		Mat mask;
		inRange(hsv, lower, upper, mask);
		Mat blank(img_copy.size(), CV_8UC3, cv::Scalar(255, 255, 255));
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(thresh_exp,thresh_exp));
		dilate(mask,mask,kernel);
		img_copy.copyTo(blank,mask);
		img_copy = blank.clone();


		img_copy = brightness_contrast(img_copy, ((double)contrast/100)*3,brightness);

		const bool hugh = true;
		int count = 0;
		if(hugh) {
			Mat gray;
			cvtColor(img_copy, gray, COLOR_BGR2GRAY);
			//medianBlur(gray, gray, 5);
			std::vector<Vec3f> circles;
			HoughCircles(gray, circles, HOUGH_GRADIENT, 1,/*dist*/min_dist,sens, sens, min_r, max_r);
			for( int i = 0; i < circles.size(); i++) {
				//checking if center is inside an ignore area
				Vector3 c = circles[i];
				Point center = Point(c[0], c[1]);
				bool was_ignored = false;
				for(int ii =0; ii < ignore_areas.size(); ++ii) {
					struct cutouts c = ignore_areas[ii];
					unsigned int left_bound = 0;
					unsigned int right_bound = 0;

					unsigned int low_bound = 0;
					unsigned int up_bound = 0;
					if(c.a.x < c.b.x) { // use a's x as left bound
						left_bound = c.a.x;
						right_bound = c.b.x;
					} else {
						left_bound = c.b.x;
						right_bound = c.a.x;
					}

					if(c.a.y < c.b.y) { // use a's y as low bound
						low_bound = c.a.y;
						up_bound = c.b.y;
					} else {
						low_bound = c.b.y;
						up_bound = c.a.y;
					}

					if(center.x >= left_bound && center.x <= right_bound &&
					   center.y >= low_bound && center.y <= up_bound) { // this is in the ignore area
						if(was_ignored == false)
							circles.erase(circles.begin()+i--);
						was_ignored = true;
					}
				}
				if(ready_to_write == false && was_ignored == false) {
					unsigned char radius = c[2];
					circle( img_copy, center, radius, Scalar(255,0,255), 2, LINE_AA);
				}
			}
			count = circles.size();
			std::cout << "Frame " << start_frame << " has " << count << " berries." << endl;
		}
		for(int i = 0; i < ignore_areas.size() ; ++i) {
			struct cutouts c = ignore_areas[i];
			rectangle(img_copy, c.a, c.b, Scalar(355,60,76), 3);
		}
		if(ready_to_write == false) {
			cv::waitKey(1000);
			if(ready_to_write == true) {
				cvDestroyAllWindows();
				continue;
			}
			do {
				cv::namedWindow("circles orig", WINDOW_NORMAL | WINDOW_KEEPRATIO);
				cv::resizeWindow("circles orig", 1080, 720);
				setMouseCallback("circles orig", mouse, &img_copy);
				cv::imshow("circles orig",img_copy);
				cv::waitKey(1);
			} while(clicking != 0);
			if(ready_to_write == true) {
				cvDestroyAllWindows();
				continue;
			}
			cv::waitKey(1000);
			if(ready_to_write == true) {
				cvDestroyAllWindows();
				continue;
			}
			save_params();
		} else {
			std::ofstream log(path_dir + "count.txt", std::ios_base::app | std::ios_base::out);
			log << "Frame " << start_frame << ": " << count <<"\n";
			if(file_exists(path_dir + file_base_name + std::to_string(++start_frame) + "." + file_base_ext) == false) {
				std::cout << "Processing has finished. Please see count.txt in the same directory as images.\n";
				return 0;
			}
		}

	}
	return 0;
}

Mat brightness_contrast(Mat img, double contrast, int brightness)
{

	Mat img_Copy = Mat::zeros( img.size(), img.type() );


	for(unsigned int y = 0; y < img.rows; y++ ) {
		for(unsigned int x = 0; x < img.cols; x++ ) {
			for(unsigned int c = 0; c < img.channels(); c++ ) {
				img_Copy.at<Vec3b>(y,x)[c] =
				    saturate_cast<unsigned char>( contrast*img.at<Vec3b>(y,x)[c] + brightness );
			}
		}
	}
	return img_Copy;
}
