#include "WFSNode.h"
#include "SparseFeatureLearner.h"


WfsStrongClassifier::WfsStrongClassifier()
{

}

WfsStrongClassifier::WfsStrongClassifier(const WfsStrongClassifier &copy)
: nodeId(copy.nodeId), nc(copy.nc), ntype(copy.ntype), parentId(copy.parentId), is_trained(copy.is_trained), finalPose(copy.finalPose), cascadeId(copy.cascadeId), cascadeStage(copy.cascadeStage), isFinalStage(copy.isFinalStage)
{
	weakClassifiers = copy.weakClassifiers;
}

WfsStrongClassifier::WfsStrongClassifier(int nId, int pId, EWfsNodeType atype, int nChilds)
{
	nodeId = nId;
	parentId = pId;
	ntype = atype;
	nc = nChilds;
	finalPose = 0;
	weakClassifiers.clear();
	cascadeId = -1;
	cascadeStage = 0;
	is_trained = false;
	isFinalStage = false;

	if (ntype==BRANCHING && WfsTreeStructure[nId][0] > 0) {
		finalPose = WfsTreeStructure[nId][0];
	}
}

int WfsStrongClassifier::TrainStrongClassifier(ImgSet *p_pos_set, ImgSet *p_neg_set, double max_false_alarm,
				   double min_detection_rate, /*LUTAdaData* data, */double threshold)
{	

//	num_weak = 0;		//number of weak learners used
	int ret = 0;

	cout << "TrainStrongClassifier " << nodeId << "\n";

	int num_pos_img = p_pos_set->Size();
	int num_neg_img = p_neg_set->Size();
	int num_train_example = num_pos_img + num_neg_img;

	int* pos_feat_values = new int[num_pos_img];
	int* neg_feat_values = new int[num_neg_img];

	weakClassifiers.clear();

	// init weights
	double pos_init_weight = 1.0 / (double) num_train_example;
	double neg_init_weight = 1.0 / (double) num_train_example;

	//double pos_init_weight = 0.5 / (double) num_pos_img;
	//double neg_init_weight = 0.5 / (double) num_neg_img;

	for(int i=0; i<num_pos_img; i++) {
		p_pos_set->GetGranVec(i)->weight = pos_init_weight;
	}
	for(int i=0; i<num_neg_img; i++) {
		p_neg_set->GetGranVec(i)->weight = neg_init_weight;
	}

	if (ntype == BRANCHING) {
		// f(x)
		double **pos_weak_confs;
		double **neg_weak_confs;
		// F(x)
		double **pos_str_confs;
		double **neg_str_confs;

		pos_weak_confs = Make2dDoubleArray(num_pos_img, nc);
		neg_weak_confs = Make2dDoubleArray(num_neg_img, nc);
		pos_str_confs = Make2dDoubleArray(num_pos_img, nc);
		neg_str_confs = Make2dDoubleArray(num_neg_img, nc);

		double current_false_alarm = 1.0;
		double current_detection_rate = 0.0;

		SparseFeatureLearner sfl;
		sfl.nC = nc;
		sfl.ntype = ntype;
//		sfl.SetTrainingSamples(num_pos_img, &p_pos_set->gran_vec[0], num_neg_img, &p_neg_set->gran_vec[0]);
		sfl.SetTrainingSamples(*p_pos_set, *p_neg_set);
		
		while(ret==0){
			
			cout << weakClassifiers.size() << " WfsWeakClassifier\n";
			WfsWeakClassifier weak(BRANCHING, nc);

			sfl.SelectBestFeature(weak);
			weak.calculateConfTable();

			weakClassifiers.push_back(weak);
		
			//EvaluateFeatureForMultipleSamples(pos_feat_values, weak.getSparseFeature(), &p_pos_set->gran_vec[0], num_pos_img);
			//EvaluateFeatureForMultipleSamples(neg_feat_values, weak.getSparseFeature(), &p_neg_set->gran_vec[0], num_neg_img);
			EvaluateFeatureForMultipleSamples(pos_feat_values, weak.getSparseFeature(), *p_pos_set);
			EvaluateFeatureForMultipleSamples(neg_feat_values, weak.getSparseFeature(), *p_neg_set);

			weak.ComputeWeakConfidenceVectorList(pos_weak_confs, pos_feat_values, num_pos_img);
			weak.ComputeWeakConfidenceVectorList(neg_weak_confs, neg_feat_values, num_neg_img);
			
			// w.r.t. intrinsic projection vector
			UpdateWeightForBranchingNode(p_pos_set, pos_weak_confs, p_neg_set, neg_weak_confs);

			normalizeWeight(p_pos_set, p_neg_set);

			sfl.UpdateSampleWeights(*p_pos_set, *p_neg_set);

			for(int i=0; i<num_pos_img; i++) {
				for(int j=0; j<nc; j++) {
					pos_str_confs[i][j] += pos_weak_confs[i][j];
				}
			}
			for(int i=0; i<num_neg_img; i++) {
				for(int j=0; j<nc; j++) {
					neg_str_confs[i][j] += neg_weak_confs[i][j];
				}
			}

			// count false positive & detected faces
			CheckPerformanceBranchingNode(current_false_alarm, current_detection_rate, p_pos_set, pos_str_confs, p_neg_set, neg_str_confs);

			cout << "detection rate: " << current_detection_rate << endl;
			cout << "false alarm rate: " << current_false_alarm << endl;

			if(current_detection_rate >= min_detection_rate && current_false_alarm <= max_false_alarm) //done
				break;
			if(weakClassifiers.size() > MAX_NUM_WEAK_LEARNER) {
				cout << "cannot satisfy the required max false alarm rate and min detection rate" << endl;
				cout << "the number of weak learners exceed " << MAX_NUM_WEAK_LEARNER << endl;
				cout << "current detection rate: " << current_detection_rate << " " <<
					"current false alarm rate: " << current_false_alarm << endl;
				ret = -1;		
				break;
			}

		}

		if (ret == 0) {
			//save the result to LUTAdaData => don't save now
			//data->lut_classifiers.clear();
			//saveLUTAdaData(data);

			//mark active or not in the training set => don't need
			//markPass(p_pos_set, cum_pred_pos_conf, threshold);
			//markPass(p_neg_set, cum_pred_neg_conf, threshold);
		}

		is_trained = true;

		Delete2dDoubleArray(&pos_weak_confs, num_pos_img);
		Delete2dDoubleArray(&neg_weak_confs, num_neg_img);
		Delete2dDoubleArray(&pos_str_confs, num_pos_img);
		Delete2dDoubleArray(&neg_str_confs, num_neg_img);

	} else {
		// f(x)
		double *pos_weak_confs = new double[num_pos_img];
		double *neg_weak_confs = new double[num_neg_img];
		// F(x)
		double *pos_str_confs = new double[num_pos_img];
		double *neg_str_confs = new double[num_neg_img];

		double current_false_alarm = 1.0;
		double current_detection_rate = 0.0;

		memset(pos_weak_confs, 0, num_pos_img * sizeof(double));
		memset(neg_weak_confs, 0, num_neg_img * sizeof(double));
		memset(pos_str_confs, 0, num_pos_img * sizeof(double));
		memset(neg_str_confs, 0, num_neg_img * sizeof(double));

		SparseFeatureLearner sfl;
		sfl.nC = nc;
		sfl.ntype = ntype;
//		sfl.SetTrainingSamples(num_pos_img, &p_pos_set->gran_vec[0], num_neg_img, &p_neg_set->gran_vec[0]);
		sfl.SetTrainingSamples(*p_pos_set, *p_neg_set);
		
		while(ret==0){
			
			cout << weakClassifiers.size() << " WfsWeakClassifier\n";
			WfsWeakClassifier weak(BINARY, 0);

			sfl.SelectBestFeature(weak);
			weak.calculateConfTable();

			weakClassifiers.push_back(weak);
		
			//EvaluateFeatureForMultipleSamples(pos_feat_values, weak.getSparseFeature(), &p_pos_set->gran_vec[0], num_pos_img);
			//EvaluateFeatureForMultipleSamples(neg_feat_values, weak.getSparseFeature(), &p_neg_set->gran_vec[0], num_neg_img);
			EvaluateFeatureForMultipleSamples(pos_feat_values, weak.getSparseFeature(), *p_pos_set);
			EvaluateFeatureForMultipleSamples(neg_feat_values, weak.getSparseFeature(), *p_neg_set);

			weak.ComputeWeakConfidenceList(pos_weak_confs, pos_feat_values, num_pos_img);
			weak.ComputeWeakConfidenceList(neg_weak_confs, neg_feat_values, num_neg_img);
			
			// w.r.t. intrinsic projection vector
			UpdateWeightForBinaryNode(p_pos_set, pos_weak_confs, p_neg_set, neg_weak_confs);

			normalizeWeight(p_pos_set, p_neg_set);

			sfl.UpdateSampleWeights(*p_pos_set, *p_neg_set);

			for(int i=0; i<num_pos_img; i++) {
				pos_str_confs[i] += pos_weak_confs[i];
			}

			for(int i=0; i<num_neg_img; i++) {
				neg_str_confs[i] += neg_weak_confs[i];
			}

			// count false positive & detected faces
			CheckPerformanceBinaryNode(current_false_alarm, current_detection_rate, p_pos_set, pos_str_confs, p_neg_set, neg_str_confs);

			cout << "detection rate: " << current_detection_rate << endl;
			cout << "false alarm rate: " << current_false_alarm << endl;

			if(current_detection_rate >= min_detection_rate && current_false_alarm <= max_false_alarm) //done
				break;
			if(weakClassifiers.size() > MAX_NUM_WEAK_LEARNER) {
				cout << "cannot satisfy the required max false alarm rate and min detection rate" << endl;
				cout << "the number of weak learners exceed " << MAX_NUM_WEAK_LEARNER << endl;
				cout << "current detection rate: " << current_detection_rate << " " <<
					"current false alarm rate: " << current_false_alarm << endl;
				ret = -1;		
				break;
			}

		}

		if (ret == 0) {
			//save the result to LUTAdaData => don't save now
			//data->lut_classifiers.clear();
			//saveLUTAdaData(data);

			//mark active or not in the training set => don't need
			//markPass(p_pos_set, cum_pred_pos_conf, threshold);
			//markPass(p_neg_set, cum_pred_neg_conf, threshold);
		}

		is_trained = true;

		delete [] pos_weak_confs;
		delete [] neg_weak_confs;
		delete [] pos_str_confs;
		delete [] neg_str_confs;

	}


	delete [] pos_feat_values;
	delete [] neg_feat_values;


	return ret;
}


