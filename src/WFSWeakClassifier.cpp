#include "WFSWeakClassifier.h"



WfsWeakClassifier::WfsWeakClassifier(const WfsWeakClassifier& rhs) 
: ntype(rhs.ntype), nc(rhs.nc), sparse_feature(rhs.sparse_feature), norm_factor(rhs.norm_factor), 
min_featValue(rhs.min_featValue), max_featValue(rhs.max_featValue), min_conf(rhs.min_conf), max_conf(rhs.max_conf)
{
	conf_table = NULL;
	w_plus = NULL;
	w_minus = NULL;
	vecConfTable = NULL;	

	if (ntype == BRANCHING) {
		vecConfTable = Make2dDoubleArray(K_LUTBINS, nc);
		Copy2dDoubleArray(vecConfTable, rhs.vecConfTable, K_LUTBINS, nc);
	} else {
		conf_table = new double[K_LUTBINS];
		w_plus = new double[K_LUTBINS];
		w_minus = new double[K_LUTBINS];

		memcpy(conf_table, rhs.conf_table, sizeof(double)*K_LUTBINS);
		memcpy(w_plus, rhs.w_plus, sizeof(double)*K_LUTBINS);
		memcpy(w_minus, rhs.w_minus, sizeof(double)*K_LUTBINS);
	}	
}

WfsWeakClassifier::WfsWeakClassifier(EWfsNodeType atype, int nChilds)
{
	ntype = atype;
	nc = nChilds;

	conf_table = NULL;
	w_plus = NULL;
	w_minus = NULL;
	vecConfTable = NULL;

	sparse_feature.size = 0;
	sparse_feature.id.clear();
	sparse_feature.sign.clear();

	if (ntype == BRANCHING) {
		vecConfTable = Make2dDoubleArray(K_LUTBINS, nc);
	} else {
		conf_table = new double[K_LUTBINS];
		w_plus = new double[K_LUTBINS];
		w_minus = new double[K_LUTBINS];

		memset(conf_table, 0, sizeof(double) * K_LUTBINS);
		memset(w_plus, 0, sizeof(double) * K_LUTBINS);
		memset(w_minus, 0, sizeof(double) * K_LUTBINS);
	}

	norm_factor = 0.0;
	min_featValue = 0;
	max_featValue = 0;
	min_conf = 0.0;
	max_conf = 0.0;	
}

WfsWeakClassifier::WfsWeakClassifier(SparseFeature& sF, EWfsNodeType atype, int nChilds)
{
	ntype = atype;
	nc = nChilds;

	conf_table = NULL;
	w_plus = NULL;
	w_minus = NULL;
	vecConfTable = NULL;

	sparse_feature = sF;

	if (ntype == BRANCHING) {
		vecConfTable = Make2dDoubleArray(K_LUTBINS, nc);
	} else {
		conf_table = new double[K_LUTBINS];
		w_plus = new double[K_LUTBINS];
		w_minus = new double[K_LUTBINS];

		memset(conf_table, 0, sizeof(double) * K_LUTBINS);
		memset(w_plus, 0, sizeof(double) * K_LUTBINS);
		memset(w_minus, 0, sizeof(double) * K_LUTBINS);
	}

	norm_factor = 0.0;
	min_featValue = 0;
	max_featValue = 0;
	min_conf = 0.0;
	max_conf = 0.0;	
}

WfsWeakClassifier& WfsWeakClassifier::operator=(const WfsWeakClassifier &rhs) {
	if (this != &rhs) {
		//ntype = atype;
		//nc = nChilds;

		//conf_table = NULL;
		//w_plus = NULL;
		//w_minus = NULL;
		//vecConfTable = NULL;

		sparse_feature = rhs.sparse_feature;

		if (ntype == BRANCHING) {
			Delete2dDoubleArray(&vecConfTable, K_LUTBINS);

			vecConfTable = Make2dDoubleArray(K_LUTBINS, nc);
			Copy2dDoubleArray(vecConfTable, rhs.vecConfTable, K_LUTBINS, nc);
		} else {
			delete [] conf_table;
			delete [] w_plus;
			delete [] w_minus;

			conf_table = new double[K_LUTBINS];
			w_plus = new double[K_LUTBINS];
			w_minus = new double[K_LUTBINS];

			memcpy(conf_table, rhs.conf_table, sizeof(double)*K_LUTBINS);
			memcpy(w_plus, rhs.w_plus, sizeof(double)*K_LUTBINS);
			memcpy(w_minus, rhs.w_minus, sizeof(double)*K_LUTBINS);
		}	

		norm_factor = rhs.norm_factor;
		min_featValue = rhs.min_featValue;
		max_featValue = rhs.max_featValue;
		min_conf = rhs.min_conf;
		max_conf = rhs.max_conf;
	}

	return *this;
}


