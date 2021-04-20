# Import modules
import os
import sys
import glob
import math
import folium
import pickle
import requests # For website
import geopandas
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats


def inst_dicts_to_dfs(inst):
    inst['stats'] = pd.DataFrame(inst['stats'])
    inst['abi'] = pd.DataFrame(inst['abi'])
    inst['dst'] = pd.DataFrame(inst['dst'])
    return inst
### --- End inst_dicts_to_dfs --- ###

def timemins_to_string(mins):
    minsRound = math.floor(mins)
    # print('minsRound: ', minsRound)
    hours = mins // 60
    # print('hours:', hours)
    minutes = (minsRound % 60)
    # print('minutes: ', minutes)
    seconds = (mins - minsRound) * 60
    seconds = round(seconds)
    # print('seconds: ', seconds)
    return('{0:0>2}:{1:0>2}:{2:0>2}'.format(int(hours), int(minutes), int(seconds)))
### --- End def timemins_to_string --- ###

def create_df(filename, a_name=''):
    # filename = '../outputs/results/HC.csv' #File from Abicare
    # a_name = 'Hampshire'
    df = pd.read_csv(filename)
    # df['Date'] = pd.to_datetime(df['Date'], format='%d-%b-%y %I:%M %p')

    inst = {
            'name' : a_name,
            'all_dates' : [],
            'nNurses' : 0,
            'nJobs' : 0,
            'nClients' : 0,
            'stats' : {'nurses' : [], 'jobs' : [], 'clients' : [], 'servicetime' : []},
            'abi' : {'ttime' : [], 'ttravel' : [], 'ttravelosrm' : [], 'twaiting' : [], 'twaitingosrm' : [], 'dist' : [], 'distjobs' : [], 'distjobsosrm' : [], 'aith': [], 'mk' : [], 'wb' : [], 'paper' : [], 'aithosrm': [], 'mkosrm' : [], 'wbosrm' : [], 'paperosrm' : []},
            'dst' : {'ttime' : [], 'ttravel' : [], 'twaiting' : [], 'dist' : [], 'distjobs' : [], 'aith': [], 'mk' : [], 'wb' : [], 'paper' : [], 'algtime' : [], 'iterations' : []},
            'best' : {'ttime' : [], 'ttravel' : [], 'ttravelosrm' : [], 'twaiting' : [], 'twaitingosrm' : [], 'dist' : [], 'distjobs' : [], 'distjobsosrm' : [], 'aith' : [], 'mk' : [], 'wb' : [], 'paper' : [], 'aithosrm': [], 'mkosrm' : [], 'wbosrm' : [], 'paperosrm' : []}
            }

    inst['nNurses'] = df.iloc[0]['nNurses']
    inst['nJobs'] = df.iloc[0]['nJobs']
    inst['nClients'] = df.iloc[0]['nClients']

    days_in_df = df['Date'].unique() # List of Dates in df
    # print(days_in_df)

    # exit(-1)

    for day in days_in_df:
        day_df = df[df['Date'] == day] # day_df = df but only containing infor for current area (area_df) and current day
        inst['all_dates'].append(day)
        inst['stats']['nurses'].append(day_df.iloc[0]['Nurses'])
        inst['stats']['jobs'].append(day_df.iloc[0]['Jobs'])
        inst['stats']['clients'].append(day_df.iloc[0]['Clients'])
        inst['stats']['servicetime'].append(day_df.iloc[0]['Service Time'])

        inst['abi']['ttime'].append(day_df.iloc[0]['Total Abi'])
        inst['abi']['ttravel'].append(day_df.iloc[0]['Travel Abi'])
        inst['abi']['ttravelosrm'].append(day_df.iloc[0]['Travel AbiOsrm'])
        inst['abi']['twaiting'].append(day_df.iloc[0]['Waiting Abi'])
        inst['abi']['twaitingosrm'].append(day_df.iloc[0]['Waiting AbiOsrm'])
        inst['abi']['dist'].append(day_df.iloc[0]['Dist Abi'])
        inst['abi']['distjobs'].append(day_df.iloc[0]['DistJobs Abi'])
        inst['abi']['distjobsosrm'].append(day_df.iloc[0]['DistJobs AbiOsrm'])

        inst['abi']['aith'].append(day_df.iloc[0]['AitH Abi'])
        inst['abi']['mk'].append(day_df.iloc[0]['MK Abi'])
        inst['abi']['wb'].append(day_df.iloc[0]['WB Abi'])
        inst['abi']['paper'].append(day_df.iloc[0]['Paper Abi'])

        inst['abi']['aithosrm'].append(day_df.iloc[0]['AitH AbiOsrm'])
        inst['abi']['mkosrm'].append(day_df.iloc[0]['MK AbiOsrm'])
        inst['abi']['wbosrm'].append(day_df.iloc[0]['WB AbiOsrm'])
        inst['abi']['paperosrm'].append(day_df.iloc[0]['Paper AbiOsrm'])

        inst['dst']['ttime'].append(day_df.iloc[0]['Total DST'])
        inst['dst']['ttravel'].append(day_df.iloc[0]['Travel DST'])
        inst['dst']['twaiting'].append(day_df.iloc[0]['Waiting DST'])
        inst['dst']['dist'].append(day_df.iloc[0]['Dist DST'])
        inst['dst']['distjobs'].append(day_df.iloc[0]['DistJobs DST'])

        inst['dst']['aith'].append(day_df.iloc[0]['AitH DST'])
        inst['dst']['mk'].append(day_df.iloc[0]['MK DST'])
        inst['dst']['wb'].append(day_df.iloc[0]['WB DST'])
        inst['dst']['paper'].append(day_df.iloc[0]['Paper DST'])
        inst['dst']['algtime'].append(day_df.iloc[0]['Time DST'])
        inst['dst']['iterations'].append(day_df.iloc[0]['Iterations DST'])

        inst['best']['ttime'].append(day_df.iloc[0]['Total Best'])
        inst['best']['ttravel'].append(day_df.iloc[0]['Travel Best'])
        inst['best']['ttravelosrm'].append(day_df.iloc[0]['Travel Osrm Best'])
        inst['best']['twaiting'].append(day_df.iloc[0]['Waiting Best'])
        inst['best']['twaitingosrm'].append(day_df.iloc[0]['Waiting Osrm Best'])
        inst['best']['dist'].append(day_df.iloc[0]['Dist Best'])
        inst['best']['distjobs'].append(day_df.iloc[0]['DistJobs Best'])
        inst['best']['distjobsosrm'].append(day_df.iloc[0]['DistJobs Osrm Best'])

        inst['best']['aith'].append(day_df.iloc[0]['AitH Best'])
        inst['best']['mk'].append(day_df.iloc[0]['MK Best'])
        inst['best']['wb'].append(day_df.iloc[0]['WB Best'])
        inst['best']['paper'].append(day_df.iloc[0]['Paper Best'])

        inst['best']['aithosrm'].append(day_df.iloc[0]['AitH Osrm Best'])
        inst['best']['mkosrm'].append(day_df.iloc[0]['MK Osrm Best'])
        inst['best']['wbosrm'].append(day_df.iloc[0]['WB Osrm Best'])
        inst['best']['paperosrm'].append(day_df.iloc[0]['Paper Osrm Best'])

    return inst
