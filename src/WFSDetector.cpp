#include <fstream>
#include <iostream>

#include "WFSDetector.h"
#include "Markup.h"



WfsMvfdDetector::WfsMvfdDetector(const string &xml_file)
{
	poseSets = new ImgSet* [K_WFS_BRANCH_NODES];
	for(int i=0; i<K_WFS_BRANCH_NODES; i++) {
		poseSets[i] = new ImgSet();
	}
	pIH = new ImageHandler();
	load(xml_file);
}

WfsMvfdDetector::~WfsMvfdDetector()
{
	for(int i=0; i<K_WFS_BRANCH_NODES; i++) {
		delete poseSets[i];
	}
	delete [] poseSets;

	if (pIH)
		delete pIH;
}

void WfsMvfdDetector::CheckPass(vector<WfsFaceInfo> &faces) 
{
	//int n = poseSets[0]->Size();

	//for(int i=0; i<n; i++) {
	//	poseSets[0]->GetGranVec(i)->active = true;
	//}

	/// check branching node first

	for(int i=0; i<K_WFS_BRANCH_NODES; i++) {
		if (wfsNodes[i].finalPose == 0) {
			int n = poseSets[i]->Size();

			poseSets[i]->PrepareDetection();
			vector<ImgSet*> child_set;

			for(int j=0; j<WfsTreeInfo[i][1]; j++) {
				child_set.push_back(poseSets[WfsTreeInfo[i][2+j]]);
			}

			wfsNodes[i].CheckStagePassForDetectionBranching(poseSets[i], child_set);
		} else {

		}
	}


	// check single cascades
	for(int i=0; i<K_WFS_BRANCH_NODES; i++) {
		if (wfsNodes[i].finalPose == 0) {
		} else {
			ImgSet *set = poseSets[i];
			int n = set->Size();
			if (n > 0) {
				int cas_id = wfsNodes[i].cascadeId;
				int pose = wfsNodes[i].finalPose;

	//			cout << "pass branching " << pose << " " << n << "\n";
				for(int j=0; j<n; j++) {
					set->GetGranVec(j)->active = true;
					GranVec *gv = set->GetGranVec(j);
					DEB_SHOW(gv, "gv")
				}

				// or j < cas_id + max stage
				for(int j=cas_id; j<wfsNodes.size() && wfsNodes[j].finalPose == pose; j++) {
					// check through each stage
					wfsNodes[j].CheckStagePassForDetectionBinary(set);
					set->DeleteInactiveGranVecs(false);

					if (set->Size() == 0) break;
				}

//				if (set->Size() != n) cout << " single cas works\n";

				for(int j=0; j<set->Size(); j++) {
					if (set->GetGranVec(j)->active) {
						WfsFaceInfo face;
						face.x = set->GetGranVec(j)->x;
						face.y = set->GetGranVec(j)->y;
						face.pose = pose;

						faces.push_back(face);
					}
				}
			}
		}
	}


}