WfsWeakClassifier::~WfsWeakClassifier()
{
	if (ntype == BRANCHING) {
		Delete2dDoubleArray(&vecConfTable, K_LUTBINS);
	} else {
		delete [] conf_table;
		delete [] w_plus;
		delete [] w_minus;
	}

	//conf_table = 0;
	//w_plus = 0;
	//w_minus = 0;
}

void WfsWeakClassifier::SetBranchingClassifier(double **vec_confs, int minf, int maxf)
{
	if (ntype == BRANCHING) 
	{
		min_featValue = minf;
		max_featValue = maxf;
		Copy2dDoubleArray(vecConfTable, vec_confs, K_LUTBINS, nc);
	}
}

void WfsWeakClassifier::SetBinaryClassifier(double *confs, int minf, int maxf)
{
	if (ntype == BINARY) 
	{
		min_featValue = minf;
		max_featValue = maxf;
		memcpy(conf_table, confs, sizeof(double) * K_LUTBINS);
	}
}

//void OptimizeVectorConfidence

// return norm_factor
double WfsWeakClassifier::countSamples(int nPos, int *pos_eval_feat_values, GranVec *pos_samples,  
		int nNeg, int *neg_eval_feat_values, GranVec* neg_samples, 
		int _min_featValue, int _max_featValue)
{
	int num_train_example = nPos + nNeg;
	min_featValue = _min_featValue;
	max_featValue = _max_featValue + 1;

	int bin_num;
	int diff = max_featValue - min_featValue;

	if(diff == 0){
		cout << "diff = 0 in BuildConfidenceTable" << endl;
		diff = 1;
		//return -1;
	}

	norm_factor = 0.0;

	if (ntype == BRANCHING) {
		int *pos_bins = new int [nPos];
		int *neg_bins = new int [nNeg];

		FindBinList(pos_bins, pos_eval_feat_values, nPos);
		FindBinList(neg_bins, neg_eval_feat_values, nNeg);

		///// Optimize constant output of vector typed piece-wise functions = confidence

		// after spending long time, it turned out that the optimal constant values are the same as before
		// and it is unnecessary to use newton's method (as paper did) 

		norm_factor = 0.0;

		double **WSum_pos = Make2dDoubleArray(K_LUTBINS, nc);
		double **WSum_neg = Make2dDoubleArray(K_LUTBINS, nc);

		for(int i=0; i<nPos; i++) {
			WSum_pos[pos_bins[i]][pos_samples[i].category] += pos_samples[i].weight;
		}
		for(int i=0; i<nNeg; i++) {
			WSum_neg[neg_bins[i]][neg_samples[i].category] += neg_samples[i].weight;
		}

		//double *grad_C = new double [nc];

		//// NEWTON's METHOD
		//// IF intrinsic vector is standard unit vector like (1,0,0)
		//// THEN hessian of loss function is diagonal matrix.
		//double *hess_C = new double [nc];

		//double child_norm[5];
		//memset(child_norm, 9, sizeof(double) * nc);

		for(int bin =0; bin < K_LUTBINS; bin ++) {
//			// initialize c
//			for(int i=0; i<nc; i++) {
//				vecConfTable[bin][i] = 0.0;	// use better choice?  (ratio pos-neg in each bin)
//			}
//
//			// iterations
//			int it = 0;
//			const int max_it = 500;
//			double step_size = 0.1;
//			double old_norm_factor = 0.0;
//			double single_norm_factor = 0.0;
//			while(it++ < max_it) {
//				for(int i=0; i<nc; i++) {
//					grad_C[i] = 0;
//					hess_C[i] = 0;
//				}
//
//				single_norm_factor = 0.0;
//				double exp_table[K_WFS_MAX_BRANCHES];
//				for(int i=0; i<nc; i++) {
//					exp_table[i] = exp( -vecConfTable[bin][i] );
//				}
//
//				// TODO : precompute exp( -vecConfTable[bin][ each category ] ) - done
//				// TODO : sum weight, later multiply once
//
//				//for(int i=0; i<nPos; i++) {
//				//	if (pos_bins[i] == bin) {
//				//		double loss = pos_samples[i].weight * exp_table[pos_samples[i].category];
//				//		grad_C[pos_samples[i].category] += -loss;
//				//		hess_C[pos_samples[i].category] += loss;
//				//		single_norm_factor += loss;
//				//	}
//				//}
//
//				//for(int i=0; i<nNeg; i++) {
//				//	if (neg_bins[i] == bin) {
//				//		double loss = neg_samples[i].weight / exp_table[neg_samples[i].category];
//				//		grad_C[neg_samples[i].category] += loss;
//				//		hess_C[neg_samples[i].category] += loss;
//				//		single_norm_factor += loss;
//				//	}
//				//}
//
//				for(int i=0; i<nc; i++) {
//					grad_C[i] = - WSum_pos[bin][i] * exp_table[i] + WSum_neg[bin][i] / exp_table[i];
//					hess_C[i] = + WSum_pos[bin][i] * exp_table[i] + WSum_neg[bin][i] / exp_table[i];
//					single_norm_factor += hess_C[i];
//				}
//
////				cout << it << " grad descent it, norm f : " << single_norm_factor << "\n";
//
//				for(int i=0; i<nc; i++) {
//					if (0) {
//						// gradient descent
//						vecConfTable[bin][i] -= step_size * grad_C[i];
//					} else {
//						// newton
//						if (hess_C[i] > 0)
//							vecConfTable[bin][i] -= grad_C[i] / hess_C[i];
//					}
//				}
//
//				if (it > 1 && (old_norm_factor - single_norm_factor <= old_norm_factor * 0.001 || single_norm_factor < 1e-8)) break;
//				old_norm_factor = single_norm_factor;
//			}

//			cout << single_norm_factor << "\n";
			double single_norm_factor = 0.0;
			for(int i=0; i<nc; i++) 
			{
				vecConfTable[bin][i] = log( (WSum_pos[bin][i] + K_EPSILON) / (WSum_neg[bin][i] + K_EPSILON) ) /2.0;

				double nf = sqrt(WSum_pos[bin][i] * WSum_neg[bin][i]);

				single_norm_factor += nf;
				//child_norm[i] += nf;
			}
			norm_factor += single_norm_factor;
		}
		norm_factor *= 2.0;


//		cout << norm_factor << "\n";

		delete [] pos_bins;
		delete [] neg_bins;
		//delete [] grad_C;
		//delete [] hess_C;

		Delete2dDoubleArray(&WSum_pos, K_LUTBINS);
		Delete2dDoubleArray(&WSum_neg, K_LUTBINS);


	} else {
		// JS : for multi-scaled search, acculmulating data might be necessary.
		//      But we don't know min & max until reaching the last stage.
		//      So we need to process entire set of samples, so initialize tables at each stage.
		memset(w_plus, 0, sizeof(double) * K_LUTBINS);
		memset(w_minus, 0, sizeof(double) * K_LUTBINS);

		//count the positive sampels
		for(int i=0; i<nPos; ++i){
			bin_num = ((pos_eval_feat_values[i] - min_featValue) << K_NUM_SHIFT) / diff;
			bin_num = min(K_LUTBINS-1, max(0, bin_num));

			w_plus[bin_num] += pos_samples[i].weight;	
		}

		//count the negative sampels
		for(int i=0; i<nNeg; ++i){
			bin_num = ((neg_eval_feat_values[i] - min_featValue) << K_NUM_SHIFT) / diff;
			bin_num = min(K_LUTBINS-1, max(0, bin_num));

			w_minus[bin_num] += neg_samples[i].weight;	
		}

		norm_factor = 0.0;

		//calculate the normalization factor
		for(int i=0; i<K_LUTBINS; ++i){
			norm_factor += sqrt(w_plus[i]*w_minus[i]);
		}
		
		norm_factor *= 2;
	}

	return norm_factor;
}

