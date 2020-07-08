#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdint>

#include <iostream>
#include <fstream>
#include <vector>

#include <cxxopts.hpp>
#include "polygon.hpp"

using namespace cv;

int main(int argc, char* argv[]) {
	cxxopts::Options options("Cell extractor", "Extracts information from cell images from the cell tracking challenge. Call with -h or --help to see full help.");
	options.add_options()
		("h,help", "Shows full help")
		("i", "Mandatory. Grayscale pre segmented image to extract polygons.", cxxopts::value<std::string>())
		("o", "Mandatory. Output folder. WKT's will be output to this folder.", cxxopts::value<std::string>());

	if (argc==1) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	auto result = options.parse(argc, argv);

	if (result["help"].as<bool>()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	if (!result.count("i") && !result.count("o")) {
		std::cout << "Error. Need to specify input image and output folder.\n";
		return 1;
	}

	Mat img = imread(result["i"].as<std::string>(), IMREAD_ANYDEPTH);

	std::vector<uint16_t> done;

	for (int i = 0; i < img.rows; ++i) { 
		for (int j = 0; j < img.cols; ++j) {
			uint16_t pixel = img.at<uint16_t>(i, j);
			if (pixel != 0) {
				bool skip = false;
				for (uint16_t d: done)
					if (d == pixel) {
						skip = true;
						break;
					}

				if (skip) break;

				Mat filtered = (img == pixel);

				std::vector<std::vector<Point>> vertexes;

				findContours(filtered, vertexes, RETR_EXTERNAL, CHAIN_APPROX_NONE);

				std::fstream fs(result["o"].as<std::string>() + std::to_string(pixel) + ".wkt",
					std::fstream::out | std::fstream::trunc);
				fs << "POLYGON((";

				bool first = true;
				Point p1;
				for (Point p: vertexes[0]) {
					if (!first) {
						fs << ",";
					}
					else
					{
						p1 = p;
						first = false;
					}
					fs << p.x << " " << p.y;					
				}
				if (!first) {
					fs << ",";
				}
				fs << p1.x << " " << p1.y << "))\n";
				fs.close();

				done.push_back(pixel);
			}
		}
	}
}
