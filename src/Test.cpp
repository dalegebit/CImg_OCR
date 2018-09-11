#include "CropA4.h"
#include "Binarizer.h"
#include "Chopper.h"
#include "Classifier.h"
#include "Extractor.h"
#include "image_utils.h"
#include <iostream>
#include <string>

using namespace std;



int main() {
	Classifier cl("train/model/svm.model");
	Extractor ext(cl);
	for (int i = 1; i < 9; ++i) {
		string ss(int2string(i));
		Img img(("pics/" + ss + ".jpg").c_str());
		img.display(0, false);
		CropA4 c4(img, 3, 1.5, 2.0, 30.0, 0.0, 2*PI, PI/512, 0.0, SQRT2, 0.002, 0.005);
		Img A4 = c4.getA4(400);
		A4.display(0, false);
		Binarizer b(A4);
		Img bin = b.getBinary();
		bin.display(0, false);
		Chopper chopper(bin);
		vector<ImgWithBB> bb = chopper.getImgWithBBs();
		vector<ImgWithBBLabel> bbl = ext.getImgWithBBLabels(bb);

		Img test(A4, false);

		for (auto & b : bbl) {
			const unsigned char red[] = { 255, 0, 0 };
			test.draw_line(b.bb.top_x, b.bb.top_y, b.bb.top_x, b.bb.top_y + b.bb.height, red);
			test.draw_line(b.bb.top_x, b.bb.top_y + b.bb.height, b.bb.top_x + b.bb.width, b.bb.top_y + b.bb.height, red);
			test.draw_line(b.bb.top_x + b.bb.width, b.bb.top_y + b.bb.height, b.bb.top_x + b.bb.width, b.bb.top_y, red);
			test.draw_line(b.bb.top_x, b.bb.top_y, b.bb.top_x + b.bb.width, b.bb.top_y, red);
			test.draw_text(b.bb.top_x + b.bb.width/2, b.bb.top_y + b.bb.height, int2string(b.label).c_str(), red, 0, 1, 15);
			//b.img.display(0, false);
		}

		test.display(0, false);
		test.save(("result/" + ss + ".jpg").c_str());
		cout << endl;
	} 
}