void WfsWeakClassifier::calculateConfTable()
{
	if (ntype == BRANCHING) {
	} else {
		for(int i=0; i<K_LUTBINS; ++i){
			conf_table[i] = log( (w_plus[i] + K_EPSILON) / (w_minus[i] + K_EPSILON) ) /2.0;
		}
	}
}

//double WfsWeakClassifier::countNestSamples(int nPos, int *pos_bin_ind, GranVec *pos_samples,  
//		int nNeg, int *neg_bin_ind, GranVec* neg_samples, 
//		double _min_conf, double _max_conf)
//{
//	int num_train_example = nPos + nNeg;
//	min_conf = _min_conf;
//	max_conf = _max_conf;
//
//	//count the positive sampels
//	for(int i=0; i<nPos; ++i){
//		w_plus[pos_bin_ind[i]] += pos_samples[i].weight;	
//	}
//
//	//count the negative sampels
//	for(int i=0; i<nNeg; ++i){
//		w_minus[neg_bin_ind[i]] += neg_samples[i].weight;	
//	}
//
//	norm_factor = 0;
//	//calculate the normalization factor
//	for(int i=0; i<K_LUTBINS; ++i){
//		norm_factor += sqrt(w_plus[i]*w_minus[i]);
//	}
//	
//	norm_factor *= 2;
//
//
//	return norm_factor;
//}

