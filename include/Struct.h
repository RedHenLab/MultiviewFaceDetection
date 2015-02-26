#ifndef H_STRUCT
#define H_STRUCT

#include <vector>


#define K_2424_WINDOW
#ifdef K_2424_WINDOW
// size of samples
#define K_WIN_SIZE 24
#define K_WIN_NPTS 576
// total number of granules. 1835 = 576 + 529 + 441 + 289
#define K_NGRANS 1835

const int GV_base_id[] = {0, 576, 1105, 1546, 1835, 9999999};
const int GV_size[] = {24, 23, 21, 17}; 
#define K_MAX_SCALE 3

#else
const int GV_base_id[] = {0, 256, 481, 650, 731, 9999999};
const int GV_size[] = {16, 15, 13, 9}; 

#define K_WIN_SIZE 16
#define K_WIN_NPTS 256
//#define K_NGRANS 731
#define K_NGRANS 650
#define K_MAX_SCALE 2

#endif

#define K_NORM_SHIFT 10

#define K_EPSILON 0.0000001
//#define K_LUTBINS 16
//#define K_NUM_SHIFT 4			//number of shift is log2(num_threshold)
								//K_NUM_SHIFT cannot be larger than 8!!

#define K_POS_SAMPLE_MIN_STDV 10.0

#define K_LUTBINS 32
#define K_NUM_SHIFT 5			//number of shift is log2(num_threshold)
								//K_NUM_SHIFT cannot be larger than 8!!

#define K_MAX_GRANULES 8

#define K_WFS_POSES 18
#define K_WFS_BRANCH_NODES 29
#define K_WFS_MAX_BRANCHES 3

// parent, # children, ~~~
const int WfsTreeInfo[][32] = {
        { -1,  3,  1,  2,  3, },
        {  0,  2,  4,  5, },
        {  0,  3, 14, 15, 16, },
        {  0,  2,  6,  7, },
        {  1,  3,  8,  9, 10, },
        {  1,  3, 11, 12, 13, },
        {  3,  3, 17, 18, 19, },
        {  3,  3, 20, 21, 22, },
        {  4,  0, },
        {  4,  0, },
        {  4,  0, },
        {  5,  0, },
        {  5,  2, 23, 24, },
        {  5,  0, },
        {  2,  0, },
        {  2,  2, 25, 26, },
        {  2,  0, },
        {  6,  0, },
        {  6,  2, 27, 28, },
        {  6,  0, },
        {  7,  0, },
        {  7,  0, },
        {  7,  0, },
        { 12,  0, },
        { 12,  0, },
        { 15,  0, },
        { 15,  0, },
        { 18,  0, },
        { 18,  0, },
};


