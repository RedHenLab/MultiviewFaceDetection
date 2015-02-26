#ifndef H_WFS_TRAINER
#define H_WFS_TRAINER

#include <string.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "struct.h"
#include "utils.h"

#include "ImageHandler.h"
#include "WFSNode.h"

struct TrainInfo
{
	string positive_list_file;
	string negative_list_file;
	//string dir_pos;
	//string dir_neg;
	int nBinaryStages;
	int train_num_pos;
//	int train_num_neg;
	double nNegSamplesRatio;

	double min_detection_rate;
	double max_false_alarm;
	double threshold;
};


class WfsMvfdTrainer 
{
public:
	WfsMvfdTrainer(TrainInfo &info);
	~WfsMvfdTrainer();

	void ReadFileList();

	void SetSampleSetSize();

	int DoTrain(int start_node = 0);
	void ResumeTrain(const string &classifier_file, int resume_node, int resume_pose = 0);

	int DoTrainForSinglePose(int pose_id);

	int load(const string& filename);		// load xml file into v_lutada_data
	int save(const string& filename);		// save v_lutada_data into xml file

	//int save(string directory, CascadeData cc_data);
	//int load(string directory, CascadeData &cc_data);
	wstring s2ws(const string& s);
	string ws2s(const wstring& ws);
	int ws2i(const wstring& ws);
	double ws2f(const wstring& ws);

	bool ws2b(const wstring &ws);

// for testing
//	int initializeLUTAda();					// load the v_lutada_data into the v_lutada

	//void detectMultiScale( IplImage *image,                    // detect faces in an image
	//					   vector<Rect>& objects,
 //                          double scaleFactor=1.1,
 //                          int minNeighbors=2, int flags=0,
 //                          int minSize=16, int maxSize = 300);

	//void detectMultiScaleOrg( IplImage *image,                    // detect faces in an image
	//					   vector<Rect>& objects,
 //                          double scaleFactor=1.1,
 //                          int minNeighbors=2, int flags=0,
 //                          int minSize=16, int maxSize = 300);



	//bool setImage( const Mat& image);       // Set the testing image
    //int runAt( Point point);				// detect face at point xy in the image

	int getNumPosUsed(){return pIH->getNumPosUsed();}
	int getNumNegUsed(){return pIH->getNumNegUsed();}

	int TestSamples(ImgSet *set, int node_id, vector<int> &path_id);

	void checkFeatures();

	int CollectNegativeSamples(int num,  WfsStrongClassifier &node, ImgSet *p_negset, vector<int>& path_id);
	int CollectPositiveSamples(int num,  WfsStrongClassifier &node, ImgSet *p_posset, vector<int>& path_id);

	void checkPass(ImgSet *p_set);


public:

	ImageHandler *pIH;
	TrainInfo trainInfo;

//	int num_stage;
	vector<WfsStrongClassifier> wfsNodes;
//	vector<WfsStrongClassifier> wfsCascadeNodes;
//	vector<LUTAdaData> v_lutada_data;
	
	vector<string> posSamplesFilenames;
	vector<int> posSamplesPoses;
	vector<string> negImageFilenames;

	// this is numbers of samples that we have and may not be the same as the one that we want to use.
	int nPosSamplesOfEachPose[K_WFS_POSES + 1];
	int nPosSamplesOfEachBranchNode[K_WFS_BRANCH_NODES];

//	int crtStage;

	vector<int> cascadeIdx;


};


#endif
