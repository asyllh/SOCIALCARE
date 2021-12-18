#--------------------#
# CARROT - CARe ROuting Tool
# tkinter_userinput.py
# Tkinter pop-up for users to be able to select parameter values
# 21/06/2021
#--------------------#

import os
from sys import exit
from tkinter import *
from tkinter import filedialog
from tkcalendar import Calendar

class UserInputBox:
    def __init__(self, top):
        self.areaName = 'Hampshire'
        self.twInterval = 15
        self.wbBalance = 1
        self.qualityMeasure = 'default'
        self.maxTimeSeconds = 60
        self.createHtmlWebsite = True
        self.createPythonPlots = True
        self.codepointDir = ''
        self.inputFilename = ''
        self.dateSelected = 0
        self.inputBox = Frame(top, width=510, height=370)
        self.inputBox.pack()
        self.t_areaName = StringVar()
        self.t_twInterval = IntVar()
        self.t_wbBalance = DoubleVar()
        self.t_maxTimeSeconds = IntVar()
        self.t_qualityMeasure = StringVar()
        self.t_createHtmlWebsite = BooleanVar()
        self.t_createPythonPlots = BooleanVar()
        self.t_codepointDir = StringVar()
        self.t_inputFilename = StringVar()
        self.t_dateSelected = 0


        title_label = Label(self.inputBox, text='User Input Parameters')
        title_label.configure(font=('Helvetica', 12))
        title_label.place(x=25, y=15)

        area_label = Label(self.inputBox, text='Area name')
        area_label.configure(font=('Helvetica', 10))
        area_label.place(x=25, y=45)
        area_text = Entry(self.inputBox, textvariable=self.t_areaName, bg="white")
        area_text.place(x=180, y=47, width=100, height=17)

        tw_label = Label(self.inputBox, text='Time window interval')
        tw_label.configure(font=('Helvetica', 10))
        tw_label.place(x=25, y=70)
        tw_text = Entry(self.inputBox, textvariable=self.t_twInterval, bg="white")
        tw_text.place(x=180, y=72, width=100, height=17)

        wb_label = Label(self.inputBox, text='Workload balance')
        wb_label.configure(font=('Helvetica', 10))
        wb_label.place(x=25, y=95)
        wb_text = Entry(self.inputBox, textvariable=self.t_wbBalance, bg="white")
        wb_text.place(x=180, y=97, width=100, height=17)

        maxtime_label = Label(self.inputBox, text='Time limit (s)')
        maxtime_label.configure(font=('Helvetica', 10))
        maxtime_label.place(x=25, y=120)
        maxtime_text = Entry(self.inputBox, textvariable=self.t_maxTimeSeconds, bg="white")
        maxtime_text.place(x=180, y=122, width=100, height=17)

        # quality_label = Label(self.inputBox, text='Quality measure')
        # quality_label.configure(font=('Helvetica', 10))
        # quality_label.place(x=25, y=145)
        # quality_text = Entry(self.inputBox, textvariable=self.t_qualityMeasure, bg="white")
        # quality_text.place(x=170, y=147, width=100, height=17)

        quality_label = Label(self.inputBox, text='Quality measure')
        quality_label.configure(font=('Helvetica', 10))
        quality_label.place(x=25, y=145)
        quality_default = Radiobutton(self.inputBox, text='Default', variable=self.t_qualityMeasure, value='default')
        quality_default.place(x=175, y=145)
        quality_aith = Radiobutton(self.inputBox, text='Ait H', variable=self.t_qualityMeasure, value='ait h')
        quality_aith.place(x=240, y=145)
        quality_mk = Radiobutton(self.inputBox, text='Mankowska', variable=self.t_qualityMeasure, value='mk')
        quality_mk.place(x=295, y=145)
        quality_wb = Radiobutton(self.inputBox, text='Workload Balance', variable=self.t_qualityMeasure, value='wb')
        quality_wb.place(x=385, y=145)

        # html_label = Label(self.inputBox, text='Create website')
        # html_label.configure(font=('Helvetica', 10))
        # html_label.place(x=25, y=170)
        # html_text = Entry(self.inputBox, textvariable=self.t_createHtmlWebsite, bg="white")
        # html_text.place(x=170, y=172, width=100, height=17)

        html_label = Label(self.inputBox, text='Create website')
        html_label.configure(font=('Helvetica', 10))
        html_label.place(x=25, y=170)
        html_true = Radiobutton(self.inputBox, text='Yes', variable=self.t_createHtmlWebsite, value=True)
        html_true.place(x=175, y=170)
        html_false = Radiobutton(self.inputBox, text='No', variable=self.t_createHtmlWebsite, value=False)
        html_false.place(x=220, y=170)

        # plots_label = Label(self.inputBox, text='Create plots')
        # plots_label.configure(font=('Helvetica', 10))
        # plots_label.place(x=25, y=195)
        # plots_text = Entry(self.inputBox, textvariable=self.t_createPythonPlots, bg="white")
        # plots_text.place(x=170, y=197, width=100, height=17)

        plots_label = Label(self.inputBox, text='Create plots')
        plots_label.configure(font=('Helvetica', 10))
        plots_label.place(x=25, y=195)
        plots_true = Radiobutton(self.inputBox, text='Yes', variable=self.t_createPythonPlots, value=True)
        plots_true.place(x=175, y=195)
        plots_false = Radiobutton(self.inputBox, text='No', variable=self.t_createPythonPlots, value=False)
        plots_false.place(x=220, y=195)

        codepoint_label1 = Label(self.inputBox, text='Select Code-Point folder')
        codepoint_label1.configure(font=('Helvetica', 10))
        codepoint_label1.place(x=25, y=222)
        codepoint_button = Button(self.inputBox, text='Browse', command=self.SelectCodepointDir)
        codepoint_button.configure(font=('Helvetica', 10))
        codepoint_button.place(x=180, y=222, width=60, height=22)

        datafile_label1 = Label(self.inputBox, text='Select input file')
        datafile_label1.configure(font=('Helvetica', 10))
        datafile_label1.place(x=25, y=249)
        datafile_button = Button(self.inputBox, text='Browse', command=self.SelectInputFile)
        datafile_button.configure(font=('Helvetica', 10))
        datafile_button.place(x=180, y=249, width=60, height=22)
        # datafile_label2 = Label(self.inputBox, text='No file selected')
        # datafile_label2.configure(font=('Helvetica', 8))
        # datafile_label2.place(x=170, y=230)

        date_label = Label(self.inputBox, text='Date')
        date_label.configure(font=('Helvetica', 10))
        date_label.place(x=25, y=276)
        date_button = Button(self.inputBox, text='Calendar', command=self.ShowCal)
        date_button.configure(font=('Helvetica', 10))
        date_button.place(x=180, y=276, width=60, height=22)
        # dayindex_text = Entry(self.inputBox, textvariable=self.t_dayIndex, bg="white")
        # dayindex_text.place(x=180, y=277, width=100, height=17)

        ok_button = Button(self.inputBox, text='OK', command=self.GetVals)
        ok_button.configure(font=('Helvetica', 10))
        # ok_button.place(x=190, y=300, width=50, height=25)
        ok_button.place(x=190, y=320, width=50, height=25)
        exit_button = Button(self.inputBox, text='Exit', command=self.ExitProgram)
        exit_button.configure(font=('Helvetica', 10))
        # exit_button.place(x=260, y=300, width=50, height=25)
        exit_button.place(x=260, y=320, width=50, height=25)

    def SelectCodepointDir(self):
        curr_directory = os.getcwd()
        filetypes = (('Excel files', '*xlsx'),('All files', '*.*'))
        codepoint_dir = filedialog.askdirectory()
        just_foldername = os.path.basename(codepoint_dir)
        self.t_codepointDir = codepoint_dir
        codepoint_label2 = Label(self.inputBox, text=just_foldername)
        codepoint_label2.configure(font=('Helvetica', 8))
        codepoint_label2.place(x=250, y=222)
        # datafile_label2 = Label(self.inputBox, text=just_filename)
        # datafile_label2.configure(font=('Helvetica', 8))
        # datafile_label2.place(x=170, y=245)

    def SelectInputFile(self):
        curr_directory = os.getcwd()
        filetypes = (('Excel files', '*xlsx'),('All files', '*.*'))
        file = filedialog.askopenfilename(title='Open a file', initialdir=curr_directory, filetypes=filetypes)
        just_filename = os.path.basename(file)
        self.t_inputFilename = file
        datafile_label2 = Label(self.inputBox, text=just_filename)
        datafile_label2.configure(font=('Helvetica', 8))
        datafile_label2.place(x=250, y=249)
        # print('self.t_inputFilename: ', self.t_inputFilename, ' type: ', type(self.t_inputFilename))

    def ShowCal(self):
        self.cal_root = Tk()
        self.cal_root.title('Calendar')
        self.cal_root.geometry("300x300")

        self.cal = Calendar(self.cal_root, selectmode='day',year= 2021, month=7, day=26)
        self.cal.pack(pady=20)
        

        #Create a button to pick the date from the calendar
        button = Button(self.cal_root, text= "OK", command=self.PrintCal)
        button.pack(pady=20)

        self.cal_root.mainloop()
            
    # def ShowCal(self):
    #     self.top = Toplevel(self.inputBox)
    #     # cal = Calendar(self.top, font="Arial 14", selectmode='day', locale='en_US', cursor="hand1")
    #     cal = Calendar(self.top, selectmode="day",year= 2021, month=7, day=15)
    #     cal.pack(fill="both", expand=True)
    #     self.t_dateSelected = cal.selection_get()
    #     cal_button = Button(self.top, text='OK', command=self.PrintCal).pack()
    #     # self.inputBox.quit()
    
    def PrintCal(self):
        self.t_dateSelected = self.cal.selection_get()
        print(self.t_dateSelected)
        date_label2 = Label(self.inputBox, text=self.t_dateSelected)
        date_label2.configure(font=('Helvetica', 8))
        date_label2.place(x=250, y=276)
        # self.cal_root.quit()


    def GetVals(self):
        self.areaName = self.t_areaName.get()
        self.twInterval = self.t_twInterval.get()
        self.wbBalance = self.t_wbBalance.get()
        self.qualityMeasure = self.t_qualityMeasure.get()
        self.maxTimeSeconds = self.t_maxTimeSeconds.get()
        self.createHtmlWebsite = self.t_createHtmlWebsite.get()
        self.createPythonPlots = self.t_createPythonPlots.get()
        self.codepointDir = self.t_codepointDir
        self.inputFilename = self.t_inputFilename
        self.dateSelected = self.t_dateSelected
        self.inputBox.quit()

    def ExitProgram(self):
        self.inputBox.quit()
        exit(-1)