//
void WfsStrongClassifier::UpdateWeightForBranchingNode(ImgSet* p_pos_set, double** pos_weak_conf,
						  ImgSet* p_neg_set, double** neg_weak_conf)
{
	int nPos = p_pos_set->Size();
	int nNeg = p_neg_set->Size();

	// exp ( - v dot f(x) )
	// v is intrinsic projection vector such as (1,0,0), (0, 1, 0), ...
	// f(x) is also a vector

	for(int i=0; i<nPos; ++i){
		p_pos_set->GetGranVec(i)->weight *= exp( - pos_weak_conf[i][p_pos_set->GetGranVec(i)->category] );
	}
	for(int i=0; i<nNeg; ++i){
		p_neg_set->GetGranVec(i)->weight *= exp(neg_weak_conf[i][p_neg_set->GetGranVec(i)->category]);
	}

}

void WfsStrongClassifier::UpdateWeightForBinaryNode(ImgSet* p_pos_set, double* pos_weak_conf, 
						  ImgSet* p_neg_set, double* neg_weak_conf)
{
	int nPos = p_pos_set->Size();
	int nNeg = p_neg_set->Size();

	for(int i=0; i<nPos; ++i){
		p_pos_set->GetGranVec(i)->weight *= exp(-pos_weak_conf[i]);
	}
	for(int i=0; i<nNeg; ++i){
		p_neg_set->GetGranVec(i)->weight *= exp(neg_weak_conf[i]);
	}

}

