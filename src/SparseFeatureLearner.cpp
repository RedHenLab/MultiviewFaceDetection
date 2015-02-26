#include "../include/SparseFeatureLearner.h"

#include <stdlib.h>
#include <time.h>
#include <algorithm>

using namespace std;

#define K_MAX_SPARSE_FEATURE_HEURISTIC_ITERATIONS 30
#define K_MIN_DIST_BTWN_FEATS 5

SparseFeatureLearner::SparseFeatureLearner()
{
//	srand ( time(NULL) );
	//srand(0);

	nPos = 0;
	nNeg = 0;

	posVecs = NULL;
	negVecs = NULL;

	candFeatList.clear();

	nHaarFeats = 0;
	initHaarFeatList.clear();

	initHaarFeatValsPos = NULL;
	initHaarFeatValsNeg = NULL;

	initHaarMin = NULL;
	initHaarMax = NULL;

	nC = 1;
	ntype = BINARY;

	isGranVecsCreatedHere = false;

	Initialize();

}

SparseFeatureLearner::~SparseFeatureLearner()
{
	//if (posEnabled) {
	//	delete [] posEnabled;
	//	posEnabled = NULL;
	//}


	DeleteHaarGranFeatureArrays();

}

void SparseFeatureLearner::Initialize()
{
	cout << "SparseFeatureLearner :: Initialize\n";

	InitialSeedsSelection();

}

void SparseFeatureLearner::InitialSeedsSelection()
{

	// (width, height, max scale, n, 4 * [x, y, scale, sign])
	const int preset[][20] = {
		{2, 1, 0, 2, 			0, 0, 0, 1,			1, 0, 0, -1},
		{1, 2, 0, 2,			0, 0, 0, 1,			0, 1, 0, -1},
		{2, 2, 0, 2,			0, 0, 0, 1,			1, 1, 0, -1},
		{2, 2, 0, 2,			0, 1, 0, 1,			1, 0, 0, -1},
		{3, 1, 0, 3,			0, 0, 0, 1,			1, 0, 0, -1,			2, 0, 0, 1},
		{1, 3, 0, 3,			0, 0, 0, 1,			0, 1, 0, -1,			0, 2, 0, 1},
		{4, 4, 2, 2,			0, 0, 2, -1,			1, 1, 1, 1},
		{2, 2, 1, 2,			0, 0, 1, 1,			0, 0, 0, -1},
		{2, 2, 1, 2,			0, 0, 1, 1,			0, 1, 0, -1},
		{2, 2, 1, 2,			0, 0, 1, 1,			1, 0, 0, -1},
		{2, 2, 1, 2,			0, 0, 1, 1,			1, 1, 0, -1},
		{4, 1, 0, 4,			0, 0, 0, 1,			1, 0, 0, -1,			2, 0, 0, -1,			3, 0, 0, 1},
		{1, 4, 0, 4,			0, 0, 0, 1,			0, 1, 0, -1,			0, 2, 0, -1,			0, 3, 0, 1},
		{2, 2, 0, 4,			0, 0, 0, 1,			1, 0, 0, -1,			0, 1, 0, -1,			1, 1, 0, 1},
		{2, 2, 0, 4,			0, 0, 0, 1,			1, 0, 0, 1,			0, 1, 0, -1,			1, 1, 0, -1},
		{2, 2, 0, 4,			0, 0, 0, 1,			1, 0, 0, -1,			0, 1, 0, 1,			1, 1, 0, -1},
		{-1, -1, -1, -1} };

	initHaarFeatList.clear();

	// Haar-like, regular-squared features

	int p = 0;
	int nFeats = 0;

	while(preset[p][0] != -1) {
		int w = preset[p][0];
		int h = preset[p][1];
		int ms = preset[p][2];	// max scale
		int n = preset[p][3];	// number of granules

		for(int s=0; s<=K_MAX_SCALE - ms; s++) {
			int xend = K_WIN_SIZE - (w << s);
			int yend = K_WIN_SIZE - (h << s);

			for(int x=0; x<=xend; x++) {
				for(int y=0; y<=yend; y++) {
					SparseFeature f;
					f.size = n;
					//f.nNegBlcks = 0;
					//f.nPosBlcks = 0;
					for(int g=0; g<n; g++) {
						f.id.push_back( GetFeatureIDFromXYS(x + (preset[p][4 + 4*g + 0]<<s), y + (preset[p][4 + 4*g + 1]<<s), preset[p][4 + 4*g + 2]+s) );
						
						if (preset[p][4 + 4*g + 3] == 1) {
							f.sign.push_back( true );
							//f.nPosBlcks++;
						} else {
							f.sign.push_back( false );
							//f.nNegBlcks++;
						}
					}

					if (K_AVOID_SIMILAR_INITIAL_FEATURE) { // prevent near feature
						int min_dist = INT_MAX;
						int min_id = -1;

						for(int i=0; i<nFeats; i++) {
							int dist = GetDistanceBtwnFeatures(f, initHaarFeatList[i]);
							if (dist < min_dist) {
								min_dist = dist;
								min_id = i;
							}
						}

						if (min_dist < K_MIN_DIST_BTWN_FEATS) {
							// compare current one & feature[min_id] and decide what to keep
						} else {
							sortSparseFeature(f);

							initHaarFeatList.push_back(f);
							nFeats++;
//							checkFeature(initHaarFeatList[nFeats-1]);
						}
					} else {
						sortSparseFeature(f);

						initHaarFeatList.push_back(f);
						nFeats++;
					}
				}
			}
			//cout << "total " << nFeats << " haar-like initial features were created\n";
			//checkFeature(initHaarFeatList[nFeats-1]);
		}

		p++;

	}

	nHaarFeats = nFeats;

	cout << "total " << nFeats << " haar-like initial features were created\n";
//checkFeature(f)

}


