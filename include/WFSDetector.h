#ifndef H_WFS_DETECTOR
#define H_WFS_DETECTOR

#include <string.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "struct.h"
#include "utils.h"

#include "ImageHandler.h"
#include "WFSNode.h"

typedef struct _WfsFaceInfo {
	int x;
	int y;
	int size;
	int pose;
} WfsFaceInfo;
		

class WfsMvfdDetector
{
public:
	WfsMvfdDetector(const string &xml_file);
	~WfsMvfdDetector();

	int DetectFace(vector<WfsFaceInfo> &faces, IplImage *img, double scale_factor = 1.2, int min_shift = 4, int min_size = K_WIN_SIZE, int max_size = 300);

	int load(const string& filename);		// load xml file into v_lutada_data
	int save(const string& filename);		// save v_lutada_data into xml file

	wstring s2ws(const string& s);
	string ws2s(const wstring& ws);
	int ws2i(const wstring& ws);
	double ws2f(const wstring& ws);

	bool ws2b(const wstring& ws);

	void CheckPass(vector<WfsFaceInfo> &faces);

// for testing
//	int initializeLUTAda();					// load the v_lutada_data into the v_lutada

	//void detectMultiScale( IplImage *image,                    // detect faces in an image
	//					   vector<Rect>& objects,
 //                          double scaleFactor=1.1,
 //                          int minNeighbors=2, int flags=0,
 //                          int minSize=16, int maxSize = 300);

	//void detectMultiScaleOrg( IplImage *image,                    // detect faces in an image
	//					   vector<Rect>& objects,
 //                          double scaleFactor=1.1,
 //                          int minNeighbors=2, int flags=0,
 //                          int minSize=16, int maxSize = 300);



	//bool setImage( const Mat& image);       // Set the testing image
    //int runAt( Point point);				// detect face at point xy in the image

//	int TestSamples(ImgSet *set, int node_id, vector<int> &path_id);


public:

	ImageHandler *pIH;
	ImgSet **poseSets;

	vector<WfsStrongClassifier> wfsNodes;

};


#endif
