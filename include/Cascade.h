//#ifndef H_CASCADE
//#define H_CASCADE
//
//#include "cv.h"
//#include <vector>
//#include <cstdio>
//#include "ImageHandler.h"
//#include "XMLHandler.h"
//#include "LUTAda.h"
//
//using namespace cv;
//
//
////struct TrainInfo
////{
////	string dir_pos;
////	string dir_neg;
////	int num_stage;
////	int train_num_pos;
////	int train_num_neg;
////	double min_detection_rate;
////	double max_false_alarm;
////	double threshold;
////};
//
//class Cascade
//{
//public:
//	Cascade(void);
//	Cascade(TrainInfo info);
//	~Cascade(void);
//	int load(const string& filename);		// load xml file into v_lutada_data
//	int save(const string& filename);		// save v_lutada_data into xml file
//
//// for training
//	int train();							// train cascade
//
//// for testing
//	int initializeLUTAda();					// load the v_lutada_data into the v_lutada
//
//	void detectMultiScale( IplImage *image,                    // detect faces in an image
//						   vector<Rect>& objects,
//                           double scaleFactor=1.1,
//                           int minNeighbors=2, int flags=0,
//                           int minSize=16, int maxSize = 300);
//
//	void detectMultiScaleOrg( IplImage *image,                    // detect faces in an image
//						   vector<Rect>& objects,
//                           double scaleFactor=1.1,
//                           int minNeighbors=2, int flags=0,
//                           int minSize=16, int maxSize = 300);
//
//
//
//	//bool setImage( const Mat& image);       // Set the testing image
//    //int runAt( Point point);				// detect face at point xy in the image
//
//	int getNumPosUsed(){return p_imghandler->getNumPosUsed();}
//	int getNumNegUsed(){return p_imghandler->getNumNegUsed();}
//
//	void checkFeatures();
//
//
//private:
//	int num_stage;
//	vector<LUTAda> v_lutada;         
//	vector<LUTAdaData> v_lutada_data;
//	TrainInfo info;
//	ImageHandler* p_imghandler;	
//	
//	int collectNeg(int num, int stage, ImgSet *p_negset);
//	int collectPos(int num, int stage, ImgSet *p_posset);
//
//	void checkPass(ImgSet *p_set);
//
//	int crtStage;
//
//};
//
//#endif