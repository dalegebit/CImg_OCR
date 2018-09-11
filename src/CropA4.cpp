#include "CropA4.h"
#include "Canny.h"
#include "HoughTransformer.h"
#include <math.h>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;


/*    <Util Function>   */

double dist(Point & p1, Point & p2, int w, int h) {
    return sqrt(pow((p1.x-p2.x)*w, 2)+pow((p1.y-p2.y)*h, 2));
}

double dist(PointWithNeighbor & p1, PointWithNeighbor & p2, int w, int h) {
	return sqrt(pow((p1.x - p2.x)*w, 2) + pow((p1.y - p2.y)*h, 2));
}

bool edge_len_cmp(Edge e1, Edge e2) {
    return e1.len < e2.len;
}


double det(double a11, double a12, double a21, double a22) {
    return a11*a22-a12*a21;
}

/* </Util Function> */


CropA4::CropA4() {}

CropA4::CropA4(
        Img & img,
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
        double hough_thresh_ratio ) : is_vert(true) {

    init(img, canny_filt_width, canny_sigma, canny_thresh_low, canny_thresh_high, \
        hough_begin_theta, hough_end_theta, hough_theta_step, hough_begin_rho, \
        hough_end_rho, hough_rho_step, hough_thresh_ratio);

}

void CropA4::init(
    Img & img,
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
    double hough_thresh_ratio ) {

    this->img = img;
    this->canny_filt_width = canny_filt_width;
    this->canny_sigma = canny_sigma;
    this->canny_thresh_low = canny_thresh_low;
    this->canny_thresh_high = canny_thresh_high;
    this->hough_begin_theta = hough_begin_theta;
    this->hough_end_theta = hough_end_theta;
    this->hough_theta_step = hough_theta_step;
    this->hough_begin_rho = hough_begin_rho;
    this->hough_end_rho = hough_end_rho;
    this->hough_rho_step = hough_rho_step;
    this->hough_thresh_ratio = hough_thresh_ratio;
}



void CropA4::calcCornerPoints() {

    // gaussian kernel size = 3, sigma = 2.5, thresh low = 10, thresh high = 30
    Canny canny(img.get_resize(312, int(312.0/img._width*img._height)), canny_filt_width, canny_sigma,
                    canny_thresh_low, canny_thresh_high);
	// canny.plot();
    // begin_theta = 0, end_theta = 2*pi, step_theta = pi/512
    // begin_rho = 0, end_rho = sqrt(2), step_rho = 0.002
    // thresh_ratio = 0.01
    HoughTransformer hough(canny.getEdge(), hough_begin_theta, hough_end_theta,
                            hough_theta_step, hough_begin_rho, hough_end_rho,
                            hough_rho_step, hough_thresh_ratio);

    corner_points = hough.getPointList();
	// hough.test(img);
	// hough.plot(img);
    for (int i = 0; i < corner_points.size(); ++i)
        point_list.emplace_back(corner_points[i]);
}

void CropA4::calcEdgeList() {
	PointWithNeighborList & pl = point_list;
    double top1_dist=INT_MAX, top2_dist=INT_MAX;
    int top1_idx=0, top2_idx=0, top3_idx=0;
    for (int i = 1; i < 4; ++i) {
        double d = dist(pl[0], pl[i], img._width, img._height);
        if (top1_dist > d) {
            top3_idx = top2_idx;
            top2_dist = top1_dist;
            top2_idx = top1_idx;
            top1_dist = d;
            top1_idx = i;
        } else if (top2_dist > d) {
            top3_idx = top2_idx;
            top2_dist = d;
            top2_idx = i;
        } else {
            top3_idx = i;
        }
    }
    // cout << "top1 idx: " << top1_idx << "dist: " << dist(pl[0], pl[top1_idx], img._width, img._height) << endl;
    // cout << "top2 idx: " << top2_idx << "dist: " << dist(pl[0], pl[top2_idx], img._width, img._height) << endl;
    // cout << "top3 idx: " << top3_idx << "dist: " << dist(pl[0], pl[top3_idx], img._width, img._height) << endl;
    edge_list.emplace_back(pl[0], pl[top1_idx], dist(pl[0], pl[top1_idx], img._width, img._height));
    pl[0].neighbor.push_back(&pl[top1_idx]), pl[top1_idx].neighbor.push_back(&pl[0]);
    edge_list.emplace_back(pl[0], pl[top2_idx], dist(pl[0], pl[top2_idx], img._width, img._height));
    pl[0].neighbor.push_back(&pl[top2_idx]), pl[top2_idx].neighbor.push_back(&pl[0]);
    edge_list.emplace_back(pl[top1_idx], pl[top3_idx], dist(pl[top1_idx], pl[top3_idx], img._width, img._height));
    pl[top1_idx].neighbor.push_back(&pl[top3_idx]), pl[top3_idx].neighbor.push_back(&pl[top1_idx]);
    edge_list.emplace_back(pl[top2_idx], pl[top3_idx], dist(pl[top2_idx], pl[top3_idx], img._width, img._height));
    pl[top2_idx].neighbor.push_back(&pl[top3_idx]), pl[top3_idx].neighbor.push_back(&pl[top2_idx]);
    sort(edge_list.begin(), edge_list.end(), edge_len_cmp);
    if ((edge_list[0].p1->y + edge_list[0].p2->y) >
        (edge_list[1].p1->y + edge_list[1].p2->y)) {
        swap(edge_list[0], edge_list[1]);
    }
}


