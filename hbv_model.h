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

#ifndef __hbv_h
#define __hbv_h

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <cstdlib>

namespace std{

#define ROUNDINT(x) int(x + 0.5)
#define ROUNDDOUBLE(x) double(int(x + 0.5))

struct hbv_parameters
{
    // User-specified parameters
    double hl1; // L - the storage capacity of the middle/shallow layer [mm]
    double ck0; // K0 - overflow from shallow layer
    double ck1;
    double ck2;
    double perc; // [L/T] rate of perc from upper to lower box
    double lp; // evap something or other (unitless)
    double fcap; // surface soil storage [mm]
    double beta; // surface storage exponent
    int maxbas; // routing coeff [d]
    double ttlim; // (TTH, degC)
    double degd; // (DDF - mm/(degC-D))
    double degw; // (TB, degC)
};

struct hbv_states
{
    double *sowat; //[20][200]; //Soil water storate
    double *sdep; //[20][200]; //Snow store
    double *ldep; //Depth of liquid in snow store
    double *stw1; //[20]; soil storage - shallow layer
    double *stw2; //[20]; soil storage - deep layer
};

struct hbv_fluxes
{
    double *Qrouting; // Maxbas - routing Q's 
    double *Qsim; // array of outflow Q's for simulation
    double *actualET;
};


struct HamonEvap
{
    int day;
    double P;
    double dayLength;
    double eStar;
    double *PE;
};

struct MyData
{
    string ID;          // watershed name
    double gageLat;     // latitude (decimal degrees)
    double gageLong;    // longitude (decimal degrees)
    double DA;          // drainage area
    int nDays;          // Number of days of data
    int tempData;       // Type of temperature data (1=daily average, 2=min and max)

    //Starting and ending dates of data to read
    int *dateStart;
    int *dateEnd;


    int **date;         //Date of data [year, month, day]
    double *precip;     //Mean areal precipitation (mm)
    double *evap;       //Climatic potential evaporation (mm)
    double *flow;       //Streamflow discharge (mm)
    double *maxTemp;    //Maximum air temperature (Celsius) (should be daily)
    double *minTemp;    //Minimum air temperature (Celsius) (should be daily)
    double *avgTemp;    //Average air temperature (Celsius) (should be daily)
    //double *peAdjust;  //PE adjustment factors for each month
};

class hbv_model {

public:

    /**
     * default constructor/destructor
     */
    hbv_model();
    virtual ~hbv_model();

    /**
     * hbv_model constructor with parameters (namefile with the data)
     */
    hbv_model(string dataFile);

    /**
     * clear hbv_model structures
     */
    void hbv_delete(int nDays);

    /**
     * evaluation of HBV model with parameters passed as input
     */
    void calc_HBV(double *parameters);

    /**
      * get-functions for protected data
      **/
    MyData getData();
    hbv_fluxes getFluxes();

protected:

    /**
     * Initialization of HBV model:
     *  - allocation structures
     *  - loading of data
     *  - computation of PE
     *  - setting of HBV parameters
     *  - re-initialization to zero
     */
    void hbv_allocate(int nDays);
    void readData(string filename);
    void calculateHamonPE(int dataIndex, int nDays, int startDay);
    void setParameters(double* parameters);
    void reinitStateFluxes();


    /**
     * rainfall-runoff processes
     */
    // Effective precipitation
    double snow(int modelDay);
    // Soil moisture
    void soil(double eff_precip, int modelDay);
    // Basin discharge
    double discharge(int modelDay);
    // Discharge routing
    void routing(double Qall, int modelDay);
    // Routing update/reinitialization
    void backflow();
    void reinitForMaxBas();



    int dayStartIndex;
    int startingIndex;
    double tst; // time-step

    MyData data;
    HamonEvap evap;
    hbv_parameters params;
    hbv_states states;
    hbv_fluxes fluxes;

};
}

#endif
