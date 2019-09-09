#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

using namespace cv;

double coordx(double x) {
	double coord_x_min = 615202.199;
	double coord_x_max = 616268.574;

	double map_x_min = 0.0;
	double map_x_max = 5680.0;

	return (x - map_x_min) / (map_x_max - map_x_min) * (coord_x_max - coord_x_min) + coord_x_min;
}

double coordy(double y) {
	double coord_y_min = 4583991.175;
	double coord_y_max = 4582453.191;

	double map_y_min = 0.0;
	double map_y_max = 8192.0;

	return (y - map_y_min) / (map_y_max - map_y_min) * (coord_y_max - coord_y_min) + coord_y_min;
}

//map y max = 8192

int main(int, char* argv[]) {
	std::vector<Point2f> src, dst;

	src.push_back(Point2f(1172, 100));
	src.push_back(Point2f( 202, 266));
	src.push_back(Point2f(  56, 664));
	src.push_back(Point2f(1145, 619));

	dst.push_back(Point2f(3443, 6987));
	dst.push_back(Point2f(3575, 7757));
	dst.push_back(Point2f(3871, 7746));
	dst.push_back(Point2f(3929, 7514));
	
	Mat m = findHomography(src, dst);

	std::vector<Point2f> pol;
	std::fstream fs(argv[1], std::fstream::in);
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


