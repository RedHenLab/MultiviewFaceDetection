#include <fstream>
#include <iostream>

#include "WFSTrainer.h"
//#include "XMLHandler.h"

#include "Markup.h"

WfsMvfdTrainer::WfsMvfdTrainer(TrainInfo &info)
{
	trainInfo = info;
	pIH = new ImageHandler;

	ReadFileList();
	SetSampleSetSize();

}

WfsMvfdTrainer::~WfsMvfdTrainer()
{
	if (pIH)
		delete pIH;
}

//
//struct trainInfo
//{
//	string dir_pos;
//	string dir_neg;
//	int num_stage;
//	int train_num_pos;
//	int train_num_neg;
//	double min_detection_rate;
//	double max_false_alarm;
//	double threshold;
//};


void WfsMvfdTrainer::ReadFileList()
{
	cout << "READ sample list \n";

	posSamplesFilenames.clear();
	posSamplesPoses.clear();
	negImageFilenames.clear();

	for(int i=0; i<=K_WFS_POSES; i++) {
		nPosSamplesOfEachPose[i] = 0;
	}

	ifstream face_list;
	face_list.open(trainInfo.positive_list_file.c_str());

	if (face_list.is_open()) {
		while(face_list.good()) {
			string filename;
			std::getline(face_list, filename, '\n');
//			getline(face_list, filename, '\n');

			if (filename.length() <= 1) continue;

			string posestr;
			std::getline(face_list, posestr, '\n');
			
			int pose = atoi(posestr.c_str());

			posSamplesFilenames.push_back(filename);
			posSamplesPoses.push_back(pose);

			nPosSamplesOfEachPose[pose]++;
		}
		face_list.close();
	}

	ifstream non_face_list;
	non_face_list.open(trainInfo.negative_list_file.c_str());

	if (non_face_list.is_open()) {
		while(non_face_list.good()) {
			string filename;
			std::getline(non_face_list, filename, '\n');

			if (filename.length() <= 1) continue;

			negImageFilenames.push_back(filename);

		}
		non_face_list.close();
	}

	cout << "FACES : " << posSamplesFilenames.size() << "\n";
	cout << "NON FACE IMAGES : " << negImageFilenames.size() << "\n";

	trainInfo.train_num_pos = min(trainInfo.train_num_pos, (int)posSamplesFilenames.size());

}

void WfsMvfdTrainer::SetSampleSetSize()
{
	// trainInfo.train_num_pos =  total # of positive samples that will be used for training the first branching node = root
	// For sub-nodes, the numbebr should be adjusted according to the original ratio computed from the entire sample list (recorded in nPosSamplesOfEachPose)
	// Eventually, the actual set size will be stored in nPosSamplesOfEachBranchNode

	int nP = trainInfo.train_num_pos;
//	int nN = trainInfo.train_num_neg;

	memset(nPosSamplesOfEachBranchNode, 0, sizeof(int) * K_WFS_BRANCH_NODES);

	nPosSamplesOfEachBranchNode[0] = nP;
	for(int i=1; i<K_WFS_BRANCH_NODES; i++) {
		if (WfsTreeStructure[i][0] > 0 ) {
			// leaf branching node
			nPosSamplesOfEachBranchNode[i] = nPosSamplesOfEachPose[WfsTreeStructure[i][0]];
		} else {
			for(int j=1; j<=K_WFS_POSES; j++) {
				if (WfsTreeStructure[i][j] > 0) { // pose j is sub-included in node i
					nPosSamplesOfEachBranchNode[i] += nPosSamplesOfEachPose[j];
				}
			}
		}

		nPosSamplesOfEachBranchNode[i] = nPosSamplesOfEachBranchNode[i] * nP / posSamplesFilenames.size();
	}


	for(int i=0; i<K_WFS_BRANCH_NODES; i++) {
		cout << "NODE " << i << " POS SAMPLES # : " << nPosSamplesOfEachBranchNode[i] << "\n";
	}

}

void WfsMvfdTrainer::ResumeTrain(const string &classifier_file, int resume_node, int resume_pose)
{
	// read classifier_file
	load(classifier_file);


	// train
	if (resume_pose == 0) {

		if (resume_node < (int)wfsNodes.size()) {
			wfsNodes.resize(resume_node);
		}

		DoTrain(resume_node);
	} else {
		for(int i=resume_pose; i<=K_WFS_POSES; i++) {
			DoTrainForSinglePose(i);
		}
	}

}

