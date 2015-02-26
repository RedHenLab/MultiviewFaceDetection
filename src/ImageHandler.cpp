#include "../include/ImageHandler.h"
#include "time.h"


ImageHandler::ImageHandler(void):_cur_pos_img(0),_cur_neg_img(0),cur_win_size(MIN_CROP_WIN),cur_x(0),cur_y(0),_neg_img_count(0)
{
}

ImageHandler::~ImageHandler(void)
{
}



// int node_id : node id of branching node. It will only read faces associated with poses (sub)included in the current node
int ImageHandler::ReadPositiveSamplesFromList(ImgSet *p_imgset, vector<string> &file_list, vector<int> &pose_list, int& list_offset, int node_id, int num_needed, int given_pose)
{
	if (list_offset > (int)file_list.size()) return -1;

	vector<bool> pose_of_interests;

	pose_of_interests.resize(K_WFS_POSES+1, false);

	if (given_pose > 0) {
		pose_of_interests[given_pose] = true;
	} else {
		for(int i=1; i<=K_WFS_POSES; i++) {
			if (WfsTreeStructure[node_id][i] != 0) {
				pose_of_interests[i] = true;
			}
		}
	}

	// read
	int num_collected = 0;

	SparseFeature af;

	af.id.push_back(200);
	af.id.push_back(400);
	af.id.push_back(700);
	af.id.push_back(1200);

	af.sign.push_back(true);
	af.sign.push_back(false);
	af.sign.push_back(true);
	af.sign.push_back(false);

	af.size = 4;

	for(int i=list_offset; i<(int)file_list.size(); i++) {
		if (pose_of_interests[pose_list[i]]) {
			ImgVec imgvec;
			GranVec* granvec = new GranVec();
			int result = LoadImgVecFromImageFile(file_list[i], imgvec);
			if(result!=0)
				return -1;

			int f1 = 0;
			{
				//test
				ImgVec iv2(imgvec);
				double stdd = normalizeImgVec(iv2);

				GranVec agv;
				getGranFromImg(imgvec, agv);
				agv.stdv = stdd;


				f1 = EvaluateFeature(af, agv);

				DWORD tstart, tend, tdif;
				tstart = GetTickCount();
				int ggg = 0;
				for(int q=0; q<10000000; q++) {
					agv.stdv += 0.01;
					ggg += EvaluateFeature(af, agv);
				}
				tend = GetTickCount();

				tdif = tend - tstart;
				cout<< ggg << "total training time "<<tdif<<" seconds"<<endl;
			}
			{
				DWORD tstart, tend, tdif;
				tstart = GetTickCount();
				int ggg = 0;
				for(int q=0; q<100000; q++) {
					normalizeImgVec(imgvec);
				}
				tend = GetTickCount();

				tdif = tend - tstart;
				cout<< ggg << "total training time "<<tdif<<" seconds"<<endl;

			}
			normalizeImgVec(imgvec);
//			checkImgVec(imgvec);

			getGranFromImg(imgvec, *granvec);
//			checkGranVec(granvec);

			int f2 = EvaluateFeature(af, *granvec);

			cout << f1 << " " << f2 << " " << abs(f1-f2) << "\n";

			DWORD tstart, tend, tdif;
			tstart = GetTickCount();
			int ggg = 0;
			for(int q=0; q<10000000; q++) {
//				granvec->val[0] = q % 100;
				ggg += EvaluateFeature(af, *granvec);
			}
			tend = GetTickCount();

			tdif = tend - tstart;
			cout<< ggg << "total training time "<<tdif<<" seconds"<<endl;

			granvec->active = true;
			granvec->label = true;

			granvec->poseId = pose_list[i];
			if (given_pose > 0) {
				granvec->category = -1;
			} else {
				granvec->category = WfsTreeCategoryId[node_id][pose_list[i]];
			}

			p_imgset->AddGranVec(granvec);

			num_collected++;

			if (num_collected >= num_needed) {
				list_offset = i+1;
				return num_collected;
			}
			//cout<<"coll "<<num_collected<<" need "<<num_needed<<" ";
			//_cur_pos_img++;
		}
	}

	list_offset = (int)file_list.size();
	return num_collected;

}


