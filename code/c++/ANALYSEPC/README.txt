ANALYSEPC.exe Program
02/12/2020
---------------------

This program is designed to read in a csv file containing a list of postcodes and create a new csv file containing the status of the postcodes.
This program is only suitable for Wiltshire, Hampshire, and Monmouthshire.

The csv file to be analysed must be named "input.csv", and should comprise a single list (column) of postcodes. Note that it is assumed that the 
first line of the list contains the column name, i.e. "Postcodes".
The csv file produced by the program is named "output.csv" and comprises two lists. The first, named "Postcodes", contains a list of the postcodes 
provided in "input.csv". The second, names "Status", provides information on the given postcode: "OK" - the postcode can remain in the data for 
the project; "Remove" - for privacy reasons, the postcode is not suitable for the project and must be removed; and "Not Found" - the postcode cannot 
be identified.

To run the file, simply create a folder and place the "ANALYSEPC.exe" file into the folder, along with required "input.csv" file. Double-click 
the .exe file to run the program. The "output.csv" file should appear within the same folder.

An example "input_example.csv" file is provided which shows the required format of the input file.

---------------------