int WfsMvfdDetector::DetectFace(vector<WfsFaceInfo> &faces, IplImage *image, double scale_factor, int min_shift, int min_size, int max_size)
{
	faces.clear();

	const bool half_processing = true;

	// grayscale
	IplImage *gray;
	pIH->convertIplImgToGrayscale(image, &gray);

	if (half_processing) {
		IplImage *gray_h;

		gray_h = cvCreateImage( cvSize(gray->width >> 1, gray->height >> 1), IPL_DEPTH_8U, 1 );
		cvResize(gray, gray_h, CV_INTER_LINEAR );

		cvReleaseImage(&gray);
		gray = gray_h;
	}


//	cvNamedWindow("gray2");
	// JS : this should be changed later - doing this once somewhere before processing frames of fixed size

	int org_wh = min(gray->width, gray->height);
	int wh = org_wh;

	int w = gray->width;
	int h = gray->height;

	int nS = 0;
	vector<int> ws;
	vector<int> hs;
	vector<int> ss;
	vector<int> js;
	int s = K_WIN_SIZE;
	int jump = min_shift;

	while(w > (K_WIN_SIZE << 1) && h > (K_WIN_SIZE << 1)) 
	{
		ws.push_back(w);
		hs.push_back(h);
		ss.push_back(s);
		js.push_back(jump);

		w = w / scale_factor;
		h = h / scale_factor;
		s = K_WIN_SIZE * image->width / w;
		jump = max(1, min_shift * w / image->width);
		nS++;
	}

	for(int i=0; i<K_WFS_BRANCH_NODES; i++) {
		poseSets[i]->ClearSet();
	}

	for(int i=0; i<nS; i++) {
		IplImage *gray2;

		if (i > 0) {
			gray2 = cvCreateImage( cvSize(ws[i], hs[i]), IPL_DEPTH_8U, 1 );
			cvResize(gray, gray2, CV_INTER_LINEAR );
		} else {
			gray2 = gray;
		}

		//cout << "scale " << i << " " << ws[i] << " " << hs[i] << " win" << ss[i] << "\n";
		//cvResizeWindow("gray2", ws[i], hs[i]);
		//cvShowImage("gray2", gray2);
		//cvWaitKey(10000);

		// collect granvec
		//ImgSet imgset;
		DEB_SHOW(gray2, "gray2");

		pIH->getAllGranVecInAScaleFromIplImg(*poseSets[0], gray2, ws[i], hs[i], js[i]);

		//checkPass(&imgset);

		vector<WfsFaceInfo> new_faces;

		CheckPass(new_faces);

		for(int j=0; j<(int)new_faces.size(); j++) {
			WfsFaceInfo face;
			face.x = new_faces[j].x * image->width / ws[i];
			face.y = new_faces[j].y * image->height / hs[i];
			face.pose = new_faces[j].pose;
			face.size = ss[i];

			faces.push_back(face);
		}

//		cout << imgset.Size() << " found \n";
		
		//if (0 && imgset.Size() > 0)
		//{
		//	IplImage *temp = cvCloneImage(image);
		//	cvNamedWindow("resc");
		//	cvResizeWindow("resc", image->width, image->height);

		//	for(int j=0; j<imgset.Size(); j++)
		//	{
		//		int x = imgset.GetGranVec(j)->x * image->width / ws[i];
		//		int y = imgset.GetGranVec(j)->y * image->height / hs[i];

		//		cvDrawRect(temp, cvPoint(x,y), cvPoint(x+ss[i],y+ss[i]), cvScalar(0, 0, 255)); 
		//	}

		//	cvShowImage("resc", temp);
		//	cvWaitKey(10000);

		//	for(int j=0; j<imgset.Size(); j++)
		//	{
		//		checkGranVec(imgset.gran_vec[j]);
		//	}

		//	cvReleaseImage (&temp);
		//}

		//for(int j=0; j<poseSets[0]->Size(); j++)
		//{
		//	//cv::Rect rect;
		//	//rect.x = imgset.GetGranVec(j)->x * image->width / ws[i];
		//	//rect.y = imgset.GetGranVec(j)->y * image->height / hs[i];
		//	//rect.width = ss[i];
		//	//rect.height = ss[i];
		//	//objects.push_back(rect);

		//	if (poseSets[0]->GetGranVec(j)->active) {
		//		WfsFaceInfo face;
		//		face.x = poseSets[0]->GetGranVec(j)->x * image->width / ws[i];
		//		face.y = poseSets[0]->GetGranVec(j)->y * image->height / hs[i];
		//		face.pose = poseSets[0]->GetGranVec(j)->poseId;
		//		face.size = ss[i];

		//		faces.push_back(face);
		//	}
		//}

		poseSets[0]->ClearSet(true);
		for(int j=1; j<K_WFS_BRANCH_NODES; j++) {
			poseSets[j]->ClearSet(false);
		}

		if (i > 0) {
			cvReleaseImage( &gray2);
		}
	}

	cvReleaseImage(&gray);

	return faces.size();

}




//
//void WfsMvfdDetector::detectMultiScaleOrg(IplImage *image, std::vector<Rect> &objects, double scaleFactor, int minShift, int flags, int minSize, int maxSize)
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

//void WfsMvfdDetector::detectMultiScale(IplImage *image, std::vector<Rect> &objects, double scaleFactor, int minShift, int flags, int minSize, int maxSize)
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




wstring WfsMvfdDetector::s2ws(const string &s)
{
	std::wstring w_str(s.length(), L'');
	std::copy(s.begin(), s.end(), w_str.begin());
	return w_str;
}

string WfsMvfdDetector::ws2s(const wstring &ws)
{
	std::string s_str(ws.length(), ' ');
	std::copy(ws.begin(), ws.end(), s_str.begin());
	return s_str;
}

int WfsMvfdDetector::ws2i(const wstring &ws)
{
	return atoi( ws2s(ws).c_str());
}
double WfsMvfdDetector::ws2f(const std::wstring &ws)
{
	return atof( ws2s(ws).c_str());
}


bool WfsMvfdDetector::ws2b(const wstring &ws)
{
	return (1 == atoi( ws2s(ws).c_str()));
}

int WfsMvfdDetector::load(const string& filename)  
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