int ImageHandler::ReadNegativeSamplesFromList(ImgSet *p_imgset, vector<string> &file_list, int& list_offset, int num_needed, int max_samples_per_image)
{
	// read
	int num_collected = 0;

	if (list_offset > (int)file_list.size()) return -1;

	// maybe it should be randomly chosen...

	for(int i=list_offset; i<(int)file_list.size(); i++) {
		IplImage *img = cvLoadImage( file_list[i].c_str() );

		if (img) {
			if (img->width < (K_WIN_SIZE << 3) || img->height < (K_WIN_SIZE << 3)) {
				cvReleaseImage(&img);
				continue;
			}

			ImgSet newset;

			if (max_samples_per_image > 0) {
				CropRandomSamplesFromImage(&newset, img, max_samples_per_image);
			} else {
				double scale_jump = 1.25;
				int min_shift = 8;

				if (img->width > 1200 || img->height > 1200) {
					//scale_jump = 2.0;
					//min_shift = max(img->width, img->height) / 40;
					scale_jump = 1.4;
					min_shift = max(img->width, img->height) / 120;
				}
				CropAllSamplesFromImage(&newset, img, scale_jump, min_shift);
				//cout << newset.Size() << "\n";
			}

			cvReleaseImage(&img);

			if (newset.Size() > 0) {

				for(int j=0; j<newset.Size(); j++) {
					newset.GetGranVec(j)->active = true;
					newset.GetGranVec(j)->label = false;
				}

				if (max_samples_per_image > 0 && newset.Size() > max_samples_per_image) {
					for(int j=0; j<max_samples_per_image; j++) {
						int r = rand() % max_samples_per_image;
						p_imgset->AddGranVec(newset.GetGranVec(r), true);
						num_collected++;
					}
					newset.ClearSet();
				} else {
					num_collected += newset.Size();
					p_imgset->MergeSet(newset);
				}

				if (num_collected >= num_needed) {
					list_offset = i+1;
					return num_collected;
				}
			}
		}
//		cout<<"coll "<<num_collected<<" need "<<num_needed<<" ";
		//_cur_pos_img++;
	}

	list_offset = (int)file_list.size();
	return num_collected;

}

void ImageHandler::NormalizeGranVecs(ImgSet &set)
{
	for(int i=0; i<set.Size(); i++) {
		if (!set.GetGranVec(i)->normalized) {
			// stdv was set

			ImgVec img_vec;

			getImgFromGran(*set.GetGranVec(i), img_vec);
			normalizeImgVec(img_vec);
			getGranFromImg(img_vec, *set.GetGranVec(i));

//			gran_list[i].normalized = true;
		}
	}
}

