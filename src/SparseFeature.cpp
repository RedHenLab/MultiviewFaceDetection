
#include <math.h>
#include "../include/SparseFeature.h"


//extern bool sample_normalized;


int EvaluateFeature(SparseFeature& feature, GranVec& sample)
{
	int r = 0;

	for(int i=0; i<feature.size; i++) {
		if (feature.sign[i]) {
			r += sample.val[feature.id[i]];
		} else {
			r -= sample.val[feature.id[i]];
		}
	}

	if (!sample.normalized) {
		r = (int)((double)(r << K_NORM_SHIFT) / sample.stdv);
	}

	//int r = (feature.nNegBlcks << 8) - feature.nNegBlcks;

	//for(int i=0; i<feature.size; i++) {
	//	if (feature.sign[i]) {
	//		r += sample.val[feature.id[i]];
	//	} else {
	//		r -= sample.val[feature.id[i]];
	//	}
	//}

	//if (feature.size > 1) {
	//	r = r / feature.size;
	//}

	return r;
}

void EvaluateFeatureForMultipleSamples(int *values, SparseFeature& feature, GranVec* samples, int n_samples)
{

	for(int i=0; i<n_samples; i++) {
		values[i] = 0;
	}

	for(int i=0; i<feature.size; i++) {
		if (feature.sign[i]) {
			for(int j=0; j<n_samples; j++) {
				values[j] += samples[j].val[feature.id[i]];
			}
		} else {
			for(int j=0; j<n_samples; j++) {
				values[j] -= samples[j].val[feature.id[i]];
			}
		}
	}

	for(int j=0; j<n_samples; j++) {
		if (!samples[j].normalized) {
			values[j] = (int)((double)((int)values[j]  << K_NORM_SHIFT) / samples[j].stdv);
		}
	}

	//if (!sample_normalized) {
	//	for(int j=0; j<n_samples; j++) {
	//		values[j] = (int)((double)(values[j]  << K_NORM_SHIFT) / samples[j].stdv);
	//	}
	//}


	//for(int i=0; i<n_samples; i++) {
	//	values[i] = (feature.nNegBlcks << 8) - feature.nNegBlcks;
	//}

	//for(int i=0; i<feature.size; i++) {
	//	if (feature.sign[i]) {
	//		for(int j=0; j<n_samples; j++) {
	//			values[j] += samples[j].val[feature.id[i]];
	//		}
	//	} else {
	//		for(int j=0; j<n_samples; j++) {
	//			values[j] -= samples[j].val[feature.id[i]];
	//		}
	//	}
	//}

	//if (feature.size > 1) {
	//	for(int j=0; j<n_samples; j++) {
	//		values[j] = values[j] / feature.size;
	//	}
	//}

}

void EvaluateFeatureForMultipleSamples(int *values, SparseFeature& feature, ImgSet& set)
{
	int n_samples = set.Size();

	for(int i=0; i<n_samples; i++) {
		values[i] = 0;
	}

	for(int i=0; i<feature.size; i++) {
		if (feature.sign[i]) {
			for(int j=0; j<n_samples; j++) {
				values[j] += set.GetGranVec(j)->val[feature.id[i]];
			}
		} else {
			for(int j=0; j<n_samples; j++) {
				values[j] -= set.GetGranVec(j)->val[feature.id[i]];
			}
		}
	}

	for(int j=0; j<n_samples; j++) {
		if (!set.GetGranVec(j)->normalized) {
			values[j] = (int)((double)((int)values[j]  << K_NORM_SHIFT) / set.GetGranVec(j)->stdv);
		}
	}
}


int GetFeatureIDFromXYS(int x, int y, int s)
{

#ifdef _DEBUG
	if (s<0 || s>K_MAX_SCALE) {
		exit(1);
		return -1;
	}

	if (x<0 || x>=GV_size[s]) {
		exit(1);
		return -1;
	}

	if (y<0 || y>=GV_size[s]) {
		exit(1);
		return -1;
	}

#endif

	return GV_base_id[s] + y * GV_size[s] + x;
	
}

void GetFeatureXYSFromID(int& x, int& y, int& s, int id)
{

	int r = 0;

#ifdef _DEBUG
	if (id<0 || id>=K_NGRANS) {
		exit(1);
	}

#endif

	s = 0;
	while(GV_base_id[s+1] <= id) s++;

	id -= GV_base_id[s];

	y = id / GV_size[s];
	x = id - y * GV_size[s];

}

void GetVecFromImage(ImgVec &input, GranVec &result)
{
	// Features at scale 0 = original image
	memcpy(result.val, input.val, sizeof(pixel) * K_WIN_NPTS);

	pixel *op = &result.val[GV_base_id[1]];
	for(int i=1; i<=K_MAX_SCALE; i++) {
		pixel *ip1 = &result.val[GV_base_id[i-1]];
		pixel *ip2 = &result.val[GV_base_id[i-1] + GV_size[i-1]];

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
}

int GetNeighborGranules(std::vector<int>& neightbors, int id) 
{
	neightbors.clear();

	int x,y,s;
	x=y=s=0;
	GetFeatureXYSFromID(x,y,s,id);

	// same scale
	{
		int x0 = MAX(0, x-1);
		int x1 = MIN(x+1, GV_size[s] - 1);
		int y0 = MAX(0, y-1);
		int y1 = MIN(y+1, GV_size[s] - 1);

		for(int i=x0; i<=x1; i++) 
			for(int j=y0; j<=y1; j++) 
				if (i!=x || j!=y)
					neightbors.push_back(GetFeatureIDFromXYS(i,j,s));
	}

	// different scale
	if (s == 0) {
		int x0 = MAX(0, x-1);
		int x1 = MIN(x, GV_size[s+1] - 1);
		int y0 = MAX(0, y-1);
		int y1 = MIN(y, GV_size[s+1] - 1);

		for(int i=x0; i<=x1; i++) 
			for(int j=y0; j<=y1; j++) 
				neightbors.push_back(GetFeatureIDFromXYS(i,j,s+1));

	}

	if (s == 1) {
		neightbors.push_back(GetFeatureIDFromXYS(x,y,0));
		neightbors.push_back(GetFeatureIDFromXYS(x+1,y,0));
		neightbors.push_back(GetFeatureIDFromXYS(x,y+1,0));
		neightbors.push_back(GetFeatureIDFromXYS(x+1,y+1,0));

		if (x>0 && y>0 && x<GV_size[s]-1 && y<GV_size[s]-1)
			neightbors.push_back(GetFeatureIDFromXYS(x-1,y-1,2));
	}

	if (s == 2) {
		neightbors.push_back(GetFeatureIDFromXYS(x+1,y+1,1));

		if (K_MAX_SCALE > 2)
			if (x>1 && y>1 && x<GV_size[s]-2 && y<GV_size[s]-2)
				neightbors.push_back(GetFeatureIDFromXYS(x-2,y-2,3));
	}

	if (K_MAX_SCALE > 2)
		if (s == 3) {
			neightbors.push_back(GetFeatureIDFromXYS(x+2,y+2,2));
		}

	return neightbors.size();
}


