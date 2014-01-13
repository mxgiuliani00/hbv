/*
Copyright (C) 2013 Jon Herman, Josh Kollat, Patrick Reed and others.

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

/****************************************************************************
C/C++ Version of HBV: Lumped model, one catchment. Uses Hamon ET and MOPEX forcing data.
*****************************************************************************/

#include "hbv_model.h"
#include "moeaframework.h"
#include "utils.h"
#include <math.h>

using namespace std;

// Time period: 10/1/1961 to 9/30/1972 (1 year of warmup plus 10-year period)
int dayStartIndex = 274; //10-1 is day 274 of the year
int nDays = 4017; // length of simulation, including leap years
int startingIndex = 5023-1; //The starting index of the data file corresponding with the start date

double evaluate(double* Qobs, double* Qsim){

    // calibration minimizes the RMSE
    vector<double> Qerr, Qv, Qv1;
    for(unsigned int i=0; i<nDays; i++){
        Qerr.push_back( (Qobs[i] - Qsim[i])*(Qobs[i] - Qsim[i]) );
        Qv.push_back( Qobs[i] );
        Qv1.push_back( Qsim[i] );
    }
    double MSE = pow( utils::computeMean(Qerr), 0.5 );
    return MSE;
}



int main(int argc, char **argv)
{
    // read user input: single input for calibration, two inputs for simulation
    string input_file = argv[1];
    string output_file;
    if(argc>2){
        output_file = argv[2];
    }

    int nobjs = 1;
    int nvars = 12;
    double objs[nobjs];
    double vars[nvars];

    // hbv model
    hbv_model myHBV(dayStartIndex,nDays,startingIndex);
    myHBV.init_HBV(input_file);

    // optimization
    MOEA_Init(nobjs, 0);
    while (MOEA_Next_solution() == MOEA_SUCCESS) {
        MOEA_Read_doubles(nvars, vars);
        myHBV.calc_HBV(vars);
        objs[0] = evaluate(myHBV.getData().flow, myHBV.getFluxes().Qsim);
        MOEA_Write(objs, NULL);
    }

    // save simulation results
    if(argc>2){
        utils::logArray(myHBV.getFluxes().Qsim, nDays, output_file);
    }

    myHBV.hbv_delete(nDays);

    return 0;
}