//
//int ImageHandler::CropNonFaceFromImageFile(std::string& filename, ImgSet &is, int num_needed)
//{
//	IplImage *img = cvLoadImage( filename.c_str() );
//	//IplImage *img2;
//
//	if (!img) {
//		cout << "CropNonFaceFromImageFile : image fail : " << filename << "\n";
//		cur_y = 0;
//		cur_x = 0;
//		cur_win_size = MIN_CROP_WIN;
//		ImageHandler::_cur_neg_img++;
//		return -1;
//	} else {
////		cout << "CropNonFaceFromImageFile : image     : " << filename << "\n";
//	}
//
//	if (num_needed==0) return 0;
//
//	int count = 0;
//
//	for(cur_win_size; cur_win_size<=min(img->width,img->height); cur_win_size+=10)
//	{
//		for(cur_x; cur_x<=img->width-cur_win_size; cur_x+=2)
//		{
//			for(cur_y; cur_y<=img->height-cur_win_size; cur_y+=2)
//			{
//				
//				cvSetImageROI(img, cvRect(cur_x, cur_y, cur_win_size, cur_win_size));
//				IplImage *img_window = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
//				IplImage *img_size24 = NULL;
//
//				cvCopy(img, img_window, NULL);
//
//				ImgVec imgvec; 
//				GranVec granvec;
//
//				resizeIplImgToWindowSize(img_window, &img_size24);
//				convertIplImgToImgVec(img_size24, imgvec);
//				
//				normalizeImgVec(imgvec);
//
//				cvReleaseImage(&img_window);
//				cvReleaseImage(&img_size24);
//
//				getGranFromImg(imgvec, granvec);
//
//				is.gran_vec.push_back(granvec);
//				is.gran_vec[count].active = true;
//				is.gran_vec[count].label = false;
//				is.gran_vec[count].weight = 0;
//				is.gran_vec[count].conf = 0;
//
//				cvResetImageROI(img);
//				count ++;
//				_neg_img_count ++;
//				if(count >= num_needed){			// return if achieve goal
//					is.num_img = count;
//					cvReleaseImage(&img);
//					cur_y+=2;
//					return count;
//				}
//				
//			}
//			cur_y = 0;
//		}
//		cur_x = 0;
//	}
//	cur_win_size = MIN_CROP_WIN;
//	ImageHandler::_cur_neg_img++;
//	is.num_img = count;
//	cvReleaseImage(&img);
//	return count;
//}
//
//int ImageHandler::CropRectFromImage(IplImage *o_img, ImgSet &is, int rectsize, int minshift)
//{
//	if (!o_img) return -1;
//
//	double scale = K_WIN_SIZE/double(rectsize);
//	CvSize new_size;
//	new_size.height = (int)(o_img->height*scale);
//	new_size.width = (int)(o_img->width*scale);
//	CvSize win_size;
//	win_size.height = K_WIN_SIZE;
//	win_size.width = K_WIN_SIZE;
//
//
//	IplImage *img = cvCreateImage(new_size, o_img->depth, o_img->nChannels);
//	cvResize(o_img, img, CV_INTER_LINEAR );
//	IplImage *img_window = cvCreateImage(win_size, img->depth, img->nChannels);
//
//	is.gran_vec.clear();
//	is.num_img = 0;
//	int count = 0;
//
//	for(int i=0; i<img->width-K_WIN_SIZE; i+=minshift){
//		for(int j=0; j<img->height-K_WIN_SIZE; j+=minshift){
//			count++;
//			cvSetImageROI(img, cvRect(i, j, K_WIN_SIZE, K_WIN_SIZE));
//			
//			cvCopy(img, img_window, NULL);
//
//			ImgVec imgvec; 
//			GranVec granvec;
//
//			convertIplImgToImgVec(img_window, imgvec);
//			
//			getGranFromImg(imgvec, granvec);
//
//			granvec.x = (int)(i/scale);
//			granvec.y = (int)(j/scale);
//			granvec.size = rectsize;
//			granvec.active = true;
//			granvec.conf = 0;
//			granvec.label = false;
//			granvec.weight = 0;
//
//			is.gran_vec.push_back(granvec);
//			cvResetImageROI(img);
//		}
//	}
//	cvReleaseImage(&img_window);
//	cvReleaseImage(&img);
//	is.num_img = is.gran_vec.size();
//	return is.num_img;
//}


int ImageHandler::convertIplImgToImgVec(IplImage *in, ImgVec& out)
{

	if (!in) 
	{
		return -1;
	}

	if (in->width != K_WIN_SIZE || in->height != K_WIN_SIZE) return -1;

	int nc = in->nChannels;

	pixel *op = out.val;
	unsigned char *ipl = (unsigned char*) in->imageData;

	switch(nc) {
		case 1:
			for(int y=0; y<K_WIN_SIZE; y++) {
				unsigned char *ip = ipl;

				for(int x=0; x<K_WIN_SIZE; x++) {
					*op = (pixel) (*ip);
					op++;
					ip++;
				}

				ipl = ipl + in->widthStep;
			}
			break;
		case 2:
			// ?
			return -1;
			break;
		case 3:
			for(int y=0; y<K_WIN_SIZE; y++) {
				unsigned char *ip = ipl;

				for(int x=0; x<K_WIN_SIZE; x++) {
					*op = ((pixel) (ip[0] + ip[1] + ip[2])) / 3;
					op++;
					ip+=3;
				}

				ipl = ipl + in->widthStep;
			}
			break;
		case 4:
			for(int y=0; y<K_WIN_SIZE; y++) {
				unsigned char *ip = ipl;

				for(int x=0; x<K_WIN_SIZE; x++) {
					*op = ((pixel) (ip[0] + ip[1] + ip[2])) / 3;
					op++;
					ip+=4;
				}

				ipl = ipl + in->widthStep;
			}
			break;
		default:
			return -1;
			break;
	}

//	normalizeImgVec(out);

	return 0;

}