### --- End create_df --- ###

def compare_abi_dst():
    all_instances = pickle.load(open('all_compare_abi_dst.p', 'rb'))
    h_dict = all_instances[0]
    m_dict = all_instances[1]
    a_dict = all_instances[2]
    area_name = 'All Areas'
    # print(ad_dict['abi'])
    # area_name = ad_dict['name']
    # print(h_dict['name'])
    # print(m_dict['name'])
    # print(a_dict['name'])
    # print(h_dict['abi']['ttime'].to_list())
    # print(m_dict['abi']['ttime'].to_list())
    # print(a_dict['abi']['ttime'].to_list())
    # print(type(h_dict['abi']['ttime']))
    # exit(-1)
    # h_abittime = h_dict['abi']['ttime'].to_numpy(copy=True)
    # h_abittime = h_dict['abi']['ttime'].to_numpy()
    # print('type')
    # print(type(h_abittime))
    # print(type(h_dict['abi']['ttime']))
    # print(h_abittime)
    # print(h_dict['abi']['ttime'])
    # exit(-1)
    ttime_abi = h_dict['abi']['ttime'].to_list() + m_dict['abi']['ttime'].to_list() + a_dict['abi']['ttime'].to_list()
    # print(ttime_abi)
    # exit(-1)
    ttime_dst = h_dict['dst']['ttime'].to_list() + m_dict['dst']['ttime'].to_list() + a_dict['dst']['ttime'].to_list()
    # ttime_dst = ad_dict['dst']['ttime'].to_list()
    # print(ttime_dst)
    numpoints = len(ttime_abi)
    # print('numpoints: ', numpoints)
    plot_time_comparison(ttime_abi, ttime_dst, numpoints, area_name, plot_type='ttime')
### --- End compare_abi_dst --- ###

