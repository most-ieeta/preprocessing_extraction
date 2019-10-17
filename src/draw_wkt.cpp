#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "polygon.hpp"
#include "simplifier.hpp"

#include "cxxopts.hpp"

using namespace cv;

int main(int argc, char *argv[]) {
	cxxopts::Options options("Draw WKT on an image", "Draw a WKT polygon on an image. Call with -h or --help to see full help.");
	options.add_options()
		("h,help", "Shows full help")
		("i", "Mandatory. Image to plot the WKT.", cxxopts::value<std::string>())
		("p", "Mandatory. Text file WKT polygon to be plotted", cxxopts::value<std::string>())
		("o", "Mandatory. Output image.", cxxopts::value<std::string>())
		("m", "Draw Markers.");
	
	if (argc==1) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	auto result = options.parse(argc, argv);

	if (result["help"].as<bool>()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	if (!result.count("p") && !result.count("i") && !result.count("o")) {
		std::cout << "Error.\n";
		std::cout << options.help() << std::endl;
		return 1;
	}

	Mat image = imread(result["i"].as<std::string>());

	std::fstream fs(result["p"].as<std::string>());
	Polygon p(fs, Polygon::FileType::FILE_WKT);

	std::vector<std::vector<Point>> pol;
	pol.emplace_back();
	for (SimplePoint pt: p.points)
		pol[0].emplace_back(pt.x, pt.y);

	drawContours(image, pol, 0, Scalar(0, 165, 255), -1);
	if (result.count("m")) {
		for (Point p: pol[0]) {
			drawMarker(image, p, Scalar(255, 0, 0), MARKER_SQUARE, 20);
		}
	}

	imwrite(result["o"].as<std::string>(), image);
	return 0;
}