int SparseFeatureLearner::GetDistanceBtwnFeatures(SparseFeature &f1, SparseFeature &f2)
{
	int x1, y1, s1;
	int x2, y2, s2;

	int max_dist = INT_MIN;

	for(int i=0; i<f1.size; i++) {
		int min_dist = INT_MAX;
		GetFeatureXYSFromID(x1, y1, s1, f1.id[i]);
		for(int j=0; j<f2.size; j++) {
			GetFeatureXYSFromID(x2, y2, s2, f2.id[j]);

			int dx = x1+(1<<(s1-1)) - x2-(1<<(s2-1));
			int dy = y1+(1<<(s1-1)) - y2-(1<<(s2-1));
			int ds = (1<<s1) - (1<<s2);

			int dist = dx*dx + dy*dy + ds*ds;
			if (dist < min_dist) {
				min_dist = dist;
			}
		}
		if (max_dist < min_dist) {
			max_dist = min_dist;
		}
	}

	for(int i=0; i<f2.size; i++) {
		int min_dist = INT_MAX;
		GetFeatureXYSFromID(x2, y2, s2, f2.id[i]);
		for(int j=0; j<f1.size; j++) {
			GetFeatureXYSFromID(x1, y1, s1, f1.id[j]);

			int dx = x1+(1<<(s1-1)) - x2-(1<<(s2-1));
			int dy = y1+(1<<(s1-1)) - y2-(1<<(s2-1));
			int ds = (1<<s1) - (1<<s2);

			int dist = dx*dx + dy*dy + ds*ds;
			if (dist < min_dist) {
				min_dist = dist;
			}
		}
		if (max_dist < min_dist) {
			max_dist = min_dist;
		}
	}

	return max_dist;

}


void SparseFeatureLearner::SetTrainingSamples(int n_Pos, GranVec *pos_samples, int n_Neg, GranVec *neg_samples)
{
	nPos = n_Pos;
	nNeg = n_Neg;

	//cout << nPos << " " << nNeg << "\n";
	//cout << pos_samples[0].val[0] << "\n";

	posVecs = pos_samples;
	negVecs = neg_samples;

	SetStepsForMultiScaleSearch();

	// recompute feature values stored for basic haar features
	DeleteHaarGranFeatureArrays();

	initHaarFeatValsPos = new int* [nHaarFeats];
	initHaarFeatValsNeg = new int* [nHaarFeats];

	for(int i=0; i<nHaarFeats; i++) {
		initHaarFeatValsPos[i] = new int [nPos];
		initHaarFeatValsNeg[i] = new int [nNeg];
	};

	initHaarMin = new int [nHaarFeats];
	initHaarMax = new int [nHaarFeats];

	// compute
	for(int i=0; i<nHaarFeats; i++) {
		EvaluateFeatureForMultipleSamples(initHaarFeatValsPos[i], initHaarFeatList[i], posVecs, nPos);
		EvaluateFeatureForMultipleSamples(initHaarFeatValsNeg[i], initHaarFeatList[i], negVecs, nNeg);

		int minp, maxp;
		int minn, maxn;

		minp = maxp = minn = maxn = 0;

		getMinMax(minp, maxp, nPos, initHaarFeatValsPos[i]);
		getMinMax(minn, maxn, nNeg, initHaarFeatValsNeg[i]);

		initHaarMin[i] = min(minp, minn);
		initHaarMax[i] = max(maxp, maxn);

//		std::cout << initHaarFeatValsPos[i][0] << " " << minp << " " << maxp << " " << minn << " " << maxn << "\n";

//		exit(1);
		//checkFeature(initHaarFeatList[i]);
	}


}

