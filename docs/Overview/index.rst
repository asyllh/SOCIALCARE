Overview
========

* Takes in an input file containing details for carers and clients availabilities and locations, and uses the information to produce a schedule and route for each carer to visit the clients.
* The algorithm used to determine a solution is a Greedy Randomised Adaptive Search Procedure (GRASP) combined with a Variable Neighbourhood Search (VNS) method, which is implemented in C. The pre- and post-processing of information is implemented in Python.
* This program is designed for use in social care to improve schedules/routes of carers to clients.