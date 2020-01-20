#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sstream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "cxxopts.hpp"
#include "polygon.hpp"

using namespace cv;
using std::string;
using std::vector;

std::vector<Mat> frames;

int mode = 0;
int cur_obj = 0;
const char* WHNDL = "IntermediateProc";

/** Returns a mask after converting src image to HSV space, and auto-finding
 * parameters
 */
Mat drawMask(const Mat& src, const std::string& filename) {
	//std::cout << "Building mask" << std::endl;
	Mat mask; // Mask to be returned
	mask = Mat::zeros(src.size(), CV_8UC1);

	std::fstream filter(filename, std::fstream::in);
	std::string line;

	while (std::getline(filter, line)) {
		bool fg;
		//std::cout << line << "\n";

		if (line[0] == '#' || line.size() == 0) {
			continue;
		} else if (line[0] == 'f') {
			fg = true;
		} else if (line[0] == 'b') {
			fg = false;
		} else {
			std::cerr << "Error. First char of filter line is not 'f' or 'b'. Line: " << line << "\n";
			exit(2);
		}

		if (line[2] == 'p') { //Positional mask
			std::stringstream ss(line);
			ss.seekg(4);
			int x, y;
			ss >> x >> y;
			//std::cout << "Adding point. Foreground(" << fg << "), Coords(" << x << "," << y <<")\n";
			mask.at<unsigned char>(y, x) = (fg == true? 255 : 128);
		} else if (line[2] == 'r') { //Rectangle
			Point p1, p2;
			std::stringstream ss(line);
			ss.seekg(4);
			int x, y;
			ss >> x >> y;
			p1.x = x;
			p1.y = y;

			ss >> x >> y;
			p2.x = x;
			p2.y = y;
			//std::cout << "Adding rectangle. Foreground(" << fg << "), p1(" << p1.x << "," << p1.y <<")"
			     //" p2(" << p2.x << "," << p2.y << ")\n";
			rectangle(mask, p1, p2, (fg?255:128), -1);
		} else if (line[2] == 'h') { //HSV mask filter
			std::stringstream ss(line);
			ss.seekg(4);

			float hlow, slow, vlow;
			float hhigh, shigh, vhigh;
			unsigned short int openings;

			ss >> openings >> hlow >> slow >> vlow >> hhigh >> shigh >> vhigh;
			//std::cout << "Adding HSV filter. Foreground(" << fg << "). Openings(" << openings << "). "
		//		"Low(" << hlow << "," << slow << "," << vlow << "). "
		//		"High(" << hhigh << "," << shigh << "," << vhigh << ").\n";

			Mat hsv_img, temp_bin;
			cvtColor(src, hsv_img, COLOR_BGR2HSV); // Converts to HSV
			inRange(hsv_img, Scalar(hlow, slow, vlow), Scalar(hhigh, shigh, vhigh), temp_bin);
	
			if (openings != 0) {
				erode(temp_bin, temp_bin, Mat(), Point(-1, 1), openings);
				dilate(temp_bin, temp_bin, Mat(), Point(-1, 1), openings);
			}

			if (fg) {
				add(mask, temp_bin, mask, noArray(), CV_8UC1);
			} else { 
				addWeighted(mask, 1, temp_bin, 0.5, 0, mask, CV_8UC1);
			}
		} else {
			std::cerr << "Error. Unknown filter. Line: " << line << "\n";
			exit(3);
		}
	}
	mask.convertTo(mask, CV_32SC1);
	return mask;
}