void SparseFeatureLearner::UpdateSampleWeights(ImgSet &pos_set, ImgSet &neg_set)
{
	for(int i=0; i<nPos; i++) {
		posVecs[i].weight = pos_set.GetGranVec(i)->weight;
	}

	for(int i=0; i<nNeg; i++) {
		negVecs[i].weight = neg_set.GetGranVec(i)->weight;
	}
}

void SparseFeatureLearner::SetTrainingSamples(ImgSet &pos_set, ImgSet &neg_set)
{
	nPos = pos_set.Size();
	nNeg = neg_set.Size();

	SetStepsForMultiScaleSearch();

	// recompute feature values stored for basic haar features
	DeleteHaarGranFeatureArrays();

	posVecs = new GranVec[nPos];
	negVecs = new GranVec[nNeg];

	for(int i=0; i<nPos; i++) {
		posVecs[i] = *(pos_set.GetGranVec(i));
	}

	for(int i=0; i<nNeg; i++) {
		negVecs[i] = *(neg_set.GetGranVec(i));
	}

	initHaarFeatValsPos = new int* [nHaarFeats];
	initHaarFeatValsNeg = new int* [nHaarFeats];

	for(int i=0; i<nHaarFeats; i++) {
		initHaarFeatValsPos[i] = new int [nPos];
		initHaarFeatValsNeg[i] = new int [nNeg];
	};

	initHaarMin = new int [nHaarFeats];
	initHaarMax = new int [nHaarFeats];

	// compute
	for(int i=0; i<nHaarFeats; i++) {
		EvaluateFeatureForMultipleSamples(initHaarFeatValsPos[i], initHaarFeatList[i], posVecs, nPos);
		EvaluateFeatureForMultipleSamples(initHaarFeatValsNeg[i], initHaarFeatList[i], negVecs, nNeg);

		int minp, maxp;
		int minn, maxn;

		minp = maxp = minn = maxn = 0;

		getMinMax(minp, maxp, nPos, initHaarFeatValsPos[i]);
		getMinMax(minn, maxn, nNeg, initHaarFeatValsNeg[i]);

		initHaarMin[i] = min(minp, minn);
		initHaarMax[i] = max(maxp, maxn);
	}
}


void SparseFeatureLearner::DeleteHaarGranFeatureArrays()
{
	if (initHaarFeatValsPos) {
		for(int i=0; i<nHaarFeats; i++) {
			delete [] initHaarFeatValsPos[i];
		};
		delete [] initHaarFeatValsPos;
		initHaarFeatValsPos = NULL;
	}

	if (initHaarFeatValsNeg) {
		for(int i=0; i<nHaarFeats; i++) {
			delete [] initHaarFeatValsNeg[i];
		};
		delete [] initHaarFeatValsNeg;
		initHaarFeatValsNeg = NULL;
	}

	if (initHaarMin) {
		delete [] initHaarMin;
		initHaarMin = NULL;
	}

	if (initHaarMax) {
		delete [] initHaarMax;
		initHaarMax = NULL;
	}

	if (isGranVecsCreatedHere) {
		cout << "DELETE gran vecs in sparse feature learner\n";
		if (posVecs) delete [] posVecs;
		if (negVecs) delete [] posVecs;

		posVecs = NULL;
		negVecs = NULL;
	}
}
		
