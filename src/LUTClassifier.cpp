//#include "../include/WfsWeakClassifier.h"
//
//
//WfsWeakClassifier::WfsWeakClassifier(SparseFeature& sF)//, int id)
//{ 
//	sparse_feature = sF;
//	
//	epsilon = K_EPSILON;
//	conf_table = new double[K_NUM_THRESHOLD];
//	w_plus = new double[K_NUM_THRESHOLD];
//	w_minus = new double[K_NUM_THRESHOLD];
//	initializeDouble(w_plus, K_NUM_THRESHOLD, 0);
//	initializeDouble(w_minus, K_NUM_THRESHOLD, 0);
//
//	norm_factor = 0.0;
//	min_featValue = 0;
//	max_featValue = 0;
//	min_conf = 0.0;
//	max_conf = 0.0;
//	
//}
//WfsWeakClassifier::WfsWeakClassifier(const WfsWeakClassifier& rhs)
//{
//	sparse_feature = rhs.sparse_feature;
//	
//	epsilon = rhs.epsilon;
//
//	conf_table = new double[K_NUM_THRESHOLD];
//	w_plus = new double[K_NUM_THRESHOLD];
//	w_minus = new double[K_NUM_THRESHOLD];
//
//	memcpy(conf_table, rhs.conf_table, sizeof(double)*K_NUM_THRESHOLD);
//	memcpy(w_plus, rhs.w_plus, sizeof(double)*K_NUM_THRESHOLD);
//	memcpy(w_minus, rhs.w_minus, sizeof(double)*K_NUM_THRESHOLD);
//
//	norm_factor = rhs.norm_factor;
//	min_featValue = rhs.min_featValue;
//	max_featValue = rhs.max_featValue;
//	min_conf = rhs.min_conf;
//	max_conf = rhs.max_conf;
//
//}
//
//WfsWeakClassifier::WfsWeakClassifier()
//{ 
//	sparse_feature.size = 0;
//	sparse_feature.id.clear();
//	sparse_feature.sign.clear();
//
//	epsilon = K_EPSILON;
//	conf_table = new double[K_NUM_THRESHOLD];
//	w_plus = new double[K_NUM_THRESHOLD];
//	w_minus = new double[K_NUM_THRESHOLD];
//
//	initializeDouble(w_plus, K_NUM_THRESHOLD, 0);
//	initializeDouble(w_minus, K_NUM_THRESHOLD, 0);
//
//	norm_factor = 0.0;
//	min_featValue = 0;
//	max_featValue = 0;
//	min_conf = 0.0;
//	max_conf = 0.0;
//
//	
//}
//
//void WfsWeakClassifier::initialize(int min_f, int max_f)
//{
//	min_featValue = min_f;
//	max_featValue = max_f;
//}
//
//void WfsWeakClassifier::initialize(double min_c, double max_c)
//{
//	min_conf = min_c;
//	max_conf = max_c;
//}
//
//WfsWeakClassifier::~WfsWeakClassifier()
//{
//	delete[] conf_table;
//	delete[] w_plus;
//	delete[] w_minus;
//
//	conf_table = 0;
//	w_plus = 0;
//	w_minus = 0;
//}
//
//double WfsWeakClassifier::countSamples(int nPos, int *pos_eval_feat_values, GranVec *pos_samples,  
//		int nNeg, int *neg_eval_feat_values, GranVec* neg_samples, 
//		int _min_featValue, int _max_featValue)
//{
//	int num_train_example = nPos + nNeg;
//	min_featValue = _min_featValue;
//	max_featValue = _max_featValue + 1;
//
//	int bin_num;
//	int diff = max_featValue - min_featValue;
//
//	if(diff == 0){
//		cout << "diff = 0 in BuildConfidenceTable" << endl;
//		diff = 1;
//		//return -1;
//	}
//
//	// JS : for multi-scaled search, acculmulating data might be necessary.
//	//      But we don't know min & max until reaching the last stage.
//	//      So we need to process entire set of samples, so initialize tables at each stage.
//	initializeDouble(w_plus, K_NUM_THRESHOLD, 0);
//	initializeDouble(w_minus, K_NUM_THRESHOLD, 0);
//
//	//count the positive sampels
//	for(int i=0; i<nPos; ++i){
//		bin_num = ((pos_eval_feat_values[i] - min_featValue) << K_NUM_SHIFT) / diff;
//
//		bin_num = min(K_NUM_THRESHOLD-1, max(0, bin_num));
//
////		if (bin_num < 0 || bin_num >= K_NUM_THRESHOLD) exit(0);
//
//		w_plus[bin_num] += pos_samples[i].weight;	
//	}
//
//	//count the negative sampels
//	for(int i=0; i<nNeg; ++i){
//		bin_num = ((neg_eval_feat_values[i] - min_featValue) << K_NUM_SHIFT) / diff;
//
//		bin_num = min(K_NUM_THRESHOLD-1, max(0, bin_num));
//
////		if (bin_num < 0 || bin_num >= K_NUM_THRESHOLD) exit(0);
//
//		w_minus[bin_num] += neg_samples[i].weight;	
//	}
//
//	norm_factor = 0;
//
//	//calculate the normalization factor
//	for(int i=0; i<K_NUM_THRESHOLD; ++i){
//		norm_factor += sqrt(w_plus[i]*w_minus[i]);
//	}
//	
//	norm_factor *= 2;
//
//	return norm_factor;
//}
//
//void WfsWeakClassifier::calculateConfTable()
//{
//	for(int i=0; i<K_NUM_THRESHOLD; ++i){
//		conf_table[i] = log( (w_plus[i] + epsilon) / (w_minus[i] + epsilon) ) /2.0;
//	}
//}
//
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
//	for(int i=0; i<K_NUM_THRESHOLD; ++i){
//		norm_factor += sqrt(w_plus[i]*w_minus[i]);
//	}
//	
//	norm_factor *= 2;
//
//
//	return norm_factor;
//}
//
//void WfsWeakClassifier::predictSamples(int* eval_feat_value, int size, double* conf)
//{
//	for(int i=0; i<size; ++i)
//		conf[i] = predict(eval_feat_value[i]);
//}
//
//double WfsWeakClassifier::predict(int eval_feat_value)
//{
//	int bin_num;
//	int diff = max_featValue - min_featValue;
//
//	bin_num = ((eval_feat_value - min_featValue) << K_NUM_SHIFT) / diff;
//	//bin_num = (int)floor( num_threshold*(eval_feat_value-min_featValue)/diff );
//
//	bin_num = min(K_NUM_THRESHOLD-1, max(0, bin_num));
//
//	return conf_table[bin_num];
//}
//
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
//			bin_num = K_NUM_THRESHOLD - 1;
//	} else{
//		bin_num = (int)floor( K_NUM_THRESHOLD*(conf-min_conf)/diff );
//
//		bin_num = min(K_NUM_THRESHOLD-1, max(0, bin_num));
//	}
//
//	//bin_num = (int)floor( K_NUM_THRESHOLD*(conf-min_conf)/diff );
//
//	
//	return conf_table[bin_num];
//}
