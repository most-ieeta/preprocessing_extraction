#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include "cxxopts.hpp"

using namespace cv;

//Map parameters are fixed. TODO read from config file
double coordx(double x) {
	double coord_x_min = 615202.199;
	double coord_x_max = 616268.574;

	double map_x_min = 0.0;
	double map_x_max = 5680.0;

	return (x - map_x_min) / (map_x_max - map_x_min) * (coord_x_max - coord_x_min) + coord_x_min;
}

//Map parameters are fixed. TODO read from config file
double coordy(double y) {
	double coord_y_min = 4583991.175;
	double coord_y_max = 4582453.191;

	double map_y_min = 0.0;
	double map_y_max = 8192.0;

	return (y - map_y_min) / (map_y_max - map_y_min) * (coord_y_max - coord_y_min) + coord_y_min;
}

int main(int argc, char* argv[]) {
	cxxopts::Options options("warp", "Performs warp perspective on a wkt polygon. Prints results to stdout, redirect with >>.");
	options.add_options()
		("h,help", "Shows full help")
		("s,source", "File with points on source (oblique) image. Each line should have 2 double-precision numbers separated by a space (x y\\n).", cxxopts::value<std::string>())
		("t,target", "File with points on target (map) image. Each line should have 2 double-precision numbers, separated by a space (x y \\n).", cxxopts::value<std::string>())
		("p,poly", "Polygon file, as .pof", cxxopts::value<std::string>());

	if (argc==1) {
		std::cout << options.help() << std::endl;
		return 0;
	}
	auto result = options.parse(argc, argv);

	if (result["help"].as<bool>()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	std::vector<Point2d> src, dst;

	//Reads source file
	{
		std::fstream src_file(result["s"].as<std::string>(), std::fstream::in);
		std::string line;

		while (std::getline(src_file, line)) {
			double x, y;
			std::stringstream ss(line);
			ss >> x >> y;
			src.push_back(Point2d(x, y));
		}
	}
	
	//Reads destination file
	{
		std::fstream dst_file(result["t"].as<std::string>(), std::fstream::in);
		std::string line;

		while (std::getline(dst_file, line)) {
			double x, y;
			std::stringstream ss(line);
			ss >> x >> y;
			dst.push_back(Point2d(x, y));
		}
	}

	Mat m = findHomography(src, dst);

	std::vector<Point2f> pol;
	std::fstream fs(result["p"].as<std::string>(), std::fstream::in);
	float x, y;
	while ((fs >> x >> y)) {
		pol.push_back(Point2f(x, y));
	}

	std::vector<Point2f> pol2;

	perspectiveTransform(pol, pol2, m);

	std::cout << "POLYGON ((";
	bool first = true;
	for (Point2f p:pol2) {
		if (!first)
			std::cout << ",";
		std::cout << std::fixed << coordx(p.x) << " " << coordy(p.y);
		first = false;
	}
	std::cout << "))\n";

	return 0;
}


