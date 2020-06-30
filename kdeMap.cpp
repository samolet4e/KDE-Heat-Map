#include "auxiliaries.h"
#define SAVE_TO_FILE

int main() {
bool playVideo = true;
Array gaze;

	cv::VideoCapture source("world.mp4");
	if (!source.isOpened()) {
		std::cout << "Error opening video stream or file" << std::endl;
		return -1;
	}//if
	const int imageWidth = (int)source.get(cv::CAP_PROP_FRAME_WIDTH);
	const int imageHeight = (int)source.get(cv::CAP_PROP_FRAME_HEIGHT);

	// Allocates the heatmap
	Real** kde = new Real*[imageWidth];
	for (int i = 0; i < imageWidth; i++) kde[i] = new Real[imageHeight];

	std::cout << "readGazePoints" << std::endl;
	gaze.fill(imageWidth, imageHeight);

#ifdef SAVE_TO_FILE
	cv::Size S = cv::Size(imageWidth, imageHeight);
	int ex = static_cast<int>(source.get(cv::CAP_PROP_FOURCC));
	cv::VideoWriter destination;
	destination.open("output.avi", ex, source.get(cv::CAP_PROP_FPS), S, true);
#endif

	std::cout << "mainLoop" << std::endl;
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE); // create a window for display
	cv::Mat overlay(imageHeight, imageWidth, CV_8UC3);

	int frameCounter = 0;
	while (1) {

		Real ms = source.get(cv::CAP_PROP_POS_MSEC);
		// Adjust the modulo right operand for you to get desirable speed!
		if (frameCounter % 10 == 0) makeHeatMap(kde, imageWidth, imageHeight, gaze, ms / 1000.);

		if (playVideo) source >> overlay;
		if (overlay.empty()) break;

		updatePixels(overlay, kde);

		std::cout << ms / 1000. << "\t" << std::endl;
		imshow("Display window", overlay);

#ifdef SAVE_TO_FILE
		destination << overlay;
#endif

		frameCounter++;

		// flow control
		char key = cv::waitKey(20);
		if (key == 'p') playVideo = !playVideo;
		else if (key == 27) break;
		else {}

	}//while

	 // Deallocates the heatmap
	for (int i = 0; i < imageWidth; i++) delete[] kde[i];
	delete[] kde;

	source.release();
	overlay.release();

	gaze.~Array();

	return 0;
}//main