int WfsMvfdTrainer::DoTrain(int start_node)
{
	int result = 0;

	// Train branching nodes
	for(int n=start_node; n<K_WFS_BRANCH_NODES; n++) {
		int nC = WfsTreeInfo[n][1];
		WfsStrongClassifier node(n, WfsTreeInfo[n][0], BRANCHING, nC );

		if (nC == 0) {
			node.finalPose = WfsTreeStructure[n][0];
			node.cascadeId = 100 * (n + 1);
			//cascadeIdx[node.cascadeId] = wfsNodes.size();
			wfsNodes.push_back(node);
		} else {
			ImgSet *p_posset = new ImgSet;
			ImgSet *p_negset = new ImgSet;

			int nP = nPosSamplesOfEachBranchNode[n];
			int nN = (int) ((double)nP * trainInfo.nNegSamplesRatio);

			vector<int> path_id;
			int trace_node = n;
			do {
				path_id.insert(path_id.begin(), trace_node);
				trace_node = WfsTreeInfo[trace_node][0]; // parent
			} while (trace_node >= 0);

			cout<<"TRAIN cascade "<< n << endl;
			cout<<"              collect pos samples "<< endl;
			result = CollectPositiveSamples(nP, node, p_posset, path_id);
			if(result<0){
				cerr<<"collect pos failed"<<endl;
				return -1;
			}

			if (p_posset->Size() < 100) {
				cout << "not enough samples\n";
			} else {
				result = CollectNegativeSamples(nN/nC, node, p_negset, path_id);
				if(result<0){
					cerr<<"collect neg failed"<<endl;
					return -1;
				}

				// duplicate negative samples (-1, 0,0), (0,-1,0), ...
				ImgSet *new_negset = new ImgSet;

				for(int i=0; i<p_negset->Size(); i++) {
					for(int j=0; j<nC; j++) {
						GranVec *gv = new GranVec(*p_negset->GetGranVec(i));
						gv->poseId = 0;
						gv->category = j;
						new_negset->AddGranVec(gv);
					}
				}

				delete p_negset;
				p_negset = new_negset;

				node.TrainStrongClassifier(p_posset, p_negset, trainInfo.max_false_alarm, trainInfo.min_detection_rate, trainInfo.threshold); 
			}

			wfsNodes.push_back(node);

			delete p_posset;
			delete p_negset;
		}

		string xml_name = "./output_middle";
		char nstr[10];
		itoa(n, nstr, 10);
		xml_name += string(nstr) + ".xml";
		save(xml_name);
	}

	cout << "\nBRANCHING TREE is completed\n\n";

	for(int i=1; i<=K_WFS_POSES; i++) {
		DoTrainForSinglePose(i);
	}

	return 0;
}

// this will train a cascade for single pose, which follows leaf node of branching tree
int WfsMvfdTrainer::DoTrainForSinglePose(int pose_id)
{
	int result = 0;

	// single pose cascade
	for(int n=0; n<K_WFS_BRANCH_NODES; n++) {
		if (wfsNodes[n].finalPose == pose_id) {
//			int pose = wfsNodes[n].finalPose;
//			int cas_id = wfsNodes[n].cascadeId;
			int node_id = wfsNodes.size();

			wfsNodes[n].cascadeId = node_id;

			vector<int> path_id;
			int trace_node = n;
			do {
				path_id.insert(path_id.begin(), trace_node);
				trace_node = WfsTreeInfo[trace_node][0]; // parent
			} while (trace_node >= 0);
			
			for(int i=0; i<trainInfo.nBinaryStages; i++) {
				int parent = node_id - 1;
				if (i == 0) parent = n;

				WfsStrongClassifier node(node_id, parent, BINARY, 0 );
				node.finalPose = pose_id;
				node.cascadeStage = i;
				node.isFinalStage = true;

				if (i > 0) {
					wfsNodes[parent].isFinalStage = false;
				}
				
				ImgSet *p_posset = new ImgSet;
				ImgSet *p_negset = new ImgSet;

				int nP = nPosSamplesOfEachBranchNode[n];
				int nN = (int) ((double)nP * trainInfo.nNegSamplesRatio);

				path_id.push_back(node_id);

				cout<<"TRAIN cascade single cascade "<< node_id << endl;
				result = CollectPositiveSamples(nP, node, p_posset, path_id);

				if (p_posset->Size() < 100) {
					cout << "not enough samples\n";
				} else {
					cout<<"              collect neg samples "<< endl;
					result = CollectNegativeSamples(nN, node, p_negset, path_id);

					node.TrainStrongClassifier(p_posset, p_negset, trainInfo.max_false_alarm, trainInfo.min_detection_rate, trainInfo.threshold); 
				}

				wfsNodes.push_back(node);

				delete p_posset;
				delete p_negset;

				string xml_name = "./output_middle_single_cascade_";
				char nstr[10];
				itoa(pose_id, nstr, 10);
				xml_name += string(nstr) + "_middle_";
				itoa(i, nstr, 10);
				xml_name += string(nstr) + ".xml";
				save(xml_name);

				node_id++;
			}

			string xml_name = "./output_middle_single_cascade_";
			char nstr[10];
			itoa(pose_id, nstr, 10);
			xml_name += string(nstr) + ".xml";
			save(xml_name);
		}


	}

	return 0;

}

