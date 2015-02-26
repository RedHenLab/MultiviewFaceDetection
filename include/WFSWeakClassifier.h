// WFSNode.h
// Jungseock Joo (jungseock@ucla.edu)
// 01/03/2011

#ifndef H_WFS_WEAK
#define H_WFS_WEAK

#include <string.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "struct.h"
#include "utils.h"

//#include "WFSNode.h"

class WfsWeakClassifier
{
public:

	WfsWeakClassifier(const WfsWeakClassifier& rhs);

	WfsWeakClassifier(EWfsNodeType atype, int nChilds);
	WfsWeakClassifier(SparseFeature& sF, EWfsNodeType atype, int nChilds);

	WfsWeakClassifier& WfsWeakClassifier::operator=(const WfsWeakClassifier &rhs);

	~WfsWeakClassifier();

	void SetBranchingClassifier(double **vec_confs, int minf, int maxf);
	void SetBinaryClassifier(double *confs, int minf, int maxf);

	//build the confidence table for LUTClassifier
	double countSamples(int nPos, int *pos_eval_feat_values, GranVec *pos_samples,  
		int nNeg, int *neg_eval_feat_values, GranVec* neg_samples, 
		int _min_featValue, int _max_featValue);


	////predict based on input feature
	//double predict(int eval_feat_value);
	
	//predict many samples
    //void predictSamples(int* eval_feat_values, int size, double* conf);
	void ComputeWeakConfidenceList(double* confs, int* feat_values, int size);

	// for branching
	void ComputeWeakConfidenceVectorList(double** confs, int* feat_values, int size);
	void AddWeakConfidenceVectorListGivenCategory(double* confs, int* feat_values, int size, int category);

	void AddWeakConfidenceVectorList(double** confs, int* feat_values, int size);

	void AddWeakConfidenceList(double* confs, int* feat_values, int size);

	void FindBinList(int *bins, int *feat_values, int size);
	int FindBin(int feat_value);


//	double* getConfTable() {return conf_table;}
	SparseFeature getSparseFeature() {return sparse_feature;}
	void setSparseFeature(SparseFeature sp) {sparse_feature = sp;}

	////build the confidence table for nesting detector (special LUTClassifier with confidence as input)
	//double countNestSamples(int nPos, int *pos_int_conf, GranVec *pos_samples,  
	//	int nNeg, int *neg_int_conf, GranVec* neg_samples, 
	//	double _min_conf, double _max_conf);

	////prediction function used by nesting detector
	//double predict(double conf);
	////predict many samples
 //   void predictSamples(double* conf_values, int size, double* conf);



	//
	
	int getMinFeatValue() {return min_featValue;}			//minimal evaluated feature value in the training set
	int getMaxFeatValue() {return max_featValue;}

	void getMinMaxFeatValue(int &min, int &max) {min = min_featValue; max = max_featValue;}
	void getMinMaxConf(double &min, double &max) {min = min_conf; max = max_conf;}

	void initialize(int min_f, int max_f);
	void initialize(double min_f, double max_f);

	//get the confidence table
	void calculateConfTable();
		
public:

	EWfsNodeType ntype;
	// if branching node
	// number of child nodes = branches, which means there are nc+1 categories (including non-face)
	int nc;

	double* conf_table;			//confidence value of j-th bin
	double* w_plus;				//number of positive examples in j-th bin
	double* w_minus;			//number of negative examples in j-th bin

	// for vector hypothesis
	double** vecConfTable;			//confidence value of j-th bin

	double norm_factor;			// the normalization factor

	int min_featValue;			//minimal evaluated feature value in the training set
	int max_featValue;			//maximal evaluated feature value in the training set

//	int num_shift;					//number of shift used for lookup table
	SparseFeature sparse_feature;		//the sparse feature used in this weak learner

	double min_conf;			//used for nesting detector: min confidence
	double max_conf;			//max confidence

};

#endif