int ImageHandler::convertIplImgToGrayscale(IplImage *in, IplImage **out)
{
	if (!in) exit(0);

	*out = cvCreateImage( cvSize(in->width, in->height), IPL_DEPTH_8U, 1 );


	//// temp :)

	int w = in->width;
	int h = in->height;
	int nc = in->nChannels;

	unsigned char *op = (unsigned char *) (*out)->imageData;
	unsigned char *ipl = (unsigned char*) in->imageData;

	switch(nc) {
		case 3:
			for(int y=0; y<h; y++) {
				unsigned char *ip = ipl;

				for(int x=0; x<w; x++) {
					*op = ((ip[0] + ip[1] + ip[2])) / 3;
					op++;
					ip+=3;
				}

				ipl = ipl + in->widthStep;
			}
			break;
		case 4:
			for(int y=0; y<h; y++) {
				unsigned char *ip = ipl;

				for(int x=0; x<w; x++) {
					*op = ((ip[0] + ip[1] + ip[2])) / 3;
					op++;
					ip+=4;
				}

				ipl = ipl + in->widthStep;
			}
			break;
		default:
			return -1;
			break;
	}

	//cvNamedWindow("gray");
	//cvShowImage("gray", *out);
	//cvWaitKey(10000);

	return 0;

	if (strcmp(in->colorModel, "RGB"))
	{
		cvCvtColor(in, *out, CV_RGB2GRAY);
	} else 
	if (strcmp(in->colorModel, "BGR"))
	{
		cvCvtColor(in, *out, CV_BGR2GRAY);
	} else {
		return -1;
	}

	//cvNamedWindow("gray");
	//cvShowImage("gray", *out);
	//cvWaitKey(10000);


	return 0;

}


//int ImageHandler::convertImgVecToIplImg(ImgVec &in, IplImage *out)
//{
//	if(out->nChannels != 1 || out->width != K_WIN_SIZE || out->height != K_WIN_SIZE || out->depth != IPL_DEPTH_8U)
//		return -1;
//
//	pixel *ip = in.val;
//	unsigned char *opl = (unsigned char*) out->imageData;
//	for(int y=0; y<K_WIN_SIZE; y++) {
//		unsigned char *op = opl;
//
//		for(int x=0; x<K_WIN_SIZE; x++) {
//			*op = (int) (*ip);
//			op++;
//			ip++;
//		}
//		opl = opl + out->widthStep;
//	}
//	return 0;
//}

int ImageHandler::resizeIplImgToWindowSize(IplImage *in, IplImage **out) {
	IplImage *out_img = *out;

	if (out_img) {
		cvReleaseImage(&out_img);
	}

	out_img = cvCreateImage( cvSize(K_WIN_SIZE, K_WIN_SIZE), in->depth, in->nChannels );

	cvResize(in, out_img);

	*out = out_img;

	return 0;
}

int ImageHandler::LoadImgVecFromImageFile(std::string& filename, ImgVec &iv) 
{
	IplImage *img = cvLoadImage( filename.c_str() );
	IplImage *img2 = NULL;

	if (!img) return -1;

	resizeIplImgToWindowSize(img, &img2);

	convertIplImgToImgVec(img2, iv);

	cvReleaseImage(&img);
	cvReleaseImage(&img2);

	return 0;
}
//int ImageHandler::SaveImageFileFromImgVec(std::string& filename, ImgVec &iv)
//{
//	IplImage *img = cvCreateImage( cvSize(K_WIN_SIZE, K_WIN_SIZE), IPL_DEPTH_8U, 1 );
//	
//	if (!img) return -1;
//
//	convertImgVecToIplImg(iv, img);
//
//	int result = cvSaveImage(filename.c_str(), img);
//
//	cvReleaseImage(&img);
//
//	return result;
//}