void WfsStrongClassifier::normalizeWeight(ImgSet* p_pos_set, ImgSet* p_neg_set)
{
	double total = 0.0;
	int num_pos_img = p_pos_set->Size();
	int num_neg_img = p_neg_set->Size();

	for(int i=0; i<num_pos_img; ++i)	
		total += p_pos_set->GetGranVec(i)->weight;

	for(int i=0; i<num_neg_img; ++i)	
		total += p_neg_set->GetGranVec(i)->weight;

	for(int i=0; i<num_pos_img; ++i)	
		p_pos_set->GetGranVec(i)->weight /= total;

	for(int i=0; i<num_neg_img; ++i)	
		p_neg_set->GetGranVec(i)->weight /= total;

}

void WfsStrongClassifier::CheckPerformanceBranchingNode(double &false_positive, double &detection_rate, ImgSet *pos_set, double **pos_confs, ImgSet *neg_set, double **neg_confs)
{
	int n_pos = pos_set->Size();
	int n_neg = neg_set->Size();

	int num_true_pos = 0;
	int num_false_pos = 0;

	int pos_but_wrong_pose = 0;

	if (1) {
		// only care about one given category = one pose. don't count cases making wrong result for poses other than the actual pose
		for(int i=0; i<n_pos; ++i){
			if (pos_confs[i][pos_set->GetGranVec(i)->category] > 0) {
				num_true_pos++;
			}

			// just temp
			for(int j=0; j<nc; j++) {
				if (pos_confs[i][j] > 0) {
					if (j == pos_set->GetGranVec(i)->category) { //right pose
					} else {
						// ?
						pos_but_wrong_pose++;
					}
				}
			}
		}

		for(int i=0; i<n_neg; ++i){
			if (neg_confs[i][neg_set->GetGranVec(i)->category] > 0) {
				num_false_pos++;
			}
		}

	} else {
		for(int i=0; i<n_pos; ++i){
			for(int j=0; j<nc; j++) {
				if (pos_confs[i][j] > 0) {
					if (j == pos_set->GetGranVec(i)->category) { //right pose
						num_true_pos++;
					} else {
						// ?
						num_false_pos++;
					}
				}
			}
		}

		for(int i=0; i<n_neg; ++i){
			bool found = false;
			for(int j=0; j<nc; j++) {
				if (neg_confs[i][j] > 0) {
					found = true; break;
				}
			}
			if (found) num_false_pos++;
		}
	}
	
	false_positive = (double)num_false_pos / n_neg;
	detection_rate = (double) num_true_pos / n_pos;

	cout << "pos_but_wrong_pose : " << pos_but_wrong_pose << "\n";

}

