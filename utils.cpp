/*
Copyright (C) 2010-2015 Matteo Giuliani, Josh Kollat, Jon Herman, and others.

HBV is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HBV is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with HBV.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "utils.h"
#include <math.h>
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

vector<vector<double> > utils::loadMatrix(string file_name, unsigned int row, unsigned int col){

    double data;
    vector<vector<double> > output;
    vector<double> temp1;
    ifstream input(file_name.c_str(), ifstream::in);
    if (input.is_open()){
        for(unsigned int i = 0; i<row; i++){
            for(unsigned int j=0; j<col; j++){
                input >> data;
                temp1.push_back(data);
            }
            output.push_back(temp1);
            temp1.clear();
        }
        input.close();
    }
    else cout << "Unable to open file";

    return output;
}

vector<double> utils::loadVector(string file_name, unsigned int l){

    double data;
    vector<double> output;
    ifstream input(file_name.c_str(), ifstream::in);
    if (input.is_open()){
        for(unsigned int i = 0; i<l; i++){
                input >> data;
                output.push_back(data);
        }
        input.close();
    }
    else cout << "Unable to open file1";

    return output;
}



void utils::logVector(vector<double> x, string filename){

    ofstream logResult;
    logResult.open(filename.c_str(), ios::out);
    for(unsigned int i = 0; i < x.size(); i++){
        logResult << x[i] << endl;
    }
    logResult.close();
}


void utils::logArray(double* x, unsigned int x_size, string filename){

    ofstream logResult;
    logResult.open(filename.c_str(), ios::out);
    for(unsigned int i = 0; i < x_size; i++){
        logResult << x[i] << endl;
    }
    logResult.close();
}

void utils::logVectorApp(vector<double> x, string filename){

    ofstream logResult;
    logResult.open(filename.c_str(), ios::app);
    for(unsigned int i = 0; i < x.size(); i++){
        logResult << x[i] << endl;
    }
    logResult.close();
}


double utils::interp_lin(vector<double> X, vector<double> Y, double x){

    int dim = X.size()-1;
    double y;

    // extreme cases (x<X(0) or x>X(end): extrapolation
    if(x <= X[0]){
        y = ( Y[1] - Y[0] ) / ( X[1] - X[0] )*( x - X[0] ) + Y[0] ;
        return y;
    }
    if(x >= X[dim]){
        y = Y[dim] + ( Y[dim] - Y[dim-1] ) / ( X[dim] - X[dim-1] ) * ( x - X[dim] );
        return y;
    }

    // otherwise
    // [ x - X(A) ] / [ X(B) - x ] = [ y - Y(A) ] / [ Y(B) - y ]
    // y = [ Y(B)*x - X(A)*Y(B) + X(B)*Y(A) - x*Y(A) ] / [ X(B) - X(A) ]
    double delta = 0.0;
    double min_d = numeric_limits<double>::max( );
    int j = -99;

    for(unsigned int i=0; i<X.size(); i++){
        if(X[i] == x){
            y = Y[i];
            return y;
        }
        delta = abs( X[i] - x ) ;
        if(delta < min_d){
            min_d = delta ;
            j = i;
        }
    }
    int k;
    if(X[j] < x){
        k = j;
    }else{
        k = j-1;
    }

    double a = (Y[k+1] - Y[k]) / (X[k+1] - X[k]) ;
    double b = Y[k] - a*X[k];
    y = a*x + b;

    return y;

}


double utils::gallonToCubicFeet(double x){

    double conv = 0.13368 ; // 1 gallon = 0.13368 cf
    return x*conv;
}

double utils::inchesToFeet(double x){

    double conv = 0.08333 ; // 1 inch = 0.08333 ft
    return x*conv;
}


double utils::cubicFeetToCubicMeters(double x){

    double conv = 0.0283 ; // 1 cf = 0.0283 m3
    return x*conv;
}


double utils::feetToMeters(double x){

    double conv = 0.3048 ; // 1 ft = 0.3048 m
    return x*conv;
}

double utils::acreToSquaredFeet(double x){
    double conv = 43560 ; // 1 acre = 43560 feet2
    return x*conv;
}

double utils::acreFeetToCubicFeet(double x){
    double conv = 43560 ; // 1 acre-feet = 43560 feet3
    return x*conv;
}

double utils::cubicFeetToAcreFeet(double x){
    double conv = 43560 ; // 1 acre = 43560 feet2
    return x/conv;
}



double utils::computeSum(vector<double> g){
    double z = 0.0;
    for(unsigned int i=0; i<g.size(); i++){
        z = z + g[i];
    }
    return z;
}

double utils::computeMax(vector<double> g){
    double m = -1*numeric_limits<double>::max( );
    for(unsigned int i=0; i<g.size(); i++){
        if(g[i]>m){
            m = g[i];
        }
    }
    return m;
}

double utils::computeMin(vector<double> g){
    double m = numeric_limits<double>::max( );
    for(unsigned int i=0; i<g.size(); i++){
        if(g[i]<m){
            m = g[i];
        }
    }
    return m;
}

double utils::computeMean(vector<double> g){
    double z = computeSum(g)/g.size();
    return z;
}

double utils::computeVariance(vector<double> g){
    double v = 0.0;
    double M = computeMean(g);
    for(unsigned int i=0; i<g.size(); i++){
        v += ( g[i]-M )*( g[i]-M );
    }
    return v/g.size();
}

double utils::computeStDev(vector<double> g){
    double v = computeVariance(g);
    double s = sqrt(v);
    return s;
}

double utils::computeCov(vector<double> x, vector<double> y){
    double x_mean = computeMean(x);
    double y_mean = computeMean(y);

    double total = 0;
    for(int i=0; i<x.size(); i++){
        total+= ( x[i]-x_mean )*( y[i]-y_mean );
    }

    double cov = total/x.size();
    return cov;
}

double utils::computeCorr(vector<double> x, vector<double> y){
    double cov = computeCov(x, y);
    double x_std = computeStDev(x);
    double y_std = computeStDev(y);
    double corr = cov / ( x_std*y_std );
    return corr;
}


vector<double> utils::normalizeVector(vector<double> X, vector<double> m, vector<double> M){

    vector<double> Y;
    double z;
    for(unsigned int i=0; i<X.size(); i++){
        z = ( X[i] - m[i] ) / ( M[i] - m[i] );
        Y.push_back( z );
    }
    return Y;
}

vector<double> utils::deNormalizeVector(vector<double> X, vector<double> m, vector<double> M){

    vector<double> Y;
    double z;
    for(unsigned int i=0; i<X.size(); i++){
        z = X[i]*( M[i] - m[i] ) + m[i] ;
        Y.push_back( z );
    }
    return Y;

}
