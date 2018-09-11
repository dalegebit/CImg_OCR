
#include "mnist.h"
#include <svm_wrapper.h>
#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include <image_utils.h>

using namespace std;

inline void my_test_model(svm::problem& problem, svm::model& model){
	int nclass = model.classes();
	vector<double> prob_estimates(nclass);
	vector<vector<int> > confuse_cnt(10, vector<int>(nclass, 0));
	vector<int> error_cnt(nclass, 0);
	vector<int> label_cnt(nclass, 0);
	vector<double> each_mean_ent(nclass, 0.0);

	std::size_t correct = 0;
	double mean_ent = 0.0, max_ent = -1, min_ent = 10;

	for (std::size_t s = 0; s < problem.n_samples; ++s){
		auto label = svm_predict_probability(model.get_model(), problem.sample(s), &prob_estimates[0]);
		double ent = entropy(prob_estimates);

		label_cnt[problem.label(s)] ++;
		if (label == problem.label(s)){
			/*if (label == 7 || label == 9 || label == 4) {
				float tmp[28 * 28];
				for (int i = 0; i < 28 * 28; ++i)
					tmp[i] = problem.sample(s)[i].value;
				Img test(tmp, 28, 28, 1, 1);
				test.display(0, false);
			}*/
			++correct;
		}
		else {
			error_cnt[problem.label(s)] ++;
			confuse_cnt[problem.label(s)][label] ++;
		}

		mean_ent += ent;
		max_ent = max(max_ent, ent);
		min_ent = min(min_ent, ent);
		each_mean_ent[problem.label(s)] += ent;

	}

	std::cout << "Samples: " << problem.n_samples << std::endl;
	std::cout << "Correct: " << correct << std::endl;
	std::cout << "Accuracy: " << (100.0 * correct / problem.n_samples) << "%" << std::endl;
	std::cout << "Error: " << (100.0 - (100.0 * correct / problem.n_samples)) << "%" << std::endl;
	std::cout << "Mean Entropy: " << mean_ent / problem.n_samples << endl;
	std::cout << "Max Entropy: " << max_ent << endl;
	std::cout << "Min Entropy: " << min_ent << endl;
	for (int i = 0; i < nclass; ++i) {
		int max_j = i;
		for (int j = 0; j < nclass; ++j)
			if (confuse_cnt[i][j] > confuse_cnt[i][max_j])
				max_j = j;

		cout << i << ": cnt=" << label_cnt[i] << ", error_rate=" << error_cnt[i] / float(label_cnt[i]) << ", most_confuse=" << max_j << ", mean_ent=" << each_mean_ent[i] / label_cnt[i] << endl;
	}
}

int main() {
	Mnist_Parser parser("data");
	vector<Sample> samples(parser.load_training());
	//for (auto & s : samples) {
	//	s.image.threshold(10);
	//}
	vector<uint8_t> labels(samples.size());
	vector<array<float, 28 * 28> > images(samples.size());
	for (int i = 0; i < samples.size(); ++i) {
		labels[i] = samples[i].label;
		samples[i].image.normalize(-1, 1);
		memcpy(&images[i][0], samples[i].image._data, 28 * 28 * sizeof(float));
	}
	auto train_problem = svm::make_problem(labels, images);
	auto params = svm::default_parameters();

	params.svm_type = C_SVC;
	params.kernel_type = RBF;
	params.probability = 1;
	params.C = 9;
	params.gamma = 2/724.0;

	svm::model model;
	model = svm::train(train_problem, params);
	
	svm::save(model, "model/svm.model");
	
	// test on training set
	//svm::test_model(train_problem, model);
	
	// test on test set
	vector<Sample> test_samples(parser.load_testing());

	labels.resize(test_samples.size());
	images.resize(test_samples.size());
	for (int i = 0; i < test_samples.size(); ++i) {
		labels[i] = test_samples[i].label;
		test_samples[i].image.normalize(-1, 1);
		memcpy(&images[i][0], test_samples[i].image._data, 28 * 28 * sizeof(float));
	}
	auto test_problem = svm::make_problem(labels, images);
	my_test_model(test_problem, model);




	//Mnist_Parser parser("data");
	//vector<Sample> samples(parser.load_testing());
	////for (auto & s : samples) {
	////	s.image.display();
	////}
	//vector<uint8_t> labels(samples.size());
	//vector<array<float, 28 * 28> > images(samples.size());


	//labels.resize(samples.size());
	//images.resize(samples.size());
	//for (int i = 0; i < samples.size(); ++i) {
	//	labels[i] = samples[i].label;
	//	samples[i].image.normalize(-1, 1);
	//	memcpy(&images[i][0], samples[i].image._data, 28 * 28 * sizeof(float));
	//}

	//svm::model model = svm::load("model/svm.model");
	//auto test_problem = svm::make_problem(labels, images);
	//my_test_model(test_problem, model);
	
	int tmp;
	cin >> tmp;
}