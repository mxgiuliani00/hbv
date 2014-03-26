/*
Copyright (C) 2014 Matteo Giuliani, Jon Herman, and others.

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

void evaluate(double* Qobs, double* Qsim, int nDays, double* objs){

    // calibration minimizes the RMSE + maximize R2 (1-year of warmup)
    vector<double> Qerr, Qerr2, Qv;
    for(unsigned int i=366; i<nDays; i++){
        Qerr.push_back( Qobs[i] - Qsim[i] ); // model error
        Qerr2.push_back( (Qobs[i] - Qsim[i])*(Qobs[i] - Qsim[i]) ); // model squared error
        Qv.push_back( Qobs[i] );
    }
    double MSE = pow( utils::computeMean(Qerr2), 0.5 );
    double R2 = 1 - ( utils::computeVariance( Qerr ) / utils::computeVariance( Qv ) ) ;
    // 2-objective calibration
    objs[0] = MSE;
    objs[1] = -R2;
}



int main(int argc, char **argv)
{
    // read user input: single input for calibration, two inputs for simulation
    string input_file = argv[1];
    string output_file;
    if(argc>2){
        output_file = argv[2];
    }

    // hbv model
    hbv_model myHBV(input_file);

    // calibration settings
    int nobjs = 2;
    int nvars = 12;
    double objs[nobjs];
    double vars[nvars];

    MOEA_Init(nobjs, 0);
    while (MOEA_Next_solution() == MOEA_SUCCESS) {
        MOEA_Read_doubles(nvars, vars);
        myHBV.calc_HBV(vars);
        evaluate(myHBV.getData().flow, myHBV.getFluxes().Qsim, myHBV.getData().nDays, objs);
        MOEA_Write(objs, NULL);
    }

    // save simulation results
    if(argc>2){
        utils::logArray(myHBV.getFluxes().Qsim, myHBV.getData().nDays, output_file);
    }

    // clear HBV
    myHBV.hbv_delete(myHBV.getData().nDays);

    return 0;
}
