#pragma once

#define cimg_use_jpeg
#include <CImg.h>
#include <vector>
#include <cmath>
#include <sstream>


using namespace cimg_library;
using namespace std;

#define SQRT2 1.4142135623731
#define PI 3.14159265358979323846

typedef CImg<float> Img;
struct PointWithNeighbor;


struct Point {
    double x, y;
	Point() : x(0), y(0) {}
	Point(const Point & p) : x(p.x), y(p.y) {}
    Point(double x_in, double y_in) : x(x_in), y(y_in) {}
	bool operator==(const Point & p) {
		return x == p.x && y == p.y;
	}
};

typedef vector<Point> PointList;

struct PointWithNeighbor {
    double x, y;
	vector<PointWithNeighbor*> neighbor;
	PointWithNeighbor() :x(0), y(0) {}
	PointWithNeighbor(const Point & p) :x(p.x), y(p.y) {}
	PointWithNeighbor(const PointWithNeighbor & p) :x(p.x), y(p.y) {}
	bool operator==(const PointWithNeighbor & p) {
		return x == p.x && y == p.y;
	}
};

typedef vector<PointWithNeighbor> PointWithNeighborList;

struct BoundingBox {
	int top_x, top_y, width, height;
	BoundingBox() : top_x(0), top_y(0), width(0), height(0) {}
	BoundingBox(const BoundingBox & bb) :
		top_x(bb.top_x), top_y(bb.top_y), width(bb.width), height(bb.height) {}
	BoundingBox(int x, int y, int w, int h) :
		top_x(x), top_y(y), width(w), height(h) {}
};

struct ImgWithBB {
	Img img;
	BoundingBox bb;
	ImgWithBB() {}
	ImgWithBB(Img & img, BoundingBox & bb) : img(img), bb(bb) {}
	ImgWithBB(Img & img, int x, int y, int w, int h) : img(img), bb(x, y, w, h) {}
};

struct ImgWithBBLabel {
	Img img;
	BoundingBox bb;
    int label;
	ImgWithBBLabel() {}
	ImgWithBBLabel(ImgWithBB & imgbb, int label = 0) : img(imgbb.img), bb(imgbb.bb), label(label) {}
	ImgWithBBLabel(Img & img, BoundingBox & bb, int label=0) : img(img), bb(bb), label(label) {}
	ImgWithBBLabel(Img & img, int x, int y, int w, int h, int label) : img(img), bb(x, y, w, h), label(label) {}
};

inline string int2string(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}

template <typename Vec>
void display_graph(const Vec & vec) {
	Img graph(vec.size(), 1, 1, 1, 0);
	for (int i = 0; i < vec.size(); ++i)
		graph._data[i] = float(vec[i]);
	graph.display_graph(0, 3);
}

template <typename Prob>
double entropy(const Prob & prob) {
	double ent = 0.0;
	for (auto & p : prob) {
		if (p != 0.0)
			ent -= p * log2(p);
	}
	return ent;
}
