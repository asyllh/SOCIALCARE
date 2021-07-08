Output Files
============

The program produces five output files: a text file, a CSV file, two image files containing plots, and a web file.

Results File
************

The results file is a text file (.txt) named as ``area_date_results.txt``, where the area and date correspond to the area provided in the user input and the date for the schedule respectively.

The results file contains the following information:

* Date: the current date and time
* Quality: the final quality of the schedule and route produced by the program
* Measure: the quality measure used to calculate the quality of the solution
* Carers: the number of carers in the schedule
* Jobs: the number of jobs in the schedule
* Total time: the total amount of time for all carers in the schedule, including service time, travel time (excluding travel to and from carers' homes), waiting time, and overtime
* Total service time: the total amount of time for all jobs
* Total travel time: the total amount of time spent travelling for all carers (excluding travel to and from carers' homes)
* Total waiting time: the total amount of waiting time for all carers
* Total tardiness: the total amount of tardiness for all jobs
* Total overtime: the total amount of overtime for all carers
*  Total mileage: the total distance travelled by all carers (excluding travel to and from carers' homes)
* Total cost: the total cost incurred by all carers as a combination of travel costs and mileage costs (excluding travel to and from carers' homes)
* Elapsed time: the total running time of the program


Solution File
*************

The solution file is a CSV file (.csv) named as ``area_date_solution.csv``, where the area and date correspond to the area provided in the user input and the date for the schedule respectively.

The solution file contains the initial information provided for the clients, along with the following information for each job:

* carer_id: the carer that is assigned to the job
* arrive_job: the time at which the carer arrives at the job location
* start_job: the time at which the carer starts the job
* depart_job: the time at which the carer leaves the job location
* travel_time: the time taken for the carer to travel to the job location from the previous job location (no travel time is provided for travel to the first job in a carer's route)
* waiting_time: the amount of time between the arrival of the carer at the job location and the start of the job
* tardiness: the amount of time between the end of the job's time window and the start of the job 


Image Files
***********

There are two image files (.png) containing plots produced by the program. The files are named ``area_date_time_info.png`` and ``area_date_workload.png``, where the area and date correspond to the area provided in the user input and the date for the schedule respectively.

The ``area_date_time_info.png`` file contains a pie chart depicting the percentage of total time spent on travel, waiting, and service.

The ``area_date_workload.png`` file provides a stacked bar chart depicting the time spent on travel, waiting, and service for each carer.

Web File
********

The final file produced is a web file (.html) which provides a visualisation of the schedule and route on a map. The file is named ``area_date.html``, where the area and date correspond to the area provided in the user input and the date for the schedule respectively.