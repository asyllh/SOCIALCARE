Requirements
============

Description of the requirements for the program (Code-Point Open and OSRM).

Code-Point Open
***************

The program uses Code-Point Open to obtain location data for carers and clients. Code-Point Open is available to download for free at https://www.ordnancesurvey.co.uk/business-government/products/code-point-open. 

The data format to download should be a CSV. Once the download is complete, extract the zip file and save the resulting ``codepo_gb`` folder into the same directory as the program file.

* Code-Point Open is used to obtain exact locations from postcodes in the form of coordinates (latitude and longitude) from a free database provided by Ordnance Survey.
* Code-Point Open is a necessary requirement for the execution of the program.

OSRM
****

* The Open Source Routing Machine (OSRM) is an open-source router designed for use with data from the OpenStreetMap project.
* OSRM is used to obtain distances and travel times between coordinates.
* OSRM is a necessary requirement for the execution of the program.