bool CropA4::leftOrRight(PointWithNeighbor & p1, PointWithNeighbor & p2) {
    if (p1.x > p2.x && p1.y < p2.y)
        return false;
    if (p2.x > p1.x && p2.y < p1.y)
        return true;
    if (p1.x > p2.x && p1.y > p2.y)
        return false;
    if (p2.x > p1.x && p2.y > p1.y)
        return true;
    return true;
}


void CropA4::calcPivots() {
	pivot_points.clear();
	/*	p : the top point
	*	p1 : the nearsest neighbor of p
	*	p2 : the farthest neighbor of p */
	PointWithNeighbor * p, * p1, * p2;
	if (edge_list[0].p1->y < edge_list[0].p2->y)
		p = edge_list[0].p1, p1 = edge_list[0].p2;
	else
		p = edge_list[0].p2, p1 = edge_list[0].p1;
	if (*(p->neighbor[0]) == *p1)
		p2 = p->neighbor[1];
	else
		p2 = p->neighbor[0];
	cout << p->x << " " << p->y << endl;
	cout << p1->x << " " << p1->y << endl;
	cout << p2->x << " " << p2->y << endl;
	if (abs((p->x - p1->x) / (p->y - p1->y)) > abs((p->x - p2->x) / (p->y - p2->y))) {
		is_vert = true;
		if (p->x > p1->x) {
			pivot_points.push_back(p1);
			pivot_points.push_back(p);
			pivot_points.push_back(p2);
			if (*p == *p2->neighbor[0])
				pivot_points.push_back(p2->neighbor[1]);
			else
				pivot_points.push_back(p2->neighbor[0]);
		}
		else {
			pivot_points.push_back(p), pivot_points.push_back(p1);
			if (*p == *p1->neighbor[0])
				pivot_points.push_back(p1->neighbor[1]);
			else
				pivot_points.push_back(p1->neighbor[0]);
			pivot_points.push_back(p2);
		}
	}
	else {
		is_vert = false;
		if (p->x > p1->x) {
			pivot_points.push_back(p);
			pivot_points.push_back(p2);
			if (*p == *p2->neighbor[0])
				pivot_points.push_back(p2->neighbor[1]);
			else
				pivot_points.push_back(p2->neighbor[0]);
			pivot_points.push_back(p1);
		}
		else {
			pivot_points.push_back(p2);
			pivot_points.push_back(p);
			pivot_points.push_back(p1);
			if (*p == *p1->neighbor[0])
				pivot_points.push_back(p1->neighbor[1]);
			else
				pivot_points.push_back(p1->neighbor[0]);
		}
	}
}