int main(int argc, char** argv) {
	cxxopts::Options options("Auto Segmenter", "Automatically segments an image or video according to given input. For more details about filters please use option --filter_help\n"
			"It is mandatory to have an input, a filter and at least one output (either media or contours file).");
	options.add_options()
		("h,help", "Shows full help")
		("filter_help", "Shows help on configuring filter files.")
		("f,filter", "Filter file. Use --filter_help for specific filter configuration.", cxxopts::value<std::string>())
		("i,image", "Input and output files are of type image. Cannot be used together with --video. One of either is obligatory.")
		("v,video", "Input and output files are of type video. Cannot be used together with --image. One of either is obligatory.")
		("m,media", "Input media.", cxxopts::value<std::string>())
		("o,output", "Output file. Output will be written as the same type of input file.", cxxopts::value<std::string>())
		("p,poly", "Output file. Output one WKT polygon per line.", cxxopts::value<std::string>())
		("b,blur", "Blurres the image before applying segmentation. This option has no effect on outputs, just on contour definition.", cxxopts::value<std::string>());

	if (argc==1) {
		std::cout << options.help() << std::endl;
		return 0;
	}
	auto result = options.parse(argc, argv);

	if (result["help"].as<bool>()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	if (result["filter_help"].as<bool>()) { //Prints filter file help and exit
		std::cout << "Filter files are text files with one filter per line\n";
		std::cout << "Each filter is composed of 3 blocks, in the format: <roi> <type> <params>\n";
		std::cout << "ROI (region of interest): a single char denoting \'b\' or \'f\'. Background \'b\' denotes regions of no interest, while foreground \'f\' denotes region of interest.\n";
		std::cout << "Type: a single char denoting the type of the filter. Currently, we support HSV filters and positional filters.\n";
		std::cout << "  HSV: char \'h\'. Accepts 7 params, space separated. Generates a binary threshold filter where:\n";
		std::cout << "    openings=param[0]\n"
			"    h_low=param[1]\n"
			"    s_low=param[2]\n"
			"    v_low=param[3]\n"
			"    h_high=param[4]\n"
			"    s_high=param[5]\n"
			"    v_high=param[6]\n";
		std::cout << "  Positional: \'p\'. Accepts 2 parameters, x and y of position.\n";
		return 0;
	}

	if (result["image"].as<bool>() && result["video"].as<bool>()) {
		std::cout << "Cannot use both --image and --video options together.\n";
		return 1;
	}

	if (!(result["image"].as<bool>() || result["video"].as<bool>())) {
		std::cout << "One of --image or --video is mandatory.\n";
		return 2;
	}

	if (!result.count("media") && !result.count("filter")) {
		std::cout << "Error. Need to specify input and filter file.\n";
		return 3;
	}

	if (result["image"].as<bool>()) {
		Mat image; // Original image
		image = imread(result["media"].as<std::string>());
		if (image.empty()) {
			std::cout << "Error - could not read file " << result["media"].as<std::string>() << "as image.\n";
			exit(2);
		}

		Mat mask; // Mask to hold the values
		mask = drawMask(image, result["filter"].as<std::string>());

		// Shows pre-segmentation mask
		{
			Mat maskshow;
			namedWindow("Pre-segmentation mask", WINDOW_NORMAL);
			mask.convertTo(maskshow, CV_8UC3);
			cvtColor(maskshow, maskshow, COLOR_GRAY2BGR);
			addWeighted(image, 0.5, maskshow, 0.5, 0, maskshow, CV_8UC3);
			imshow("Pre-segmentation mask", maskshow);
		}

		Mat proc;
		if (result.count("b")) {
			int size = std::stoi(result["b"].as<std::string>());
			std::cout << "Blur size: " << size << std::endl;
			blur(image, proc, Size(size, size));
		} else {
			proc = image;
		}

		watershed(proc, mask);
		// Shows mask watershed
		{
			Mat maskshow;
			namedWindow("Segmented", WINDOW_NORMAL);
			mask.convertTo(maskshow, CV_8UC3);
			cvtColor(maskshow, maskshow, COLOR_GRAY2BGR);
			addWeighted(image, 0.5, maskshow, 0.5, 0, maskshow, CV_8UC3);
			imshow("Segmented", maskshow);
		}

		//Finds the largest contour
		std::vector<std::vector<Point>> vertexes;
		Mat binary;
		mask.convertTo(binary, CV_32FC1);
		threshold(binary, binary, 200, 255, THRESH_BINARY);
		binary.convertTo(binary, CV_8UC1);
		findContours(binary, vertexes, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		size_t biggest = 0;
		size_t biggest_size = 0;
		for (size_t i = 0; i < vertexes.size(); ++i) {
			if (vertexes[i].size() > biggest_size) {
				biggest = i;
				biggest_size = vertexes[i].size();
			}
		}

		// Generates the overlay
		Mat segmented; // Segmented image with the overlay
		image.convertTo(segmented, CV_8UC3);
		drawContours(segmented, vertexes, biggest, Scalar(255, 255, 255), -1);
		addWeighted(segmented, 0.5, image, 0.5, 0, segmented, CV_8UC3);
		imshow("Largest polygon", segmented);

		if (result.count("output")) { //Saves image
			imwrite(result["output"].as<std::string>(), segmented);
		}

		if (result.count("poly")) { //Saves largest contour

			//Saves it to a Polygon, then to WKT
			Polygon pol;
			for (Point p : vertexes[biggest]) {
				pol.points.emplace_back(p.x, p.y);
			}
			std::fstream fs(result["poly"].as<std::string>(), std::fstream::out);
			pol.save(fs, Polygon::FileType::FILE_WKT);
		}

		/** Checks contour. Uncomment to wait for q press showing windows.
		unsigned char c;
		while ((c = waitKey(0)) != 'q') {
		} */
	} else if (result["video"].as<bool>()) {
		Mat cur_frame;
		VideoCapture vid(result["media"].as<std::string>());
		double max_frames = vid.get(CAP_PROP_FRAME_COUNT);

		//Output video
		VideoWriter w;
		if (result.count("output")) {
			vid >> cur_frame; // Pre-read first frame to get sizes
			w = VideoWriter(result["output"].as<std::string>(), VideoWriter::fourcc('F', 'M', 'P', '4'),
					vid.get(CAP_PROP_FPS), cur_frame.size());
			vid.set(CAP_PROP_POS_FRAMES, 0);
		}

		//Output polygons
		std::fstream fs;
		if (result.count("poly")) {
			fs = std::fstream(result["poly"].as<std::string>(), std::fstream::out);
		}

		while (vid.read(cur_frame)) {
			//vid >> cur_frame;

			Mat mask; // Mask to hold the values
			mask = drawMask(cur_frame, result["filter"].as<std::string>());

			Mat proc; //Frame to be processed; can be blurred

			if (result.count("b")) {
				int size = std::stoi(result["b"].as<std::string>());
				std::cout << "Blur size: " << size << std::endl;
				blur(cur_frame, proc, Size(size, size));
			} else {
				proc = cur_frame;
			}

			watershed(proc, mask);

			//Finds the largest contour
			std::vector<std::vector<Point>> vertexes;
			Mat binary;
			mask.convertTo(binary, CV_32FC1);
			threshold(binary, binary, 200, 255, THRESH_BINARY);
			binary.convertTo(binary, CV_8UC1);
			findContours(binary, vertexes, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

			if (vertexes.size() != 0) {
				size_t biggest = 0;
				size_t biggest_size = 0;
				for (size_t i = 0; i < vertexes.size(); ++i) {
					if (vertexes[i].size() > biggest_size) {
						biggest = i;
						biggest_size = vertexes[i].size();
					}
				}

				if (result.count("output")) { // Generates the overlay
					// Generates the overlay
					Mat segmented; // Segmented image with the overlay
					cur_frame.convertTo(segmented, CV_8UC3);
					drawContours(segmented, vertexes, biggest, Scalar(255, 255, 255), -1);
					addWeighted(segmented, 0.5, cur_frame, 0.5, 0, segmented, CV_8UC3);
					w << segmented;
				}

				if (result.count("poly")) { //Saves largest contour
					Polygon pol;
					for (Point p : vertexes[biggest]) {
						pol.points.emplace_back(p.x, p.y);
					}
					pol.save(fs, Polygon::FileType::FILE_WKT);
					fs << "\n";
				}
			}

			std::cout << vid.get(CAP_PROP_POS_FRAMES) * 100 / max_frames
				<< "\% "
				<< std::endl;
		}
	}
	return 0;
}
