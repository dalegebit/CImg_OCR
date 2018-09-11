//
//  Canny.cpp
//  Canny Edge Detector
//
//

#include <iostream>
#include <vector>
#include "Canny.h"
#include "gauss_filter.h"
#include <CImg.h>
using namespace std;
using namespace cimg_library;
 
Canny::Canny()
{

}

Canny::Canny(CImg<float> & img, int filterWidthIn, double sigmaIn, double thresholdLowIn, double thresholdHighIn)
{
	init(img, filterWidthIn, sigmaIn, thresholdLowIn, thresholdHighIn);
}

void Canny::init(CImg<float> & img, int filterWidthIn, double sigmaIn, double thresholdLowIn, double thresholdHighIn)
{

	this->img = CImg<float>(img);

	nx = img._width;
	ny = img._height;
	sigma = sigmaIn;
	thresholdLow = thresholdLowIn;
	// cout << thresholdLow << endl;
	thresholdHigh = thresholdHighIn;
	filterWidth = filterWidthIn;


	const int widthIn = img._width;
	const int heightIn = img._height;
	if (widthIn == 0 || heightIn == 0) {
		cerr << "Error when loading input image." << endl;
		return;
	}
	else
	{
		CImg<float> filter = createFilter(filterWidth, sigma);

		//Print filter
		// for (int i = 0; i < filter._width; i++)
		// {
		// 	for (int j = 0; j < filter._height; j++)
		// 	{
		// 		cout << filter(i, j) << " ";
		// 	}
		// }
		grayscaled = toGrayScale(img); //Grayscale the image

		/************ initialize memory ************/
		outGradient = grayscaled; outGradient.fill(0.0f);
		dirmax = CImg<int>(outGradient);
		for (int i = 0; i < 4; i++) { derivative[i] = outGradient; }
		outOrientation = outGradient;
		outThreshold = outGradient;
		outNMS = outGradient;
		outSmooth = useFilter(grayscaled, filter); //Gaussian Filter

		compGradientAndDirection(); //Gradient and Direction

		nonMaxSuppAndThresh(thresholdLow, thresholdHigh); //Non-Maxima Suppression
	}
}

CImg<float> Canny::toGrayScale(CImg<float> in)
{
	grayscaled = in.channel(0) * 0.2989 + in.channel(1) * 0.5870 + in.channel(2) * 0.1140; //To one channel
	return grayscaled;
}


CImg<float> Canny::createFilter(int width, double sigmaIn)
{
	CImg<float> filter;
	gauss_filter(filter, width, sigmaIn, 0);

	return filter;

}

CImg<float> Canny::useFilter(CImg<float> img_in, CImg<float> filterIn)
{
	CImg<float> filteredImg = img_in.get_convolve(filterIn, true).convolve(filterIn.get_transpose(), true);
	return filteredImg;
}

void Canny::compGradientAndDirection()
{

	float fct = 1.0 / (2.0*sqrt(2.0f));
	for (int y = 1; y < img._height - 1; y++) {
		for (int x = 1; x < img._width - 1; x++) {
			//***** compute directional derivatives (E,NE,N,SE) ****//
			float grad_E = (outSmooth(x + 1, y) - outSmooth(x - 1, y))*0.5; // E
			float grad_NE = (outSmooth(x + 1, y - 1) - outSmooth(x - 1, y + 1))*fct; // NE
			float grad_N = (outSmooth(x, y - 1) - outSmooth(x, y + 1))*0.5; // N
			float grad_SE = (outSmooth(x + 1, y + 1) - outSmooth(x - 1, y - 1))*fct; // SE

			//***** compute gradient magnitude *********//
			float grad_mag = grad_E*grad_E + grad_N*grad_N;
			outGradient(x, y) = grad_mag;

			//***** compute gradient orientation (continuous version)*******//
			float angle = 0.0f;
			if (grad_mag > 0.0f) { angle = atan2(grad_N, grad_E); }
			if (angle < 0.0) angle += cimg::PI;
			outOrientation(x, y) = angle*255.0 / cimg::PI + 0.5; // -> outOrientation

			//***** compute absolute derivations *******//
			derivative[0](x, y) = grad_E = fabs(grad_E);
			derivative[1](x, y) = grad_NE = fabs(grad_NE);
			derivative[2](x, y) = grad_N = fabs(grad_N);
			derivative[3](x, y) = grad_SE = fabs(grad_SE);

			//***** compute direction of max derivative //
			if ((grad_E>grad_NE) && (grad_E>grad_N) && (grad_E>grad_SE)) {
				dirmax(x, y) = 0; // E
			}
			else if ((grad_NE>grad_N) && (grad_NE>grad_SE)){
				dirmax(x, y) = 1; // NE
			}
			else if (grad_N>grad_SE) {
				dirmax(x, y) = 2; // N
			}
			else {
				dirmax(x, y) = 3; // SE
			}
			// one may compute the contiuous dominant direction computation...
			//outOrientation(x,y) = dirmax(x,y)*255.f/4;
		}
	} // for x,y
}


void Canny::nonMaxSuppAndThresh(double threshold1, double threshold2)
{
	// directing vectors (E, NE, N, SE)
	int dir_vector[4][2] = { { 1, 0 }, { 1, -1 }, { 0, -1 }, { 1, 1 } };
	// direction of max derivative of
	// current pixel and its two neighbouring pixel (in direction of dir)
	int dir, dir1, dir2;

	//***** thresholding and (canny) non-max-supression *//
	for (int y = 2; y < ny - 2; y++) {
		for (int x = 2; x < nx - 2; x++) {
			dir = dirmax(x, y);
			if (derivative[dir](x, y) < threshold1) {
				outThreshold(x, y) = 0.0f;
				outNMS(x, y) = 0.0f;
			}
			else if (derivative[dir](x, y) > threshold2) {
				outThreshold(x, y) = 255.0f;
				outNMS(x, y) = 255.0f;
			}
			else {
				outThreshold(x, y) = 255.0f;
				int dx = dir_vector[dir][0];
				int dy = dir_vector[dir][1];
				dir1 = dirmax(x + dx, y + dy);
				dir2 = dirmax(x - dx, y - dy);
				outNMS(x, y) = 255.f*
					((derivative[dir](x, y) > derivative[dir1](x + dx, y + dy)) &&
					(derivative[dir](x, y) >= derivative[dir2](x - dx, y - dy)));
			} // -> outThreshold, outNMS
		}
	} // for x, y...
}

CImg<float> Canny::getEdge()
{
	return this->outNMS;
}

void Canny::plot()
{
	//***** display output images ******//
	char  header[100];
	//cout << img._width << " " << img._height << endl;

	sprintf(header, "gaussian smoothed image: filter width = %d & sigma = %f", filterWidth, sigma);
	outSmooth.display(header);
	//cout << outSmooth._width << " " << outSmooth._height << endl;

	float maxgrad = 0.0f;
	cimg_forXY(outGradient, x, y) { maxgrad = max(maxgrad, outGradient(x, y)); }
	std::cout << "normalising [0.." << maxgrad << "] to [0..255]" << std::endl;
	sprintf(header, "gradient magnitude [0..%f]", maxgrad);
	outGradient.display(header);
	//cout << outGradient._width << " " << outGradient._height << endl;

	// outOrientation.display("orientation map");
	sprintf(header, "thresholded with %f %f", thresholdLow, thresholdHigh);
	outThreshold.display(header);
	//cout << outThreshold._width << " " << outThreshold._height << endl;

	outNMS.display("non-maximum suppression");
	//cout << outNMS._width << " " << outNMS._height << endl;

}