### --- End UserInputBox class --- ###


def get_user_input_variables():
    root = Tk()
    root.title('User Input Parameters')

    Uib = UserInputBox(root)
    root.mainloop()

    # Default values for user input variables
    area = 'Hampshire'
    tw_interval = 15
    wb_balance = 1
    quality_measure = 'default' # default = paper
    max_time_seconds = 60
    create_html_website = True
    create_python_plots = True
    input_filename = 'None'
    codepoint_directory = 'None'
    date_selected = 0

    print('\nUser Input Variables\n-----------------')

    # Assign user input values:
    if Uib.areaName == "":
        # set default
        print('No area given - area set to Hampshire')
        area = 'Hampshire'
    else:
        area = Uib.areaName
        print('Area: ', area)

    if Uib.twInterval == 0:
        print('No time window interval given - time window interval set to 15 minutes')
        tw_interval = 15
    else:
        tw_interval = Uib.twInterval
        print('Time window interval: ', tw_interval)

    if Uib.wbBalance == 0:
        print('No workload balance coefficient given - coefficient set to 1')
        wb_balance = 1
    else:
        wb_balance = Uib.wbBalance
        print('Workload balance coefficient: ', wb_balance)

    if Uib.maxTimeSeconds == 0:
        print('No time limit given - time limit set to 60 seconds')
        max_time_seconds = 60
    else:
        max_time_seconds = Uib.maxTimeSeconds
        print('Time limit: ', max_time_seconds)


    if Uib.qualityMeasure == "":
        # set default
        print('No quality measure given - quality measure set to default')
        quality_measure = 'default'
    elif Uib.qualityMeasure == 'default' or Uib.qualityMeasure == 'ait h' or Uib.qualityMeasure == 'mk' or Uib.qualityMeasure == 'wb':
        quality_measure = Uib.qualityMeasure
        print('Quality measure: ', quality_measure)
    else:
        print('[ERROR]: (constructive_wrapper.py) incorrect quality measure provided.')
        print('Quality measure must be one of the following: default, ait h, mk, or wb.')
        exit(-1)
    

    if Uib.createHtmlWebsite == "":
        print('No html website option given - create html website set to True')
        create_html_website = True
    elif Uib.createHtmlWebsite == True or Uib.createHtmlWebsite == False:
        create_html_website = Uib.createHtmlWebsite
        print('Create HTML Website: ', create_html_website)
    else:
        print('Incorrect option format for HTML website - create HTML website set to True')
        create_html_website = True
    
    if Uib.createPythonPlots == "":
        print('No plot option given - create python plots set to True')
        create_python_plots = True
    elif Uib.createPythonPlots == True or Uib.createPythonPlots == False:
        create_python_plots = Uib.createPythonPlots
        print('Create Python plots: ', create_python_plots)
    else:
        print('Incorrect option format for plots - create Python plots set to True')
        create_python_plots = True

    if Uib.codepointDir == "":
        # set default
        print('[ERROR]: No codepoint directory given.')
        print('Exit program.')
        exit(-1)
    else:
        codepoint_directory = Uib.codepointDir
        print('Code-Point Open directory: ', codepoint_directory)

    if Uib.inputFilename == "":
        print('[ERROR]: No input file given.')
        print('Exit program.')
        exit(-1)
    else:
        input_filename = Uib.inputFilename
        print('Input filename: ', input_filename)

    if Uib.dateSelected == None:
        print('No date selected - date set to 26/07/2021')
        date_selected = 0
    else:
        date_selected = Uib.dateSelected
        print('Date selected: ', date_selected)
        print(type(date_selected))

    print('--------------------\n')

    return area, tw_interval, wb_balance, quality_measure, max_time_seconds, create_html_website, create_python_plots, codepoint_directory, input_filename, date_selected
### --- End def get_user_input_variables --- ###