int WfsMvfdTrainer::CollectPositiveSamples(int num, WfsStrongClassifier &node, ImgSet *p_posset, vector<int> &path_id)
{

	if(node.nodeId == 0)
	{
		int offset = 0;			
		return pIH->ReadPositiveSamplesFromList(p_posset, posSamplesFilenames, posSamplesPoses, offset, node.nodeId, num);
	}

	// read & test
	int n_ready = 0;
	int offset = 0;			
	while(n_ready < num && offset < (int)posSamplesFilenames.size()) {
		ImgSet new_set;

		int ret = 0;

		if (node.ntype == BRANCHING) {
			ret = pIH->ReadPositiveSamplesFromList(&new_set, posSamplesFilenames, posSamplesPoses, offset, node.nodeId, num - n_ready);
		} else {
			ret = pIH->ReadPositiveSamplesFromList(&new_set, posSamplesFilenames, posSamplesPoses, offset, node.nodeId, num - n_ready, node.finalPose);
		}

		if (ret < 0) {
			break;
		}

		TestSamples(&new_set, node.nodeId, path_id);

		p_posset->MergeSet(new_set);

		n_ready = p_posset->Size();
	}

	if (n_ready < num) {
		cout << "out of pos samples at branching node " << node.nodeId << " only samples " << n_ready << "\n";
	}


	return 0;

}

int WfsMvfdTrainer::CollectNegativeSamples(int num, WfsStrongClassifier &node, ImgSet *p_negset, vector<int>& path_id)
{
	cout << "        col neg " << num << " " << node.nodeId << "\n";

	// root
	if(node.nodeId == 0)
	{
		int offset = 0;

		int ret = pIH->ReadNegativeSamplesFromList(p_negset, negImageFilenames, offset, num, max(10, (num / (int)negImageFilenames.size()) + 1) );

		// normalize samples here
		pIH->NormalizeGranVecs(*p_negset);

		return ret;
	}

	// read & test
	int n_ready = 0;
	int offset = 0;			
	while(n_ready < num && offset < (int)negImageFilenames.size()) {
		ImgSet new_set;

		cout << "READ : " ;

		// temp
		int mn = 0;
		if (node.ntype == BRANCHING) {
			if (node.nodeId < 4) mn = 20;
			else if (node.nodeId < 10) mn = 200;
		}

//		if (node.ntype == BINARY && node.cascadeStage == 0) mn = 200;

		int ret = pIH->ReadNegativeSamplesFromList(&new_set, negImageFilenames, offset, num - n_ready, mn);

		if (ret < 0) {
			break;
		}

		cout << new_set.Size() << "\n";
		TestSamples(&new_set, node.nodeId, path_id);
		cout << "PASS : " << new_set.Size() << "\n";

		// temp
		if (new_set.Size() > num * 1.2) {
			new_set.ShrinkSet(num * 1.2);
			cout << "neg new set reduce : " << new_set.Size() << "\n";
		}

		//for(int i=0; i<new_set.Size(); i++) {
		//	checkGranVec(*new_set.GetGranVec(i));
		//} 

		// normalize samples here
		pIH->NormalizeGranVecs(new_set);
		p_negset->MergeSet(new_set);

		n_ready = p_negset->Size();

		cout << "SUM : " << p_negset->Size() << "\n";

	}

	if (n_ready < num) {
		cout << "out of neg samples at branching node " << node.nodeId << " only samples " << n_ready << "\n";
	}



	return 0;

//	int num_collected = p_negset->Size();
//	int num_needed = num - p_negset->Size();
//
////	int qqq = 0;
//
//	while(num_collected<num)
//	{
//		ImgSet neg_temp;
//		neg_temp.Size() = 0;
//		int result = CollectNegativeSamples((int)(num_needed*4), stage-1, &neg_temp);
//		if( result <0)
//			return -1;
//
//		//cout << qqq++ << " \n";
//		//checkGranVec(neg_temp.gran_vec[0]);
//
//		wfsNodes[stage-1].checkPass(&neg_temp);
//		pIH->cleanImgSet(&neg_temp);
//
//		num_collected += neg_temp.Size();
//
//		//if (crtStage == stage) {
//		//	cout << "   col neg " << neg_temp.Size() << " " << num_collected << "\n";
//		//}
//
//		num_needed = num - num_collected;
//		//cout<<"neg coll "<<num_collected<<" stage "<<stage<<" ";
//		pIH->mergeImgSet(p_negset, &neg_temp);
//
//		
//	}
//	return num_collected;
}

