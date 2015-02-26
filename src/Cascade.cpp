//#include "../include/Cascade.h"
//#include "../include/LUTAda.h"
//Cascade::Cascade(void)
//{
//}
//
//Cascade::Cascade(TrainInfo info)
//{
//	this->info = info;
//	p_imghandler = new ImageHandler;
//}
//
//
//Cascade::~Cascade(void)
//{
//	delete p_imghandler;
//}
//
//int Cascade::train()
//{
//	int result = 0;
//	ImgSet *p_posset = new ImgSet;
//	ImgSet *p_negset = new ImgSet;
//
//	p_posset->num_img = 0;
//	p_negset->num_img = 0;
//
//	for(int stage=0; stage<info.num_stage; stage++)
//	{
//		crtStage = stage;
//
//		cout<<"TRAIN cascade "<< stage << endl;
//		cout<<"              collect pos samples "<< endl;
//		while(p_posset->num_img<info.train_num_pos){
//			result = Cascade::collectPos(info.train_num_pos, stage, p_posset);
//			if(result<0){
//				cerr<<"collect pos failed"<<endl;
//				return -1;
//			}
//		}
//
//		cout<<"              collect neg samples "<< endl;
//		while(p_negset->num_img<info.train_num_neg){
//			result = Cascade::collectNeg(info.train_num_neg, stage, p_negset);
//			if(result<0){
//				cerr<<"collect neg failed"<<endl;
//				return -1;
//			}
//		}
//		p_imghandler->shrinkImgSet(p_posset, info.train_num_pos);
//		p_imghandler->shrinkImgSet(p_negset, info.train_num_neg);
//
//		{
//			double w = 1.0 / (p_posset->num_img + p_negset->num_img);
//			for(int i=0; i<p_posset->num_img; i++) {
//				p_posset->gran_vec[i].weight = w;
//			}
//			for(int i=0; i<p_negset->num_img; i++) {
//				p_negset->gran_vec[i].weight = w;
//			}
//		}
//
//		//p_imghandler->initWeight(p_posset);
//		//p_imghandler->initWeight(p_negset);
//		
//		//for(int i=0; i<p_posset->num_img; i++) {
//		//	checkGranVec(p_posset->gran_vec[i]);
//		//}
//
//		p_imghandler->mix(p_posset);
//		p_imghandler->mix(p_negset);
//		
//		
//		LUTAda lutada(stage);
//		LUTAdaData lutada_data;
//		lutada.train(p_posset, p_negset, info.max_false_alarm, info.min_detection_rate, &lutada_data, info.threshold); 
//		v_lutada.push_back(lutada);
//
//		v_lutada_data.push_back(lutada_data);
//		p_imghandler->cleanImgSet(p_posset);
//		p_imghandler->cleanImgSet(p_negset);
//		cout<<"passed pos image num "<<p_posset->num_img<<" passed neg image num "<<p_negset->num_img<<"\n";
//		save("./result/output_middle.xml");
//	}
//
//	delete p_posset;
//	delete p_negset;
//	return 0;
//}
//
//int Cascade::initializeLUTAda()   // initialize all strong classifiers in cascade, need to load xml first.
//{
//	for(int i=0;i<Cascade::num_stage;i++){
//		LUTAda lut_ada;
//		lut_ada.setData(&Cascade::v_lutada_data[i]);
//		Cascade::v_lutada.push_back(lut_ada);
//	}
//	return 0;
//}
///*
//int Cascade::runAt(Point point)          // need to initialize and setImage before runAt
//{
//	classifiers[0].setLocation(point);    // this example only run for 1 stage
//	int isface = classifiers[0].predict();
//}
//*/
//int Cascade::load(const string& filename)   // load the xml into classifier_data
//{
//	XMLHandler xml_h;
//	CascadeData cc_data;
//	int result = xml_h.load(filename, cc_data);
//	if(result<0)
//		return -1;
//	Cascade::v_lutada_data = cc_data.lut_adas;
//	Cascade::num_stage = cc_data.num_stage;
//	return 0;
//}
//
//int Cascade::save(const string &filename)
//{
//	XMLHandler xml_h;
//	CascadeData cc_data;
//	cc_data.lut_adas = Cascade::v_lutada_data;
//	cc_data.num_stage = Cascade::v_lutada_data.size();//:info.num_stage;
//	xml_h.save(filename, cc_data);
//	return 0;
//}
//
//int Cascade::collectNeg(int num, int stage, ImgSet *p_negset)
//{
//	cout << "        col neg " << num << " " << stage << "\n";
//
//	if(stage == 0)
//	{
//		return p_imghandler->getNonFace(info.dir_neg, p_negset, num);
//		//return p_imghandler->openNegDir(info.dir_neg, p_negset, num, false);
//		 
//	}
//	int num_collected = p_negset->num_img;
//	int num_needed = num - p_negset->num_img;
//
////	int qqq = 0;
//
//	while(num_collected<num)
//	{
//		ImgSet neg_temp;
//		neg_temp.num_img = 0;
//		int result = collectNeg((int)(num_needed*4), stage-1, &neg_temp);
//		if( result <0)
//			return -1;
//
//		//cout << qqq++ << " \n";
//		//checkGranVec(neg_temp.gran_vec[0]);
//
//		v_lutada[stage-1].checkPass(&neg_temp);
//		p_imghandler->cleanImgSet(&neg_temp);
//
//		num_collected += neg_temp.num_img;
//
//		if (crtStage == stage) {
//			cout << "   col neg " << neg_temp.num_img << " " << num_collected << "\n";
//		}
//
//		num_needed = num - num_collected;
//		//cout<<"neg coll "<<num_collected<<" stage "<<stage<<" ";
//		p_imghandler->mergeImgSet(p_negset, &neg_temp);
//
//		
//	}
//	return num_collected;
//}
//
//int Cascade::collectPos(int num, int stage, ImgSet *p_posset)
//{
//	if(stage == 0)
//	{
//		return p_imghandler->openDir(info.dir_pos, p_posset, num, true);	 
//	}
//	int num_collected = p_posset->num_img;
//	int num_needed = num - p_posset->num_img;
//	while(num_collected<num)
//	{
//		ImgSet pos_temp; 
//		pos_temp.num_img = 0;
//		int result = collectPos((int)(num_needed*1.2), stage-1, &pos_temp);
//		if( result <0)
//			return -1;
//		v_lutada[stage-1].checkPass(&pos_temp);
//		p_imghandler->cleanImgSet(&pos_temp);
//
//		num_collected += pos_temp.num_img;
//		num_needed = num - num_collected;
//		//cout<<"pos coll "<<num_collected<<" stage "<<stage<<" ";
//		p_imghandler->mergeImgSet(p_posset, &pos_temp);
//		
//	}
//	return num_collected;
//}
//
//void Cascade::detectMultiScaleOrg(IplImage *image, std::vector<Rect> &objects, double scaleFactor, int minShift, int flags, int minSize, int maxSize)
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
//		p_imghandler->CropRectFromImage(image, imgset, scale[round], minShift);
//		//p_imghandler->saveDir("./result_test/",&imgset);
//		//system("pause");
//		cout<<"size "<<scale[round]<< " numofimg "<< imgset.num_img<<endl;
//		Cascade::checkPass(&imgset);
//		cout<<"size "<<scale[round]<<" finished"<<endl;
//		for(int j=0; j<imgset.num_img; j++)
//		{
//			cv::Rect rect;
//			rect.x = imgset.gran_vec[j].x;
//			rect.y = imgset.gran_vec[j].y;
//			rect.width = imgset.gran_vec[j].size;
//			rect.height = imgset.gran_vec[j].size;
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
//void Cascade::detectMultiScale(IplImage *image, std::vector<Rect> &objects, double scaleFactor, int minShift, int flags, int minSize, int maxSize)
//{
//	objects.clear();
//
//	const bool half_processing = false;
//
//	// grayscale
//	IplImage *gray;
//
//	p_imghandler->convertIplImgToGrayscale(image, &gray);
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
//		p_imghandler->getAllGranVecInAScaleFromIplImg(imgset, gray2, ws[i], hs[i], js[i]);
//
//		Cascade::checkPass(&imgset);
//		cout << imgset.num_img << " found \n";
//		
//		if (0 && imgset.num_img > 0)
//		{
//			IplImage *temp = cvCloneImage(image);
//			cvNamedWindow("resc");
//			cvResizeWindow("resc", image->width, image->height);
//
//			for(int j=0; j<imgset.num_img; j++)
//			{
//				int x = imgset.gran_vec[j].x * image->width / ws[i];
//				int y = imgset.gran_vec[j].y * image->height / hs[i];
//
//				cvDrawRect(temp, cvPoint(x,y), cvPoint(x+ss[i],y+ss[i]), cvScalar(0, 0, 255)); 
//			}
//
//			cvShowImage("resc", temp);
//			cvWaitKey(10000);
//
//			for(int j=0; j<imgset.num_img; j++)
//			{
//				checkGranVec(imgset.gran_vec[j]);
//			}
//
//			cvReleaseImage (&temp);
//		}
//
//		for(int j=0; j<imgset.num_img; j++)
//		{
//			cv::Rect rect;
//			rect.x = imgset.gran_vec[j].x * image->width / ws[i];
//			rect.y = imgset.gran_vec[j].y * image->height / hs[i];
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
//
//void Cascade::checkPass(ImgSet *p_set)
//{
//	for(int i=0;i<v_lutada.size() && p_set->num_img>0;i++)
//	{
//		v_lutada[i].checkPass(p_set);
//		p_imghandler->cleanImgSet(p_set);
//	}
//}
//
//void Cascade::checkFeatures()
//{
//	for(int i=0;i<v_lutada_data.size();i++)
//	{
//		for(int j=0;j<v_lutada_data[i].lut_classifiers.size();j++)
//		{
//			if(i==0 || j!=0)
//				checkFeature(v_lutada_data[i].lut_classifiers[j].features);
//		}
//	}
//}
