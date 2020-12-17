# Import modules
import pickle
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

# import convert_dict_inst as cdi

def plot_time_comparison(xlist, ylist):
    slope, intercept, r_value, p_value, std_err = stats.linregress(xlist, ylist)
    print('y = ', slope, '*x +', intercept)
    print('r_value:', r_value)
    print('p_value:', p_value)
    print('std_err:', std_err)

    # Polynomial fit:
    pfit = np.polyfit(xlist, ylist, 2)
    polyfit = np.poly1d(pfit)

    # Plot the points:
    plt.scatter(xlist, ylist, marker='.', color='black')
    plt.xticks(np.arange(min(xlist), max(xlist)+1, 2.0)) # x-axis is labelled with values 0,2,4,...
    plt.yticks(np.arange(min(ylist), max(ylist)+1, 2.0)) # y-axis is labelled with values 0,2,4,...
    plt.xlabel('X Time (mins)')
    plt.ylabel('Y Time (mins)')
    # Plot the regression line:
    xmin, xmax = np.min(xlist), np.max(xlist)
    x_reg_line = np.array([xmin, xmax])
    y_reg_line = intercept + slope*x_reg_line
    plt.plot(x_reg_line, y_reg_line, color='hotpink')

    # And the polynomial:
    x_poly = np.linspace(xmin, xmax, 100)
    plt.plot(x_poly, polyfit(x_poly), color='dodgerblue')

    plt.show()

# all_instances = pickle.load(open('tools_and_scripts/all_inst_salisbury.p', 'rb'))

# idict = 7th instance in all_instances, which is 08-Nov-2020 in Salisbury. Chose this inst as it is the only one which has all postcodes for carers and clients. ncarers = 12, ntasks = 100.
# idict = all_instances[6]
# Assign different latitudes and longitudes to the carers and clients that do not have them in idict due to privacy reasons (so lat/lon = nan) - this is just so we can use this instance for a test.
# idict = cdi.assign_nan_lon_lat(idict)
# nNurses = idict['stats']['ncarers']
# nJobs = idict['stats']['ntasks']
# print('nNurses: ', nNurses, 'nJobs: ', nJobs)

osrmtimes02 = [3.02, 6.325, 5.888333333333334, 7.4383333333333335, 4.739999999999999, 5.046666666666667, 7.858333333333333, 5.113333333333333, 6.010000000000001, 12.24, 5.091666666666667, 12.24, 3.9133333333333336, 4.286666666666666, 5.795, 3.9133333333333336, 4.739999999999999, 3.8400000000000003, 12.123333333333333, 9.706666666666667, 3.868333333333333, 7.858333333333333, 3.9133333333333336, 4.739999999999999, 3.7333333333333334, 6.348333333333333, 20.459999999999997, 3.61, 8.201666666666666, 8.363333333333333, 4.680000000000001, 0.0, 4.376666666666667, 1.0483333333333333, 13.795, 13.931666666666667]
esttimes02 = [5.41, 7.08, 9.08, 9.03, 5.24, 6.36, 7.45, 6.4, 8.41, 13.34, 6.38, 13.34, 6.16, 5.26, 6.23, 6.16, 5.24, 4.36, 14.41, 11.3, 2.06, 7.45, 6.16, 5.24, 4.21, 6.37, 24.29, 5.14, 9.48, 7.26, 6.06, 11.42, 5.23, 1.29, 12.55, 12.54]

osrmtimes03 = [7.743333333333334, 3.8400000000000003, 5.888333333333334, 7.4383333333333335, 4.739999999999999, 11.526666666666667, 5.046666666666667, 5.091666666666667, 12.24, 12.139999999999999, 4.781666666666666, 3.658333333333333, 13.795, 16.175, 3.658333333333333, 6.010000000000001, 3.9133333333333336, 4.739999999999999, 2.243333333333333, 5.793333333333334, 4.296666666666667, 3.9133333333333336, 4.376666666666667, 4.739999999999999, 7.333333333333333, 3.7333333333333334, 3.9050000000000002, 3.61, 6.010000000000001, 8.12, 4.680000000000001, 11.573333333333332, 2.9066666666666667, 5.831666666666666, 7.1066666666666665, 2.5, 4.376666666666667, 12.24, 12.816666666666666, 7.858333333333333]
esttimes03 = [8.21, 4.36, 9.08, 9.03, 5.24, 12.26, 6.36, 6.38, 13.34, 13.04, 5.48, 4.46, 12.55, 15.42, 4.46, 8.41, 6.16, 5.24, 2.29, 7.29, 6.22, 6.16, 5.23, 5.24, 7.42, 4.21, 3.45, 5.14, 6.43, 9.47, 6.06, 14.0, 4.4, 7.11, 8.58, 3.34, 5.23, 13.34, 13.42, 7.45]

osrmtimes08 = [4.585, 2.5133333333333336, 4.781666666666666, 3.658333333333333, 3.658333333333333, 6.010000000000001, 6.83, 8.3, 5.948333333333333, 5.046666666666667, 5.091666666666667, 0.0, 12.123333333333333, 9.706666666666667, 5.091666666666667, 5.2716666666666665, 10.335, 12.973333333333333, 3.7333333333333334, 5.888333333333334, 7.4383333333333335, 6.698333333333333, 6.471666666666667, 4.376666666666667]
esttimes08 = [8.05, 3.25, 5.48, 4.46, 4.46, 8.41, 9.47, 9.02, 8.26, 6.36, 6.38, 0.0, 14.41, 11.3, 6.38, 8.07, 12.38, 12.45, 4.21, 9.08, 9.03, 5.46, 5.55, 5.23] # ORIGINAL

osrmtimes = osrmtimes02 + osrmtimes03 + osrmtimes08
esttimes = esttimes02 + esttimes03 + esttimes08
print('osrmtimes: ', len(osrmtimes), ' esttimes: ', len(esttimes))

# plot_time_comparison(osrmtimes, esttimes)
# plot_time_comparison(esttimes, osrmtimes)

