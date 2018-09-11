#pragma once

#include "image_utils.h"
#include <vector>

using namespace std;

class Binarizer {
public:
    Binarizer(Img & img) : img(img) {}
    Img getBinary();
private:
    Img & img;
    int threshold;

    Img adaptiveThresh(Img &);
    Img getGray(Img &);
};
