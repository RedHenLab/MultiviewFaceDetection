#ifndef H_TRAIN
#define H_TRAIN

#include "SparseFeature.h"

#include "WfsWeakClassifier.h"
#include "utils.h"

#define USE_FEATURE_AGE_FACTOR true

#define K_AVOID_SIMILAR_INITIAL_FEATURE true
#define K_MULTI_SCALED_SEARCH false

//#define K_MIN_SAMPLES_MULTI_SCALED_SEL 500

// minimum number of samples to be used for multi scaled selection of features.
#define K_MIN_SAMPLES_MULTI_SCALED_SEL 500
#define K_MULTI_SCALED_SEARCH_MIN_FEATURES 50


class SparseFeatureLearner 
{
public:
	SparseFeatureLearner();
	~SparseFeatureLearner();

	void Initialize();
	void InitialSeedsSelection();

//	int SetTrainingSamples(std::string &face_folder, std::string &nonface_folder, int max_pos_samples = -1);

	void SetTrainingSamples(int nPos, GranVec *pos_samples, int nNeg, GranVec *neg_samples);
	void SetTrainingSamples(ImgSet &pos_set, ImgSet &neg_set);

//	void SelectBestFeature(SparseFeature& output);
	void SelectBestFeature(WfsWeakClassifier& output);

	void UpdateSampleWeights(ImgSet &pos_set, ImgSet &neg_set);
	
private:
	void SparseFeatureLearner::SetStepsForMultiScaleSearch();
	int GetDistanceBtwnFeatures(SparseFeature &f1, SparseFeature &f2);
//	int LoadPositiveSamples(int n_samples);

	void CopyHaarFeatures();
	void InitializeArraysForNewWeakRound();
	double SparseFeatureLearner::ComputeFitness(double disc, int complex, int age);

	void DeleteHaarGranFeatureArrays();
	int ExpandFeature(int id);
	int AddBestExpandedFeature(std::vector<SparseFeature>& list);
	bool FindTheSameFeature(std::vector<SparseFeature>& list, SparseFeature &feature);
	// member variables
public:

	int nodeId;
	int parentId;
	EWfsNodeType ntype;
	int nC;

private:

	// number of samples being used in current stage
	int nPos;
	int nNeg;

	GranVec* posVecs;
	GranVec* negVecs;

	int nHaarFeats;
	std::vector<SparseFeature> initHaarFeatList;

	// pre-computed feature values for sparse features in initHaarFeatList
	// only updated when SetTrainingSamples is called
	int **initHaarFeatValsPos;
	int **initHaarFeatValsNeg;

	int *initHaarMin;
	int *initHaarMax;

	std::vector<SparseFeature> candFeatList;
	std::vector<bool> isOpen;
	std::vector<int> age;
	std::vector<double> discriminability;
	std::vector<double> fitness;

	std::vector<WfsWeakClassifier*> weakClassifiers;
	
	std::vector<SparseFeature> visitedFeatList;

	std::vector<int> nPosMScale;
	std::vector<int> nNegMScale;
	int nStepsMScale;

	// temp........
	bool isGranVecsCreatedHere;

};

#endif