void SparseFeatureLearner::SetStepsForMultiScaleSearch()
{
	int n = min(nPos, nNeg);

	nStepsMScale = 1;
	nPosMScale.clear();
	nNegMScale.clear();

	nPosMScale.push_back(nPos);
	nNegMScale.push_back(nNeg);

	while ( n >= (K_MIN_SAMPLES_MULTI_SCALED_SEL<<1) ) {
		n = n >> 1;
		nStepsMScale++;

		nPosMScale.insert(nPosMScale.begin(), nPosMScale[0] >> 1);
		nNegMScale.insert(nNegMScale.begin(), nNegMScale[0] >> 1);
	}

	std::cout << nStepsMScale << "\n";
	for(int i=0; i<nStepsMScale; i++) {
		cout << nPosMScale[i] << " " ;
	}
	cout << "\n";
	for(int i=0; i<nStepsMScale; i++) {
		cout << nNegMScale[i] << " " ;
	}
	cout << "\n";

}

void SparseFeatureLearner::InitializeArraysForNewWeakRound()
{
	weakClassifiers.clear();
	candFeatList.clear();
	isOpen.clear();
	age.clear();
	fitness.clear();
	discriminability.clear();
}

double SparseFeatureLearner::ComputeFitness(double disc, int complex, int age)
{
//	return disc;

	const int comp_max = 10;
	//const double comp_term[] = { 1.000000, 0.985000, 0.970225, 0.955672, 0.941337, 
	//	0.927217, 0.913308, 0.899609, 0.886115, 0.872823, };
	const double comp_term[] = { 1.000000, 0.99000, 0.9800, 0.9700, 0.9600, 
		0.95, 0.94, 0.93, 0.92, 0.91, };

	const int age_max = 50;
	const double age_term[] = { 1.000000, 0.870409, 0.774406, 0.703285, 0.650597, 0.611565, 0.582649, 0.561228, 0.545359, 0.533603, 0.524894, 0.518442, 0.513662, 0.510121, 0.507498, 0.505554, 0.504115, 0.503048, 0.502258, 0.501673, 0.501239, 0.500918, 0.500680, 0.500504, 0.500373, 0.500277, 0.500205, 0.500152, 0.500112, 0.500083, 0.500062, 0.500046, 0.500034, 0.500025, 0.500019, 0.500014, 0.500010, 0.500008, 0.500006, 0.500004, 0.500003, 0.500002, 0.500002, 0.500001, 0.500001, 0.500001, 0.500001, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000};

	//const double age_term[] = { 1.0000000000, 0.8720797074, 0.7768866173, 0.7060477830, 
	//	0.6533323976, 0.6141037473, 0.5849113778, 0.5631876012, 0.5470216483, 0.5349916023, 
	//	0.5260393303, 0.5193774128, 0.5144198842, 0.5107306926, 0.5079853459, 0.5059423703, 
	//	0.5044220708, 0.5032907256, 0.5024488245, 0.5018223158, 0.5013560934, 0.5010091497, 
	//	0.5007509683, 0.5005588401, 0.5004158661, 0.5003094707, 0.5002302955, 0.5001713766, 
	//	0.5001275315, 0.5000949038, 0.5000706235, 0.5000525552, 0.5000391094, 0.5000291036, 
	//	0.5000216578, 0.5000161168, 0.5000119935, 0.5000089251, 0.5000066417, 0.5000049425, 
	//	0.5000036780, 0.5000027370, 0.5000020368, 0.5000015157, 0.5000011279, 0.5000008393, 
	//	0.5000006246, 0.5000004648, 0.5000003459, 0.5000002574, 0.5000001915, 0.5000001425, 
	//	0.5000001061, 0.5000000789, 0.5000000587, 0.5000000437, 0.5000000325, 0.5000000242, 
	//	0.5000000180, 0.5000000134, 0.5000000100, 0.5000000074, 0.5000000055, 0.5000000041, 
	//	0.5000000031, 0.5000000023, 0.5000000017, 0.5000000013, 0.5000000009, 0.5000000007, 
	//	0.5000000005, 0.5000000004, 0.5000000003, 0.5000000002, 0.5000000002, 0.5000000001, 
	//	0.5000000001, 0.5000000001,	0.5000000000, 0.5000000000, };

	if (USE_FEATURE_AGE_FACTOR) {
		complex = min(complex, comp_max);
		age = min(age, age_max);

//		return disc * (1.5 - age_term[age]);
		return disc * comp_term[complex] * (1.5 - age_term[age]);
	} else {
		complex = min(complex, comp_max);

		return disc * comp_term[complex] * (1.5 - age_term[0]);
	}

}

