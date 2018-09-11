#include "Extractor.h"
#include <map>

using namespace std;

int num_conn_comp(const Img & img) {
	Img label = img.get_label();
	//label.display(0, 1);
	map<int, int> conn_num;
	cimg_forXY(label, x, y) {
		auto iter = conn_num.find(label(x, y));
		if (iter == conn_num.end())
			conn_num[label(x, y)] = 1;
		else
			iter->second += 1;
	}
	int n_conn = 0;
	for (auto & p : conn_num) {
		if (p.second >= 2) {
			n_conn += 1;
		}
	}
	//label.display();
	return  n_conn;
}

int exclude_large_ind(vector<double> & prob, int max_i) {
	double maximum = -1;
	int res_ind = 0;
	for (int i = 0; i < prob.size(); ++i) {
		if (i != max_i && prob[i] > maximum) {
			maximum = prob[i];
			res_ind = i;
		}
	}
	return res_ind;
}

int Extractor::adjust_pred(int pred, Img & img) {
	if (pred == 1 || pred == 7 || pred == 8) {
		Img canvas(img._width + 2, img._height + 2, 1, 1, 0);
		cimg_forXY(img, x, y)
			canvas(x + 1, y + 1) = img(x, y);
		int num_cc = num_conn_comp(canvas);
		//cout << "cc: " << num_cc << endl;
		//canvas.display(0, false);
		if ((pred == 1 || pred == 7) && num_cc == 3)
			pred = 9;
		else if (num_cc < 3 && pred == 8) {
			vector<double> prob = cl.predictProba(img.get_erode(1), true);
			pred = exclude_large_ind(prob, 8);
		}
		//else if (num_cc == 2 || pred == 3)
		//	pred = 3;
		//else if (num_cc > 2 || pred == 3) {
		//	vector<double> prob = cl.predictProba(img.get_dilate(1), true);
		//	//canvas.display(0, false);
		//	pred = exclude_large_ind(prob, 3);
		//}
		else if (num_cc == 4 || pred == 8)
			pred = 8;
		else
			pred = cl.predict(img, true);
	}
	else if (pred == 2 || pred == 3) {
		int pred1 = cl.predict(img, true);
		if (pred1 == 7)
			pred = pred1;
	}
	return pred;
}

vector<ImgWithBBLabel> Extractor::getImgWithBBLabels(vector<ImgWithBB> & imgbb) {
	vector<ImgWithBBLabel> imgbbl;
	for (auto & b : imgbb) {
		//if (float(b.img._width) / b.img._height> width_height_ratio) {
		//	vector<vector<double> > probs;
		//	int window_width = b.img._height * 0.4 * width_height_ratio;
		//	int window_begin;
		//	b.img.display(0, false);
		//	for (window_begin = 0; window_begin < int(b.img._width) - window_width; window_begin += slide_step) {
		//		Img cropped = b.img.get_crop(window_begin, window_begin + window_width);
		//		cropped.display(0, false);
		//		probs.emplace_back(cl.predictProba(cropped));
		//		display_graph(probs.back());
		//	}
		//	/*vector<double> ents(estimate_ent(probs));
		//	if (ents.size() == 0)
		//		ents.emplace_back(entropy(cl.predictProba(b.img)));
		//	display_graph(ents); */
		//	vector<double> cum_prob(10, 0);
		//	for (auto & prob : probs)
		//		for (int i = 0; i < 10; ++i)
		//			cum_prob[i] += prob[i];
		//	display_graph(cum_prob);
		//}
		int pred = cl.predict(b.img);
		pred = adjust_pred(pred, b.img);
		imgbbl.emplace_back(b, pred);
	}
	return imgbbl;
}

void Extractor::test(const ImgWithBB & imgbb) {
	/*vector<vector<double> > probs;
	int window_width = imgbb.img._height * 0.6;
	int window_begin;
	imgbb.img.display(0, false);
	for (window_begin = 0; window_begin < int(imgbb.img._width) - window_width; window_begin += slide_step) {
		Img cropped = imgbb.img.get_crop(window_begin, window_begin + window_width);
		cropped.display(0, false);
		probs.emplace_back(cl.predictProba(cropped));
	}
	vector<double> ents(estimate_ent(probs));
	if (ents.size() == 0)
		ents.emplace_back(entropy(cl.predictProba(imgbb.img)));
	display_graph(ents);*/

	int pred = cl.predict(imgbb.img);
	if (pred == 1 || pred == 7) {
		vector<double> prob = cl.predictProba(imgbb.img, true);
		imgbb.img.display();
		display_graph(prob);
		//for (int crop_len = 0; crop_len < imgbb.img._height / 2; crop_len += slide_step) {
		//	img cropped = imgbb.img.get_crop(0, 0, imgbb.img._width - 1, imgbb.img._height - 1 - crop_len);
		//	vector<double> prob = cl.predictproba(cropped, true);
		//	cropped.display();
		//	display_graph(prob);
		//}
	}
}