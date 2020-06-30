#ifndef AUXILIARIES_H
#define AUXILIARIES_H

#include <opencv2/opencv.hpp>

typedef double Real;
#define PI 4.* atan(1.)

class Array {

public:
	Array(); // Constructor
	~Array(); // Destructor
	Real getX(int i);
	Real getY(int i);
	Real getTimeStamp(int i);
	void Array::fill(const int w, const int h);
	Real Array::kernel(Real arg);

private:
	std::vector <std::pair<Real, cv::Point2f>> p; // timeStamp, (x, y)

};//Array

#endif

void makeHeatMap(Real **I, int w, int h, Array &g, Real currTimeStamp);
Real map(Real x, Real in_min, Real in_max, Real out_min, Real out_max);
Real findHighest(Real **I, int w, int h);
void updatePixels(cv::Mat &img, Real **I);
uchar lerpColor(Real startValue, Real endValue, Real fraction);