// remove failed samples
int WfsMvfdTrainer::TestSamples(ImgSet *set, int node_id, vector<int> &path_id)
{
	for(int i=0; i<(int)path_id.size() - 1; i++) {
		if (wfsNodes[path_id[i]].ntype == BRANCHING) {
			if (wfsNodes[path_id[i]].finalPose == 0) {
				// this should be done somewhere else (before here)
				int cat = -1;
				for(int j=0; j<wfsNodes[path_id[i]].nc ; j++) {
					if (WfsTreeInfo[path_id[i]][2+j] == path_id[i+1]) {
						cat = j;
						break;
					}
				}

//				cout << "CheckStagePass ";
				wfsNodes[path_id[i]].CheckStagePass(set, cat);
//				cout << "done\n";
			}
		} else {
			// single cascade
			wfsNodes[path_id[i]].CheckStagePass(set); 
		}

//		cout << "delete ";
		set->DeleteInactiveGranVecs(true);
//		cout << "done\n";
		if (set->Size() == 0) break;
	}

	return set->Size();
}

//
//void WfsMvfdTrainer::detectMultiScaleOrg(IplImage *image, std::vector<Rect> &objects, double scaleFactor, int minShift, int flags, int minSize, int maxSize)
//{
//	int total_round=0;
//	vector<vector<Rect>> temp_object;
//	vector<int> scale;
//	for(int i=minSize; i<maxSize; i=(int)(i*scaleFactor))
//	{
//		vector<Rect> vec_rect;
//		temp_object.push_back(vec_rect);
//		scale.push_back(i);
//		total_round++;
//	}
//	
//	
//	for(int round=1; round<total_round; round++)
//	{	
//		ImgSet imgset;
//		pIH->CropRectFromImage(image, imgset, scale[round], minShift);
//		//pIH->saveDir("./result_test/",&imgset);
//		//system("pause");
//		cout<<"size "<<scale[round]<< " numofimg "<< imgset.Size()<<endl;
//		checkPass(&imgset);
//		cout<<"size "<<scale[round]<<" finished"<<endl;
//		for(int j=0; j<imgset.Size(); j++)
//		{
//			cv::Rect rect;
//			rect.x = imgset.GetGranVec(j)->x;
//			rect.y = imgset.GetGranVec(j)->y;
//			rect.width = imgset.GetGranVec(j)->size;
//			rect.height = imgset.GetGranVec(j)->size;
//			temp_object[round].push_back(rect);
//
////				checkGranVec(imgset.gran_vec[j]);
//		}
//	}
//	
//	for(int i=0; i<total_round; i++)
//	{
//		objects.insert(objects.end(), temp_object[i].begin(), temp_object[i].end());
//	}
//}
//

