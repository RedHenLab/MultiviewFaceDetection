#ifndef H_SPARSE_FEATURE
#define H_SPARSE_FEATURE

#include <string.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include "struct.h"
#include "utils.h"


int EvaluateFeature(SparseFeature& feature, GranVec& sample);

void EvaluateFeatureForMultipleSamples(int *values, SparseFeature& feature, GranVec* samples, int n_samples);
void EvaluateFeatureForMultipleSamples(int *values, SparseFeature& feature, ImgSet& set);

int GetFeatureIDFromXYS(int x, int y, int s);

void GetFeatureXYSFromID(int& x, int& y, int& s, int id);

void GetVecFromImage(ImgVec &input, GranVec &result);

int GetNeighborGranules(std::vector<int>& neightbors, int id);


#endif