void SparseFeatureLearner::CopyHaarFeatures()
{
	candFeatList = initHaarFeatList;
	int n = candFeatList.size();

	isOpen.resize(n, true);
	age.resize(n, 0);
	fitness.resize(n, 0);
	discriminability.resize(n, 0);

	// create LUT weak classifiers
	for(int i=0; i<n; i++) {
		WfsWeakClassifier *clsf = new WfsWeakClassifier(candFeatList[i], ntype, nC);

		discriminability[i] = 1.0 - clsf->countSamples(nPos, initHaarFeatValsPos[i], posVecs, 
			nNeg, initHaarFeatValsNeg[i], negVecs, initHaarMin[i], initHaarMax[i]);

		fitness[i] = ComputeFitness(discriminability[i], initHaarFeatList[i].size, 0);

//		checkFeature(candFeatList[i]);

		weakClassifiers.push_back(clsf);
	}
}



bool SparseFeatureLearner::FindTheSameFeature(std::vector<SparseFeature>& list, SparseFeature &feature)
{
	bool found = false;
	int n = feature.size;

	for(int j=0; j<(int)list.size(); j++) {
		if (n == list[j].size) {
			bool same = true;
			for(int k=0; k<n; k++) {
				if (feature.id[k] != list[j].id[k])	{
					same = false;
					break;
				}
			}

			if (same) {
				if (feature.sign[0] == list[j].sign[0]) {
					for(int k=1; k<n; k++) {
						if (feature.sign[k] != list[j].sign[k])	{
							same = false;
							break;
						}
					}
				} else {
					for(int k=1; k<n; k++) {
						if (feature.sign[k] == list[j].sign[k])	{
							same = false;
							break;
						}
					}
				}
			}

			if (same) {
				found = true;
				return found;
			}
		}
	}

	return false;

}