//void WfsMvfdTrainer::detectMultiScale(IplImage *image, std::vector<Rect> &objects, double scaleFactor, int minShift, int flags, int minSize, int maxSize)
//{
//	objects.clear();
//
//	const bool half_processing = false;
//
//	// grayscale
//	IplImage *gray;
//
//	pIH->convertIplImgToGrayscale(image, &gray);
//
//	if (half_processing) {
//		IplImage *gray_h;
//
//		gray_h = cvCreateImage( cvSize(gray->width >> 1, gray->height >> 1), IPL_DEPTH_8U, 1 );
//		cvResize(gray, gray_h, CV_INTER_LINEAR );
//
//		cvReleaseImage(&gray);
//		gray = gray_h;
//	}
//
////	cvNamedWindow("gray2");
//
//	// JS : this should be changed later - doing this once somewhere before processing frames of fixed size
//
//	int org_wh = min(gray->width, gray->height);
//	int wh = org_wh;
//
//	int w = gray->width;
//	int h = gray->height;
//
//	int nS = 0;
//	vector<int> ws;
//	vector<int> hs;
//	vector<int> ss;
//	vector<int> js;
//	int s = K_WIN_SIZE;
//	int jump = minShift;
//
//	while(w > (K_WIN_SIZE << 1) && h > (K_WIN_SIZE << 1)) 
//	{
//		ws.push_back(w);
//		hs.push_back(h);
//		ss.push_back(s);
//		js.push_back(jump);
//
//		w = w / scaleFactor;
//		h = h / scaleFactor;
//		s = K_WIN_SIZE * image->width / w;
//		jump = max(1, minShift * w / image->width);
//		nS++;
//	}
//
//	for(int i=0; i<nS; i++) {
//		IplImage *gray2;
//
//		if (i > 0) {
//			gray2 = cvCreateImage( cvSize(ws[i], hs[i]), IPL_DEPTH_8U, 1 );
//			cvResize(gray, gray2, CV_INTER_LINEAR );
//		} else {
//			gray2 = gray;
//		}
//
//		//cout << "scale " << i << " " << ws[i] << " " << hs[i] << " win" << ss[i] << "\n";
//		//cvResizeWindow("gray2", ws[i], hs[i]);
//		//cvShowImage("gray2", gray2);
//		//cvWaitKey(10000);
//
//		// collect granvec
//		ImgSet imgset;
//		pIH->getAllGranVecInAScaleFromIplImg(imgset, gray2, ws[i], hs[i], js[i]);
//
//		checkPass(&imgset);
//		cout << imgset.Size() << " found \n";
//		
//		if (0 && imgset.Size() > 0)
//		{
//			IplImage *temp = cvCloneImage(image);
//			cvNamedWindow("resc");
//			cvResizeWindow("resc", image->width, image->height);
//
//			for(int j=0; j<imgset.Size(); j++)
//			{
//				int x = imgset.GetGranVec(j)->x * image->width / ws[i];
//				int y = imgset.GetGranVec(j)->y * image->height / hs[i];
//
//				cvDrawRect(temp, cvPoint(x,y), cvPoint(x+ss[i],y+ss[i]), cvScalar(0, 0, 255)); 
//			}
//
//			cvShowImage("resc", temp);
//			cvWaitKey(10000);
//
//			for(int j=0; j<imgset.Size(); j++)
//			{
//				checkGranVec(imgset.gran_vec[j]);
//			}
//
//			cvReleaseImage (&temp);
//		}
//
//		for(int j=0; j<imgset.Size(); j++)
//		{
//			cv::Rect rect;
//			rect.x = imgset.GetGranVec(j)->x * image->width / ws[i];
//			rect.y = imgset.GetGranVec(j)->y * image->height / hs[i];
//			rect.width = ss[i];
//			rect.height = ss[i];
//			objects.push_back(rect);
//		}
//
//
//		if (i > 0) {
//			cvReleaseImage( &gray2);
//		}
//	}
//
//	cvReleaseImage(&gray);
//
//	return;
//
//}

//void WfsMvfdTrainer::checkPass(ImgSet *p_set)
//{
//	//for(int i=0;i<(int)wfsNodes.size() && p_set->Size()>0;i++)
//	//{
//	//	wfsNodes[i].checkPass(p_set);
//	//	pIH->cleanImgSet(p_set);
//	//}
//}

//void WfsMvfdTrainer::checkFeatures()
//{
//	for(int i=0;i<(int)v_lutada_data.size();i++)
//	{
//		for(int j=0;j<(int)v_lutada_data[i].lut_classifiers.size();j++)
//		{
//			if(i==0 || j!=0)
//				checkFeature(v_lutada_data[i].lut_classifiers[j].features);
//		}
//	}
//}




