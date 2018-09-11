#include "HoughTransformer.h"
#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

HoughTransformer::HoughTransformer() {}


HoughTransformer::HoughTransformer(Img & img,
                                   double begin_theta,
                                   double end_theta,
                                   double theta_step,
                                   double begin_rho,
                                   double end_rho,
                                   double rho_step,
                                   double thresh_ratio) {
    init(img, begin_theta, end_theta, theta_step, begin_rho, end_rho, rho_step, thresh_ratio);
    setCosSinMap();
}


void HoughTransformer::init(Img & img,
                            double begin_theta,
                            double end_theta,
                            double theta_step,
                            double begin_rho,
                            double end_rho,
                            double rho_step,
                            double thresh_ratio) {
    this->img = Img(img);
    this->line_img = Img(img, "x,y,1,1", 0);
    this->begin_theta = begin_theta;
    this->end_theta = end_theta;
    this->theta_step = theta_step;
    this->begin_rho = begin_rho;
    this->end_rho = end_rho;
    this->rho_step = rho_step;
    this->num_theta = (end_theta-begin_theta)/theta_step;
    this->num_rho = (end_rho-begin_rho)/rho_step;
    this->thresh_ratio = thresh_ratio;
    setHoughSpace(this->num_theta, this->num_rho);
    cos_map = vector<double>(this->num_theta, 0);
    sin_map = vector<double>(this->num_theta, 0);
}

void HoughTransformer::setHoughSpace(int theta_size, int rho_size) {
    this->hough_space = vector<vector<int> >(theta_size, vector<int>(rho_size, 0));
}

void HoughTransformer::setCosSinMap() {
    for (int i = 0; i < this->num_theta; ++i) {
        cos_map[i] = cos(begin_theta+i*theta_step);
        sin_map[i] = sin(begin_theta+i*theta_step);
    }
}

void HoughTransformer::transform() {
    this->hough_map = Img(num_theta, num_rho, 1, 1, 0);
    this->num_non_zero = 0;
    cimg_forXY(img, x, y) {
        if (img(x,y) > 0) {
            this->num_non_zero += 1;
            double rho;
            double x_ = double(x)/img._width;
            double y_ = double(y)/img._height;
            for (int i = 0; i < this->num_theta; ++i) {
                rho = x_*cos_map[i]+y_*sin_map[i];
                if (rho < begin_rho || rho >= end_rho) continue;
                int j = int((rho-begin_rho)/rho_step);
                hough_space[i][j] += 1;
                hough_map(i, j) = min(hough_map(i, j)+1, 255.0f);
            }
        }
    }
}


void HoughTransformer::suppressNonMax(int kernel_x_len = 40, int kernel_y_len = 40) {
    Space new_space = hough_space;
    Img new_hough_map(hough_map);

    int kernel_x_stride = kernel_x_len / 2;
    int kernel_y_stride = kernel_y_len / 2;
    for (int i = 0; i < this->num_theta; i+=kernel_x_stride)
    for (int j = 0; j < this->num_rho; j+=kernel_y_stride) {
        int max_score = new_space[i][j];
        int max_x = i, max_y = j;
        for (int x_m_i = 0; x_m_i < kernel_x_len; ++x_m_i)
        for (int y = j; y < min(num_rho, j+kernel_y_len); ++y) {
            int x = (i+x_m_i) % num_theta;
            if (new_space[x][y] > max_score) {
                max_score = new_space[x][y];
                max_x = x, max_y = y;
            }
            new_space[x][y] = 0;
            new_hough_map(x, y) = 0;
        }
        new_space[max_x][max_y] = max_score;
        new_hough_map(max_x, max_y) = min(255, max_score);
    }

    hough_space = new_space;
    hough_map = new_hough_map;
}

bool cmp_line(Line a, Line b) {return a.score > b.score;}

