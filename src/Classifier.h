#pragma once

#include "image_utils.h"
#include <svm_wrapper.h>
#include <string>

using namespace std;

class Classifier {
public:
    Classifier() {}
    Classifier(const string & model_file_) : model_file(model_file_) {
        loadModel(model_file);
    }
    void loadModel(const string & model_file_) {
        model_file = model_file_;
        model = svm::load(model_file);
    }
	int predict(const Img & img, bool lower_frame=false);
	vector<double> predictProba(const Img & img, bool lower_frame=false);
private:
    string model_file;
    svm::model model;
	int frame_len = 3;

	Img preprocess(const Img & img, bool lower_frame);
	vector<double> fix_prob_order(vector<double> & prob);
};