void WfsWeakClassifier::ComputeWeakConfidenceList(double* confs, int* feat_values, int size)
{
	int *bins = new int [size];

	FindBinList(bins, feat_values, size);

	for(int i=0; i<size; ++i) {
		confs[i] = conf_table[bins[i]];
	}

	delete [] bins;
}


void WfsWeakClassifier::ComputeWeakConfidenceVectorList(double** confs, int* feat_values, int size)
{
	int *bins = new int [size];

	FindBinList(bins, feat_values, size);

	for(int i=0; i<size; ++i) {
		memcpy(confs[i], vecConfTable[bins[i]], sizeof(double) * nc);
	}

	delete [] bins;
}

void WfsWeakClassifier::AddWeakConfidenceVectorListGivenCategory(double* confs, int* feat_values, int size, int category)
{
	int *bins = new int [size];

	FindBinList(bins, feat_values, size);

	for(int i=0; i<size; ++i) {
		confs[i] += vecConfTable[bins[i]][category];
	}

	delete [] bins;
}

void WfsWeakClassifier::AddWeakConfidenceVectorList(double** confs, int* feat_values, int size)
{
	int *bins = new int [size];

	FindBinList(bins, feat_values, size);

	for(int i=0; i<size; ++i) {
		for(int j=0; j<nc; j++) {
			confs[i][j] += vecConfTable[bins[i]][j];
		}
	}

	delete [] bins;
}

void WfsWeakClassifier::AddWeakConfidenceList(double* confs, int* feat_values, int size)
{
	int *bins = new int [size];

	FindBinList(bins, feat_values, size);

	for(int i=0; i<size; ++i) {
		confs[i] += conf_table[bins[i]];
	}

	delete [] bins;
}

void WfsWeakClassifier::FindBinList(int *bins, int *feat_values, int size)
{
	int bin_num;
	int diff = max_featValue - min_featValue;

	for(int i=0; i<size; i++) {
		bin_num = ((feat_values[i] - min_featValue) << K_NUM_SHIFT) / diff;
		bin_num = min(K_LUTBINS-1, max(0, bin_num));
		bins[i] = bin_num;
	}
}

int WfsWeakClassifier::FindBin(int feat_value) {
	int bin_num;
	int diff = max_featValue - min_featValue;

	bin_num = ((feat_value - min_featValue) << K_NUM_SHIFT) / diff;
	bin_num = min(K_LUTBINS-1, max(0, bin_num));

	return bin_num;
}

//double WfsWeakClassifier::predict(int eval_feat_value)
//{
//	int bin_num;
//	int diff = max_featValue - min_featValue;
//
//	bin_num = ((eval_feat_value - min_featValue) << K_NUM_SHIFT) / diff;
//	//bin_num = (int)floor( num_threshold*(eval_feat_value-min_featValue)/diff );
//
//	bin_num = min(K_LUTBINS-1, max(0, bin_num));
//
//	return conf_table[bin_num];
//}

//void WfsWeakClassifier::predictSamples(double* conf_values, int size, double* conf)
//{
//	for(int i=0; i<size; ++i)
//		conf[i] = predict(conf_values[i]);
//}

//double WfsWeakClassifier::predict(double conf)
//{
//	int bin_num;
//
//	double diff = max_conf - min_conf;
//
//	//LC 1124
//	if(diff == 0){
//		if(conf == min_conf)
//			bin_num = 0;
//		else
//			bin_num = K_LUTBINS - 1;
//	} else{
//		bin_num = (int)floor( K_LUTBINS*(conf-min_conf)/diff );
//
//		bin_num = min(K_LUTBINS-1, max(0, bin_num));
//	}
//
//	//bin_num = (int)floor( K_LUTBINS*(conf-min_conf)/diff );
//
//	
//	return conf_table[bin_num];
//}