void CropA4::calcTranMat() {
	vector<PointWithNeighbor*> & pl = pivot_points;
    for (int i = 0; i < pl.size(); ++i)
        cout << "x: " << pl[i]->x << " " << " y: " << pl[i]->y << endl;

    // const unsigned char color[][3] = {{255,0,0}, {0,255,0}, {0,0,255}, {0,255,255}};

    // for (int i = 0; i < point_list.size(); ++i)
    //    img.draw_circle(int(pl[i]->x*img._width), int(pl[i]->y*img._height), 20, color[i]);

    double dx3 = pl[0]->x-pl[1]->x + pl[2]->x-pl[3]->x;
    double dy3 = pl[0]->y-pl[1]->y + pl[2]->y-pl[3]->y;
    TranMat & a = tran_mat;
    if (dx3 == 0.0 && dy3 == 0.0) {
		a[0][0] = pl[1]->x-pl[0]->x;
        a[1][0] = pl[2]->x-pl[1]->x;
		a[2][0] = pl[0]->x;
        a[0][1] = pl[1]->y-pl[0]->y;
        a[1][1] = pl[2]->y-pl[1]->y;
		a[2][1] = pl[0]->y;
        a[0][2] = 0;
        a[1][2] = 0;
        a[2][2] = 1;
    } else {
        double dx1 = pl[1]->x - pl[2]->x;
        double dx2 = pl[3]->x - pl[2]->x;
        double dy1 = pl[1]->y - pl[2]->y;
        double dy2 = pl[3]->y - pl[2]->y;
        a[0][2] = det(dx3, dx2, dy3, dy2) / det(dx1, dx2, dy1, dy2);
        a[1][2] = det(dx1, dx3, dy1, dy3) / det(dx1, dx2, dy1, dy2);

        a[0][0] = pl[1]->x - pl[0]->x + a[0][2] * pl[1]->x;
        a[1][0] = pl[3]->x - pl[0]->x + a[1][2] * pl[3]->x;
		a[2][0] = pl[0]->x;
        a[0][1] = pl[1]->y - pl[0]->y + a[0][2] * pl[1]->y;
        a[1][1] = pl[3]->y - pl[0]->y + a[1][2] * pl[3]->y;
		a[2][1] = pl[0]->y;
        a[2][2] = 1;
    }
}

void CropA4::transform(double x0, double y0, double & x, double & y) {
    TranMat & a = tran_mat;
    x = (a[0][0]*x0 + a[1][0]*y0 + a[2][0])/(a[0][2]*x0 + a[1][2]*y0 + a[2][2]);
    y = (a[0][1]*x0 + a[1][1]*y0 + a[2][1])/(a[0][2]*x0 + a[1][2]*y0 + a[2][2]);
}

void CropA4::createA4(int width) {
	if (is_vert)
		A4 = Img(width, int(width*SQRT2), 1, 3);
	else
		A4 = Img(int(width*SQRT2), width, 1, 3);
	double x, y;
    cimg_forXY(A4, i, j) {
        transform(double(i)/A4._width, double(j)/A4._height, x, y);
		// cout << x*img._width << " " << y*img._height << endl;
        // cout << img.linear_atXYZC(x*img._width, y*img._height, 1, 0) << endl;
        A4(i,j,0) = img.linear_atXY(x*img._width, y*img._height, 0, 0, 255);
        A4(i,j,1) = img.linear_atXY(x*img._width, y*img._height, 0, 1, 255);
        A4(i,j,2) = img.linear_atXY(x*img._width, y*img._height, 0, 2, 255);
    }
    A4 = centerCrop(A4, 10);
}

Img CropA4::centerCrop(const Img & img, int frame_width) {
    return img.get_crop(frame_width, frame_width, 0, 0,
                        img._width-frame_width, img._height-frame_width, 0, 2);
}

void CropA4::saveA4(string path) {
	A4.save_bmp(path.c_str());
}

Img CropA4::getA4(int width) {
	calcCornerPoints();
	calcEdgeList();
	calcPivots();
	calcTranMat();
	createA4(width);
	return A4;
}

void CropA4::test(string path) {
	getA4();
	Img img_draw(img, false);
    const unsigned char color_green[] = {0,255,255};
    for (int i = 0; i < 4; ++i)
        img_draw.draw_line(edge_list[i].p1->x * img._width, edge_list[i].p1->y * img._height,
                        edge_list[i].p2->x * img._width, edge_list[i].p2->y * img._height, color_green);
	const unsigned char color[][3] = {{255,0,0}, {0,255,0}, {0,0,255}, {0,255,255}};

	for (int i = 0; i < point_list.size(); ++i)
		img_draw.draw_circle(int(pivot_points[i]->x*img._width), int(pivot_points[i]->y*img._height), 20, color[i]);

    img_draw.display();
	// img_draw.save_bmp(path.c_str());
    double x, y;
    transform(0, 0, x, y);
    cout << x << " " << y << endl;
    transform(1, 0, x, y);
    cout << x << " " << y << endl;
    transform(1, 1, x, y);
    cout << x << " " << y << endl;
    transform(0, 1, x, y);
    cout << x << " " << y << endl;
    A4.display();
}
