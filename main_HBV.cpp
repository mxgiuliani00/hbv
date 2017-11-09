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

/****************************************************************************
C/C++ Version of HBV: Lumped model, one catchment. Uses Hamon ET and MOPEX forcing data.
*****************************************************************************/

#include "hbv_model.h"
#include "moeaframework.h"
#include "utils.h"
#include <math.h>
#include <vector>

using namespace std;

void evaluate(double* Qobs, double* Qsim, int nDays, double* objs){

    //convert observations and simulations from array to vector removing first year which is used as warm-up 
    vector<double> Vobs(nDays, -99);
    vector<double> Vsim(nDays, -99);
    for(int i=366; i<Vobs.size(); i++){
        Vobs[i] = Qobs[i];
        Vsim[i] = Qsim[i];
    }

    // calibration using NSE decomposition from Gupta et al., 2009 
    // (see http://www.meteo.mcgill.ca/~huardda/articles/gupta09.pdf):
    // obj 1) minimize relative variability (alpha)
    // obj 2) minimize absolute value of relative bias (beta)
    // obj 3) maximize correlation coefficient (r)
    double alpha = utils::computeStDev(Vsim) / utils::computeStDev(Vobs);
    double beta = fabs( utils::computeMean(Vsim) - utils::computeMean(Vobs) ) / utils::computeStDev(Vobs);
    double r = utils::computeCorr(Vsim, Vobs);
    // 3-objective calibration
    objs[0] = alpha;
    objs[1] = beta;
    objs[2] = -r;
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
    int nobjs = 3;
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
