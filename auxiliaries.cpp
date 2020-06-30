#include "auxiliaries.h"

Array::Array() {

	p.reserve(0);

}//Constructor

Array::~Array() {

	p.clear();

}//Destructor

Real Array::getX(int i) {

	return p.at(i).second.x;

}//getX

Real Array::getY(int i) {

	return p.at(i).second.y;

}//getY

Real Array::getTimeStamp(int i) {

	return p.at(i).first;

}//getTimeStamp

// https://stackoverflow.com/questions/34724655/two-types-of-value-in-one-vector-c
void Array::fill(const int w, const int h) {

	std::ifstream myFile("gaze.txt");
	std::string line;

	Real timeStamp0;
	bool firstLine = true;
	while (getline(myFile, line)) {
		std::stringstream ss(line);
		Real timeStamp;
		cv::Point2f X;
		ss >> timeStamp >> X.x >> X.y;
		if (firstLine == true) { timeStamp0 = timeStamp; firstLine = false; }
		timeStamp -= timeStamp0;
		X.x *= w;
		X.y = 1.f - X.y; // reverse
		X.y *= h;
		(*this).p.push_back(std::make_pair(timeStamp, X));
	}//while

//	for (auto i = 0; i < p.size(); i++) std::cout << p.at(i).first << "\t" << p.at(i).second << std::endl;

	myFile.close();

	return;
}//fill

#define GAUSSIAN
Real Array::kernel(Real arg) {
Real K;

#if defined(EPANECHNIKOV)
	if (abs(arg) >= sqrt(5.)) K = 0.;
	else K = 3. / (4. * sqrt(5.)) * pow((1. - 1. / 5. * arg), 2.);
#elif defined(GAUSSIAN)
	K = 1. / sqrt(2. * PI) * exp(-0.5 * arg * arg);
#endif

	return K;
}//kernel

Real findHighest(Real **I, int w, int h) {
	Real res = 0.;

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
			if (res < I[i][j]) res = I[i][j];

	return res;
}//findHighest

Real map(Real x, Real in_min, Real in_max, Real out_min, Real out_max) {

	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

}//map

#define STEPSIZE 0.25f
#define STDDEV 20.f
void makeHeatMap(Real **I, int w, int h, Array &g, Real currTimeStamp) {
int i, j, gazePoint;
static int start = 0, end = 0;

	// Clears up the heatmap: obligatory due to recursion
	for (i = 0; i < w; i++)
		for (j = 0; j < h; j++) I[i][j] = 0.;

	if (currTimeStamp < STEPSIZE) return;

	// Finds the nearest blob (consider revising)
	gazePoint = start;
	while (g.getTimeStamp(gazePoint) < currTimeStamp - STEPSIZE) gazePoint++;
	start = gazePoint;
	while (g.getTimeStamp(gazePoint) < currTimeStamp) gazePoint++;
	end = gazePoint;

	for (i = 0; i < w; i++)
		for (j = 0; j < h; j++) {

			Real hx = 17., hy = 17.; // Adjust as appropriate!
			cv::Point2f p;

			for (gazePoint = start; gazePoint < end; gazePoint++) {

				p.x = (float)((Real)i - g.getX(gazePoint));
				p.y = (float)((Real)j - g.getY(gazePoint));
				Real Xi_X2 = p.x * p.x + p.y * p.y;
				if (sqrt(Xi_X2) <= 3. * STDDEV) I[i][j] += g.kernel(p.x / hx) * g.kernel(p.y / hy);
				else {}
//				I[i][j] += g.kernel(sqrt(pow(p.x / hx, 2.) + pow(p.y / hy, 2.)));
//				I[i][j] *= 1. / (0.1 * hx * hy);
//				I[i][j] = (Real)((int)(I[i][j] * 1.e04 + .5) / 1.e04); // Round off up to four decimal places

			}//for_gazePoint

		}//for_j

	// Remaps the heatmap within 0...1 range
	Real maxI = findHighest(I, w, h);
	for (i = 0; i < w; i++)
		for (j = 0; j < h; j++)
			if (I[i][j] != 0.) I[i][j] = map(I[i][j], 0., maxI, 0., 1.);

}//makeHeatMap

uchar lerpColor(Real startValue, Real endValue, Real fraction) {

	return (uchar)((endValue - startValue) * fraction + startValue);

}//lerpColor

void updatePixels(cv::Mat &img, Real **I) {
Real alpha = 155.f;
Real fraction = alpha / 255.f;

	for (int y = 0; y < img.rows; y++)
		for (int x = 0; x < img.cols; x++) {

			if (I[x][y] != 0.) {

				// get pixel
				cv::Vec3b color = img.at<cv::Vec3b>(cv::Point(x, y)), color0 = color;

				if (I[x][y] < 0.25f) { Real I1 = map(I[x][y], 0.f, 0.25f, 0.f, 1.f);  color = cv::Vec3b(255, lerpColor(0., 255., I1), 0); } // blue
				else if (I[x][y] >= 0.25f && I[x][y] < 0.5f) { Real I1 = map(I[x][y], 0.25f, 0.5f, 0.f, 1.f); color = cv::Vec3b(lerpColor(0., 255., 1.f - I1), 255, 0); } // cyan
				else if (I[x][y] >= 0.5f  && I[x][y] < 0.75f) { Real I1 = map(I[x][y], 0.5f, 0.75f, 0.f, 1.f); color = cv::Vec3b(0, 255, lerpColor(0., 255., I1)); } // yellow
				else if (I[x][y] >= 0.75f && I[x][y] <= 1.f) { Real I1 = map(I[x][y], 0.75f, 1.f, 0.f, 1.f);  color = cv::Vec3b(0, lerpColor(0., 255., 1.f - I1), 255); } // red
				else {}

				color[0] = (uchar)(color0[0] * fraction + color[0] * (1.f - fraction));
				color[1] = (uchar)(color0[1] * fraction + color[1] * (1.f - fraction));
				color[2] = (uchar)(color0[2] * fraction + color[2] * (1.f - fraction));

				// set pixel
				img.at<cv::Vec3b>(cv::Point(x, y)) = color;

			}//if
			else {}

		}//for_x

	return;
}//updatePixels