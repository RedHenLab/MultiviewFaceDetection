//#ifndef _LUTADA_H
//#define _LUTADA_H
//
//#include <vector>
//#include "utils.h"
//#include "LUTClassifier.h"
//#include "SparseFeature.h"
//#include "SparseFeatureLearner.h"
//
//using namespace std;
//
//#define MAX_NUM_WEAK_LEARNER 10
//
////real adaboost with weak learners using look-up tables
//class LUTAda{
//	
//public:
//	
//	LUTAda(int _stage=0);
//	LUTAda(const LUTAda& rhs);
//	~LUTAda();
//
//
//	//train the strong classifier
//	int train(ImgSet *p_pos_set, ImgSet *p_neg_set, double max_false_alarm,
//				   double min_detection_rate, LUTAdaData* data, double threshold=0.0);
//
//	
//	//predict according to the strong classifier
//	//threshold is the constant b in the paper
//	void predict(GranVec& sample, bool& pred_sign, double& pred_conf, double threshold=0.0);
//
//	// markup the active value in set
//	void checkPass( ImgSet *p_set, double threshold=0.0);  
//
//	//before testing, we need to load the LUTAdaData
//	int setData(LUTAdaData *data);
//
//private:
//	
//	//train the weak learners
//	void trainNestingDetector(LUTClassifier& nest_detector,
//								  ImgSet* p_pos_set, ImgSet* p_neg_set,
//								  double* cum_pred_pos_conf, double *cum_pred_neg_conf);
//
//	//void readConfidenceFromImgSet(ImgSet* img_set, double* conf, double& min, double& max);
//
//	//update the weight distribution
//	void updateNestingWeight(LUTClassifier& nest_detector, ImgSet* p_pos_set, ImgSet* p_neg_set);
//	void updateWeight(ImgSet* p_pos_set, double* pred_pos, ImgSet* p_neg_set, double* pred_neg);
//
//	//normailzie the weight distribution
//	void normalizeWeight(ImgSet* p_pos_set, ImgSet* p_neg_set);
//
//	void checkPerformance(double *pred_pos_conf, int num_pos_img, double* pred_neg_conf, int num_neg_img,
//							  double threshold, double& current_false_alarm, double& current_detection_rate);
//
//
//	void saveLUTAdaData(LUTAdaData* data);
//
//	//used for nesting detector
//	int transformConf2BinIndex(double *pos_conf, int num_pos_img, double *neg_conf, int num_neg_img,
//		int *pos_bin_ind, int *neg_bin_ind, double min_conf, double max_conf);
//
//	//
//	void markPass(ImgSet* p_set, double* cum_pred_conf, double threshold);
//
//
//	//LUTClassifier nesting_detector;
//	vector<LUTClassifier> picked_classifiers;			//selected weak classifiers
//	
//	int stage;					//in which stage of the cascaded structure
//	int num_weak;				//number of weak learners in this class
//
//
//};
//
//
//
//#endif