int WfsMvfdTrainer::save(const string &filename)
{
	CMarkup xml;
	xml.AddElem( MCD_T("MVFD_VECTOR_BOOSTING_CASCADE") );
	xml.IntoElem();
	for(int i=0; i<(int)wfsNodes.size(); i++)
	{
		bool branch = (wfsNodes[i].ntype == BRANCHING);
		bool is_trained = wfsNodes[i].is_trained;

		xml.AddElem( MCD_T("NODE"));
		xml.SetAttrib( MCD_T("NODE_ID"),wfsNodes[i].nodeId );
		xml.SetAttrib( MCD_T("IS_BRANCHING"),branch);
		xml.SetAttrib( MCD_T("IS_TRAINED"),is_trained);

		if (branch) {
			xml.SetAttrib( MCD_T("FINAL_POSE"),wfsNodes[i].finalPose);
			if (wfsNodes[i].finalPose == 0) {
				xml.SetAttrib( MCD_T("NUM_BRANCHES"),wfsNodes[i].nc);

				if (is_trained) {
					xml.IntoElem();
					for(int j=0; j<(int)wfsNodes[i].weakClassifiers.size(); j++)
					{
						xml.AddElem( MCD_T("WEAK_CLASSIFIER"));
						xml.SetAttrib( MCD_T("l_num"),j);
						xml.IntoElem();
						{
							//if(j==0 && i!=0){	// nested
							//	xml.AddElem(MCD_T("MINFEAT"),cc_data.lut_adas[i].lut_classifiers[j].dmin_featValue);
							//	xml.AddElem(MCD_T("MAXFEAT"),cc_data.lut_adas[i].lut_classifiers[j].dmax_featValue);
							//}
							//else{               // not nested
							xml.AddElem(MCD_T("MINFEAT"), wfsNodes[i].weakClassifiers[j].min_featValue);
							xml.AddElem(MCD_T("MAXFEAT"), wfsNodes[i].weakClassifiers[j].max_featValue);

							for(int k=0; k<wfsNodes[i].weakClassifiers[j].getSparseFeature().size; k++)
							{
								xml.AddElem( MCD_T("FEATURE"));
								xml.SetAttrib( MCD_T("f_num"),k);
								xml.IntoElem();
								xml.AddElem( MCD_T("ID"), wfsNodes[i].weakClassifiers[j].getSparseFeature().id[k]);
								xml.AddElem( MCD_T("SIGN"), wfsNodes[i].weakClassifiers[j].getSparseFeature().sign[k]);
								xml.OutOfElem();
							}

							//}

							xml.AddElem( MCD_T("CONFIDENCE"));
							xml.IntoElem();
								for(int m=0; m<K_LUTBINS; m++)
								{
									xml.AddElem( MCD_T("VECTOR_CONF"), m ); //,cc_data.lut_adas[i].lut_classifiers[j].conf[m]);
									xml.IntoElem();

			//						xml.SetAttrib( MCD_T("c_num"),m);
									for(int c=0; c<wfsNodes[i].nc; c++) {
										xml.AddElem( MCD_T("CONF_VALUE"), wfsNodes[i].weakClassifiers[j].vecConfTable[m][c] );
										xml.SetAttrib( MCD_T("c_num"), c);
									}
									xml.OutOfElem();
									//xml.AddElem( MCD_T("CONF"),cc_data.lut_adas[i].lut_classifiers[j].conf[m]);
									//xml.SetAttrib( MCD_T("c_num"),m);
								}
							
							xml.OutOfElem();
						}
						xml.OutOfElem();
					}
					xml.OutOfElem();
				}

			} else {
				// record id of single cascade
				xml.SetAttrib( MCD_T("BINARY_NODE"),wfsNodes[i].cascadeId);

			}
		} else { // single cascade
			xml.SetAttrib( MCD_T("FINAL_POSE"),wfsNodes[i].finalPose);
			xml.SetAttrib( MCD_T("STAGE"),wfsNodes[i].cascadeStage);
			xml.SetAttrib( MCD_T("IS_FINAL_STAGE"),wfsNodes[i].isFinalStage);

			if (is_trained) {
				xml.IntoElem();
				for(int j=0; j<(int)wfsNodes[i].weakClassifiers.size(); j++)
				{
					xml.AddElem( MCD_T("WEAK_CLASSIFIER"));
					xml.SetAttrib( MCD_T("l_num"),j);
					xml.IntoElem();
					{
						//if(j==0 && i!=0){	// nested
						//	xml.AddElem(MCD_T("MINFEAT"),cc_data.lut_adas[i].lut_classifiers[j].dmin_featValue);
						//	xml.AddElem(MCD_T("MAXFEAT"),cc_data.lut_adas[i].lut_classifiers[j].dmax_featValue);
						//}
						//else{               // not nested
						xml.AddElem(MCD_T("MINFEAT"), wfsNodes[i].weakClassifiers[j].min_featValue);
						xml.AddElem(MCD_T("MAXFEAT"), wfsNodes[i].weakClassifiers[j].max_featValue);

						for(int k=0; k<wfsNodes[i].weakClassifiers[j].getSparseFeature().size; k++)
						{
							xml.AddElem( MCD_T("FEATURE"));
							xml.SetAttrib( MCD_T("f_num"),k);
							xml.IntoElem();
							xml.AddElem( MCD_T("ID"), wfsNodes[i].weakClassifiers[j].getSparseFeature().id[k]);
							xml.AddElem( MCD_T("SIGN"), wfsNodes[i].weakClassifiers[j].getSparseFeature().sign[k]);
							xml.OutOfElem();
						}

						xml.AddElem( MCD_T("CONFIDENCE"));
						xml.IntoElem();
							for(int m=0; m<K_LUTBINS; m++)
							{
								xml.AddElem( MCD_T("CONF_VALUE"), wfsNodes[i].weakClassifiers[j].conf_table[m] );
								xml.SetAttrib( MCD_T("c_num"), m);
							}
						
						xml.OutOfElem();
					}
					xml.OutOfElem();
				}
				xml.OutOfElem();
			}
		}
	}
	xml.OutOfElem();

	bool result = xml.Save(s2ws(filename));
	if(result)
		return 0;
	cerr<<"failed to save xml";
	return -1;
}

