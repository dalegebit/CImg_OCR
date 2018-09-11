#include "image_utils.h"
#include "Classifier.h"
#include <vector>

using namespace std;

class Extractor {
public:
    Extractor(Classifier & cl_) : cl(cl_) {}
	vector<ImgWithBBLabel> getImgWithBBLabels(vector<ImgWithBB> & img);
	void test(const ImgWithBB &);

private:
    Classifier & cl;
    const float width_height_ratio = 1.4; 
    // mean ent = 0.01
    // max ent = 2.96
    // min ent = 9.17e-10
    const float max_digit_entropy = 0.1;
    const int slide_step = 3;

	int adjust_pred(int pred, Img & img);

    vector<double> estimate_ent(const vector<vector<double> > & probs) {
        vector<double> ents;
        for (int i = 0; i < probs.size(); ++i)
            ents.push_back(entropy(probs[i]));
        return ents;
    }
};