void HoughTransformer::createLine(int topk) {
    double threshold = this->thresh_ratio * this->num_non_zero;
    int kernel_x_len = 20;
    int kernel_y_len = 20;

    LineList new_list;

    for (int i = 0; i < this->num_theta; ++i)
    for (int j = 0; j < this->num_rho; ++j)
    if (hough_space[i][j] > threshold) {
        int max_score = hough_space[i][j];
        for (int x_m_i = -kernel_x_len; x_m_i < kernel_x_len; ++x_m_i)
        for (int y = max(0, j-kernel_y_len); y < min(num_rho, j+kernel_y_len); ++y) {
            int x = (i+x_m_i+num_theta) % num_theta;
            if (hough_space[x][y] > max_score) {
                max_score = hough_space[x][y];
                break;
            }
        }
        if (max_score != hough_space[i][j]) continue;
        new_list.push_back(Line(i, j, hough_space[i][j]));
    }

    sort(new_list.begin(), new_list.end(), cmp_line);

    line_list = LineList(new_list.begin(), (topk<int(new_list.size()))?(new_list.begin()+topk):(new_list.end()));
}

void HoughTransformer::calcCrossPoints() {
    point_list = PointList();
    for (int k = 0; k < line_list.size(); ++k) {
        int theta_i = line_list[k].theta_i;
        double rho = begin_rho + line_list[k].rho_j * rho_step;
        for (int k1 = k+1; k1 < line_list.size(); ++k1) {
             int theta_i1 = line_list[k1].theta_i;
             double rho1 = begin_rho + line_list[k1].rho_j * rho_step;
             double x = (rho*sin_map[theta_i1] - rho1*sin_map[theta_i]) / (cos_map[theta_i]*sin_map[theta_i1] - cos_map[theta_i1]*sin_map[theta_i]);
             double y = (rho1*cos_map[theta_i] - rho*cos_map[theta_i1]) / (cos_map[theta_i]*sin_map[theta_i1] - cos_map[theta_i1]*sin_map[theta_i]);
             if (x > 2 || x < -1 || y > 2 || y < -1)
                continue;
             point_list.push_back(Point(x,y));
        }
    }
}

void HoughTransformer::plot(Img & color_img) {

	hough_map.display();
    // Plot lines
    line_img = Img(img, "x,y,1,1", 0);
    Img lines = Img(color_img);
    for (int k = 0; k < line_list.size(); ++k) {
        int i = line_list[k].theta_i;
        int j = line_list[k].rho_j;
        double rho = begin_rho + j * rho_step;
        double theta = begin_theta + i * theta_step;
        double x0 = rho * cos_map[i] * img._width;
        double y0 = rho * sin_map[i] * img._height;
        int x1 = int(x0 - 1000*sin_map[i]*img._width);
        int x2 = int(x0 + 1000*sin_map[i]*img._width);
        int y1 = int(y0 + 1000*cos_map[i]*img._height);
        int y2 = int(y0 - 1000*cos_map[i]*img._height);
        const unsigned char color_white[] = {255,255,255};
        const unsigned char color_green[] = {0,255,255};
        line_img.draw_line(x1, y1, x2, y2, color_white);
        x0 = rho * cos_map[i] * color_img._width;
        y0 = rho * sin_map[i] * color_img._height;
        x1 = int(x0 - 100*sin_map[i]*color_img._width);
        x2 = int(x0 + 100*sin_map[i]*color_img._width);
        y1 = int(y0 + 100*cos_map[i]*color_img._height);
        y2 = int(y0 - 100*cos_map[i]*color_img._height);
        lines.draw_line(x1, y1, x2, y2, color_green);
    }
    line_img.display();
    lines.display();

    // Plot intersection points
    Img points(color_img);
    const unsigned char color[] = {255,120,120};
    for (int i = 0; i < point_list.size(); ++i)
        points.draw_circle(int(point_list[i].x*color_img._width), int(point_list[i].y*color_img._height), 20, color);
    points.display();
	
}

Img HoughTransformer::getLine() {
    transform();
    suppressNonMax();
    createLine(4);
    return line_img;
}

PointList HoughTransformer::getPointList() {
    transform();
    suppressNonMax();
    createLine(4);
    calcCrossPoints();
	return point_list;
}


void HoughTransformer::test(Img & img) {
    transform();
    hough_map.display();
    suppressNonMax();
    hough_map.display();
    createLine(4);
    calcCrossPoints();
    plot(this->img);
}