const int WfsTreeStructure[][K_WFS_POSES+1] = {
        // AP  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
        {  0,  1,  2,  3,  1,  1,  2,  3,  3,  1,  1,  2,  3,  3,  1,  1,  2,  3,  3, },   //  0
        {  0,  5,  0,  0,  4,  5,  0,  0,  0,  4,  5,  0,  0,  0,  4,  5,  0,  0,  0, },   //  1
        {  0,  0, 15,  0,  0,  0, 14,  0,  0,  0,  0, 15,  0,  0,  0,  0, 16,  0,  0, },   //  2
        {  0,  0,  0,  6,  0,  0,  0,  6,  7,  0,  0,  0,  6,  7,  0,  0,  0,  6,  7, },   //  3
        {  0,  0,  0,  0,  8,  0,  0,  0,  0,  9,  0,  0,  0,  0, 10,  0,  0,  0,  0, },   //  4
        {  0, 12,  0,  0,  0, 11,  0,  0,  0,  0, 12,  0,  0,  0,  0, 13,  0,  0,  0, },   //  5
        {  0,  0,  0, 18,  0,  0,  0, 17,  0,  0,  0,  0, 18,  0,  0,  0,  0, 19,  0, },   //  6
        {  0,  0,  0,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0, 21,  0,  0,  0,  0, 22, },   //  7
        {  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   //  8
        {  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   //  9
        { 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 10
        {  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 11
        {  0, 24,  0,  0,  0,  0,  0,  0,  0,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0, },   // 12
        { 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 13
        {  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 14
        {  0,  0, 26,  0,  0,  0,  0,  0,  0,  0,  0, 25,  0,  0,  0,  0,  0,  0,  0, },   // 15
        { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 16
        {  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 17
        {  0,  0,  0, 28,  0,  0,  0,  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,  0,  0, },   // 18
        { 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 19
        {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 20
        { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 21
        { 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 22
        { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 23
        {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 24
        { 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 25
        {  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 26
        { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 27
        {  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, },   // 28
};


const int WfsTreeCategoryId[][K_WFS_POSES+1] = {
        // AP  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
        {  0,  0,  1,  2,  0,  0,  1,  2,  2,  0,  0,  1,  2,  2,  0,  0,  1,  2,  2, },   //  0
        {  0,  1, -1, -1,  0,  1, -1, -1, -1,  0,  1, -1, -1, -1,  0,  1, -1, -1, -1, },   //  1
        {  0, -1,  1, -1, -1, -1,  0, -1, -1, -1, -1,  1, -1, -1, -1, -1,  2, -1, -1, },   //  2
        {  0, -1, -1,  0, -1, -1, -1,  0,  1, -1, -1, -1,  0,  1, -1, -1, -1,  0,  1, },   //  3
        {  0, -1, -1, -1,  0, -1, -1, -1, -1,  1, -1, -1, -1, -1,  2, -1, -1, -1, -1, },   //  4
        {  0,  1, -1, -1, -1,  0, -1, -1, -1, -1,  1, -1, -1, -1, -1,  2, -1, -1, -1, },   //  5
        {  0, -1, -1,  1, -1, -1, -1,  0, -1, -1, -1, -1,  1, -1, -1, -1, -1,  2, -1, },   //  6
        {  0, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1,  1, -1, -1, -1, -1,  2, },   //  7
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   //  8
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   //  9
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 10
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 11
        {  0,  1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, },   // 12
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 13
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 14
        {  0, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, },   // 15
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 16
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 17
        {  0, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, },   // 18
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 19
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 20
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 21
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 22
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 23
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 24
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 25
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 26
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 27
        {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },   // 28
};

//#define K_WFS_POSES 18
//#define K_WFS_BRANCH_NODES 6
//#define K_WFS_MAX_BRANCHES 3
//
//// [branch node] [pose]
//const int WfsTreeStructure[][K_WFS_POSES+1] = {
//	// AP  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
//	{ 00, 01, 02, 03, 01, 01, 02, 03, 03, 01, 01, 02, 03, 03, 01, 01, 02, 03, 03 },		// 00
//	{ 00, 05, 00, 00, 04, 05, 00, 00, 00, 04, 05, 00, 00, 00, 04, 05, 00, 00, 00 },		// 01
//	{ 00, 00, 15, 00, 00, 00, 14, 00, 00, 00, 00, 15, 00, 00, 00, 00, 16, 00, 00 },		// 02
//	{ 00, 00, 00, 06, 00, 00, 00, 06, 07, 00, 00, 00, 06, 07, 00, 00, 00, 06, 07 },		// 03
//	{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },		// 26
//	{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },		// 27
//	{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },		// 28
//};
//
//const int WfsTreeCategoryId[][K_WFS_POSES+1] = {
//	// NA  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
//	{ 00, 00, 01, 02, 00, 00, 01, 02, 02, 00, 00, 01, 02, 02, 00, 00, 01, 02, 02 },
//	{ 00, 01, -1, -1, 00, 01, -1, -1, -1, 00, 01, -1, -1, -1, 00, 01, -1, -1, -1 },
//	{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
//	{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
//	{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
//	{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
//};
//
//// parent, # children, ~~~
//const int WfsTreeInfo[][6] = {
//	{ -1, 03, 01, 02, 03 },
//	{ 00, 02, 04, 05 },
//	{ 00, 01, 00 },
//	{ 00, 01, 00 },
//	{ 01, 01, 00 },
//	{ 01, 01, 00 },
//};


enum EWfsNodeType {	
	BRANCHING = 0,
	BINARY
};

using namespace std;

typedef short pixel;

class ImgVec
{
public:
	ImgVec(){
		normalized = false;
		for(int i=0;i<K_WIN_NPTS;i++)
			val[i] = 0;
	}
	ImgVec(const ImgVec& rhs){
		normalized = rhs.normalized;
		memcpy(val,rhs.val,sizeof(pixel)*K_WIN_NPTS);
	}
	pixel val[K_WIN_NPTS];
	bool normalized;
};

class GranVec
{
public:
	GranVec(){
		conf = 0;
		label = false;
		active = true;
		weight = 0;
		x = 0;
		y = 0;
		size = K_WIN_SIZE;
		stdv = 1.0;
		poseId = 0;
		category = -1;
		normalized = false;
		memset(val, 0, sizeof(pixel) * K_NGRANS);
		memset(negVecWeight, 0, sizeof(double) * K_WFS_MAX_BRANCHES);
	}

	GranVec(const GranVec& rhs){
		conf = rhs.conf;
		label = rhs.label;
		active = rhs.active;
		weight = rhs.weight;
		x = rhs.x;
		y = rhs.y;
		size = rhs.size;
		stdv = rhs.stdv;
		memcpy(val,rhs.val,sizeof(pixel)*K_NGRANS);
		poseId = rhs.poseId;
		category = rhs.category;
		normalized = rhs.normalized;
		memcpy(negVecWeight, rhs.negVecWeight, sizeof(double) * K_WFS_MAX_BRANCHES);
	}

	GranVec& GranVec::operator=(const GranVec &rhs) {
		if (this != &rhs) {
			conf = rhs.conf;
			label = rhs.label;
			active = rhs.active;
			weight = rhs.weight;
			x = rhs.x;
			y = rhs.y;
			size = rhs.size;
			stdv = rhs.stdv;
			memcpy(val,rhs.val,sizeof(pixel)*K_NGRANS);
			poseId = rhs.poseId;
			category = rhs.category;
			normalized = rhs.normalized;
			memcpy(negVecWeight, rhs.negVecWeight, sizeof(double) * K_WFS_MAX_BRANCHES);
		}

		return *this;
	}

	pixel val[K_NGRANS];
	double conf;
	bool label;     // true for face
	bool active;	// true for active	
	double weight;

	double negVecWeight[K_WFS_MAX_BRANCHES];

	int x,y,size;

	double stdv; // standard deviation

	// for branching node
	int poseId;
	int category;

	bool normalized;
};

class ImgSet
{
public:
	ImgSet(){
		num_img = 0;
		//LC
		gran_vec.clear();
	}

	~ImgSet(){
		ClearSet();
	}

	void ClearSet(bool destroy_gran_vec = true) {
		if (destroy_gran_vec) {
			for(int i=0; i<(int)gran_vec.size(); i++) {
				delete gran_vec[i];
			}
		}
		gran_vec.clear();
		sucess.clear();
		num_img = 0;
	}

	void DeleteInactiveGranVecs(bool destroy_gran_vec)
	{
		if (destroy_gran_vec) {
			for(int i=0; i<(int)gran_vec.size(); i++) {
				if (!gran_vec[i]->active) {
					delete gran_vec[i];

					gran_vec[i] = gran_vec[gran_vec.size()-1];
					gran_vec.pop_back();

					i--;
				}
			}
		} else {
			for(int i=0; i<(int)gran_vec.size(); i++) {
				if (!gran_vec[i]->active) {
					gran_vec[i] = gran_vec[gran_vec.size()-1];
					gran_vec.pop_back();

					i--;
				}
			}
		}
		num_img = (int) gran_vec.size();
	}

	// aset will lose gran_vec
	void MergeSet(ImgSet& aset)
	{
		for(int i=0; i<(int)aset.gran_vec.size(); i++) {
			gran_vec.push_back(aset.gran_vec[i]);
		}

		aset.gran_vec.clear();
		aset.num_img = 0;

		num_img = (int) gran_vec.size();
	}

	void AddGranVec(GranVec *pGv, bool copy = false)
	{
		if (copy) {
			GranVec *gv = new GranVec(*pGv);
			gran_vec.push_back(gv);
		} else {
			gran_vec.push_back(pGv);
		}
		num_img++;
	}

	void ShrinkSet(int n)
	{
		if (num_img > n) 
		{
			for(int i=n; i<num_img; i++) {
				delete gran_vec[i];
			}
			gran_vec.resize(n);
			num_img = n;
		}
	}

	int Size() { return num_img; }

	GranVec* GetGranVec(int id) { 
		return gran_vec[id];
	}

	void PrepareDetection() {
		sucess.resize(num_img, true);
	}

private:

	int num_img;
	vector<GranVec*> gran_vec;

	// for detection
	vector<bool> sucess;

};

class SparseFeature
{
public:
	SparseFeature(){
		size = 0;

		//LC
		id.clear();
		sign.clear();
	}
	int size;
	//int nNegBlcks;
	//int nPosBlcks;
	vector<int> id;
	vector<bool> sign;
};


#endif