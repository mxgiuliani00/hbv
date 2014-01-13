/*
Copyright (C) 2013 Matteo Giuliani, Jon Herman, and others.

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

#include "hbv_model.h"

using namespace std;

#define PI 3.141592


hbv_model::hbv_model() {
    // TODO Auto-generated constructor stub
}

hbv_model::~hbv_model() {
    // TODO Auto-generated destructor stub
}

hbv_model::hbv_model(int pDayStartId, int pNdays, int pStartId){

    dayStartIndex = pDayStartId;
    nDays = pNdays;
    startingIndex = pStartId;

}

void hbv_model::hbv_allocate(int nDays)
{
    states.stw1   = new double [nDays];
    states.stw2   = new double [nDays];

    states.sowat   = new double [nDays];
    states.sdep    = new double [nDays];

    tst = 24*3600; // daily timestep

    // (these will be reset after MaxBas is read in)
    fluxes.Qrouting = new double [1];
    
    //Allocate the array used to store the modelled Q, and other things
    fluxes.Qsim = new double [nDays];
    fluxes.actualET = new double [nDays];

    return;
}


double hbv_model::snow(int modelDay)
{
    double smelt = 0.0;
    double eff_precip = 0.0; //effective precip initialized to zero

    // Read in temperature and precip data for this time step
    double avg_temp = data.avgTemp[startingIndex + modelDay];
    double precip = data.precip[startingIndex + modelDay];

    // starting point: equal to yesterday
    states.sdep[modelDay] = states.sdep[modelDay-1];

    // Snow/Rain
    if (avg_temp < params.ttlim)
        states.sdep[modelDay] += precip;    // if temperature is lower than threshold (ttlim) --> precip is all snow
	else 
        eff_precip += precip;               // otherwise --> add precip to effective precip

    // Snow melt if temperature > threshold (degw)
    if (avg_temp > params.degw)
    {
        //If there is actually snow to melt in the snow store...
        if (states.sdep[modelDay] > 0.0)
        {
            //Calculate snow melt using degree-day factor (degd)
            smelt = (avg_temp - params.degw)*params.degd;
            //If snow melt that wants to occur is more than what is actually stored...
            if (smelt > states.sdep[modelDay])
            {
                eff_precip += states.sdep[modelDay];    //add full snow depth to effective precip
                states.sdep[modelDay] = 0.0;            //All of the snow has melted
            }
            else //Otherwise, we melt a portion of the snow store
            {
                eff_precip += smelt;                //effective precip is precip together with what acutally melted
                states.sdep[modelDay] -= smelt;     //Remove the amount that melted from the snow store
            }
        }
    }

    return eff_precip;
}


void hbv_model::soil(double eff_precip, int modelDay)
{
    double hsw, AET, runoff_depth;

    double fcap = params.fcap;
    double lp = params.lp;
    double beta = params.beta;
    double PET = evap.PE[modelDay];

    // starting point: equal to yesterday's storage
    states.sowat[modelDay] = states.sowat[modelDay-1];

    //If the soil moisture storage is already at capacity, runoff = all precip + excess
    if (states.sowat[modelDay] >= fcap) {
        runoff_depth = eff_precip + (states.sowat[modelDay] - fcap);
        states.sowat[modelDay] = fcap;
    }
    else
    {
        //This is the portion of the effective precip that goes into storage
        hsw = eff_precip * (1.0 - pow((states.sowat[modelDay]/fcap), beta));
        states.sowat[modelDay] += hsw;
        runoff_depth = eff_precip - hsw;

        //If the amount going into the soil moisture storage will result in exceeding the capacity of the store...
        if (states.sowat[modelDay] > fcap)
        {
            runoff_depth += (states.sowat[modelDay] - fcap);
            states.sowat[modelDay] = fcap; //We are at capacity
        }
    }

    AET = PET*min(states.sowat[modelDay-1]/(fcap*lp), 1.0); // actual ET, after adjusting for saturation in soil layer
    if (AET < 0.0) AET = 0.0;

    //If there is enough in the soil moisture store to supply the AET, subtract it
    if (states.sowat[modelDay] > AET) {
        fluxes.actualET[modelDay] = AET;
        states.sowat[modelDay] -= AET;
    }
    else {
        fluxes.actualET[modelDay] = states.sowat[modelDay];
        states.sowat[modelDay] = 0.0; // all of it evaporates
    }

    states.stw1[modelDay] += states.stw1[modelDay-1] + runoff_depth;

    return;
}


double hbv_model::discharge(int modelDay)
{

    double Q0, Q1, Q2, Qall;

    //If the upper reservoir water level is above the threshold for near surface flow
    if (states.stw1[modelDay] > params.hl1)
    {
        //Calculate it, and remove it from the reservoir
        Q0 = (states.stw1[modelDay] - params.hl1)*params.ck0;
        states.stw1[modelDay] -= Q0;
    }
    else Q0 = 0.0;

    //If there is still water left in the upper reservoir
    if (states.stw1[modelDay] > 0.0)
    {
        //Calculate what now goes into interflow, and remove it
        Q1 = states.stw1[modelDay] * params.ck1;
        states.stw1[modelDay] -= Q1;
    }
    else Q1 = 0.0;

    //If there is still anough water in the upper reservois to completely supply percolation...
    if (states.stw1[modelDay] > params.perc)
    {
        // Move the amount from the upper to the lower reservoir
        states.stw1[modelDay] -= params.perc;
        states.stw2[modelDay] += params.perc;
    }
    else
    {
        //We just put what we can from the upper into the lower
        states.stw2[modelDay] += states.stw1[modelDay];
        states.stw1[modelDay] = 0.0;
    }

    //If there is water in the lower reservoir...
    if (states.stw2[modelDay] > 0.0)
    {
        //Calculate base flow, and remove it
        Q2 = states.stw2[modelDay] * params.ck2;
        states.stw2[modelDay] -= Q2;
    }
    else Q2 = 0.0;

    Qall = (Q0 + Q1 + Q2); // total dischargearge - mm per timestep
    return Qall;
}


void hbv_model::routing(double Qall, int modelDay)
{
    ///////////////////////////////////////////////////////////
    //Parameter in code | parameter in manual/lit | description
    ///////////////////////////////////////////////////////////
    //Qall | Q0+Q1+Q2 | Total dischargearge from both reservoirs

    int m2;
    double wsum;
    double *wei = new double [params.maxbas];

    ///////////////////////////////////////////////////////////
    //Variable in code | variable in manual/lit | description
    ///////////////////////////////////////////////////////////
    //wei | g(t,MAXBAS) | transformation function consisting os a triangular weighting function and one free parameter
    //Qrouting | NA | This is the flow from the single Qall spread out over time according to the transformation function
    //Qsim | NA | The final flow output by the model

    m2   = (params.maxbas / 2)-1;
    wsum =  0.0;

    //Calculate the values of the transformation function according to maxbas
    for (int i=0; i< params.maxbas; i++)
    {
        if (i <= m2) wei[i] = double(i+1);
        else wei[i] = double(params.maxbas - (i+1)) + 1.0;
        wsum += wei[i];
    }

    //Now, spread the flow Qall out over Qind according to the transformation function
    for (int i=0; i < params.maxbas; i++)
    {
        wei[i] /= wsum;
        //Qind is constantly added to by the transformed Qall.  In other words, when Qall is transformed (spread out over time)
        //it is then added to whatever currently exists in Qind for those time steps.  In other words, a previous transformation of
        //Qall for the previous time step placed flows in Qind in times that overlapped with the currently transformed flow times.
        fluxes.Qrouting[i] += Qall * wei[i];
    }

    fluxes.Qsim[modelDay] = fluxes.Qrouting[0];

    delete[] wei;
    return;
}


void hbv_model::backflow()
{
    int klen = 2*params.maxbas-1;
  
    for (int k = 0; k < klen; k++)
    {
        fluxes.Qrouting[k] = fluxes.Qrouting[k+1];
    }
    fluxes.Qrouting[klen] = 0.0;
    return;
}


void hbv_model::reinitForMaxBas()
{
    delete[] fluxes.Qrouting;
    fluxes.Qrouting = new double [2*params.maxbas];

    for (int i = 0; i < 2*params.maxbas; i++)
    {
        fluxes.Qrouting[i] = 0.0;
    }

    return;
}

void hbv_model::hbv_delete(int nDays)
{
    delete[] states.stw1;
    delete[] states.stw2;
    delete[] states.sowat;
    delete[] states.sdep;
    delete[] fluxes.Qrouting;
    delete[] fluxes.Qsim;
    delete[] fluxes.actualET;

    for (int i = 0; i < nDays; i++) delete[] data.date[i];
    delete[] data.date;
    delete[] data.precip;
    delete[] data.evap;
    delete[] data.flow;
    delete[] evap.PE;
    if(data.tempData>1){
        delete[] data.maxTemp;
        delete[] data.minTemp;
    }
    delete[] data.avgTemp;

    return;
}

void hbv_model::init_HBV(string dataFile)
{
    hbv_allocate(nDays);
    readData(dataFile);

    //Calculate the Hamon Potential Evaporation for the time series
    calculateHamonPE(startingIndex, nDays, dayStartIndex);

    return;
}

void hbv_model::calc_HBV(double* parameters)
{

    // Zero everything at the first timestep
    states.stw1[0] = 0.0; // shallow layer storage
    states.stw2[0] = 0.0; // deep layer storage
    states.sowat[0]   = 0.0; // surface soil storage
    states.sdep[0]    = 0.0; // snow depth
    fluxes.Qsim[0] = 0.0;
    fluxes.actualET[0] = 0.0;

    // assign parameters to HBV structure
    // Rate constants K0, K1, K2: entered with units of 1/day, but converted to unitless
    params.ck2 = 1.0 / parameters[0] * tst / (3600.0 * 24.0);
    params.ck1 = 1.0 / parameters[1] * tst / (3600.0 * 24.0);
    params.ck0 = 1.0 / parameters[2] * tst / (3600.0 * 24.0);
    params.maxbas  = ROUNDINT(parameters[3] / 24); // Number of days for hydrograph routing
    params.degd = parameters[4] * tst / (3600.0 * 24.0); // Degree-day factor [mm/(degC-d)]
    params.degw = parameters[5]; // Snowmelt threshold [degC]
    params.ttlim = parameters[6]; // Temp to start snowing [degC]
    params.perc = parameters[7]; // Percolation [mm/d]
    params.beta = parameters[8]; // Beta (soil moisture exponent, unitless)
    params.lp = parameters[9]; // Unitless evaporation constant
    params.fcap = parameters[10]; // Max storage of soil layer [mm]
    params.hl1 = parameters[11]; // Max storage of shallow layer [mm]


    //After we have read in maxbas, make sure we reinitialize the arrays based on maxbas
    reinitForMaxBas();

    // Now run the components of the model
    double Qall, eff_precip;

    // Run over daily timesteps (starting at 1)
    for (int day = 1; day < nDays; day++)
    {
        //Degree-day snow module (sets eff_precip value)
        eff_precip = snow(day);

        //Soil/ET module (sets runoff_depth value)
        soil(eff_precip, day);

        // Calculate the resulting dischargearge Qall
        Qall = discharge(day);

        // Route Qall using MaxBas routing
        routing(Qall, day);

        // Shift the routing arrays to the next timestep
        backflow();
    }

    return;
}


void hbv_model::readData(string filename){

    ifstream in;
    string sJunk = "";
    int ijunk;
    double dTemp;

    in.open(filename.c_str(), ios_base::in);
    if(!in)
    {
        cout << "The input file specified: " << filename << " could not be found!" << endl;
        exit(1);
    }

    //Look for the <WATERSHED_NAME> key
    while (sJunk != "<WATERSHED_NAME>")
    {
        in >> sJunk;
    }
    in >> data.ID;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <GAGE_LATITUDE> key
    while (sJunk != "<GAGE_LATITUDE>")
    {
        in >> sJunk;
    }
    in >> data.gageLat;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <GAGE_LONGITUDE> key
    while (sJunk != "<GAGE_LONGITUDE>")
    {
        in >> sJunk;
    }
    in >> data.gageLong;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <DRAINAGE_AREA> key
    while (sJunk != "<DRAINAGE_AREA>")
    {
        in >> sJunk;
    }
    in >> data.DA;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <TIME_STEPS> key
    while (sJunk != "<TIME_STEPS>")
    {
        in >> sJunk;
    }
    in >> data.nDays;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <TEMP_DATA> key
    while (sJunk != "<TEMP_DATA>")
    {
        in >> sJunk;
    }
    in >> data.tempData;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Allocate the arrays
    data.date = new int* [data.nDays];
    for (int i=0; i<data.nDays; i++) data.date[i] = new int[3];
    data.precip   = new double[data.nDays];
    data.evap     = new double[data.nDays];
    data.flow     = new double[data.nDays];

    if(data.tempData>1){
        data.maxTemp  = new double[data.nDays];
        data.minTemp  = new double[data.nDays];
    }
    data.avgTemp  = new double[data.nDays];


    //Look for the <DATA_START> key
    while (sJunk != "<DATA_START>")
    {
        in >> sJunk;
    }
    //Once we found the key, ignore the rest of the line and move to the data
    in.ignore(1000,'\n');
    //Loop through all of the input data and read in this order:
    for (int i=0; i<data.nDays; i++)
    {
        in >> dTemp;
        data.date[i][0] = int(dTemp);
        in >> dTemp;
        data.date[i][1] = int(dTemp);
        in >> dTemp;
        data.date[i][2] = int(dTemp);
        if(data.tempData > 1){ // max and min temperatures
            in >> data.precip[i] >> data.flow[i] >> data.maxTemp[i] >> data.minTemp[i];
            data.avgTemp[i] = (data.maxTemp[i] + data.minTemp[i])/2.0;
        }else{
            in >> data.precip[i] >> data.flow[i] >> data.avgTemp[i] ;
        }

        in.ignore(1000,'\n');
    }

    //Close the input file
    in.close();

    return;

}


void hbv_model::calculateHamonPE(int dataIndex, int nDays, int startDay){

    int oldYear;
    int counter;

    //Allocate
    evap.PE        = new double [nDays];

    //Initialize the starting year
    oldYear = data.date[dataIndex][0];
    counter = startDay-1;

    //Fill out each of the arrays
    for (int i=0; i<nDays; i++)
    {

        //If the years hasn't changed, increment counter
        if (data.date[dataIndex+i][0] == oldYear) counter++;
        //If it has changed, reset counter - this handles leap years
        else counter = 1;

        evap.day = counter;

        evap.P = asin(0.39795*cos(0.2163108 + 2.0 * atan(0.9671396*tan(0.00860*double(evap.day-186)))));
        evap.dayLength = 24.0 - (24.0/PI)*(acos((sin(0.8333*PI/180.0)+sin(data.gageLat*PI/180.0)*sin(evap.P))/(cos(data.gageLat*PI/180.0)*cos(evap.P))));
        evap.eStar = 0.6108*exp((17.27*data.avgTemp[dataIndex+i])/(237.3+data.avgTemp[dataIndex+i]));
        evap.PE[i] = (715.5*evap.dayLength*evap.eStar/24.0)/(data.avgTemp[dataIndex+i] + 273.2);

        oldYear = data.date[dataIndex+i][0];
    }

    return;
}


MyData hbv_model::getData(){
    return data;
}

hbv_fluxes hbv_model::getFluxes(){
    return fluxes;
}