int SparseFeatureLearner::AddBestExpandedFeature(std::vector<SparseFeature>& list)
{
	int ret=0;

	for(int i=0; i<(int)list.size(); i++) {
		sortSparseFeature(list[i]);

		bool found = FindTheSameFeature(candFeatList, list[i]);

		if (!found && FindTheSameFeature(visitedFeatList, list[i])) found = true;

		if (found) {
			list.erase(list.begin() + i);
			i--;
		} else {
			visitedFeatList.push_back(list[i]);
		}
	}



	int n = list.size();
	if (n == 0) return 0;

	//for(int i=0; i<list.size(); i++) {
	//	list[i].nNegBlcks = 0;
	//	list[i].nPosBlcks = 0;
	//	for(int j=0; j<list[i].size; j++) {
	//		if (list[i].sign[j]) {
	//			list[i].nPosBlcks++;
	//		} else 
	//		{
	//			list[i].nNegBlcks++;
	//		}
	//	}
	//};

		


	// create weak classifier
	std::vector<WfsWeakClassifier*> new_classifiers;
	for(int i=0; i<(int)list.size(); i++) {
		WfsWeakClassifier *clsf = new WfsWeakClassifier(list[i], ntype, nC);
		new_classifiers.push_back(clsf);
	}

	if (K_MULTI_SCALED_SEARCH && n > K_MULTI_SCALED_SEARCH_MIN_FEATURES) {

		// multi-scaled search........ is this really better?
		for(int step=0; step<nStepsMScale; step++) {
			int n = list.size();
			vector<double> discrm;

			int nP = nPosMScale[step];
			int nN = nNegMScale[step];

			int *posFeatVals = new int [nP * n];
			int *negFeatVals = new int [nN * n];

			int *val_min = new int [n];
			int *val_max = new int [n];

			//cout << "MULTI_SEARCH : step " << step << "\n";
			//cout << "               n " << n << "\n";
			//cout << "               nSamples " << nP + nN << "\n";

			// test
			if (n < 10) {
				cout << "MULTI_SEARCH n < 10 " << n << "\n";
			}


			// feature evaluation
			for(int i=0; i<n; i++) {
				EvaluateFeatureForMultipleSamples(&posFeatVals[i * nP], list[i], posVecs, nP);
				EvaluateFeatureForMultipleSamples(&negFeatVals[i * nN], list[i], negVecs, nN);

				int minp, maxp;
				int minn, maxn;

				getMinMax(minp, maxp, nP, &posFeatVals[i * nP]);
				getMinMax(minn, maxn, nN, &negFeatVals[i * nN]);

				val_min[i] = min(minp, minn);
				val_max[i] = max(maxp, maxn);
			}


			for(int i=0; i<n; i++) {
				discrm.push_back(1.0 - new_classifiers[i]->countSamples(nP, &posFeatVals[i * nP], posVecs, 
					nN, &negFeatVals[i * nN], negVecs, val_min[i], val_max[i]));
			}

			if (step == nStepsMScale - 1) {
				// last step : select the best
				int max_id = 0;
				for(int i=1; i<n; i++) {
					if (discrm[max_id] < discrm[i]) 
						max_id = i;
				}
				// add
				candFeatList.push_back( list[max_id] );

			//	checkFeature(list[max_id]);

//				cout << "               final disc " << discrm[max_id] << "\n";

				age.push_back(0);
				isOpen.push_back(true);
				discriminability.push_back(discrm[max_id]);
				fitness.push_back( ComputeFitness(discrm[max_id], list[max_id].size, 0) );

				weakClassifiers.push_back(new_classifiers[max_id]);

				for(int i=0; i<n; i++) {
					if (i != max_id) {
						delete new_classifiers[i];
					}
				}
			} else {
				// cut down by half
				int new_n = (n+1) / 2;

				// need to be improved
				for(int i=0; i<new_n; i++) {
					for(int j=i+1; j<n; j++) {
						if (discrm[i] < discrm[j]) {
							swap(discrm[i], discrm[j]);
							swap(new_classifiers[i], new_classifiers[j]);
							swap(list[i], list[j]);
						}
					}
				}

				//checkFeature(list[0]);

				//cout << "               mid max disc " << discrm[0] << "\n";
				//cout << "               mid min-surv disc " << discrm[new_n-1] << "\n";
				//cout << "               mid min disc " << discrm[n-1] << "\n";

				for(int i=new_n; i<n; i++) {
					delete new_classifiers[i];
				}

				new_classifiers.resize(new_n);
				list.resize(new_n);
			}


			delete [] posFeatVals;
			delete [] negFeatVals;
			delete [] val_min;
			delete [] val_max;
		}
	} else {

		// BF search
		vector<double> discrm;

		// feature evaluation
		int *posFeatVals = new int [nPos];
		int *negFeatVals = new int [nNeg];

		for(int i=0; i<n; i++) {

			EvaluateFeatureForMultipleSamples(posFeatVals, list[i], posVecs, nPos);
			EvaluateFeatureForMultipleSamples(negFeatVals, list[i], negVecs, nNeg);

			int minp, maxp;
			int minn, maxn;

			getMinMax(minp, maxp, nPos, posFeatVals);
			getMinMax(minn, maxn, nNeg, negFeatVals);

			int val_min = min(minp, minn);
			int val_max = max(maxp, maxn);

			discrm.push_back(1.0 - new_classifiers[i]->countSamples(nPos, posFeatVals, posVecs, 
				nNeg, negFeatVals, negVecs, val_min, val_max));
		}

		delete [] posFeatVals;
		delete [] negFeatVals;

		int max_id = 0;
		for(int i=1; i<n; i++) {
			if (discrm[max_id] < discrm[i]) 
				max_id = i;
		}

//		cout << "               final disc " << discrm[max_id] << "\n";

		// add
		candFeatList.push_back( list[max_id] );

	//	checkFeature(list[max_id]);

		age.push_back(0);
		isOpen.push_back(true);
		discriminability.push_back(discrm[max_id]);
		fitness.push_back( ComputeFitness(discrm[max_id], list[max_id].size, 0) );

		weakClassifiers.push_back(new_classifiers[max_id]);

		for(int i=0; i<n; i++) {
			if (i != max_id) {
				delete new_classifiers[i];
			}
		}
	}

	return 1;
}