void ImageHandler::getGranFromImg(ImgVec &input, GranVec &result)
{

	// Features at scale 0
	memcpy(result.val, input.val, sizeof(pixel) * K_WIN_NPTS);
	
	pixel *op = &result.val[GV_base_id[1]];
	for(int i=1; i<=K_MAX_SCALE; i++) {
		pixel *ip1 = &result.val[GV_base_id[i-1]];
//		int *ip2 = &result.val[GV_base_id[i-1] + GV_size[i-1]];
		pixel *ip2 = &result.val[GV_base_id[i-1] + (GV_size[i-1] << (i-1))];

		int xg = 1 << (i-1);
		int yn = GV_size[i];

		for(int y=0; y<yn; y++) {
			for(int x=0; x<yn; x++) {
				*op = (ip1[0] + ip1[xg] + ip2[0] + ip2[xg] + 2) >> 2;
				op++;
				ip1++;
				ip2++;
			}

			ip1 += xg;
			ip2 += xg;
		}
	}

	result.normalized = input.normalized;
}

void ImageHandler::getImgFromGran(GranVec &input, ImgVec &result)
{
	memcpy(result.val, input.val, sizeof(pixel) * K_WIN_NPTS);
}


int ImageHandler::CropRandomSamplesFromImage(ImgSet *set, IplImage *image, int num)
{
	const bool half_processing = true;
	const double scaleFactor = 1.4;

	set->ClearSet();

	// grayscale
	IplImage *gray;

	convertIplImgToGrayscale(image, &gray);

	if (half_processing) {
		IplImage *gray_h;

		gray_h = cvCreateImage( cvSize(gray->width >> 1, gray->height >> 1), IPL_DEPTH_8U, 1 );
		cvResize(gray, gray_h, CV_INTER_LINEAR );

		cvReleaseImage(&gray);
		gray = gray_h;
	}

	int org_wh = min(gray->width, gray->height);
	int wh = org_wh;

	int w = gray->width;
	int h = gray->height;

	int nS = 0;
	//vector<int> ws;
	//vector<int> hs;
	vector<int> ss;
	//vector<int> js;
	int s = K_WIN_SIZE;

	while(w > (K_WIN_SIZE << 1) && h > (K_WIN_SIZE << 1)) 
	{
		//ws.push_back(w);
		//hs.push_back(h);
		ss.push_back(s);

		w = w / scaleFactor;
		h = h / scaleFactor;
		s = K_WIN_SIZE * image->width / w;
		nS++;
	}

	w = gray->width;
	h = gray->height;

	IplImage *img_win_size = cvCreateImage( cvSize(K_WIN_SIZE, K_WIN_SIZE), IPL_DEPTH_8U, 1 );

	for(int i=0; i<num; i++) {
		int scale = rand() % nS;
		int x = rand() % (w - ss[scale] - 1);
		int y = rand() % (h - ss[scale] - 1);

		IplImage *img_window_org = cvCreateImage(cvSize(ss[scale], ss[scale]), IPL_DEPTH_8U, 1);

		cvSetImageROI(gray, cvRect(x, y, ss[scale], ss[scale]));		
		cvCopy(gray, img_window_org, NULL);

		cvResize(img_window_org, img_win_size, CV_INTER_LINEAR );

		ImgVec imgvec; 
		GranVec *granvec = new GranVec();

		convertIplImgToImgVec(img_win_size, imgvec);
		
		double stdv = normalizeImgVec(imgvec);
		if (stdv > K_POS_SAMPLE_MIN_STDV) {

			getGranFromImg(imgvec, *granvec);

			//granvec.x = (int)(i/scale);
			//granvec.y = (int)(j/scale);
			//granvec.size = rectsize;
			granvec->active = true;
			granvec->label = false;

			set->AddGranVec(granvec);
		}

		cvResetImageROI(gray);

		cvReleaseImage(&img_window_org);

	}

	cvReleaseImage(&img_win_size);

	cvReleaseImage(&gray);

	return set->Size();

}

