// WFSNode.h
// Jungseock Joo (jungseock@ucla.edu)
// 01/03/2011
// Updated from LUTAda.h

#ifndef H_WFS_STRONG
#define H_WFS_STRONG

#include <string.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "struct.h"
#include "utils.h"

#include "WFSWeakClassifier.h"
//class WfsWeakClassifier;

#define MAX_NUM_WEAK_LEARNER 50


class WfsStrongClassifier
{
public:

	WfsStrongClassifier();

	WfsStrongClassifier(const WfsStrongClassifier &copy);
	WfsStrongClassifier(int nId, int pId, EWfsNodeType atype, int nChilds);

	~WfsStrongClassifier()
	{
		weakClassifiers.clear();
	}

	//train the strong classifier
	int TrainStrongClassifier(ImgSet *p_pos_set, ImgSet *p_neg_set, double max_false_alarm,
				   double min_detection_rate, /*LUTAdaData* data, */double threshold=0.0);

	//predict according to the strong classifier
	//threshold is the constant b in the paper
	void predict(GranVec& sample, bool& pred_sign, double& pred_conf, double threshold=0.0);

	// markup the active value in set
	void checkPass( ImgSet *p_set, double threshold=0.0);  

	////before testing, we need to load the LUTAdaData
	//int setData(LUTAdaData *data);

		//train the weak learners
	//void trainNestingDetector(WfsWeakClassifier& nest_detector,
	//							  ImgSet* p_pos_set, ImgSet* p_neg_set,
	//							  double* cum_pred_pos_conf, double *cum_pred_neg_conf);

	//void readConfidenceFromImgSet(ImgSet* img_set, double* conf, double& min, double& max);

	//update the weight distribution
	//void updateNestingWeight(WfsWeakClassifier& nest_detector, ImgSet* p_pos_set, ImgSet* p_neg_set);
	//void updateWeight(ImgSet* p_pos_set, double* pred_pos, ImgSet* p_neg_set, double* pred_neg);


	//void checkPerformance(double *pred_pos_conf, int num_pos_img, double* pred_neg_conf, int num_neg_img,
	//						  double threshold, double& current_false_alarm, double& current_detection_rate);

	void CheckStagePass(ImgSet *set, int category = 0);

	void CheckStagePassForDetectionBranching(ImgSet *set, vector<ImgSet *>& child_sets);
	void CheckStagePassForDetectionBinary(ImgSet *set) ;

private:
	void UpdateWeightForBranchingNode(ImgSet* p_pos_set, double** pos_weak_conf,
							  ImgSet* p_neg_set, double** neg_weak_conf);


	void UpdateWeightForBinaryNode(ImgSet* p_pos_set, double* pos_weak_conf, 
						  ImgSet* p_neg_set, double* neg_weak_conf);

	//normailzie the weight distribution
	void normalizeWeight(ImgSet* p_pos_set, ImgSet* p_neg_set);

	void CheckPerformanceBranchingNode(double &false_positive, double &detection_rate, ImgSet *pos_set, double **pos_confs, ImgSet *neg_set, double **neg_confs);
	void CheckPerformanceBinaryNode(double &false_positive, double &detection_rate, ImgSet *pos_set, double *pos_confs, ImgSet *neg_set, double *neg_confs);

	//void saveLUTAdaData(LUTAdaData* data);

	////used for nesting detector
	//int transformConf2BinIndex(double *pos_conf, int num_pos_img, double *neg_conf, int num_neg_img,
	//	int *pos_bin_ind, int *neg_bin_ind, double min_conf, double max_conf);

	////
	//void markPass(ImgSet* p_set, double* cum_pred_conf, double threshold);

public:

	int nodeId;
	int parentId;
	EWfsNodeType ntype;
	// if branching node
	// number of child nodes = branches, which means there are nc+1 categories (including non-face)
	int nc;

	bool is_trained;

	// if this is not zero, this is the leaf node of branching tree and will be connected to cascade of binary (non-branching) classifiers
	int finalPose;
	int cascadeId;	// binary cascade

	int cascadeStage;
	bool isFinalStage;

	vector<WfsWeakClassifier> weakClassifiers;

};

#endif





	
	//LUTAda(int _stage=0);
	//LUTAda(const LUTAda& rhs);
	//~LUTAda();


	




