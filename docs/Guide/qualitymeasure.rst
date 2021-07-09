Quality Measures
================

The four quality measures, 'Default', 'Ait H', 'Mankowska', and 'Workload Balance', have different objective functions, calculated as follows:

Default
*******
.. math::
		\text{quality} &= (-1\times \text{total travel time}) + (-1 \times \text{total waiting time}) \\
		&+ (-5 \times \text{allowed tardiness}) + (-5 \times \text{total overtime}) \\
		&+ (\text{wb coefficient} \times \text{min spare time}) + \text{total preference score}

where the variable 'wb coefficient' is the Workload Balance value provided by the user.

Ait Haddadene
*************
.. math::
	\text{quality} = -1\times ((0.3 \times \text{total travel time}) + \text{total preference score}) \\


Mankowska
*********
.. math::
	\text{quality} = -1\times \frac{\text{total travel time} + \text{allowed tardiness} + \text{max tardiness}}{3} \\


Workload Balance
****************
.. math::
	\text{quality} &= -1 \times ((0.3 \times \text{total travel time}) + \text{total preference score}) \\
	& + (-0.1 \times \text{length of day}) + (0.1 \times \text{min spare time})
	
where the variable 'length of day' is the difference between the longest and shortest days in the solution.