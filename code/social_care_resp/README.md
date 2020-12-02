# README #

This repository contains the code supporting the IAA project "Increasing productivity in the domiciliary care sector through automated routing and scheduling of carers".

### Compiling the code ###

The file ```dll_compilation_anaconda3.bat``` can compile the C code when run from the ```Visual Studio 2019 Developer Command Prompt v16.7.5```.
It needs to be linked with Anaconda, and should generate a library (DLL in Windows) file in ```./bin```

### Running the code ###

The main file to run the code is ```constructive_wrapper.py```. 
The variable ```instance_type``` should be set to the type of file that needs to be run. It can be one of the following: ```'excel', 'mankowska', 'ait_h', 'pfile'```.
In the rest of the script, the folders, and files to be run can be selected commenting or enabling the appropriate lines. A good first example is the ```'mankowska'``` instance ```'saved_InstanzCPLEX_HCSRP_10_1.mat'```.
The code will run a list of instances in a sequence if the ```file_list``` variable has more than one element.


### Notes on using excel format ###

The Excel format currently requires access to an OSRM server to calculate distances.