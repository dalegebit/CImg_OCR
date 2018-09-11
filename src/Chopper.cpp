
#include "Chopper.h"
#include <iostream>
#include <map>
using namespace std;

vector<int> cumulate(Img & m, int axis=0) {
	int len = axis ? m._height : m._width;
	vector<int> cum(len, 0);
	cimg_forXY(m, x, y)
		if (m(x, y) > 0)
			cum[axis ? y : x] += 1;
	return cum;
}

vector<vector<Point> > label2point(Img && label, Img & img, int min_size=20, int max_comp_size=50) {
	map<int, vector<Point> > point;
	cimg_forXY(label, x, y) {
		if (img(x, y) > 0) {
			auto iter = point.find(label(x, y));
			if (iter == point.end())
				point[label(x, y)] = vector<Point>(1, Point(x, y));
			else
				iter->second.emplace_back(x, y);
		}
	}
	vector<vector<Point> > point_list;
	vector<Point> trash;
	for (auto & p : point) {
		//cout << p.first << " " << p.second.size() << " " << int(p.second.size() >= min_size) << endl;
		if (p.second.size() >= min_size) {
			point_list.emplace_back(p.second);
		}
		/*else if (p.second.size() < max_comp_size) {
			trash.insert(trash.end(), p.second.begin(), p.second.end());
		}*/
	}
	//label.display();
	return point_list;
}

vector<BoundingBox> point2bb(const vector<vector<Point> > & point, int frame_width=3) {
	vector<BoundingBox> bb;
	for (auto & p : point) {
		int max_x=-1, max_y=-1, min_x=-1, min_y=-1;
		for (auto & iter : p) {
			if (iter.x > max_x)
				max_x = iter.x;
			else if (min_x == -1 || iter.x < min_x)
				min_x = iter.x;
			if (iter.y > max_y)
				max_y = iter.y;
			else if (min_y == -1 || iter.y < min_y)
				min_y = iter.y;
		}
		bb.emplace_back(min_x, min_y, max_x - min_x, max_y - min_y);
	}
	return bb;
}

vector<ImgWithBB> point2imgbb(const vector<vector<Point> > & point, int frame_width = 0, int frame_height = 0) {
	vector<ImgWithBB> bb(point.size());
	int cnt = 0;
	for (auto & p : point) {
		int max_x = -1, max_y = -1, min_x = -1, min_y = -1;
		for (auto & iter : p) {
			if (iter.x > max_x)
				max_x = iter.x;
			else if (min_x == -1 || iter.x < min_x)
				min_x = iter.x;
			if (iter.y > max_y)
				max_y = iter.y;
			else if (min_y == -1 || iter.y < min_y)
				min_y = iter.y;
		}
		bb[cnt].img = Img(max_x - min_x + 2 * frame_width + 1, max_y - min_y + 2 * frame_height + 1, 1, 1, 0);
		for (auto & iter : p) {
			bb[cnt].img(iter.x - min_x + frame_width, iter.y - min_y + frame_height) = 1;
		}
		bb[cnt].bb = BoundingBox(min_x, min_y, max_x - min_x, max_y - min_y);
		cnt++;
	}
	return bb;
}

int max_width(vector<ImgWithBB> & bb) {
	int maxw = -1;
	for (auto & b : bb)
		if (b.bb.width > maxw)
			maxw = b.bb.width;
	return maxw;
}

int max_height(vector<ImgWithBB> & bb) {
	int maxh = -1;
	for (auto & b : bb)
		if (b.bb.height > maxh)
			maxh = b.bb.height;
	return maxh;
}

vector<Range> extractIntervals(vector<int> & cum, int min_val = 1, int min_range = 3, int max_interval = 2) {
	int start_i = -1, end_i = -1;
	vector<Range> ranges;
	for (int i = 0; i < cum.size(); ++i) {
		if (cum[i] >= min_val && start_i == -1) {
			if (end_i >= 0 && i - end_i < max_interval) {
				Range back = ranges.back();
				end_i = back.high = i;
				continue;
			}
			start_i = i;
		}
		else if (cum[i] < min_val && start_i >= 0) {
			if (i - start_i >= min_range) {
                end_i = i;
				ranges.emplace_back(start_i, end_i);
                start_i = -1;
			}
            else {
                start_i = -1;
            }
		}
	}
	if (ranges.size() == 0)
		ranges.emplace_back(0, cum.size());
	return ranges;
}

