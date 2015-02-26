#ifndef H_IMAGEHANDLER
#define H_IMAGEHANDLER

#include <string.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "utils.h"
#include "Struct.h"
using namespace std;
#define MIN_CROP_WIN 16  // the minimum crop window for a negative image

class ImageHandler
{
public:
	ImageHandler(void);
	~ImageHandler(void);

	int ReadPositiveSamplesFromList(ImgSet *p_imgset, vector<string> &file_list, vector<int> &pose_list, int& list_offset, int node_id, int num_needed, int given_pose=0);
	int ReadNegativeSamplesFromList(ImgSet *p_imgset, vector<string> &file_list, int& list_offset, int num_needed, int max_samples_per_image);

	void NormalizeGranVecs(ImgSet &set);

	int openDir(string directory, ImgSet *p_imgset, int num_needed, bool label);	// read the images in the directory to ImageSet struct
	//int openNegDir(string directory, ImgSet *p_imgset, int num_needed, bool label);
	int saveDir(string directory, ImgSet *p_imgset, string name = "img");			// save the imageset into images at directory
	int getNonFace(string directory, ImgSet *p_imgset, int num_needed);				// get certain number of nonface images from a folder
	//int mergeImgSet(ImgSet *p_resultset, ImgSet *p_appendset);						// merge two set of ImageSets to one
	int shrinkImgSet(ImgSet *p_imgset, int length);									// shrink imgset to length
	void cleanImgSet(ImgSet *p_imgset);											// get rid of inactive images
	void mix(ImgSet *p_imgset);
	//void initWeight(ImgSet *p_imgset);

	// from utils
	int CropNonFaceFromImageFile(std::string& filename, ImgSet &is, int num_needed);
	int CropRectFromImage(IplImage *image, ImgSet &is, int rectsize, int minshift);

	int CropAllSamplesFromImage(ImgSet *set, IplImage *image, double scaleFactor, int minShift);
	int CropRandomSamplesFromImage(ImgSet *set, IplImage *image, int num);

	int convertIplImgToImgVec(IplImage *in, ImgVec& out);
	int convertImgVecToIplImg(ImgVec &in, IplImage *out);
	int resizeIplImgToWindowSize(IplImage *in, IplImage **out);

	int LoadImgVecFromImageFile(std::string& filename, ImgVec &iv);
	int SaveImageFileFromImgVec(std::string& filename, ImgVec &iv);
	void getGranFromImg(ImgVec &input, GranVec &result);
	void getImgFromGran(GranVec &input, ImgVec &result);

	int getNumPosUsed(){return _cur_pos_img;}
	int getNumNegUsed(){return _neg_img_count;}

	int buildRandNegImg(std::string source, std::string dest, std::string name, int num);
	int convertIplImgToGrayscale(IplImage *in, IplImage **out);
	void getAllGranVecInAScaleFromIplImg(ImgSet& img_set, IplImage *img, int w, int h, int jump);

private:
	vector<string> pos_output_files;
	vector<string> neg_output_files;
	int _cur_pos_img;
	int _cur_neg_img;
	//int negseed_dir;   // for openNegDir

	// for CropNonFaceFromImageFile
	int cur_win_size;
	int cur_x;
	int cur_y;

	int _neg_img_count;
};


#endif