#ifndef STATISTIC_HPP
#define STATISTIC_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;

double getMean(vector<double> data);
double getVariance(vector<double> data);
double getStdDev(vector<double> data);
double median(vector<double> data);


#endif