int ImageHandler::CropAllSamplesFromImage(ImgSet *set, IplImage *image, double scaleFactor, int minShift)
{
	const bool half_processing = true;

	set->ClearSet();

	// grayscale
	IplImage *gray;

	convertIplImgToGrayscale(image, &gray);

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
	int jump = minShift;

	while(w > (K_WIN_SIZE << 1) && h > (K_WIN_SIZE << 1)) 
	{
		ws.push_back(w);
		hs.push_back(h);
		ss.push_back(s);
		js.push_back(jump);

		w = (int) w / scaleFactor;
		h = (int) h / scaleFactor;
		s = (int) K_WIN_SIZE * image->width / w;
		jump = max(1, minShift * w / image->width);
		nS++;
	}

	for(int i=0; i<nS; i++) {
		IplImage *gray2;

		if (i > 0) {
			gray2 = cvCreateImage( cvSize(ws[i], hs[i]), IPL_DEPTH_8U, 1 );
			cvResize(gray, gray2, CV_INTER_LINEAR );
		} else {
			gray2 = gray;
		}

//		cout << "scale " << i << " " << ws[i] << " " << hs[i] << " win" << ss[i] << "\n";
		//cvResizeWindow("gray2", ws[i], hs[i]);
		//cvShowImage("gray2", gray2);
		//cvWaitKey(10000);

		// collect granvec
		ImgSet imgset;
		getAllGranVecInAScaleFromIplImg(imgset, gray2, ws[i], hs[i], js[i]);

		//cout << imgset.Size() << " found \n";
		//cout << set->Size() << " set \n";

		set->MergeSet(imgset);
		
		
		if (i > 0) {
			cvReleaseImage( &gray2);
		}
	}

	cvReleaseImage(&gray);

	return set->Size();

}