void WfsStrongClassifier::CheckPerformanceBinaryNode(double &false_positive, double &detection_rate, 
													 ImgSet *pos_set, double *pos_confs, ImgSet *neg_set, double *neg_confs)
{
	int n_pos = pos_set->Size();
	int n_neg = neg_set->Size();

	int num_true_pos = 0;
	int num_false_pos = 0;

	for(int i=0; i<n_pos; ++i){
		if( pos_confs[i] > 0)
			++num_true_pos;
	}

	for(int i=0; i<n_neg; ++i){
		if( neg_confs[i] > 0)
			++num_false_pos;
	}
	
	false_positive = (double) num_false_pos / n_neg;
	detection_rate = (double) num_true_pos / n_pos;

}

void WfsStrongClassifier::CheckStagePass(ImgSet *set, int category) 
{
	if (!is_trained) {
		if (finalPose != 0)	return;

		for(int i=0; i<set->Size(); i++) {
			set->GetGranVec(i)->active = false;
		}

		return;
	}

	int n = set->Size();
	double *confs = new double [n];
	int *feats = new int [n];
	memset(confs, 0, sizeof(double) * n);
	memset(feats, 0, sizeof(int) * n);

	if (ntype == BRANCHING) {
		// output of "category" should be positive

//		cout << "CheckStagePass \n";
		for(int i=0; i<(int)weakClassifiers.size(); i++) {
//			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, &set->gran_vec[0], n);
			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, *set);
			weakClassifiers[i].AddWeakConfidenceVectorListGivenCategory(confs, feats, n, category);
		}

		for(int i=0; i<n; i++) {
			if (confs[i] < 0) {
				set->GetGranVec(i)->active = false;
			}
		}

	} else {

//		cout << "CheckStagePass \n";
		for(int i=0; i<(int)weakClassifiers.size(); i++) {
//			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, &set->gran_vec[0], n);
			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, *set);
			weakClassifiers[i].AddWeakConfidenceList(confs, feats, n);
		}

		for(int i=0; i<n; i++) {
			if (confs[i] < 0) {
				set->GetGranVec(i)->active = false;
			}
		}

	}

	delete []confs;
	delete []feats;


}

