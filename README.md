###HBV Rainfall-Runoff Model (C++)

By [Matteo Giuliani](http://www.deib.polimi.it/personale/ruolo/dettaglio.php?id_persona=1250&idlang=eng&ruolo=6&lettera=), [Jon Herman](http://reed.cee.cornell.edu/index.php/Jon_Herman), and others.

HBV Rainfall-runoff model, based on the work by ([Bergstrom 1995](http://www.cabdirect.org/abstracts/19961904773.html)). Runs on a daily timestep and saves all states and fluxes from each day for further analysis. 

Both simulation and optimization (calibration) are available. Simulation mode is currently configured to read multiple parameter sets from `stdin` and evaluate them in order. Calibration is currently configured to work with [MOEAFramework](http://moeaframework.org), but may be easily modified for use with another application.

Contents:
* `data/`: Example forcing data files showing the input format
* `hbv_model.h`: Defines the `HBV` class to store all states and fluxes at each timestep over the course of the evaluation.
* `hbv_model.cpp`: Defines the functions for the processes in the model: degree-day snow, PDM soil moisture, Hamon PE, and the water balance between reservoirs. 
* `main_HBV.cpp`: Defines the initialization function (called once), the calculation function (called for each model evaluation), and the main function
* `CalHBV.java`: Example Java class for calibration with [MOEAFramework](http://moeaframework.org) (optional).
* `moeaframework.c/h`: Required libraries for communication with stdin/out
* `utils.cpp/h`: Utilities for vector operations

To compile and run:

* Run `make` to compile. Modify the makefile first to use a different compiler or flags.
* Run `./hbv my_forcing_data.txt my_output_file.txt < my_parameter_samples.txt` to perform simulation
* For calibration using [MOEAFramework](http://moeaframework.org), follow the instructions for connecting an external optimization problem [here](http://moeaframework.org/examples.html#example5). More detailed instructions are available from the [MOEAFramework Setup Guide](https://docs.google.com/document/pub?id=1Ts_tnvzZ-nDQ-Ym-RFtqM_LJMUNYKFZJ5WJdZxRmmrY). 
* Note that the second argument (the output filename) is only available in simulation mode.

Arguments:
* `my_forcing_data.txt`: see the `data/` directory for the format being used.
* `my_output_file.txt`: name of file to output performance metric(s) (simulation mode only)
* `my_parameter_samples.txt`: parameter sets to be evaluated in the model, with one parameter per column. Currently there are 12 parameters being read into the model, which would correspond to 12 columns per row of this file. The parameters are read from `stdin`, hence the `<` operator to pipe the contents of the file to the executable. The order of parameters to be read in can be modified at `main.cpp:60`.

In its current form, the model will output (or optimize) the mean squared error (MSE) from the time period. However, the output can easily be modified to include any combination of states/fluxes or error metrics from any time during the simulation.

Based on work from the following paper:
Herman, J.D., P.M. Reed, and T. Wagener (2013), Time-varying sensitivity analysis clarifies the effects of watershed model formulation on model behavior, Water Resour. Res., 49, doi:10.1002/wrcr.20124.
([Link to Paper](http://onlinelibrary.wiley.com/doi/10.1002/wrcr.20124/abstract))

Copyright (C) 2010-2013 Matteo Giuliani, Jon Herman, and others.

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
