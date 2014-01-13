#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>

namespace std{

class utils
{
public:

    /**
     * functions to load files
     */
    static vector<vector<double> > loadMatrix(string file_name, unsigned int row, unsigned int col);
    static vector<double> loadVector(string file_name, unsigned int l);

    /**
     * function to print a vetor to a file (App append the file)
     */
    static void logVector(vector<double> x, string file_name);
    static void logVectorApp(vector<double> x, string file_name);
    static void logArray(double* x, unsigned int x_size, string file_name);

    /**
      * linear interpolation
      */
    static double interp_lin(vector<double> X, vector<double> Y, double x);

    /**
      * conversion from gallon to cubic feet
      */
    static double gallonToCubicFeet(double x);

    /**
      * conversion from inches to feet
      */
    static double inchesToFeet(double x);

    /**
      * conversion from cubic feet to cubic meters
      */
    static double cubicFeetToCubicMeters(double x);

    /**
      * conversion from feet to meters
      */
    static double feetToMeters(double x);

    /**
      * conversion from acre to squared feet
      */
    static double acreToSquaredFeet(double x);

    /**
      * conversion from acre-feet to cubic feet
      */
    static double acreFeetToCubicFeet(double x);

    /**
      * conversion from cubic feet to acre-feet
      */
    static double cubicFeetToAcreFeet(double x);

    /**
      * Basic operations on vector
      */
    static double computeSum(vector<double> g);
    static double computeMax(vector<double> g);
    static double computeMin(vector<double> g);
    static double computeMean(vector<double> g);
    static double computeVariance(vector<double> g);

    /**
      * Normalization and De-normalization
      */
    static vector<double> normalizeVector( vector<double> X, vector<double> m, vector<double> M );
    static vector<double> deNormalizeVector( vector<double> X, vector<double> m, vector<double> M );


};
}

#endif // UTILS_H