// not for training.
// deal with unlabeled data and find out to which child node each sample should be sent
void WfsStrongClassifier::CheckStagePassForDetectionBranching(ImgSet *set, vector<ImgSet *>& child_sets) 
{
	if (!is_trained) {
		if (finalPose != 0)	{
			// go to single casade
			return;
		}

		// mark false for all samples

		//for(int i=0; i<set->Size(); i++) {
		//	set->GetGranVec(i)->active = false;
		//}

		return;
	}

	int n = set->Size();
	int *feats = new int [n];
	memset(feats, 0, sizeof(int) * n);

	if (ntype == BRANCHING) {
		double **confs = Make2dDoubleArray(n, nc);

//		cout << "CheckStagePassForDetection \n";
		for(int i=0; i<(int)weakClassifiers.size(); i++) {
//			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, &set->gran_vec[0], n);
			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, *set);
			weakClassifiers[i].AddWeakConfidenceVectorList(confs, feats, n);
		}

		for(int i=0; i<n; i++) {
			for(int c=0; c<nc; c++) {
				if (confs[i][c] > -1.0) {
					child_sets[c]->AddGranVec(set->GetGranVec(i));
				}
			}
		}

		Delete2dDoubleArray(&confs, n);

	} else {
		double *confs = new double [n];
		memset(confs, 0, sizeof(double) * n);

//		cout << "CheckStagePass \n";
		for(int i=0; i<(int)weakClassifiers.size(); i++) {
//			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, &set->gran_vec[0], n);
			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, *set);
			weakClassifiers[i].AddWeakConfidenceList(confs, feats, n);
		}

		for(int i=0; i<n; i++) {
			if (confs[i] < 0) {
				set->GetGranVec(i)->active = false;
			}
		}
		delete []confs;

	}

	delete []feats;


}


// not for training.
// deal with unlabeled data and find out to which child node each sample should be sent
void WfsStrongClassifier::CheckStagePassForDetectionBinary(ImgSet *set) 
{
	if (!is_trained) {
		for(int i=0; i<set->Size(); i++) {
			set->GetGranVec(i)->active = false;
		}
		return;
	}

	int n = set->Size();
	int *feats = new int [n];
	memset(feats, 0, sizeof(int) * n);
	double *confs = new double [n];
	memset(confs, 0, sizeof(double) * n);

	for(int i=0; i<(int)weakClassifiers.size(); i++) {
//			EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, &set->gran_vec[0], n);
		EvaluateFeatureForMultipleSamples(feats, weakClassifiers[i].sparse_feature, *set);
		weakClassifiers[i].AddWeakConfidenceList(confs, feats, n);
	}

	for(int i=0; i<n; i++) {
		if (confs[i] < -1.0) {
			set->GetGranVec(i)->active = false;
		}
	}

	delete []confs;
	delete []feats;
}