int WfsMvfdTrainer::load(const string& filename)  
{
	CMarkup xml;
	bool result = xml.Load(s2ws(filename));
	if(!result){
		cerr<<"failed to load xml";
		return -1;
	}

	wfsNodes.clear();

	xml.FindElem(MCD_T("MVFD_VECTOR_BOOSTING_CASCADE"));
	xml.IntoElem();

	while(xml.FindElem(MCD_T("NODE")))
	{
		int node_id = ws2i(xml.GetAttrib(MCD_T("NODE_ID")));
		bool is_branching = ws2b(xml.GetAttrib(MCD_T("IS_BRANCHING")));
		bool is_trained = ws2b(xml.GetAttrib(MCD_T("IS_TRAINED")));

		int final_pose = 0;
		int nc = 0;
		int cascade_id = 0;

		vector<WfsWeakClassifier> weak_list;
		weak_list.clear();

		if (is_branching) {
			final_pose = ws2i(xml.GetAttrib(MCD_T("FINAL_POSE")));
			
			if (final_pose == 0) {
				nc = ws2i(xml.GetAttrib(MCD_T("NUM_BRANCHES")));

				if (is_trained) {
					xml.IntoElem(); // node
					while(xml.FindElem(MCD_T("WEAK_CLASSIFIER"))) {
						int weak_id = ws2i(xml.GetAttrib(MCD_T("l_num")));
						SparseFeature sf;
						int minf = 0;
						int maxf = 0;
						double **conf_table = Make2dDoubleArray(K_LUTBINS, nc);

						xml.IntoElem(); // weak classifier

						xml.FindElem(MCD_T("MINFEAT"));
						minf = ws2i(xml.GetData());
						xml.FindElem(MCD_T("MAXFEAT"));
						maxf = ws2i(xml.GetData());
						
						while(xml.FindElem(MCD_T("FEATURE")))
						{
							sf.size++;
							xml.IntoElem(); // Feature
							xml.FindElem(MCD_T("ID"));
							sf.id.push_back(ws2i(xml.GetData()));
							xml.FindElem(MCD_T("SIGN"));
							sf.sign.push_back(ws2i(xml.GetData())!=0);
							xml.OutOfElem(); // Feature
						}

						{
							xml.FindElem(MCD_T("CONFIDENCE"));
							xml.IntoElem(); // Confidence
							for(int i=0;i<K_LUTBINS;i++)
							{
								xml.FindElem(MCD_T("VECTOR_CONF"));
		//						lutc_data.conf[i] = ws2f(xml.GetData());
								xml.IntoElem();

								for(int c=0; c<nc; c++) {
									xml.FindElem( MCD_T("CONF_VALUE") );
									double val = ws2f(xml.GetData());
									int cid = ws2i(xml.GetAttrib( MCD_T("c_num") ));

									conf_table[i][cid] = val;
								}
								xml.OutOfElem();
							}
							xml.OutOfElem(); // Confidence
						}

						xml.OutOfElem();  // weak classifier

						WfsWeakClassifier weak_classifier(sf, BRANCHING, nc);
						weak_classifier.SetBranchingClassifier(conf_table, minf, maxf);
						weak_list.push_back(weak_classifier);

						Delete2dDoubleArray(&conf_table, K_LUTBINS);
					} // end of while
					xml.OutOfElem();
				}
			} else { // if else final_pose == 0
				cascade_id = ws2i(xml.GetAttrib(MCD_T("BINARY_NODE")));
			}

			// create node
			int parent_id = WfsTreeInfo[node_id][0];
			WfsStrongClassifier node(node_id, parent_id, BRANCHING, nc );
			node.weakClassifiers = weak_list;
			node.is_trained = is_trained;
			node.cascadeId = cascade_id;
			wfsNodes.push_back(node);


		} else { // not branching
			final_pose = ws2i(xml.GetAttrib(MCD_T("FINAL_POSE")));
			int cascade_stage = ws2i(xml.GetAttrib(MCD_T("STAGE")));
			bool is_final_stage = ws2b(xml.GetAttrib(MCD_T("IS_FINAL_STAGE")));
			
			if (is_trained) {
				xml.IntoElem(); // node
				while(xml.FindElem(MCD_T("WEAK_CLASSIFIER"))) {
					int weak_id = ws2i(xml.GetAttrib(MCD_T("l_num")));
					SparseFeature sf;
					int minf = 0;
					int maxf = 0;
					double *conf_table = new double [K_LUTBINS];

					xml.IntoElem(); // weak classifier

					xml.FindElem(MCD_T("MINFEAT"));
					minf = ws2i(xml.GetData());
					xml.FindElem(MCD_T("MAXFEAT"));
					maxf = ws2i(xml.GetData());
					
					while(xml.FindElem(MCD_T("FEATURE")))
					{
						sf.size++;
						xml.IntoElem(); // Feature
						xml.FindElem(MCD_T("ID"));
						sf.id.push_back(ws2i(xml.GetData()));
						xml.FindElem(MCD_T("SIGN"));
						sf.sign.push_back(ws2i(xml.GetData())!=0);
						xml.OutOfElem(); // Feature
					}

					{
						xml.FindElem(MCD_T("CONFIDENCE"));
						xml.IntoElem(); // Confidence
						for(int i=0;i<K_LUTBINS;i++)
						{
							xml.FindElem( MCD_T("CONF_VALUE") );
							double val = ws2f(xml.GetData());
							int cid = ws2i(xml.GetAttrib( MCD_T("c_num") ));

							conf_table[cid] = val;
						}
						xml.OutOfElem(); // Confidence
					}

					xml.OutOfElem();  // weak classifier

					WfsWeakClassifier weak_classifier(sf, BINARY, 0);
					weak_classifier.SetBinaryClassifier(conf_table, minf, maxf);
					weak_list.push_back(weak_classifier);

					delete [] conf_table;
				} // end of while
				xml.OutOfElem();
			}

			// create node
			int parent_id = 0; //WfsTreeInfo[node_id][0];
			WfsStrongClassifier node(node_id, parent_id, BINARY, 0 );
			node.weakClassifiers = weak_list;
			node.is_trained = is_trained;
			node.cascadeStage = cascade_stage;
			node.isFinalStage = is_final_stage;
			node.finalPose = final_pose;
			wfsNodes.push_back(node);

		}

		weak_list.clear();
	}

	xml.OutOfElem(); // Cascade		

	return 0;
}

wstring WfsMvfdTrainer::s2ws(const string &s)
{
	std::wstring w_str(s.length(), L'');
	std::copy(s.begin(), s.end(), w_str.begin());
	return w_str;
}

string WfsMvfdTrainer::ws2s(const wstring &ws)
{
	std::string s_str(ws.length(), ' ');
	std::copy(ws.begin(), ws.end(), s_str.begin());
	return s_str;
}

int WfsMvfdTrainer::ws2i(const wstring &ws)
{
	return atoi( ws2s(ws).c_str());
}

bool WfsMvfdTrainer::ws2b(const wstring &ws)
{
	return (1 == atoi( ws2s(ws).c_str()));
}

double WfsMvfdTrainer::ws2f(const std::wstring &ws)
{
	return atof( ws2s(ws).c_str());
}