// img is grayscale
void ImageHandler::getAllGranVecInAScaleFromIplImg(ImgSet& img_set, IplImage *img, int w, int h, int jump)
{
	pixel *pyrms[K_MAX_SCALE+1];
	int pyr_w[K_MAX_SCALE+1];
	int pyr_h[K_MAX_SCALE+1];

	for(int i=0; i<=K_MAX_SCALE; i++) {
		pyr_w[i] = w - ((1 << i) - 1);
		pyr_h[i] = h - ((1 << i) - 1);
//		cout << pyr_w[i] << " " << pyr_h[i] << "\n";
	}

	// too small to make pyramids
	if (pyr_w[K_MAX_SCALE] < K_WIN_SIZE || pyr_w[K_MAX_SCALE] < K_WIN_SIZE) return;

	int *integral_img = new int [(w+1)*(h+1)];
	int *integral_sq_img = new int [(w+1)*(h+1)];

	for(int i=0; i<=K_MAX_SCALE; i++) {
		pyrms[i] = new pixel [pyr_w[i] * pyr_h[i]];
	}


	unsigned char *p = (unsigned char *) img->imageData;
	int pp = 0;
	for(int y=0; y<h; y++) {
		for(int x=0; x<w; x++) {
			pyrms[0][pp] = (pixel)p[x];
			pp++;
		}
		p+=img->widthStep;
	}

	// build integral_img
	for(int x=0; x<w+1; x++) {
		integral_img[x] = 0;
		integral_sq_img[x] = 0;
	}
	for(int y=0; y<h+1; y++) {
		integral_img[y*(w+1)] = 0;
		integral_sq_img[y*(w+1)] = 0;
	}
	for(int y=1; y<=h; y++) {
		for(int x=1; x<=w; x++) {
			integral_img[y*(w+1)+x] = integral_img[y*(w+1)+x-1] + pyrms[0][(y-1)*w+(x-1)];
			integral_sq_img[y*(w+1)+x] = integral_sq_img[y*(w+1)+x-1] + pyrms[0][(y-1)*w+(x-1)] * pyrms[0][(y-1)*w+(x-1)];
		}
	}
	for(int x=1; x<=w; x++) {
		for(int y=2; y<=h; y++) {
			integral_img[y*(w+1)+x] += integral_img[(y-1)*(w+1)+x];
			integral_sq_img[y*(w+1)+x] += integral_sq_img[(y-1)*(w+1)+x];
		}
	}


	// build pyramids
	for(int i=1; i<=K_MAX_SCALE; i++) {
		pixel *ip1 = &pyrms[i-1][0];
		pixel *ip2 = &pyrms[i-1][pyr_w[i-1] << (i-1)];

		pixel *op = &pyrms[i][0];

		int xg = 1 << (i-1);

		for(int y=0; y<pyr_h[i]; y++) {
			for(int x=0; x<pyr_w[i]; x++) {
				op[x] = (ip1[x] + ip1[x+xg] + ip2[x] + ip2[x+xg] + 2) >> 2;
			}

			op += pyr_w[i];
			ip1 += pyr_w[i-1];
			ip2 += pyr_w[i-1];
		}
	}

	// check
	//{
	//	cvNamedWindow("pyr");

	//	for(int i=0; i<=K_MAX_SCALE; i++) {
	//		IplImage *test = cvCreateImage( cvSize(pyr_w[i], pyr_h[i]), IPL_DEPTH_8U, 1 );

	//		for(int y=0; y<pyr_h[i]; y++) {
	//			for(int x=0; x<pyr_w[i]; x++) {
	//				test->imageData[x + y * test->widthStep] = (unsigned char) pyrms[i][x+pyr_w[i]*y];
	//			}
	//		}
	//		cvResizeWindow("pyr", pyr_w[i], pyr_h[i] );
	//		cvShowImage("pyr", test);
	//		cvWaitKey(10000);
	//		cvReleaseImage(&test);
	//	}
	//}

	for(int y=0; y<=h - K_WIN_SIZE; y+=jump) {
		for(int x=0; x<=w - K_WIN_SIZE; x+=jump) {
			GranVec *gv = new GranVec();

			int p=0;
			for(int s=0; s<=K_MAX_SCALE; s++) {
				pixel *p_pyr = &pyrms[s][y*pyr_w[s] + x];
				for(int yy=0; yy<GV_size[s]; yy++) {
					for(int xx=0; xx<GV_size[s]; xx++) {
						gv->val[p] = p_pyr[xx];
						p++;
					}
					p_pyr += pyr_w[s];
				}
			}

			gv->x = x;
			gv->y = y;

			// computes variance
			int sum = integral_img[(w+1) * y + x] + integral_img[(w+1) * (y + K_WIN_SIZE) + (x + K_WIN_SIZE)]
				- integral_img[(w+1) * (y + K_WIN_SIZE) + x] - integral_img[(w+1) * y + (x + K_WIN_SIZE)];
			int sq_sum = integral_sq_img[(w+1) * y + x] + integral_sq_img[(w+1) * (y + K_WIN_SIZE) + (x + K_WIN_SIZE)]
				- integral_sq_img[(w+1) * (y + K_WIN_SIZE) + x] - integral_sq_img[(w+1) * y + (x + K_WIN_SIZE)];

			//// check
			//{
			//	int s1 = 0;
			//	int s2 = 0;

			//	int s = 0;
			//	int *p_pyr = &pyrms[s][y*pyr_w[s] + x];
			//	for(int yy=0; yy<GV_size[s]; yy++) {
			//		for(int xx=0; xx<GV_size[s]; xx++) {
			//			s1 += p_pyr[xx];
			//			s2 += p_pyr[xx] * p_pyr[xx];
			//		}
			//		p_pyr += pyr_w[s];
			//	}

			//	if (s1 != sum || s2 != sq_sum) {
			//		cout << s1 << " " << s2 << " " << sum << " " << sq_sum << "\n";
			//			
			//		cout << "integral image sucks\n";
			//		exit(1);
			//	}
			//}

			double mean = (double) sum / K_WIN_NPTS;
			double sq_mean = (double) sq_sum / K_WIN_NPTS;

			gv->stdv = max(1.0, sqrt(sq_mean - mean * mean));
//			cout << gv.stdv  << "\n";

			// if variance is small....
			if (gv->stdv > K_POS_SAMPLE_MIN_STDV) {
				img_set.AddGranVec(gv);
			} else {
				delete gv;
			}

			//{ // check
			//	cout << x << " " << y <<"\n";
			//	IplImage *img2 = cvCloneImage(img);

			//	cvNamedWindow("gv_from");
			//	cvDrawRect(img2, cvPoint(x,y), cvPoint(x+K_WIN_SIZE,y+K_WIN_SIZE), cvScalar(255, 0,0,0));
			//	cvShowImage("gv_from", img2);

			//	cvReleaseImage(&img2);
			//	checkGranVec(gv);
			//}

		}
	}


	for(int i=0; i<=K_MAX_SCALE; i++) {
		delete [] pyrms[i];
	}

	delete [] integral_img;
	delete [] integral_sq_img;

}

