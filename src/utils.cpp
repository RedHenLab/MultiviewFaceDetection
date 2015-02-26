#include "cv.h"
#include "highgui.h"

#include "../include/utils.h"

using namespace std;
using namespace cv;

//extern bool sample_normalized;

void showDebugWindow(IplImage * img, const string & win_name)
{
	cvNamedWindow(win_name.c_str(), 0);
	cvResizeWindow(win_name.c_str(), img->width<<1, img->height<<1);	
	cvShowImage(win_name.c_str(), img);
	cvWaitKey(10000);
}

void showDebugWindow(GranVec *vec, const string & win_name)
{
	checkGranVec(*vec);
}

void showDebugWindow(ImgVec *vec, const string & win_name)
{
	checkImgVec(*vec);
}


bool getMinMax(int &min, int &max, int n, int *vals)
{
	min = 0;
	max = 0;

	if (n > 0) {
		min = vals[0];
		max = vals[0];

		for(int i=1; i<n; i++) {
			if (vals[i] > max) max = vals[i];
			if (vals[i] < min) min = vals[i];
		}

		return true;
	}

	return false;
}



void checkFeature(SparseFeature &f, bool save, int save_id) 
{
	std::cout << "check feature : " << f.size << "\n";
	for(int i=0; i<f.size; i++) {
		cout << f.id[i] << " " << f.sign[i] << "    " ;
	}
	cout <<"\n";


	cvNamedWindow("sparse feature", 0);
	cvResizeWindow("sparse feature", 500, 500);

	IplImage *img = cvCreateImage( cvSize(K_WIN_SIZE * 10, K_WIN_SIZE * 10), IPL_DEPTH_8U, 3 );

	cvRectangle(img, cvPoint(0, 0), cvPoint(K_WIN_SIZE * 10 -1, K_WIN_SIZE * 10-1), cvScalar(0,0,0,0), CV_FILLED);

	int x1, y1, s1;

	const int c[] = {255, 180, 120, 60};

	for(int s=K_MAX_SCALE; s>=0; s--) {
		for(int i=0; i<f.size; i++) {
			GetFeatureXYSFromID(x1, y1, s1, f.id[i]);

			if (s1 == s) {
				for(int x = x1; x < x1 + (1 << s); x++) {
					for(int y = y1; y < y1 + (1 << s); y++) {
						int cb,cr;

						if (f.sign[i]) {
							cb = c[s];
							cr = 0;
						} else {
							cr = c[s];
							cb = 0;
						}
						cvRectangle(img, cvPoint(x * 10 + 1, y * 10 + 1), 
						cvPoint(x * 10 + 8, y * 10 + 8), cvScalar(cb,0,cr,0), CV_FILLED);
					}
				}
			}
		}
	}

	for(int i=0; i<K_WIN_SIZE; i++) {
		cvLine(img, cvPoint(i*10,0), cvPoint(i*10, K_WIN_SIZE*10), cvScalar(0,80,0));
		cvLine(img, cvPoint(0, i*10), cvPoint(K_WIN_SIZE*10, i*10), cvScalar(0,80,0));
	}

	//static int fff = 0;
	//fff++;

	if (save) {
		char fn[256];
		sprintf(fn, "./feats/f%06d.png", save_id);
		cvSaveImage(fn, img);
	}

	cvShowImage("sparse feature", img);
	cvReleaseImage(&img);

	cvWaitKey(2000);

}

void checkGranVec(GranVec &vec)
{
	cvNamedWindow("granvec", 0);

	IplImage *img = cvCreateImage( cvSize(K_WIN_SIZE * (K_MAX_SCALE+1), K_WIN_SIZE), IPL_DEPTH_8U, 1 );

	int yn = K_WIN_SIZE;
	int tt = 1;
	int q=0;
	for(int i=0; i<=K_MAX_SCALE; i++) {
		for(int y=0; y<yn; y++)	{
			for(int x=0; x<yn; x++) {
				if (vec.normalized) {
					img->imageData[y*img->widthStep + (x + K_WIN_SIZE * i)] =(char) (min(255, max(0, vec.val[q++] + 127)));
				} else {
					img->imageData[y*img->widthStep + (x + K_WIN_SIZE * i)] =(char) vec.val[q++];
				}
			}
		}

		yn -= tt;
		tt = tt << 1;
	}


	cvShowImage("granvec", img);
	cvReleaseImage(&img);

	cvWaitKey(100000);
}

void checkImgVec(ImgVec &vec)
{
	cvNamedWindow("imgvec", 0);

	IplImage *img = cvCreateImage( cvSize(K_WIN_SIZE, K_WIN_SIZE), IPL_DEPTH_8U, 1 );

	int yn = K_WIN_SIZE;
	int q=0;
	for(int y=0; y<yn; y++)	{
		for(int x=0; x<yn; x++) {
			img->imageData[y*img->widthStep + x] = (char)vec.val[q++];
		}
	}

	cvShowImage("imgvec", img);
	cvReleaseImage(&img);
	cvWaitKey(100000);
}

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}



