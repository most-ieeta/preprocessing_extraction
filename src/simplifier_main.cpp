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

void drawPolygon(Mat src, const Polygon& pol, const Scalar& color, double displace_x = 0, double displace_y = 0) {
	std::vector<std::vector<Point>> polys;
	polys.emplace_back();
	for (SimplePoint p: pol.points) {
		polys[0].emplace_back(p.x + displace_x * FACTOR, p.y + displace_y * FACTOR);
	}
	drawContours(src, polys, 0, color);
}

void drawWindow() {
	Mat src = Mat::ones(globals.max_x * 3 * FACTOR, globals.max_y * 3 * FACTOR, CV_8U)*255;
	cvtColor(src, src, COLOR_GRAY2BGR);
	namedWindow("Polygons", WINDOW_NORMAL);

	drawPolygon(src, globals.p1, Scalar(0, 0, 0));
	drawPolygon(src, globals.p2, Scalar(0, 0, 0), globals.max_x * 2, 0);

	Polygon vv_p1 = globals.p1, vv_p2 = globals.p2;
	Simplifier::visvalingam_until_n(vv_p1, globals.red_per);
	Simplifier::visvalingam_until_n(vv_p2, globals.red_per);
	drawPolygon(src, vv_p1, Scalar(0, 0, 0), 0, globals.max_y);
	drawPolygon(src, vv_p2, Scalar(0, 0, 0), globals.max_x * 2, globals.max_y);

	Polygon dp_p1 = globals.p1, dp_p2 = globals.p2;
	Simplifier::douglas_peucker_until_n(dp_p1, globals.red_per);
	Simplifier::douglas_peucker_until_n(dp_p2, globals.red_per);
	drawPolygon(src, vv_p1, Scalar(0, 0, 0), globals.max_x, globals.max_y);
	drawPolygon(src, vv_p2, Scalar(0, 0, 0), globals.max_x * 3, globals.max_y);

	std::vector<Polygon> vvt_pols;
	vvt_pols.push_back(globals.p1);
	vvt_pols.push_back(globals.p2);
	Simplifier::visvalingam_with_time(vvt_pols, globals.red_per, globals.t_value);
	drawPolygon(src, vvt_pols[0], Scalar(0, 0, 0), globals.max_x, 0);
	drawPolygon(src, vvt_pols[1], Scalar(0, 0, 0), globals.max_x * 3, 0);

	imshow("Polygons", src);
}

void change_red_per(int new_value, void*) {
	globals.red_per = new_value/100.0;
	drawWindow();
}

void change_t_value(int new_value, void*) {
	globals.t_value = new_value/10.0;
	drawWindow();
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cout << "Wrong usage! Correct usages:\n"
			"  ./simplifier <points file - pof or wkt> <points file - pof or wkt> \n";
		exit(1);
	}

	std::fstream fs(argv[1], std::fstream::in);
	std::fstream fs2(argv[2], std::fstream::in);

	if (!fs.is_open()) {
		std::cout << "Error, could not load one of the files\n";
		exit(2);
	}
	
	Polygon p1(fs);
	Polygon p2(fs2);
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


	drawWindow();
	
	createTrackbar("\% points to remove:", "Polygons", nullptr, 100, change_red_per);
	createTrackbar("time weigth:", "Polygons", nullptr, 100, change_t_value);

	
	char c;
	while ((c = waitKey()) != 'q') {
		continue;
	}
	/*	createTrackbar("Tolerance (px squared*10)", W_NAME, &gui_area, 2000,
				recalcTolerance);
		recalcTolerance(0, nullptr);

		char c;
		while ((c = waitKey()) != 'q') {
			if (c == 's') {
				std::vector<Point> simp;
				simp = simplify(points, gui_area / RATIO);
				std::fstream fs2(fname + "_simplified.pof",
						std::fstream::out | std::fstream::trunc);

				for (Point p : simp) {
					fs2 << p.x << " " << p.y << "\n";
				}
			}
		}
	} else if (argc == 4) {
		std::fstream fs2(fname + "_simplification_chart.csv",
				std::fstream::out | std::fstream::trunc);

		unsigned int step = std::stoi(argv[2]);
		unsigned int iterations = std::stoi(argv[3]);

		fs2 << "0 " << points.size() << "\n";
		for (float f = step; f < (step * iterations); f += step) {
			std::vector<Point> simp;
			simp = simplify(points, f);
			fs2 << f << " " << simp.size() << "\n";
		}
	} else if (argc == 5) {
		float n = std::stof(argv[4]);
		std::vector<Point> simp;
		simp = simplify_n(points, static_cast<int>(n*points.size()));
		std::fstream fs2(argv[2],
				std::fstream::out | std::fstream::trunc);

		for (Point p : simp) {
			fs2 << p.x << " " << p.y << "\n";
		}
	}*/
}
