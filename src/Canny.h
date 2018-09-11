//
//  canny.h
//  Canny Edge Detector
//

#pragma once

#include "CImg.h"
#include <vector>

using namespace std;
using namespace cimg_library;

class Canny {
private:
	CImg<float> img; //Original Image
	CImg<float> grayscaled; //Grayscale
	CImg<float> outSmooth; //Smoothed
	CImg<float> outGradient; //Gradient
	CImg<float> outOrientation; //Orientation(angle map)
	CImg<float> outNMS; //Non-maxima supp.
	CImg<int> dirmax; //Max direction
	CImg<float> outThreshold; //Double threshold and final
	CImg<float> derivative[4];
	int ny, nx; // height, width
	float sigma, thresholdLow, thresholdHigh;
	int filterWidth;

	void init(CImg<float> &, int, double, double, double);
	CImg<float> toGrayScale(CImg<float>);
	CImg<float> createFilter(int, double); //Creates a gaussian filter
	CImg<float> useFilter(CImg<float>, CImg<float>); //Use some filter
	void compGradientAndDirection(); //Compute grdient and angles, save in the member variables
	void nonMaxSuppAndThresh(double, double); //Non-maxima supp and apply thresholding, save in the member variables
public:

	Canny();
	Canny(CImg<float> & img, int filterWidth = 3, double sigma = 1.5f, double threshold1 = 12.0f, double threshold2 = 24.0f); //Constructor
	void plot(); //Plot
	CImg<float> getEdge(); // Return edge-detected binary image
};