int SearchAllFiles(std::string &format, std::vector <std::string>& output_files)
{
	output_files.clear();

//	wstring face_files = face_folder + L"*.png";
	std::wstring wformat = s2ws(format);

	string key("/");
	string prepath;
	size_t found = format.rfind(key);

	if (found != string::npos) {
		prepath = format.substr(0, found+1);
	}

	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(wformat.c_str(),&data);

	int n = 0;

	if( h!=INVALID_HANDLE_VALUE ) 
	{
		do
		{
			char*   nPtr = new char [lstrlen( data.cFileName ) + 1];
			for( int i = 0; i < lstrlen( data.cFileName ); i++ )
				nPtr[i] = char( data.cFileName[i] );

			nPtr[lstrlen( data.cFileName )] = '\0';

			string filename(nPtr);

			if (filename.compare(".") == 0) continue;
			if (filename.compare("..") == 0) continue;

			filename = prepath + filename;
			output_files.push_back(filename);

			n++;
		} while(FindNextFile(h,&data));
	} 
	else {
		cout << "Error: No such folder." << endl;
		n = -1;
	}
	
	FindClose(h);

	return n;
}


void sortSparseFeature(SparseFeature &f)
{
	int n = f.size;

	for(int i=0; i<n-1; i++)
		for(int j=i+1; j<n; j++) 
			if (f.id[i] > f.id[j]) {
				swap(f.id[i], f.id[j]);
				swap(f.sign[i], f.sign[j]);
			}

}


double **Make2dDoubleArray(int r, int c) 
{
	double **d = new double* [r];

	for(int i=0; i<r; i++) {
		d[i] = new double [c];
		memset(d[i], 0, sizeof(double) * c);
	}

	return d;
}

void Copy2dDoubleArray(double **dst, double **src, int r, int c) 
{
	for(int i=0; i<r; i++) {
		memcpy(dst[i], src[i], sizeof(double) * c);
	}
}

void Delete2dDoubleArray(double ***pA, int r) 
{
	for(int i=0; i<r; i++) {
		delete [] (*pA)[i];
	}

	delete [] (*pA);
	*pA = 0;
}






























////////Liang-Chieh
//void initializeDouble(double* data, int size, double val)		
//{
//	for(int i=0; i<size; ++i)
//		data[i] = val;
//}
//
//void readConfidenceFromImgSet(ImgSet* img_set, double* conf_table, double* min_conf, double* max_conf)
//{
//	int num_img = img_set->num_img;
//
//	double minC = img_set->gran_vec[0].conf;
//	double maxC = img_set->gran_vec[0].conf;
//
//	conf_table[0] = minC;
//
//	for(int i=1; i<num_img; ++i){
//		conf_table[i] = img_set->GetGranVec(i)->conf;
//		
//		if(conf_table[i] < minC)
//			minC = conf_table[i];
//		if(maxC < conf_table[i])
//			maxC = conf_table[i];
//	}
//	*min_conf = minC;
//	*max_conf = maxC;
//}
//
//void cumTwoArray(double* dst, double* src, int size)
//{
//	for(int i=0; i<size; ++i)
//		dst[i] += src[i];negVecs
//}
//
//int round2Int(double n)
//{
//	 double temp;
//      
//	 temp = n-floor(n);
//      
//	 if (temp>=0.5)    
//		return (int)n+1; 
//	 else
//		 return (int)n;
//
//}

///////



double normalizeImgVec(ImgVec &iv)
{
	int imean = 0;
	int isquare_mean = 0;

	for(int i=0; i<K_WIN_NPTS; ++i){
		imean += iv.val[i];
		isquare_mean += iv.val[i] * iv.val[i];
	}

	double mean = (double) imean / K_WIN_NPTS;
	double square_mean = (double) isquare_mean / K_WIN_NPTS;
	double std = max(1.0, sqrt(square_mean - mean*mean));

//	for(int i=0; i<K_WIN_NPTS; ++i){
////		iv.val[i] -=  m;
////		iv.val[i] = (int)((double)iv.val[i] * st / std + 127.5);
////		iv.val[i] = min(255, max(0, iv.val[i]));
//
//		iv.val[i] = (int) (((double)(  ((int)((double)iv.val[i]-mean)) << K_NORM_SHIFT )) / std);
//
//		// test
////		iv.val[i] = min(255, max(0, iv.val[i]+128));
//	}
//
//	//static double std_min = 99999;
//	//if (std < std_min) 
//	//{	std_min = std;
//	//	cout << "std_min " << std_min <<"\n";
//	//}
//
//	iv.normalized = true;

	iv.val[0] = (int)std;

	return std;
}

