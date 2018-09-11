#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <image_utils.h>

using namespace std;

struct Sample
{
	uint8_t label; // label for a specific digit
	Img image;
	Sample(float_t label_, Img & image_) :label(label_), image(image_){}
};


class Mnist_Parser
{
public:
	Mnist_Parser(string data_path) :
		test_img_fname(data_path + "/t10k-images.idx3-ubyte"),
		test_lbl_fname(data_path + "/t10k-labels.idx1-ubyte"),
		train_img_fname(data_path + "/train-images.idx3-ubyte"),
		train_lbl_fname(data_path + "/train-labels.idx1-ubyte"){}

	vector<Sample> load_testing(){
		test_sample = load(test_img_fname, test_lbl_fname);
		return test_sample;
	}

	vector<Sample> load_training(){
		train_sample = load(train_img_fname, train_lbl_fname);
		return train_sample;
	}

	void test(){
		srand((int)time(0));
		size_t i = (int)(rand());
		cout << i << endl;
		cout << (int)test_sample[i].label << endl;
		//test_sample[i]->image->display();

		size_t j = (int)(rand() * 60000);
		cout << (int)(train_sample[i].label) << endl;
		//train_sample[i]->image->display();

	}

	// vector for store test and train samples
	vector<Sample> test_sample;
	vector<Sample> train_sample;

private:
	vector<Sample> load(string fimage, string flabel){
		ifstream in;
		in.open(fimage, ios::binary | ios::in);
		if (!in.is_open()){
			cout << "file opened failed." << endl;
		}

		uint32_t magic = 0;
		uint32_t number = 0;
		uint32_t rows = 0;
		uint32_t cols = 0;

		in.read((char*)&magic, sizeof(uint32_t));
		in.read((char*)&number, sizeof(uint32_t));
		in.read((char*)&rows, sizeof(uint32_t));
		in.read((char*)&cols, sizeof(uint32_t));

		assert(swapEndien_32(magic) == 2051);
		cout << "number:" << swapEndien_32(number) << endl;
		assert(swapEndien_32(rows) == 28);
		assert(swapEndien_32(cols) == 28);

		float_t img[28 * 28];
		vector<Img> images;

		uint8_t pixel = 0;
		size_t col_index = 0;
		size_t row_index = 0;
		size_t cnt = 0;
		while (!in.eof()){
			in.read((char*)&pixel, sizeof(uint8_t));
			img[cnt++] = (float_t)pixel;
			if (cnt == 28 * 28){
				//i.display();
				images.emplace_back(img, 28, 28);
				cnt = 0;
			}
			
		}

		in.close();

		assert(images.size() == swapEndien_32(number));

		//label
		in.open(flabel, ios::binary | ios::in);
		if (!in.is_open()){
			cout << "failed opened label file";
		}

		in.read((char*)&magic, sizeof(uint32_t));
		in.read((char*)&number, sizeof(uint32_t));

		assert(2049 == swapEndien_32(magic));
		assert(swapEndien_32(number) == images.size());

		vector<uint8_t>	labels;

		uint8_t label;
		while (!in.eof())
		{
			in.read((char*)&label, sizeof(uint8_t));
			//cout << (int)label << endl;
			labels.push_back(label);
		}

		vector<Sample> samples;
		for (int i = 0; i < swapEndien_32(number); i++){
			samples.emplace_back(labels[i], images[i]);
		}

		cout << "Loading complete" << endl;
		in.close();
		return samples;
	}

	// reverse endien for uint32_t
	uint32_t swapEndien_32(uint32_t value){
		return ((value & 0x000000FF) << 24) |
			((value & 0x0000FF00) << 8) |
			((value & 0x00FF0000) >> 8) |
			((value & 0xFF000000) >> 24);
	}

	// filename for mnist data set
	string test_img_fname;
	string test_lbl_fname;
	string train_img_fname;
	string train_lbl_fname;


};