int SparseFeatureLearner::ExpandFeature(int id)
{
	int n=0;

	// REMOVE
	if (candFeatList[id].size > 1) {
		std::vector<SparseFeature> remove_list;
		SparseFeature org = candFeatList[id];
		for(int i=0; i<candFeatList[id].size; i++) {
			SparseFeature newF = org;
			newF.id.erase(newF.id.begin() + i);
			newF.sign.erase(newF.sign.begin() + i);
			newF.size--;
			remove_list.push_back(newF);
		}

		n += AddBestExpandedFeature(remove_list);
	}


	// ADD
	if (candFeatList[id].size < K_MAX_GRANULES) {
		std::vector<SparseFeature> add_list;
		SparseFeature org = candFeatList[id];
		for(int i=0; i<K_NGRANS; i++) {
			bool found = false;
			for (int j=0; j<org.size; j++) {
				if (i == org.id[j]) {
					found = true;
					break;
				}
			}

			if (!found) {
				SparseFeature newF = org;
				newF.id.push_back(i);
				newF.sign.push_back(true);
				newF.size++;
				add_list.push_back(newF);

//				checkFeature(newF);

				newF.sign[newF.size - 1] = false;
				add_list.push_back(newF);

//				checkFeature(newF);
			}
		}

		n += AddBestExpandedFeature(add_list);
	}


	// REFINE
	{
		std::vector<SparseFeature> refine_list;
		SparseFeature org = candFeatList[id];
		for(int i=0; i<candFeatList[id].size; i++) {
			int id = org.id[i];
			bool sign = org.sign[i];

			SparseFeature newF = org;
			newF.id.erase(newF.id.begin() + i);
			newF.sign.erase(newF.sign.begin() + i);

			newF.sign.push_back(sign);
			newF.id.push_back(0);

			vector<int> neighbors;
			int r = GetNeighborGranules(neighbors, id);
			for(int j=0; j<r; j++) {
				bool found = false;
				for (int k=0; k<newF.size-1; k++) {
					if (neighbors[j] == newF.id[k]) {
						found = true;
						break;
					}
				}

				if (!found) {
					newF.id[newF.size - 1] = neighbors[j];
					refine_list.push_back(newF);
				}
			}
		}

		n += AddBestExpandedFeature(refine_list);
	}

	return n;
}

// TODO : decide parameters... like min number of features in feature pool when the best one is picked-up 
// or min round of expansion
// or fitness parameters.... like age penalty

