#include "Classifier.h"
#include <array>
#include <memory>

using namespace std;

vector<double> Classifier::fix_prob_order(vector<double> & prob) {
	vector<double> new_prob(model.classes());
	vector<int> label = model.label();
	for (int i = 0; i < model.classes(); ++i)
		new_prob[label[i]] = prob[i];
	return new_prob;
}

int Classifier::predict(const Img & img, bool lower_frame) {
	Img preprocessed = preprocess(img, lower_frame);

	vector<double> prob_estimates(model.classes());

	auto features = preprocessed.size();
	vector<svm_node> svm_sample(features + 1);

	for (std::size_t i = 0; i < features; ++i){
		svm_sample[i].index = i + 1;
		svm_sample[i].value = preprocessed._data[i];
	}

	//End the vector
	svm_sample[features].index = -1;
	svm_sample[features].value = 0.0;

	double val = svm_predict_probability(model.get_model(), &svm_sample[0], &prob_estimates[0]);
	fix_prob_order(prob_estimates);
	return val;
}

vector<double> Classifier::predictProba(const Img & img, bool lower_frame) {
	Img preprocessed = preprocess(img, lower_frame);

	vector<double> prob_estimates(model.classes());

	auto features = preprocessed.size();
	vector<svm_node> svm_sample(features + 1);

	for (std::size_t i = 0; i < features; ++i){
		svm_sample[i].index = i + 1;
		svm_sample[i].value = preprocessed._data[i];
	}

	//End the vector
	svm_sample[features].index = -1;
	svm_sample[features].value = 0.0;

	svm_predict_probability(model.get_model(), &svm_sample[0], &prob_estimates[0]);

	return fix_prob_order(prob_estimates);
}

Img Classifier::preprocess(const Img & img, bool lower_frame) {
	int max_len = max(img._width, img._height);
	Img resized = img.get_resize((28 - 2*frame_len)*img._width / max_len, (28 - 2*frame_len)*img._height / max_len);
	int delta_x = 28 - resized._width, delta_y = 28 - resized._height;
	int x_frame = (delta_x + 1) / 2, y_frame = (delta_y + 1) / 2;
	if (lower_frame)
		y_frame = delta_y-1;
	Img canvas(28, 28, 1, 1, 0);
	cimg_forXY(resized, x, y)
		canvas(x + x_frame, y + y_frame) = resized(x, y);
	canvas.normalize(-1, 1);
	//canvas.display(0, false);
	return canvas;
}