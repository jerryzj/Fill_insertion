#include "statistic.hpp"

double getMean(vector<double> data) {
    double sum = 0.0;

    for(double a : data) sum += a;

    return sum/data.size();
}

double getVariance(vector<double> data) {
    double mean = getMean(data);
    double temp = 0;
    for(double a :data)
        temp += (a-mean)*(a-mean);
    return temp/(data.size()-1);
}

double getStdDev(vector<double> data) {
    return sqrt(getVariance(data));
}

double median(vector<double> data){
    sort(data.begin(), data.end());

    if (data.size() % 2 == 0) {
        return (data[(data.size() / 2) - 1] + data[data.size() / 2]) / 2.0;
    } 

    return data[data.size() / 2];
}