//
//void WfsStrongClassifier::predict(GranVec& sample, bool& pred_sign, double& pred_conf, double threshold)
//{
//  
//	//int value;
//	//double conf = 0.0;
//	//int num_weak = (int)weakClassifiers.size();
//
//	//for(int i=0; i<num_weak; ++i){
//	//	if(stage != 0 && i==0) {
//	//		conf += weakClassifiers[0].predict(sample.conf);
//	//	} else{
//	//		SparseFeature sp = weakClassifiers[i].getSparseFeature();
//	//		value = EvaluateFeature( sp, sample);
//	//		conf += weakClassifiers[i].predict(value);
//	//	}
//	//}
//
//	//pred_conf = conf - threshold;
//
//	//if(pred_conf >= 0)
//	//	pred_sign = true;
//	//else
//	//	pred_sign = false;
//}
//
//void WfsStrongClassifier::checkPass(ImgSet *p_set, double threshold)
//{
//	//int num_img = p_set->num_img;
//
//	////evaluated sparse feature value
//	//int* esfv = new int[num_img];
//	//double* pred_conf = new double[num_img];
//	//double* cum_pred_conf = new double[num_img];
//	//
//	//initializeDouble(cum_pred_conf, num_img, 0.0); 
//	//
//	//double minC, maxC;	//dummy
//	//double* conf_values = new double[num_img];
//
//	//int num_weak = (int)weakClassifiers.size();
//
//	//for(int i=0; i<num_weak; ++i){
//	//	if(stage != 0 && i==0) {
//	//		readConfidenceFromImgSet(p_set, conf_values, &minC, &maxC);
//	//		weakClassifiers[0].predictSamples(conf_values, num_img, pred_conf);
//	//	} else{
//	//		SparseFeature sp = weakClassifiers[i].getSparseFeature();
//	//		EvaluateFeatureForMultipleSamples(esfv, 
//	//			sp, &p_set->gran_vec[0], num_img);
//	//		weakClassifiers[i].predictSamples(esfv, num_img, pred_conf);		
//	//	}
//	//
//	//	cumTwoArray(cum_pred_conf, pred_conf, num_img);
//	//}
//	//
//	//markPass(p_set, cum_pred_conf, threshold);
//
//	//	
//
//	//delete[] esfv;
//	//delete[] pred_conf;
//	//delete[] cum_pred_conf;
//	//delete[] conf_values;
//
//	//esfv = 0;
//	//pred_conf = 0;
//	//cum_pred_conf = 0;
//	//conf_values = 0;
//}
//
//void WfsStrongClassifier::markPass(ImgSet* p_set, double* cum_pred_conf, double threshold)
//{
//	//int num_img = p_set->num_img;
//
//	//for(int i=0; i<num_img; ++i){
//	//	if(cum_pred_conf[i] - threshold >= 0){
//	//		p_set->GetGranVec(i)->active = true;
//	//		p_set->GetGranVec(i)->conf = cum_pred_conf[i];
//	//	}
//	//	else
//	//		p_set->GetGranVec(i)->active = false;
//	//}
//
//}
//



