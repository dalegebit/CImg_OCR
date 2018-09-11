#include "Binarizer.h"
#include "Canny.h"
#include <iostream>

/*
	Apply BotHat tranoformation and ostu algorithm to binarize
*/
Img Binarizer::adaptiveThresh(Img & m) {
	Img gray = m.get_channel(1);
    gray = (gray.get_blur(4, true, true) - gray);
	gray = gray - 0.68 * gray.min();
	//gray.laplacian();

    gray.cut(0, 255);
	gray.display(0, false);
	int hist[256] = { 0 };
	cimg_forXY(gray, x, y) {
		hist[int(gray(x, y))] += 1;
	}
	int size = m._width * m._height;
	int cnt0 = 0, cnt1 = size;
    int sum0 = 0, sum1 = 0;
    int thresh = 0;
    double u0, u1, w0, w1, var, maximum = 0.0;
    for ( int i = 0; i < 256; ++i) {
        sum1 += i * hist[i];
    }
    for ( int i = 0; i < 255; ++i) {
		//cout << hist[i] << endl;
        cnt0 += hist[i];
        sum0 += i * hist[i];
        cnt1 -= hist[i];
        sum1 -= i * hist[i];
		//cout << cnt0 << " " << cnt1 << " " << sum0 << " " << sum1 << endl;

        if (cnt0 == 0 || cnt1 == 0)
            continue;

        u0 = double(sum0) / cnt0;
        w0 = double(cnt0) / size;
        u1 = double(sum1) / cnt1;
        w1 = 1 - w0;
        var = w0 * w1 * (u0-u1) * (u0-u1);
		//cout << var << endl;
        if (maximum < var) {
            maximum = var;
            thresh = i;
        }
    }
	// cout << thresh << endl;
	Img binary = gray.get_threshold(thresh);

    return binary;
}

Img Binarizer:: getGray(Img & img) {
    return img.channel(0) * 0.2989 + img.channel(1) * 0.5870 + img.channel(2) * 0.1140;
}

Img Binarizer::getBinary() {
    return adaptiveThresh(img);
	//Canny canny(img, 2, 0.3, 6.0);
	//return canny.getEdge();
}
