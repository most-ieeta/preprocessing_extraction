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

#define FACTOR 1.2 //Spacing factor

using namespace cv;

struct {
	Polygon p1;
	Polygon p2;
	double t_value = 1;
	double red_per = 0;
	double max_x = 0;
	double max_y = 0;
} globals;

void drawPolygon(Mat src, const Polygon& pol, const Scalar& color, double displace_x, double displace_y, bool drawMarkers) {
	std::vector<std::vector<Point>> polys;
	polys.emplace_back();
	for (SimplePoint p: pol.points) {
		polys[0].emplace_back(p.x + displace_x * globals.max_x * FACTOR, p.y + displace_y * globals.max_y * FACTOR);
	}
	drawContours(src, polys, 0, color);

	if (drawMarkers) {
		for (Point p: polys[0]) {
			drawMarker(src, p, color, MARKER_CROSS, 20);
		}
	}
}

Mat genImage(bool drawWindow) {
	Mat src = Mat::ones(globals.max_y * 4 * FACTOR, globals.max_x * 4 * FACTOR, CV_8U)*255;
	cvtColor(src, src, COLOR_GRAY2BGR);
	namedWindow("Polygons", WINDOW_NORMAL);

	//Originals
	drawPolygon(src, globals.p1, Scalar(0, 0, 0), 0, 0, false);
	drawPolygon(src, globals.p1, Scalar(0, 0, 0), 0, 2, true);

	drawPolygon(src, globals.p2, Scalar(0, 0, 0), 2, 0, false);
	drawPolygon(src, globals.p2, Scalar(0, 0, 0), 2, 2, true);

	std::fstream fs("p1_orig.wkt", std::fstream::out);
	globals.p1.save(fs, Polygon::FileType::FILE_WKT);
	fs = std::fstream("p2_orig.wkt", std::fstream::out);
	globals.p2.save(fs, Polygon::FileType::FILE_WKT);

	//Visvalingam
	Polygon vv_p1 = globals.p1, vv_p2 = globals.p2;
	Simplifier::visvalingam_until_n(vv_p1, globals.red_per);
	Simplifier::visvalingam_until_n(vv_p2, globals.red_per);

	fs = std::fstream("p1_vv.wkt", std::fstream::out);
	vv_p1.save(fs, Polygon::FileType::FILE_WKT);
	fs.close();
	fs = std::fstream("p2_vv.wkt", std::fstream::out);
	vv_p2.save(fs, Polygon::FileType::FILE_WKT);

	drawPolygon(src, vv_p1, Scalar(0, 0, 0), 0, 1, false);
	drawPolygon(src, vv_p1, Scalar(0, 0, 0), 0, 3, true);

	drawPolygon(src, vv_p2, Scalar(0, 0, 0), 2, 1, false);
	drawPolygon(src, vv_p2, Scalar(0, 0, 0), 2, 3, true);

	//Douglas-Peucker
	Polygon dp_p1 = globals.p1, dp_p2 = globals.p2;
	Simplifier::douglas_peucker_until_n(dp_p1, globals.red_per);
	Simplifier::douglas_peucker_until_n(dp_p2, globals.red_per);

	fs = std::fstream("p1_dp.wkt", std::fstream::out);
	dp_p1.save(fs, Polygon::FileType::FILE_WKT);
	fs = std::fstream("p2_dp.wkt", std::fstream::out);
	dp_p2.save(fs, Polygon::FileType::FILE_WKT);

	drawPolygon(src, dp_p1, Scalar(0, 0, 0), 1, 1, false);
	drawPolygon(src, dp_p1, Scalar(0, 0, 0), 1, 3, true);

	drawPolygon(src, dp_p2, Scalar(0, 0, 0), 3, 1, false);
	drawPolygon(src, dp_p2, Scalar(0, 0, 0), 3, 3, true);

	//Temporal Visvalingam
	std::vector<Polygon> mas_pols;
	mas_pols.push_back(globals.p1);
	mas_pols.push_back(globals.p2);
	//Simplifier::visvalingam_with_time(mas_pols, globals.red_per, globals.t_value);
	Simplifier::douglas_with_time(mas_pols, globals.red_per, globals.t_value);
	fs = std::fstream("p1_mas.wkt", std::fstream::out);
	mas_pols[0].save(fs, Polygon::FileType::FILE_WKT);
	fs = std::fstream("p2_mas.wkt", std::fstream::out);
	mas_pols[1].save(fs, Polygon::FileType::FILE_WKT);

	drawPolygon(src, mas_pols[0], Scalar(0, 0, 0), 1, 0, false);
	drawPolygon(src, mas_pols[0], Scalar(0, 0, 0), 1, 2, true);
	
	drawPolygon(src, mas_pols[1], Scalar(0, 0, 0), 3, 0, false);
	drawPolygon(src, mas_pols[1], Scalar(0, 0, 0), 3, 2, true);

	if (drawWindow)
		imshow("Polygons", src);
	return src;
}