//int WfsStrongClassifier::transformConf2BinIndex(double *pos_conf, int num_pos_img, double *neg_conf, int num_neg_img,
//		int *pos_bin_ind, int *neg_bin_ind, double min_conf, double max_conf)
//{
//
//	//double diff = max_conf - min_conf;
//	//
//	////LC 1124
//	//int bin_num;
//
//	//if(diff == 0){
//	//	cout << "diff = 0 in transformConf2BinIndex" << endl;
//
//	//	//set bin_num=0
//	//	for(int i=0; i<num_pos_img; ++i)
//	//		pos_bin_ind[i] = 0;
//	//	for(int i=0; i<num_neg_img; ++i)
//	//		neg_bin_ind[i] = 0;
//	//}
//	//else{ 
//	//	for(int i=0; i<num_pos_img; ++i){
//	//		bin_num = (int)floor( K_LUTBINS*(pos_conf[i]-min_conf)/diff );
//
//	//		//implicitly assume pos_conf[i] >= min_conf
//	//		if (bin_num >= K_LUTBINS)
//	//			bin_num = K_LUTBINS - 1;
//	//		pos_bin_ind[i] = bin_num;
//	//	}
//
//	//	for(int i=0; i<num_neg_img; ++i){
//	//		bin_num = (int)floor( K_LUTBINS*(neg_conf[i]-min_conf)/diff );
//
//	//		if (bin_num >= K_LUTBINS)
//	//			bin_num = K_LUTBINS - 1;
//	//		neg_bin_ind[i] = bin_num;
//	//	}
//	//}
//
//	return 0;
//}
//
//void WfsStrongClassifier::trainNestingDetector(WfsWeakClassifier& nest_detector,
//								  ImgSet* p_pos_set, ImgSet* p_neg_set,
//								  double* cum_pred_pos_conf, double *cum_pred_neg_conf)
//{
//	////build confidence table
//	//int num_pos_img = p_pos_set->num_img;
//	//int num_neg_img = p_neg_set->num_img;
//
//	//double pos_minC, pos_maxC, neg_minC, neg_maxC;
//	//double* pos_conf = new double[num_pos_img];
//	//double* neg_conf = new double[num_neg_img];
//	//int *pos_bin_ind = new int[num_pos_img];
//	//int *neg_bin_ind = new int[num_neg_img];
//
//	//readConfidenceFromImgSet(p_pos_set, pos_conf, &pos_minC, &pos_maxC);
//	//readConfidenceFromImgSet(p_neg_set, neg_conf, &neg_minC, &neg_maxC);
//
//	//if(pos_minC > neg_minC)
//	//	pos_minC = neg_minC;
//	//if(pos_maxC < neg_maxC)
//	//	pos_maxC = neg_maxC;
//
//	//transformConf2BinIndex(pos_conf, num_pos_img, neg_conf, num_neg_img,
//	//	pos_bin_ind, neg_bin_ind, pos_minC, pos_maxC);
//
//	//nest_detector.countNestSamples(num_pos_img, pos_bin_ind, &p_pos_set->gran_vec[0],
//	//	num_neg_img, neg_bin_ind, &p_neg_set->gran_vec[0],
//	//	pos_minC, pos_maxC);
//
//	//nest_detector.calculateConfTable();
//
//	////read confidence
//	//double feat;
//
//	//for(int i=0; i<num_pos_img; ++i){
//	//	feat = p_pos_set->GetGranVec(i)->conf;
//	//	cum_pred_pos_conf[i] = nest_detector.predict(feat);
//	//}
//	//for(int i=0; i<num_neg_img; ++i){
//	//	feat = p_neg_set->GetGranVec(i)->conf;
//	//	cum_pred_neg_conf[i] = nest_detector.predict(feat);
//	//}
//
//	//delete[] pos_conf;
//	//delete[] neg_conf;
//	//delete[] pos_bin_ind;
//	//delete[] neg_bin_ind;
//
//	//pos_conf = 0;
//	//neg_conf = 0;
//	//pos_bin_ind = 0;
//	//neg_bin_ind = 0;
//}
//




//void WfsStrongClassifier::updateNestingWeight(WfsWeakClassifier& nest_detector,
//								 ImgSet* p_pos_set, ImgSet* p_neg_set)
//{
//	int num_pos_img = p_pos_set->num_img;
//	int num_neg_img = p_neg_set->num_img;
//
//	double conf;
//
//	for(int i=0; i<num_pos_img; ++i){
//		conf = p_pos_set->GetGranVec(i)->conf;
//		p_pos_set->GetGranVec(i)->weight *= exp(-nest_detector.predict(conf));
//	}
//	for(int i=0; i<num_neg_img; ++i){
//		conf = p_neg_set->GetGranVec(i)->conf;
//		p_neg_set->GetGranVec(i)->weight *= exp(nest_detector.predict(conf));
//	}
//}
//

