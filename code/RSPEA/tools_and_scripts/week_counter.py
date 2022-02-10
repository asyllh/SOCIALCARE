#--------------------#
# RSPEA
# Functions to calculate the index value for the day of the week based on 2-week and 8-week schedules
# 26/01/2022
#--------------------#

import pandas as pd

def date_in_visit_cycle(given_date, ref_date, week_cycle):
    """
    Calculates whether there is a visit due
    in the week of "given_date", based on a
    contract that started on "ref_date" and is
    on a "week_cycle" visit cycle.
    """
    week_diff = calculate_week_difference(given_date, ref_date)
    return week_diff % week_cycle == 0


def date_to_mon_9am(given_date):
    mon = given_date - pd.Timedelta(days=given_date.dayofweek)
    # print('mon before replace: ', mon)
    mon.replace(hour=9, minute=0, second=0)
    return mon

def calculate_week_difference(planning_date, ref_date):
    # We use Monday 9 am for reference
    monday1 = date_to_mon_9am(planning_date)
    # print('monday1: ', monday1)
    monday2 = date_to_mon_9am(ref_date)
    # print('monday2: ', monday2)

    days_diff = abs((monday2 - monday1).days)
    # print('days_diff: ', days_diff)
    weeks_diff = int(days_diff/7 + 0.001) # Tolerance to make int weeks
    return weeks_diff


def calculate_cycle_day(planning_date, week_period, ref_date=pd.Timestamp("2021-05-31 09:00")):
    """
    planning_date  - Pandas timestamp that represents the date of interest
    week_period - integer containing the week period (2 will return day 0 to 13, 8 will return 0 to 55)
    ref_date    - Must be a date in what is considered "Week 1"
    """
    # print('planning_date: ', planning_date, ' week_period: ', week_period, ' ref_date: ', ref_date)
    weeks_diff = calculate_week_difference(planning_date, ref_date)
    # print('weeks_diff: ', weeks_diff)
    weeks_off_cycle = weeks_diff % week_period
    # print('weeks_off_cycle: ', weeks_off_cycle)
    day_of_cycle = weeks_off_cycle*7 + planning_date.dayofweek
    # print('day_of_cycle: ', day_of_cycle)
    return day_of_cycle


# Example of carer availability: this actually returns the day (0-55) that the planning_date is on.
# planning_date = pd.Timestamp("2021-08-17 13:25") # Can remove the time, don't need it.
# week_period = 2
# ref_date = pd.Timestamp("2021-07-28")  # This a date on week 1
# cd = calculate_cycle_day(planning_date, week_period, ref_date=ref_date)
# print('The day is:', cd)

# Example of client contracts
# contract_start = pd.Timestamp("2021-05-04")
# contract_cycle = 2
# due_this_week = date_in_visit_cycle(planning_date, contract_start, contract_cycle) # due_this_week is boolean: true or false
# print('Client visit due this week:', due_this_week)