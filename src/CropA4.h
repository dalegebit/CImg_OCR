#pragma once


#include <vector>
#include <math.h>
#include "image_utils.h"


using namespace std;


// Structure Edge, saving the indices of two adjacent vertices and length.
struct Edge {
	PointWithNeighbor * p1, * p2;
    double len;
	Edge(const Edge & e) : p1(e.p1), p2(e.p2), len(e.len) {}
	Edge(PointWithNeighbor & p1, PointWithNeighbor & p2, double len) : p1(&p1), p2(&p2), len(len) {}
};

typedef vector<Edge> EdgeList;	// Edge List
typedef double TranMat[3][3]; // Transaltion Matrix


class CropA4 {
private:
    Img img;	// source image
    Img A4;		// cropped A4
    PointList corner_points;	// 4 detected corner points
    PointWithNeighborList point_list; // 4 detected corner points with their two neighbors (for computation)
	vector<PointWithNeighbor*> pivot_points;
    EdgeList edge_list;	// 4 edges
    TranMat tran_mat;	// translation matrix
	bool is_vert;

	// <Canny parameters>
	int canny_filt_width;
	double canny_sigma;
	double canny_thresh_low;
	double canny_thresh_high;
	// </Canny parameters>

	// <Hough parameters>
	double hough_begin_theta;
	double hough_end_theta;
	double hough_theta_step;
	double hough_begin_rho;
	double hough_end_rho;
	double hough_rho_step;
	double hough_thresh_ratio;
	// </Hough parameters>

	// initialize object parameters
    void init(
        Img &,
        int canny_filt_width,
        double canny_sigma,
        double canny_thresh_low,
        double canny_thresh_high,
        double hough_begin_theta,
        double hough_end_theta,
        double hough_theta_step,
        double hough_begin_rho,
        double hough_end_rho,
        double hough_rho_step,
        double hough_thresh_ratio
    );
    void calcCornerPoints(); // calculate corner point through hough transformation
    void calcEdgeList();	// calculate 4 edges
	void calcPivots();		// calculate 4 pivots
    void calcTranMat();		// calculate the profective translation matrix

	// transform the (x0, y0) to (x,y) with translation matrix
    void transform(double x0, double y0, double & x, double & y);

	// create cropped A4 image
    void createA4(int width);

	// decide which corner point is the left-top one in A4
    bool leftOrRight(PointWithNeighbor &, PointWithNeighbor &);

	// Remove the frame
	Img centerCrop(const Img &, int);

public:

    CropA4();
	CropA4(
		Img &,
        int canny_filt_width = 3,
        double canny_sigma = 1.5,
        double canny_thresh_low = 6.0,
        double canny_thresh_high = 30.0,
        double hough_begin_theta = 0,
        double hough_end_theta = 2*PI,
        double hough_theta_step = PI/512,
        double hough_begin_rho = 0,
        double hough_end_rho = SQRT2,
        double hough_rho_step = 0.002,
        double hough_thresh_ratio = 0.005
    );
    Img getA4(int width=400);	// get cropped A4 image
	void test(string);	// for testing
	void test();	// for testing
	void saveA4(string);	//	save cropped A4 to file
};
