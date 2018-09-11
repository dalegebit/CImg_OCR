# CImg_OCR
It's the final project for SYSU Computer Vision 2017 Spring.

## Example
<img src="https://github.com/dalegebit/CImg_OCR/blob/master/pics/1.jpg" width="400">

![](https://github.com/dalegebit/CImg_OCR/blob/master/result/1.jpg)

<img src="https://github.com/dalegebit/CImg_OCR/blob/master/pics/5.jpg" width="400">

![](https://github.com/dalegebit/CImg_OCR/blob/master/result/5.jpg)

## Algorithm
### Basic Algorithm 
1. Crop the A4 paper from source image. 
2. Segment the digits from the A4 paper. 
3. Classify each segment into a digit. 
 
### Step 1 
1. Use Canny to extract all possible edges of a paper in the image. 
2. Use Hough transformation to obtain the line parameters of the 4 four edges of the paper, and calculate the 4 corner points.  
3. Determine the correspondence between the 4 corner points and 4 pivotal points of an A4 paper, and in the meantime, determine whether the paper is straight or not.  
4. Calculate the projective translation matrix based on the correspondence discovered in step 3. For each pixel of the result A4 image, assign it the value of the linear interpolation to the position where it is projected according to the translation matrix. 
 
### Step 2 
1. Apply BotHat transformation to the A4 image and use ostu algorithm to automatically choose a threshold to binarize the transformed image. (BotHat transformation: apply a big gaussian kernel to an image and then subtract it)
2. Use basic vertical projection method to chop the binary image into blobs along y-axis then x-axis. (Vertical projection method: obtain the histogram by cumulating the pixels’ value along a specific axis, and find the valleys as possible seams between two blobs) 
3. Find all the connected components inside a blob and regard them as possible digits, and remove connected components with few pixels(denoise). 
 
### Step 3 
1. Normalize all images in Mnist into [-1, 1]. 
2. Train an SVM classifier with Mnist. (Directly feeding the normalized images without taking any features) 
3. Use the classifier to preliminarily judge the digit of each (also normalized) segment found in step 2. 4. Utilize some strategy to adjust the classifier’s result for some digits with high error rate. For example: (1) adjust the position of the binary digit in the segment and do prediction again, (2) apply erosion to the digit and do prediction again. (3) directly use the Euler number of the segment to judge the result

## Dir Structure
* All source codes relate to my program are included in `src/`.
* All source codes relate to svm training are included in `train/train/`.
* All source codes relate third party library are included in `lib/`.
* All test images are included in `pics/`.
* All result images are included in `result/`.
* The training data (MNIST) is included in `train/data/`.
* The pretrained svm model is included in `train/model/`.

## Code Structure
There are mainly 7 classes included in the program: Canny, HoughTransformer, CropA4, Binarizer, Chopper, Classifier and Extractor. Canny, HoughTransformer and CropA4 mostly implement the Step 1. Binarizer is to binarize the A4 image with BotHat transformation and ostu auto thresholding. Chopper is to chop the binary image into segments, each of which contains a possible digit. Classifier loads the pretrained svm model and use it to classify the segments. Extractor firstly uses Classifier to classify the segments and then uses some strategies to adjust the predicted results. The code related to those classes is included in src/ directory 
 
Apart from that, to facilitate svm training, I also implement Mnist_Parser to help loading Mnist data and a libsvm wrapper to make libsvm handy. The code related to training is included in train/ directory. 
 
As for third party library, I only use CImg to operate on images, LibSVM to implement svm training and predicting, LibJPEG to read and save jpeg images and Zlib to read Mnist data set that is compressed in *.gz. All code related to third party library is included in lib/

## Compiling
Use Visual Studio 2015 to open the *.sln file.

## Testing
Tap `final.exe`, the test program with run. For each image in `pics/`, the A4 image cropped from it, the result of BotHat transformtion, the binary image after ostu auto thresholding and the final digit recognizing result will be displayed successively in the test program. Tap `train.exe` in `train/` you can train a new svm model with MNIST and do test on the test set. The test result includes accuracy, error rate in general and for each class, etc. If there is any problem with these two programs, please open the visual studio project by tapping `final.sln` and regenerate them, the new released programs will appear in `Release/`.

## To SYSU Students
I had a very bad experience in this class because there was a very irresponsible TA who never examined the source code and scored me below 3/4 of my classmates for not having a pretty pdf report because it was generated from markdown. I was angry. So feel free to use my code, and don't forget to star :)