def plot_time_comparison(xlist, ylist, numpoints, area_name, plot_type=''):

    slope, intercept, r_value, p_value, std_err = stats.linregress(xlist, ylist)
    print('DST Time = ', slope, '* Abicare Time +', intercept)
    # print('Abicare Time = ', slope, '* OSRM Time +', intercept)
    print('r_value:', r_value)
    print('p_value:', p_value)
    print('std_err:', std_err)

    # Create figure:
    fig = plt.figure()
    # plt.axis('square')
    if plot_type == 'ttime':
        fig.suptitle(area_name + ' Total Time Comparison', fontsize=13, fontweight='bold')
    elif plot_type == 'ttravel':
        fig.suptitle(area_name + ' Total Travel Time Comparison', fontsize=13, fontweight='bold')
    elif plot_type == 'twaiting':
        fig.suptitle(area_name + ' Total Waiting Time Comparison', fontsize=13, fontweight='bold')
    else:
        print('ERROR: No plot_type given.')
        exit(-1)
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.82)
    ax.set_xlabel('Abicare Time (mins)')
    ax.set_ylabel('DST Time (mins)')
    # ax.set_xlabel('OSRM Time (mins)')
    # ax.set_ylabel('Abicare Time (mins)')

    # Create subtitle:
    slope = round(slope, 3)
    intercept = round(intercept, 3)
    r_value = round(r_value, 3)
    p_value = round(p_value, 3)
    std_err = round(std_err, 3)
    strslope = str(slope)
    strintercept = str(intercept)
    strr_value = str(r_value)
    strp_value = str(p_value)
    strstd_err = str(std_err)
    strnumpoints = str(numpoints)
    subtitle = 'gradient: ' + strslope + '  intercept: ' + strintercept + '\nr_val: ' + strr_value + '  p_val: ' + strp_value + '  err: ' + strstd_err + '\npoints: ' + strnumpoints 
    # if timeofday == 'peak':
    #     subtitle = subtitle + '\n7am-10am, 4pm-7pm'
    # elif timeofday == 'offpeak':
    #     subtitle = subtitle + '\n12am-7am, 10am-4pm, 7pm-12am'
    ax.set_title(subtitle, fontsize=8)

    # Polynomial fit:
    pfit = np.polyfit(xlist, ylist, 2)
    polyfit = np.poly1d(pfit)

    # Regression line:
    xmin, xmax = np.min(xlist), np.max(xlist)
    x_reg_line = np.array([xmin, xmax])
    y_reg_line = intercept + slope*x_reg_line
    # plt.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Polynomial:
    x_poly = np.linspace(xmin, xmax, 100)
    # plt.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')

    # Plot the points (scatter) and polynomial and regression lines:
    # plt.xticks(np.arange(xmin, xmax+1, 250.0))
    # plt.yticks(np.arange(xmin, xmax+1, 250.0))
    # plt.xticks(np.arange(0, 51, 5))
    # plt.yticks(np.arange(0, 11, 1))
    ax.scatter(xlist, ylist, marker='.', color='black')
    # ax.plot(x_poly, polyfit(x_poly), color='b', label='Polynomial')
    ax.plot(x_reg_line, y_reg_line, color='r', label='Regression Line')

    # Create the legend, save and show the plot:
    # ax.legend()
    if plot_type == 'ttime':
        plt.savefig('all_ttime_comparison.png')
        # plt.savefig(area_name +'_peaktime_comparison_osrm.png')
    elif plot_type == 'ttravel':
        plt.savefig('all_ttravel_comparison.png')
        # plt.savefig(area_name +'_offpeaktime_comparison_osrm.png')
    elif plot_type == 'twaiting':
        plt.savefig('all_twaiting_comparison.png')
        # plt.savefig(area_name + '_time_comparison_osrm.png')

    plt.show()
### --- End of def plot_time_comparison --- ###   

# compare_abi_dst()

# all_instances = []

# filename1 = '../outputs/results/HampshireComparison.csv' #File from Abicare
# inst1 = create_df(filename1, 'Hampshire')
# all_instances.append(inst_dicts_to_dfs(inst1).copy())

# filename2 = '../outputs/results/MonmouthComparison.csv' #File from Abicare
# inst2 = create_df(filename2, 'Monmouth')
# all_instances.append(inst_dicts_to_dfs(inst2).copy())

# filename3 = '../outputs/results/AldershotComparison.csv' #File from Abicare
# inst3 = create_df(filename3, 'Aldershot')
# all_instances.append(inst_dicts_to_dfs(inst3).copy())


# # pickle.dump(inst, open('all_compare_abi_dst.p', 'wb'))

# pickle.dump(all_instances, open('all_compare_abi_dst.p', 'wb'))
# print('Done.')
# exit(-1)