void SparseFeatureLearner::SelectBestFeature(WfsWeakClassifier& weak_c)
{
	cout << "SelectBestFeature\n";

	// weight check
	//{
	//	double pt = 0.0;
	//	double nt = 0.0;
	//	for(int i=0; i<nPos; i++) {
	//		pt += posVecs[i].weight;
	//	}
	//	for(int i=0; i<nNeg; i++) {
	//		nt += negVecs[i].weight;
	//	}
	//	cout << "total weight : " << pt+nt << " " << pt << " " << nt << "\n";
	//}



	//////////////////////////////////////////////////////
	//  copy haar seed
	//
	// NOTE : if we want to select initial features considering fitness (when min distance constraint (between initial features) is applied),
	//        we need to call InitialSeedsSelection() again.
	//////////////////////////////////////////////////////

	if (0) {
		InitialSeedsSelection();
	}

	InitializeArraysForNewWeakRound();

	CopyHaarFeatures();

	visitedFeatList.clear();

	// create some combination with best basic features...
	if (0)
	{
		const int n_best = 5;
		double best_disc[n_best];
		int best_id[n_best];
		for (int i=0; i<n_best; i++) {
			best_id[i] = -1;
			best_disc[i] = -1.0;
		}

		for(int i=0; i<candFeatList.size(); i++) {
			int rank = n_best;
			while(rank > 0 && best_disc[rank-1] < discriminability[i]) rank--;

			if (rank < n_best) {
				for(int j=n_best-1; j>rank; j--) {
					best_id[j] = best_id[j-1];
					best_disc[j] = best_disc[j-1];
				}
				best_id[rank] = i;
				best_disc[rank] = discriminability[i];
			}
		}

		cout << "best init feature\n";
		for(int i=0; i<n_best; i++) {
			SparseFeature f = candFeatList[best_id[i]];

			for(int j=0; j<f.size; j++) {
				cout << f.id[j] << " " << f.sign[j] << "    " ;
			}
			cout << best_disc[i] << "\n";
		}

		cout << "new feature\n";
		for(int i=0; i<n_best; i++) {
			SparseFeature f = candFeatList[best_id[i]];
			for(int j=1; j<n_best; j++) {
				SparseFeature f2 = candFeatList[best_id[j]];

				if (f.size + f2.size <= K_MAX_GRANULES) {
					SparseFeature new_f;
					SparseFeature new_f2;

					new_f = f;
					new_f2 = f;
					for(int k=0; k<f2.size; k++) {
						bool found = false;
						for(int l=0; l<f.size; l++) {
							if (f.id[l] == f2.id[k]) {
								found = true; break;
							}
						}

						if (!found) {
							new_f.id.push_back(f2.id[k]);
							new_f.sign.push_back(f2.sign[k]);
							new_f2.id.push_back(f2.id[k]);
							new_f2.sign.push_back(!f2.sign[k]);
						}
					}
					new_f.size = new_f.id.size();
					new_f2.size = new_f2.id.size();

					sortSparseFeature(new_f);
					sortSparseFeature(new_f2);

					vector<SparseFeature> sfv;
					sfv.push_back(new_f);
					sfv.push_back(new_f2);

					AddBestExpandedFeature(sfv);

					for(int i=0; i<new_f.size; i++) {
						cout << new_f.id[i] << " " <<new_f.sign[i] << "    " ;
					}

					cout << discriminability[candFeatList.size()-1] << "\n";

					//WfsWeakClassifier *clsf = new WfsWeakClassifier(new_f, ntype, nC);

					//double discriminability = 1.0 - clsf->countSamples(nPos, initHaarFeatValsPos[i], posVecs, nNeg, initHaarFeatValsNeg[i], negVecs, initHaarMin[i], initHaarMax[i]);

					//isOpen.push_back(true);
					//age.push_back(0);

					//fitness.push_back(
					//discriminability.resize(n, 0);

					//// create LUT weak classifiers
					//for(int i=0; i<n; i++) {


					//	fitness[i] = ComputeFitness(discriminability[i], initHaarFeatList[i].size, 0);

//					cout << best_disc[i] << "\n";
				}
			}

		}
	}


	// iterations for expansion
	const int max_it = K_MAX_SPARSE_FEATURE_HEURISTIC_ITERATIONS;

	int it = max_it;
	int nOpenFeats = candFeatList.size();

	double all_best = 0.0;

	while(it-- > 0 && nOpenFeats > 0) 
	{
		int nFeats = candFeatList.size();

//		cout << " new it " << nFeats << "\n";

		double max_fitness = 0;
		int max_id = -1;

		for(int i=0; i<nFeats; i++) {
			if (isOpen[i])
			{
				if (max_id == -1 || max_fitness < fitness[i]) {
					max_fitness = fitness[i];
					max_id = i;
				}
				age[i]++;
//				if (age[i] < 50) {
					fitness[i] = ComputeFitness(discriminability[i], candFeatList[i].size, age[i]);
//				}
			}
		}

		nOpenFeats--;

		if (it % 1 == 0) {
			cout << max_it - it << " iters. current best : ";
			cout << max_id << " " << discriminability[max_id] << " " << fitness[max_id] << "\n";
		}
//		checkFeature(candFeatList[max_id]);

		if (max_id >= 0) {
			if (it % 10 == 0) {
				int max_id = 0;

				for(int i=0; i<nFeats; i++) 
				{
					if (discriminability[i] > discriminability[max_id]){
						max_id	= i;
					}
				}
				all_best = discriminability[max_id];
				cout << max_it - it << " iters. current best : " << all_best << "\n";
				cout << nFeats << " features in pool\n";
				checkFeature(candFeatList[max_id]);
			}
//			checkFeature(candFeatList[max_id]);
			isOpen[max_id] = false;
			nOpenFeats += ExpandFeature(max_id);
		}

	}


	// select the best one
	int max_id = -1;
	double max_fitness = 0.0;

	for(int i=0; i<(int)candFeatList.size(); i++) {
		fitness[i] = ComputeFitness(discriminability[i], candFeatList[i].size, 0);

		if (max_id == -1 || max_fitness < fitness[i]) {
			// constraint check (# negative granules = # pos)

			int sign_sum = 0;
			for(int j=0; j<candFeatList[i].size; j++)
			{
				if (candFeatList[i].sign[j]) {
					sign_sum ++;
				} else sign_sum--;
			}

			if (sign_sum == 0) {
				max_fitness = fitness[i];
				max_id = i;
			}
		}
	}

	static int wid = 0;
	checkFeature(candFeatList[max_id], true, wid);
	wid++;

	weak_c = *weakClassifiers[max_id];

	for(int i=0; i<(int)candFeatList.size(); i++) {
		delete weakClassifiers[i];
	}

	//cout << weakClassifiers[max_id]->getMinFeatValue() << "\n";
	//cout << weakClassifiers[max_id]->getMaxFeatValue() << "\n";
	//cout << weakClassifiers[max_id]->getConfTable()[10] << "\n";

	InitializeArraysForNewWeakRound();

}

