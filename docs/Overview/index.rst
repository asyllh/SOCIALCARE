Overview
========

Faced with a large number of carers and clients, each with different availabilities and locations, producing a schedule and travel routes for each carer by hand is extremely difficult and time consuming, and can result in an inefficient rota and/or incur extra costs.

CARROT is a program that develops a schedule and corresponding routes for carers to provide domiciliary care. The algorithm used to determine a solution is a Greedy Randomised Adaptive Search Procedure (GRASP) combined with a Variable Neighbourhood Search (VNS) method, which is implemented in C. The pre- and post-processing of information is implemented in Python.

..
    * Takes in an input file containing details for carers and clients availabilities and locations, and uses the information to produce a schedule and route for each carer to visit the clients.
    * The algorithm used to determine a solution is a Greedy Randomised Adaptive Search Procedure (GRASP) combined with a Variable Neighbourhood Search (VNS) method, which is implemented in C. The pre- and post-processing of information is implemented in Python.
    * This program is designed for use in social care to improve schedules/routes of carers to clients.