vector<ImgWithBB> Chopper::project_chop(Img & img) {
	vector<ImgWithBB> bb;
	vector<Range> vert_ranges;
	vert_ranges = extractIntervals(cumulate(img, 1));
	for (auto & vert_range : vert_ranges) {
		Img cropped = img.get_crop(0, vert_range.low, img._width - 1, vert_range.high-1);
		//cropped.display();
		vector<Range> horiz_ranges = extractIntervals(cumulate(cropped), 1, 3, 1);
		for (auto & horiz_range : horiz_ranges) {
			bb.emplace_back(cropped.get_crop(horiz_range.low, horiz_range.high-1),
				horiz_range.low, vert_range.low,
				horiz_range.high - horiz_range.low, vert_range.high - vert_range.low);
		}
	}
	return bb;
}

vector<ImgWithBB> Chopper::getImgWithBBs() {
	vector<ImgWithBB> bb;
	vector<ImgWithBB> bb_new;
	bb = project_chop(img.get_dilate(3));
	for (auto & b : bb) {
		vector<ImgWithBB> bb1 = conn_comp_chop(b.img);
		for (auto & b1 : bb1)
			bb_new.emplace_back(b1.img, b.bb.top_x + b1.bb.top_x, b.bb.top_y + b1.bb.top_y, b1.bb.width, b1.bb.height);
		//for (auto & b1 : btmp) {
		//	b1.img.display();
		//	if (float(b1.bb.height) / img._height > max_height_ratio ||
		//		float(b1.bb.width) / b1.bb.height > width_height_ratio) {
		//		vector<ImgWithBB> btmp1 = connCompChop(b1.img);
		//		int max_w = max_width(btmp1), max_h = max_height(btmp1);
		//		if (float(max_h) / b1.bb.height < min_split_ratio ||
		//			float(max_w) / b1.bb.width < min_split_ratio) {
		//			for (auto & b2 : btmp1) {
		//				//b1.img.display();
		//				bb_new.emplace_back(b1.img,
		//					b.bb.top_x + b1.bb.top_x + b2.bb.top_x,
		//					b.bb.top_y + b1.bb.top_y + b2.bb.top_y,
		//					b2.bb.width, b2.bb.height);
		//			}
		//			continue;
		//		}
		//		/*else {
		//			int num_split = ceil(double(b1.bb.width) / b1.bb.height / width_height_ratio);
		//			int split_width = b1.bb.width / num_split;
		//			int split_begin = 0;
		//			for (int i = 0; i < num_split; ++i) {
		//			bb_new.emplace_back(b1.img.get_crop(split_begin, split_begin + split_width),
		//			b.bb.top_x + b1.bb.top_x + split_begin, b.bb.top_y + b1.bb.top_y, split_width, b1.bb.height);
		//			split_begin += split_width;
		//			}
		//			}*/
		//	}
		//	//b1.img.display();
		//	bb_new.emplace_back(b1.img, b.bb.top_x + b1.bb.top_x, b.bb.top_y + b1.bb.top_y, b1.bb.width, b1.bb.height);
		//}
	}
	return bb_new;
}

vector<ImgWithBB> Chopper::conn_comp_chop(Img & i) {
	return point2imgbb(label2point(i.get_label(), i, min_conn_thresh * img.size()));
}


void Chopper::test() {
    //vector<Range> ranges(extractIntervals(cumulate(img, 1)));
    //for (auto & range : ranges)
    //    cout << range.low << " " << range.high << endl;
	Img test(img, false);
	vector<ImgWithBB> bb = getImgWithBBs();
	for (auto & b : bb) {
		//preprocess(b.img).display();
		//b.img.display();
		const unsigned char red[] = { 1, 0, 0 };
		test.draw_line(b.bb.top_x, b.bb.top_y, b.bb.top_x, b.bb.top_y + b.bb.height, red);
		test.draw_line(b.bb.top_x, b.bb.top_y + b.bb.height, b.bb.top_x + b.bb.width, b.bb.top_y + b.bb.height, red);
		test.draw_line(b.bb.top_x + b.bb.width, b.bb.top_y + b.bb.height, b.bb.top_x + b.bb.width, b.bb.top_y, red);
		test.draw_line(b.bb.top_x, b.bb.top_y, b.bb.top_x + b.bb.width, b.bb.top_y, red);

	}
	test.display();
}
