#pragma once

#include <vector>
#include <string>
#include <math.h>
#include "image_utils.h"

using namespace std;

struct Line {
    int theta_i, rho_j;
    int score;
    Line(int i, int j, int s):theta_i(i), rho_j(j), score(s) {}
};


typedef vector<vector<int> > Space;
typedef vector<Line> LineList;




class HoughTransformer {
private:
    Img img;    // Source image
    Img line_img;   // Result image of hough transformation, basically detected lines.
    Img hough_map; // Hough map, representing hough space for plotting
    Space hough_space; // Hough space
    LineList line_list; // List of detected lines
    PointList point_list; // List of intersection points of detected lines
    vector<double> cos_map; // Precomputed cos(theta) for each theta level
    vector<double> sin_map; // Precomputed sin(theta) for each theta level
    int num_theta, num_rho; // Number of theta levels, number of rho levels
    double begin_theta, end_theta, theta_step;
    double begin_rho, end_rho, rho_step;
    int num_non_zero; // Total number of edge points

    // The ratio decides the score of points in hough space higher than what percentage of the
    // total number of edge points will be determined as points of significance.
    double thresh_ratio;


    void init(Img &, double, double, double, double, double, double, double); // Initiate
    void setHoughSpace(int, int); // Set up hough space according to begin,end,step
    void setCosSinMap(); // Set up cos(theta) and sin(theta) map for each theta level

    // Set all non-local-maximun points in the same kernel zero in hough space.
    // The width and height of the kernel is given in as parameters.
    void suppressNonMax(int, int);
    void transform(); // Perform Hough transformation
    // Choose top k points(lines in image perspective) that has score higher
    // than the threshold in hough space.
    void createLine(int=4);
    // Calculate the cross points of the detected lines.
    void calcCrossPoints();

public:
    HoughTransformer();
    HoughTransformer(Img &, double=0, double=2*PI, double=PI/8, double=0, double=1.4, double=0.05, double=0.08); // Constructor

    Img getLine(); // Return image with detected lines
    PointList getPointList(); // Return point list

    void plot(Img &); // Plot image with detected lines, original color image with intersection points pointed out.
    void test(Img &); // For testing
};
