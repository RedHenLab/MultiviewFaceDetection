//#include "../include/LUTAda.h"
//
//LUTAda::LUTAda(int _stage){
//	
//	stage = _stage;
//	//picked_classifiers.clear();
//	num_weak = 0;
//}
//
//LUTAda::LUTAda(const LUTAda& rhs)
//{
//	stage = rhs.stage;
//	picked_classifiers = rhs.picked_classifiers;
//	num_weak = rhs.num_weak;
//}
//
//LUTAda::~LUTAda()
//{
//}
//
//
//void LUTAda::updateNestingWeight(LUTClassifier& nest_detector,
//								 ImgSet* p_pos_set, ImgSet* p_neg_set)
//{
//	int num_pos_img = p_pos_set->num_img;
//	int num_neg_img = p_neg_set->num_img;
//
//	double conf;
//
//	for(int i=0; i<num_pos_img; ++i){
//		conf = p_pos_set->gran_vec[i].conf;
//		p_pos_set->gran_vec[i].weight *= exp(-nest_detector.predict(conf));
//	}
//	for(int i=0; i<num_neg_img; ++i){
//		conf = p_neg_set->gran_vec[i].conf;
//		p_neg_set->gran_vec[i].weight *= exp(nest_detector.predict(conf));
//	}
//}
//
//void LUTAda::updateWeight(ImgSet* p_pos_set, double* pred_pos, 
//						  ImgSet* p_neg_set, double* pred_neg)
//{
//	int nPos = p_pos_set->num_img;
//	int nNeg = p_neg_set->num_img;
//
//	for(int i=0; i<nPos; ++i){
//		p_pos_set->gran_vec[i].weight *= exp(-pred_pos[i]);
//	}
//	for(int i=0; i<nNeg; ++i){
//		p_neg_set->gran_vec[i].weight *= exp(pred_neg[i]);
//	}
//
//}
//
//void LUTAda::normalizeWeight(ImgSet* p_pos_set, ImgSet* p_neg_set)
//{
//	double total = 0.0;
//	int num_pos_img = p_pos_set->num_img;
//	int num_neg_img = p_neg_set->num_img;
//
//	for(int i=0; i<num_pos_img; ++i)	
//		total += p_pos_set->gran_vec[i].weight;
//
//	for(int i=0; i<num_neg_img; ++i)	
//		total += p_neg_set->gran_vec[i].weight;
//
//	for(int i=0; i<num_pos_img; ++i)	
//		p_pos_set->gran_vec[i].weight /= total;
//
//	for(int i=0; i<num_neg_img; ++i)	
//		p_neg_set->gran_vec[i].weight /= total;
//
//}
//
//int LUTAda::transformConf2BinIndex(double *pos_conf, int num_pos_img, double *neg_conf, int num_neg_img,
//		int *pos_bin_ind, int *neg_bin_ind, double min_conf, double max_conf)
//{
//
//	double diff = max_conf - min_conf;
//	
//	//LC 1124
//	int bin_num;
//
//	if(diff == 0){
//		cout << "diff = 0 in transformConf2BinIndex" << endl;
//
//		//set bin_num=0
//		for(int i=0; i<num_pos_img; ++i)
//			pos_bin_ind[i] = 0;
//		for(int i=0; i<num_neg_img; ++i)
//			neg_bin_ind[i] = 0;
//	}
//	else{ 
//		for(int i=0; i<num_pos_img; ++i){
//			bin_num = (int)floor( K_NUM_THRESHOLD*(pos_conf[i]-min_conf)/diff );
//
//			//implicitly assume pos_conf[i] >= min_conf
//			if (bin_num >= K_NUM_THRESHOLD)
//				bin_num = K_NUM_THRESHOLD - 1;
//			pos_bin_ind[i] = bin_num;
//		}
//
//		for(int i=0; i<num_neg_img; ++i){
//			bin_num = (int)floor( K_NUM_THRESHOLD*(neg_conf[i]-min_conf)/diff );
//
//			if (bin_num >= K_NUM_THRESHOLD)
//				bin_num = K_NUM_THRESHOLD - 1;
//			neg_bin_ind[i] = bin_num;
//		}
//	}
//
//	return 0;
//}
//
//void LUTAda::trainNestingDetector(LUTClassifier& nest_detector,
//								  ImgSet* p_pos_set, ImgSet* p_neg_set,
//								  double* cum_pred_pos_conf, double *cum_pred_neg_conf)
//{
//	//build confidence table
//	int num_pos_img = p_pos_set->num_img;
//	int num_neg_img = p_neg_set->num_img;
//
//	double pos_minC, pos_maxC, neg_minC, neg_maxC;
//	double* pos_conf = new double[num_pos_img];
//	double* neg_conf = new double[num_neg_img];
//	int *pos_bin_ind = new int[num_pos_img];
//	int *neg_bin_ind = new int[num_neg_img];
//
//	readConfidenceFromImgSet(p_pos_set, pos_conf, &pos_minC, &pos_maxC);
//	readConfidenceFromImgSet(p_neg_set, neg_conf, &neg_minC, &neg_maxC);
//
//	if(pos_minC > neg_minC)
//		pos_minC = neg_minC;
//	if(pos_maxC < neg_maxC)
//		pos_maxC = neg_maxC;
//
//	transformConf2BinIndex(pos_conf, num_pos_img, neg_conf, num_neg_img,
//		pos_bin_ind, neg_bin_ind, pos_minC, pos_maxC);
//
//	nest_detector.countNestSamples(num_pos_img, pos_bin_ind, &p_pos_set->gran_vec[0],
//		num_neg_img, neg_bin_ind, &p_neg_set->gran_vec[0],
//		pos_minC, pos_maxC);
//
//	nest_detector.calculateConfTable();
//
//	//read confidence
//	double feat;
//
//	for(int i=0; i<num_pos_img; ++i){
//		feat = p_pos_set->gran_vec[i].conf;
//		cum_pred_pos_conf[i] = nest_detector.predict(feat);
//	}
//	for(int i=0; i<num_neg_img; ++i){
//		feat = p_neg_set->gran_vec[i].conf;
//		cum_pred_neg_conf[i] = nest_detector.predict(feat);
//	}
//
//	delete[] pos_conf;
//	delete[] neg_conf;
//	delete[] pos_bin_ind;
//	delete[] neg_bin_ind;
//
//	pos_conf = 0;
//	neg_conf = 0;
//	pos_bin_ind = 0;
//	neg_bin_ind = 0;
//}
//
//void LUTAda::checkPerformance(double *pred_pos_conf, int num_pos_img,
//							  double *pred_neg_conf, int num_neg_img,
//							  double threshold, 
//							  double& current_false_alarm, double& current_detection_rate)
//{
//	int num_true_pos = 0;
//	int num_false_pos = 0;
//
//	for(int i=0; i<num_pos_img; ++i){
//		if( (pred_pos_conf[i]-threshold) >= 0)
//			++num_true_pos;
//	}
//
//	for(int i=0; i<num_neg_img; ++i){
//		if( (pred_neg_conf[i]-threshold) >= 0)
//			++num_false_pos;
//	}
//	
//	current_false_alarm = (double)num_false_pos / num_neg_img;
//	current_detection_rate = (double) num_true_pos / num_pos_img;
//
//}
//
//void LUTAda::saveLUTAdaData(LUTAdaData* data)
//{
//	int num_weak_saved = 0;
//
//	data->stage = stage;
//	int total = picked_classifiers.size();
//
//	data->num_total = total;
//
//	//save nesting detector first
//	if(stage != 0){
//		LUTClassifierData weakData;
//		memcpy(weakData.conf, picked_classifiers[0].getConfTable(), sizeof(double)*K_NUM_THRESHOLD);
//		picked_classifiers[0].getMinMaxConf(weakData.dmin_featValue, weakData.dmax_featValue);
//		++num_weak_saved;
//
//		data->lut_classifiers.push_back(weakData);
//	}
//
//	for(int i=num_weak_saved; i<total; ++i){
//		LUTClassifierData weakData;
//
//		//weakData.num_lut = i;
//
//		memcpy(weakData.conf, picked_classifiers[i].getConfTable(), sizeof(double)*K_NUM_THRESHOLD);
//		picked_classifiers[i].getMinMaxFeatValue(weakData.imin_featValue, weakData.imax_featValue);
//		weakData.features = picked_classifiers[i].getSparseFeature();
//		
//		data->lut_classifiers.push_back(weakData);
//	}
//
//}
//
//void LUTAda::predict(GranVec& sample, bool& pred_sign, double& pred_conf, double threshold)
//{
//
//	int value;
//	double conf = 0.0;
//
//	for(int i=0; i<num_weak; ++i){
//		if(stage != 0 && i==0) {
//			conf += picked_classifiers[0].predict(sample.conf);
//		} else{
//			SparseFeature sp = picked_classifiers[i].getSparseFeature();
//			value = EvaluateFeature( sp, sample);
//			conf += picked_classifiers[i].predict(value);
//		}
//	}
//
//	pred_conf = conf - threshold;
//
//	if(pred_conf >= 0)
//		pred_sign = true;
//	else
//		pred_sign = false;
//}
//
//void LUTAda::checkPass(ImgSet *p_set, double threshold)
//{
//	int num_img = p_set->num_img;
//
//	//evaluated sparse feature value
//	int* esfv = new int[num_img];
//	double* pred_conf = new double[num_img];
//	double* cum_pred_conf = new double[num_img];
//	
//	initializeDouble(cum_pred_conf, num_img, 0.0); 
//	
//	double minC, maxC;	//dummy
//	double* conf_values = new double[num_img];
//
//	for(int i=0; i<num_weak; ++i){
//		if(stage != 0 && i==0) {
//			readConfidenceFromImgSet(p_set, conf_values, &minC, &maxC);
//			picked_classifiers[0].predictSamples(conf_values, num_img, pred_conf);
//		} else{
//			SparseFeature sp = picked_classifiers[i].getSparseFeature();
//			EvaluateFeatureForMultipleSamples(esfv, 
//				sp, &p_set->gran_vec[0], num_img);
//			picked_classifiers[i].predictSamples(esfv, num_img, pred_conf);		
//		}
//	
//		cumTwoArray(cum_pred_conf, pred_conf, num_img);
//	}
//	
//	markPass(p_set, cum_pred_conf, threshold);
//
//		
//
//	delete[] esfv;
//	delete[] pred_conf;
//	delete[] cum_pred_conf;
//	delete[] conf_values;
//
//	esfv = 0;
//	pred_conf = 0;
//	cum_pred_conf = 0;
//	conf_values = 0;
//}
//
//void LUTAda::markPass(ImgSet* p_set, double* cum_pred_conf, double threshold)
//{
//	int num_img = p_set->num_img;
//
//	for(int i=0; i<num_img; ++i){
//		if(cum_pred_conf[i] - threshold >= 0){
//			p_set->gran_vec[i].active = true;
//			p_set->gran_vec[i].conf = cum_pred_conf[i];
//		}
//		else
//			p_set->gran_vec[i].active = false;
//	}
//
//}
//
//int LUTAda::train(ImgSet *p_pos_set, ImgSet *p_neg_set, double max_false_alarm,
//				   double min_detection_rate, LUTAdaData* data, double threshold)
//{	
//	num_weak = 0;		//number of weak learners used
//	int num_pos_img = p_pos_set->num_img;
//	int num_neg_img = p_neg_set->num_img;
//	int num_train_example = num_pos_img + num_neg_img;
//
//	double* cum_pred_pos_conf = new double[num_pos_img];
//	double* cum_pred_neg_conf = new double[num_neg_img]; 
//
//	initializeDouble(cum_pred_pos_conf, num_pos_img, 0.0); 
//	initializeDouble(cum_pred_neg_conf, num_neg_img, 0.0); 
//
//	double* pred_pos_conf = new double[num_pos_img];
//	double* pred_neg_conf = new double[num_neg_img];
//
//
//	//train the nesting detector first
//	if(stage != 0) {
//		LUTClassifier nest_detector;
//		trainNestingDetector(nest_detector, p_pos_set, p_neg_set, cum_pred_pos_conf,
//			cum_pred_neg_conf);
//
//		updateNestingWeight(nest_detector, p_pos_set, p_neg_set);
//		normalizeWeight(p_pos_set, p_neg_set);
//		
//		picked_classifiers.push_back(nest_detector);
//		++num_weak;
//	}
//	
//	double current_false_alarm = 1.0;
//	double current_detection_rate = 0.0;
//
//	int* pos_values = new int[num_pos_img];
//	int* neg_values = new int[num_neg_img];
//
//	SparseFeatureLearner sfl;
//	sfl.SetTrainingSamples(num_pos_img, &p_pos_set->gran_vec[0], 
//		num_neg_img, &p_neg_set->gran_vec[0]);
//	
//	while(true){
//		
//		LUTClassifier weak;
//
//		sfl.SelectBestFeature(weak);
//		weak.calculateConfTable();
//
//		picked_classifiers.push_back(weak);
//
//		++num_weak;
//	
//		EvaluateFeatureForMultipleSamples(pos_values, 
//			weak.getSparseFeature(), &p_pos_set->gran_vec[0], num_pos_img);
//
//		EvaluateFeatureForMultipleSamples(neg_values, 
//			weak.getSparseFeature(), &p_neg_set->gran_vec[0], num_neg_img);
//
//		weak.predictSamples(pos_values, num_pos_img, pred_pos_conf);
//		weak.predictSamples(neg_values, num_neg_img, pred_neg_conf);
//		
//		
////cout << "p_pos_set, weight:" << p_pos_set->gran_vec[0].weight << endl;
//		updateWeight(p_pos_set, pred_pos_conf, 
//			p_neg_set, pred_neg_conf);
////cout << "p_pos_set, weight:" << p_pos_set->gran_vec[0].weight << endl;
//
//
//		normalizeWeight(p_pos_set, p_neg_set);
//
//		cumTwoArray(cum_pred_pos_conf, pred_pos_conf, num_pos_img);
//		cumTwoArray(cum_pred_neg_conf, pred_neg_conf, num_neg_img);
//
//
//		checkPerformance(cum_pred_pos_conf, num_pos_img, 
//			cum_pred_neg_conf, num_neg_img, threshold,
//			current_false_alarm, current_detection_rate);
//
//		if(current_detection_rate >= min_detection_rate && current_false_alarm <= max_false_alarm)
//			break;
//		if(num_weak > MAX_NUM_WEAK_LEARNER) {
//			cout << "cannot satisfy the required max false alarm rate and min detection rate" << endl;
//			cout << "the number of weak learners exceed " << MAX_NUM_WEAK_LEARNER << endl;
//			cout << "current detection rate: " << current_detection_rate << " " <<
//				"current false alarm rate: " << current_false_alarm << endl;
//			return -1;
//		}
//
//		cout << "detection rate: " << current_detection_rate << endl;
//		cout << "false alarm rate: " << current_false_alarm << endl;
//	}
//
//	//save the result to LUTAdaData
//	data->lut_classifiers.clear();
//	saveLUTAdaData(data);
//
//	//mark active or not in the training set
//	markPass(p_pos_set, cum_pred_pos_conf, threshold);
//	markPass(p_neg_set, cum_pred_neg_conf, threshold);
//
//
//	delete[] cum_pred_pos_conf;
//	delete[] cum_pred_neg_conf;
//	delete[] pred_pos_conf;
//	delete[] pred_neg_conf;
//	delete[] pos_values;
//	delete[] neg_values;
//
//	cum_pred_pos_conf = 0;
//	cum_pred_neg_conf = 0;
//	pred_pos_conf = 0;
//	pred_neg_conf = 0;
//	pos_values = 0;
//	neg_values = 0;
//
//
//	return 0;
//}
//
//int LUTAda::setData(LUTAdaData *data)
//{
//	int num_weak_loaded = 0;
//
//	stage = data->stage;
//
//	num_weak = data->num_total;
//
//	//initialize and load nesting detector
//	if(stage != 0){
//		LUTClassifier weak;
//		LUTClassifierData weakData = data->lut_classifiers[0];
//
//		memcpy(weak.getConfTable(), weakData.conf, sizeof(double) * K_NUM_THRESHOLD);
//		weak.initialize(weakData.dmin_featValue, weakData.dmax_featValue);
//		picked_classifiers.push_back(weak);
//
//		++num_weak_loaded;
//	}
//
//	//load other LUT weak learners
//	for(int i=num_weak_loaded; i<num_weak; ++i){
//		LUTClassifier weak;
//		LUTClassifierData weakData = data->lut_classifiers[i];
//
//		memcpy(weak.getConfTable(), weakData.conf, sizeof(double) * K_NUM_THRESHOLD);
//
//		weak.initialize(weakData.imin_featValue, weakData.imax_featValue);
//		weak.setSparseFeature(data->lut_classifiers[i].features);
//		
//		picked_classifiers.push_back(weak);
//	}
//
//
//	return 0;
//}