void change_red_per(int new_value, void*) {
	globals.red_per = new_value/100.0;
	genImage(true);
}

void change_t_value(int new_value, void*) {
	globals.t_value = new_value/10.0;
	genImage(true);
}

int main(int argc, char *argv[]) {
	cxxopts::Options options("Simplifier", "Simplifies two given polygons, with options to visualize or generate images. Call with -h or --help to see full help.");
	options.add_options()
		("h,help", "Shows full help")
		("p", "Mandatory. First polygon to be simplified.", cxxopts::value<std::string>())
		("q", "Mandatory. Second polygon to be simplified", cxxopts::value<std::string>())
		("o,output", "File to save image from simplified polygons", cxxopts::value<std::string>())
		("r", "Percentage of points to be removed, between 0 and 1", cxxopts::value<double>())
		("t", "Time value for visvalingam-with-time method", cxxopts::value<double>());
	
	if (argc==1) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	auto result = options.parse(argc, argv);

	if (result["help"].as<bool>()) {
		std::cout << options.help() << std::endl;
		return 0;
	}

	if (!result.count("p") && !result.count("q")) {
		std::cout << "Error. Need to specify polygons.\n";
		return 1;
	}

	std::fstream fs(result["p"].as<std::string>(), std::fstream::in);
	std::fstream fs2(result["q"].as<std::string>(), std::fstream::in);

	if (!fs.is_open()) {
		std::cout << "Error, could not load one of the files\n";
		exit(2);
	}
	
	Polygon p1;
	if (fs.peek() == 'P') 
		p1 = Polygon(fs, Polygon::FileType::FILE_WKT);
	else
		p1 = Polygon(fs);

	Polygon p2;
	if (fs2.peek() == 'P')
		p2 = Polygon(fs2, Polygon::FileType::FILE_WKT);
	else
		p2 = Polygon(fs2);

	globals.p1 = p1;
	globals.p2 = p2;

	for (SimplePoint p: p1.points) {
		if (p.x > globals.max_x) globals.max_x = p.x;
		if (p.y > globals.max_y) globals.max_y = p.y;
	}
	for (SimplePoint p: p2.points) {
		if (p.x > globals.max_x) globals.max_x = p.x;
		if (p.y > globals.max_y) globals.max_y = p.y;
	}
	if (!result.count("output")) { //Show window
		genImage(true);
		
		createTrackbar("\% points to remove:", "Polygons", nullptr, 100, change_red_per);
		createTrackbar("time weigth:", "Polygons", nullptr, 100, change_t_value);

		char c;
		while ((c = waitKey()) != 'q') {
			continue;
		}
	} else {
		globals.red_per = result["r"].as<double>();
		globals.t_value = result["t"].as<double>();
		
		Mat img = genImage(false);
		imwrite(result["o"].as<std::string>(), img);
	}
}
