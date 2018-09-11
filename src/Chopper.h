#pragma once

#include "image_utils.h"
#include <vector>

using namespace std;


struct Range {
	int low, high;
	Range() : low(0), high(0) {}
	Range(int low, int high) : low(low), high(high) {}
};



class Chopper {
public:
    Chopper(Img & img) : img(img) {}
	vector<ImgWithBB> getImgWithBBs();
    void test();
private:
	vector<ImgWithBB> project_chop(Img &);
	vector<ImgWithBB> conn_comp_chop(Img &);
    Img & img;
	const float min_conn_thresh = 0.0002;
};
