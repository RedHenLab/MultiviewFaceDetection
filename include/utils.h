#ifndef H_UTILS
#define H_UTILS



////// 	*out = cvCreateImage( cvSize(in->width, in->height), IPL_DEPTH_8U, 1 );


#include "cv.h"
#include "highgui.h"

#include "SparseFeature.h"

//void showDebugWindow(const string & win_name, void* p);
void showDebugWindow(IplImage *img, const string & win_name = "image");
void showDebugWindow(GranVec *gv, const string & win_name = "gran vec");
void showDebugWindow(ImgVec *iv, const string & win_name = "img vec");


//#define DISP_DEBUG
#ifdef DISP_DEBUG

#define DEB_SHOW(X) showDebugWindow(X);
#define DEB_SHOW(X,Y) showDebugWindow(X,Y);

#else
//nothing
#define DEB_SHOW(X) 
#define DEB_SHOW(X,Y) 

#endif

//#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
//#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

//class IplImage;

//int convertIplImgToImgVec(IplImage *in, ImgVec& out);

int resizeIplImgToWindowSize(IplImage *in, IplImage **out);

void checkGranVec(GranVec &vec);

void checkImgVec(ImgVec &vec);
int SearchAllFiles(std::string &format, std::vector <std::string>& output_files);

//int LoadImgVecFromImageFile(std::string& filename, ImgVec &iv) ;

void checkFeature(SparseFeature &f, bool save = false, int save_id = 0) ;

bool getMinMax(int &min, int &max, int n, int *vals);

void sortSparseFeature(SparseFeature &f);

double **Make2dDoubleArray(int r, int c); 

void Delete2dDoubleArray(double ***pA, int r); 
void Copy2dDoubleArray(double **dst, double **src, int r, int c) ;

//////Liang-Chieh
void initializeDouble(double* data, int size, double val);

void readConfidenceFromImgSet(ImgSet* img_set, double* conf_table, double* min_conf, double* max_conf);

void cumTwoArray(double* dst, double* src, int size);

int round2Int(double n);

void transform2PosInt(int* dst, double* src, int size, double min, double max);

double normalizeImgVec(ImgVec &imgvec);

#endif
