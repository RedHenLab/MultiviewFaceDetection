//#ifndef H_XMLHANDLER
//#define H_XMLHANDLER
//
////#include "Cascade.h"
//#include "Markup.h"
//#include <iostream>
//#include <vector>
//#include <string>
//#include "Struct.h"
//
//using namespace std;
//
///*
//// size of samples
//#define K_WIN_SIZE 24
//#define K_WIN_NPTS 576
//// total number of granules. 1835 = 576 + 529 + 441 + 289
//#define K_NGRANS 1835
//#define K_MAX_SCALE 3
//#define K_LUTBINS 16
//
//struct SparseFeature
//{
//	int size;
//	std::vector<int> id;
//	std::vector<bool> sign;
//};
//
//struct LUTClassifierData
//{
//	int num_lut;					// give a number to each LUTClassifier
//	double conf[K_LUTBINS ];
//	SparseFeature features;
//};
//
//struct LUTAdaData
//{
//	int stage;
//	int num_total;					// total number of classifiers in vector
//	std::vector<LUTClassifierData> lut_classifiers;
//};
//
//struct CascadeData
//{
//	int num_stage;
//	std::vector<LUTAdaData> lut_adas;
//};
//*/
//class XMLHandler
//{
//public:
//	XMLHandler(void);
//	~XMLHandler(void);
//	int save(string directory, CascadeData cc_data);
//	int load(string directory, CascadeData &cc_data);
//	wstring s2ws(const string& s);
//	string ws2s(const wstring& ws);
//	int ws2i(const wstring& ws);
//	double ws2f(const wstring& ws);
//};
//
//#endif 