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
from tkinter import ttk

class UserInputBox:
    def __init__(self, top):
        self.areaName = 'Hampshire'
        self.maxTimeSeconds = 60
        self.twInterval = 15
        self.wbCoeff = -1
        self.maxTravelCoeff = -1
        self.maxWaitingCoeff = -1
        self.qualityMeasure = 'default'
        self.createHtmlWebsite = True
        self.createPythonPlots = True
        self.codepointDir = ''
        self.inputFilename = ''
        self.dateSelected = 0
        self.inputBox = Frame(top, width=510, height=420)
        self.inputBox.pack()
        self.t_areaName = StringVar()
        self.t_maxTimeSeconds = IntVar()
        self.t_twInterval = IntVar()
        self.t_wbCoeff = DoubleVar()
        self.t_maxTravelCoeff = DoubleVar() # NEW 01/12/2021
        self.t_maxWaitingCoeff = DoubleVar() # NEW 01/12/2021
        # self.t_qualityMeasure = StringVar()
        self.t_qualityMeasure = IntVar()
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
        # area_text = Entry(self.inputBox, textvariable=self.t_areaName, bg="white")
        area_text = ttk.Combobox(self.inputBox, textvariable=self.t_areaName)
        area_text['values'] = ['Aldershot', 'Andover', 'Hampshire', 'Monmouth', 'N.Wiltshire', 'Salisbury', 'Swindon']
        area_text.set('Hampshire')
        area_text.place(x=180, y=47, width=100, height=17)
        
        self.t_maxTimeSeconds.set(60)
        maxtime_label = Label(self.inputBox, text='Time limit (s)')
        maxtime_label.configure(font=('Helvetica', 10))
        maxtime_label.place(x=25, y=70)
        maxtime_text = Entry(self.inputBox, textvariable=self.t_maxTimeSeconds, bg="white")
        maxtime_text.place(x=180, y=72, width=100, height=17)

        self.t_twInterval.set(15)
        tw_label = Label(self.inputBox, text='Time window interval')
        tw_label.configure(font=('Helvetica', 10))
        tw_label.place(x=25, y=95)
        tw_text = Entry(self.inputBox, textvariable=self.t_twInterval, bg="white")
        tw_text.place(x=180, y=97, width=100, height=17)

        self.t_wbCoeff.set(-1)
        wb_label = Label(self.inputBox, text='Workload balance')
        wb_label.configure(font=('Helvetica', 10))
        wb_label.place(x=25, y=120)
        wb_text = Entry(self.inputBox, textvariable=self.t_wbCoeff, bg="white")
        wb_text.place(x=180, y=122, width=100, height=17)

        self.t_maxTravelCoeff.set(-1)
        maxtravel_label = Label(self.inputBox, text='Max travel')
        maxtravel_label.configure(font=('Helvetica', 10))
        maxtravel_label.place(x=25, y=145)
        maxtravel_text = Entry(self.inputBox, textvariable=self.t_maxTravelCoeff, bg="white")
        maxtravel_text.place(x=180, y=147, width=100, height=17)

        self.t_maxWaitingCoeff.set(-1)
        maxwaiting_label = Label(self.inputBox, text='Max waiting')
        maxwaiting_label.configure(font=('Helvetica', 10))
        maxwaiting_label.place(x=25, y=170)
        maxwaiting_text = Entry(self.inputBox, textvariable=self.t_maxWaitingCoeff, bg="white")
        maxwaiting_text.place(x=180, y=172, width=100, height=17)


        self.t_qualityMeasure.set(1)
        quality_label = Label(self.inputBox, text='Quality measure')
        quality_label.configure(font=('Helvetica', 10))
        quality_label.place(x=25, y=195)
        # quality_default = Radiobutton(self.inputBox, text='Default', variable=self.t_qualityMeasure, value='default')
        quality_default = Radiobutton(self.inputBox, text='Default', variable=self.t_qualityMeasure, value=1)
        quality_default.place(x=175, y=195)
        # quality_aith = Radiobutton(self.inputBox, text='Ait H', variable=self.t_qualityMeasure, value='ait h')
        quality_aith = Radiobutton(self.inputBox, text='Ait H', variable=self.t_qualityMeasure, value=2)
        quality_aith.place(x=240, y=195)
        # quality_mk = Radiobutton(self.inputBox, text='Mankowska', variable=self.t_qualityMeasure, value='mk')
        quality_mk = Radiobutton(self.inputBox, text='Mankowska', variable=self.t_qualityMeasure, value=3)
        quality_mk.place(x=295, y=195)
        # quality_wb = Radiobutton(self.inputBox, text='Workload Balance', variable=self.t_qualityMeasure, value='wb')
        quality_wb = Radiobutton(self.inputBox, text='Workload Balance', variable=self.t_qualityMeasure, value=4)
        quality_wb.place(x=385, y=195)

        self.t_createHtmlWebsite.set(True)
        html_label = Label(self.inputBox, text='Create website')
        html_label.configure(font=('Helvetica', 10))
        html_label.place(x=25, y=220)
        html_true = Radiobutton(self.inputBox, text='Yes', variable=self.t_createHtmlWebsite, value=True)
        html_true.place(x=175, y=220)
        html_false = Radiobutton(self.inputBox, text='No', variable=self.t_createHtmlWebsite, value=False)
        html_false.place(x=220, y=220)

        self.t_createPythonPlots.set(True)
        plots_label = Label(self.inputBox, text='Create plots')
        plots_label.configure(font=('Helvetica', 10))
        plots_label.place(x=25, y=245)
        plots_true = Radiobutton(self.inputBox, text='Yes', variable=self.t_createPythonPlots, value=True)
        plots_true.place(x=175, y=245)
        plots_false = Radiobutton(self.inputBox, text='No', variable=self.t_createPythonPlots, value=False)
        plots_false.place(x=220, y=245)

        cwd = os.getcwd()
        codepoint_path = os.path.join(cwd, 'codepo_gb')
        self.t_codepointDir = codepoint_path
        # print('t_codepoint: ', self.t_codepointDir)
        codepoint_label1 = Label(self.inputBox, text='Select Code-Point folder')
        codepoint_label1.configure(font=('Helvetica', 10))
        codepoint_label1.place(x=25, y=272)
        codepoint_button = Button(self.inputBox, text='Browse', command=self.SelectCodepointDir)
        codepoint_button.configure(font=('Helvetica', 10))
        codepoint_button.place(x=180, y=272, width=60, height=22)
        self.codepoint_label2 = Label(self.inputBox, text='Default selected')
        self.codepoint_label2.configure(font=('Helvetica', 8))
        self.codepoint_label2.place(x=250, y=272)


        datafile_label1 = Label(self.inputBox, text='Select input file')
        datafile_label1.configure(font=('Helvetica', 10))
        datafile_label1.place(x=25, y=299)
        datafile_button = Button(self.inputBox, text='Browse', command=self.SelectInputFile)
        datafile_button.configure(font=('Helvetica', 10))
        datafile_button.place(x=180, y=299, width=60, height=22)
        self.datafile_label2 = Label(self.inputBox, text='No file selected')
        self.datafile_label2.configure(font=('Helvetica', 8))
        self.datafile_label2.place(x=250, y=299)

        date_label = Label(self.inputBox, text='Date')
        date_label.configure(font=('Helvetica', 10))
        date_label.place(x=25, y=326)
        date_button = Button(self.inputBox, text='Calendar', command=self.ShowCal)
        date_button.configure(font=('Helvetica', 10))
        date_button.place(x=180, y=326, width=60, height=22)
        self.date_label2 = Label(self.inputBox, text='No date selected')
        self.date_label2.configure(font=('Helvetica', 8))
        self.date_label2.place(x=250, y=326)
        # dayindex_text = Entry(self.inputBox, textvariable=self.t_dayIndex, bg="white")
        # dayindex_text.place(x=180, y=277, width=100, height=17)

        ok_button = Button(self.inputBox, text='OK', command=self.GetVals)
        ok_button.configure(font=('Helvetica', 10))
        ok_button.place(x=190, y=370, width=50, height=25)
        exit_button = Button(self.inputBox, text='Exit', command=self.ExitProgram)
        exit_button.configure(font=('Helvetica', 10))
        exit_button.place(x=260, y=370, width=50, height=25)

    def SelectCodepointDir(self):
        curr_directory = os.getcwd()
        filetypes = (('Excel files', '*xlsx'),('All files', '*.*'))
        codepoint_dir = filedialog.askdirectory()
        just_foldername = os.path.basename(codepoint_dir)
        self.t_codepointDir = codepoint_dir
        self.codepoint_label2.configure(text=just_foldername)
        # self.codepoint_label2 = Label(self.inputBox, text=just_foldername)
        # self.codepoint_label2.configure(font=('Helvetica', 8))
        # self.codepoint_label2.place(x=250, y=272)

    def SelectInputFile(self):
        curr_directory = os.getcwd()
        filetypes = (('Excel files', '*xlsx'),('All files', '*.*'))
        file = filedialog.askopenfilename(title='Open a file', initialdir=curr_directory, filetypes=filetypes)
        just_filename = os.path.basename(file)
        self.t_inputFilename = file
        self.datafile_label2.configure(text=just_filename)
        # self.datafile_label2 = Label(self.inputBox, text=just_filename)
        # datafile_label2.configure(font=('Helvetica', 8))
        # datafile_label2.place(x=250, y=299)
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
    
    def PrintCal(self):
        self.t_dateSelected = self.cal.selection_get()
        # print(self.t_dateSelected)
        self.date_label2.configure(text=self.t_dateSelected)
        # date_label2 = Label(self.inputBox, text=self.t_dateSelected)
        # self.date_label2.configure(font=('Helvetica', 8))
        # self.date_label2.place(x=250, y=326)
        self.cal_root.destroy()


    def GetVals(self):
        self.areaName = self.t_areaName.get()
        self.maxTimeSeconds = self.t_maxTimeSeconds.get()
        self.twInterval = self.t_twInterval.get()
        self.wbCoeff = self.t_wbCoeff.get()
        self.maxTravelCoeff = self.t_maxTravelCoeff.get()
        self.maxWaitingCoeff = self.t_maxWaitingCoeff.get()
        self.qualityMeasure = self.t_qualityMeasure.get()
        self.createHtmlWebsite = self.t_createHtmlWebsite.get()
        self.createPythonPlots = self.t_createPythonPlots.get()
        self.codepointDir = self.t_codepointDir
        self.inputFilename = self.t_inputFilename
        self.dateSelected = self.t_dateSelected
        self.inputBox.quit()
        # self.inputBox.quit()

    def ExitProgram(self):
        self.inputBox.quit()
        exit(-1)
### --- End UserInputBox class --- ###


def get_user_input_variables():
    root = Tk()
    root.title('User Input Parameters')

    Uib = UserInputBox(root)
    root.mainloop()
    root.destroy()

    # Default values for user input variables
    area = 'Hampshire'
    max_time_seconds = 60
    tw_interval = 15
    wb_coeff = -1
    maxtravel_coeff = -1
    maxwaiting_coeff = -1
    quality_measure = 'default' # default = paper
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

    if Uib.maxTimeSeconds == 0:
        print('No time limit given - time limit set to 60 seconds')
        max_time_seconds = 60
    else:
        max_time_seconds = Uib.maxTimeSeconds
        print('Time limit: ', max_time_seconds)

    if Uib.twInterval == 0:
        print('No time window interval given - time window interval set to 15 minutes')
        tw_interval = 15
    else:
        tw_interval = Uib.twInterval
        print('Time window interval: ', tw_interval)

    if Uib.wbCoeff == 0:
        print('No workload balance coefficient given - coefficient set to -1')
        wb_coeff = -1
    else:
        wb_coeff = Uib.wbCoeff
        print('Workload balance coefficient: ', wb_coeff)

    if Uib.maxTravelCoeff == 0:
        print('No max travel coefficient given - coefficient set to -1')
        maxtravel_coeff = -1
    else:
        maxtravel_coeff = Uib.maxTravelCoeff
        print('Max travel coefficient: ', maxtravel_coeff)
    
    if Uib.maxWaitingCoeff == 0:
        print('No max waiting coefficient given - coefficient set to -1')
        maxwaiting_coeff = -1
    else:
        maxwaiting_coeff = Uib.maxWaitingCoeff
        print('Max waiting coefficient: ', maxwaiting_coeff)

    # if Uib.qualityMeasure == "":
    #     # set default
    #     print('No quality measure given - quality measure set to default')
    #     quality_measure = 'default'
    # elif Uib.qualityMeasure == 'default' or Uib.qualityMeasure == 'ait h' or Uib.qualityMeasure == 'mk' or Uib.qualityMeasure == 'wb':
    #     quality_measure = Uib.qualityMeasure
    #     print('Quality measure: ', quality_measure)
    # else:
    #     print('[ERROR]: (constructive_wrapper.py) incorrect quality measure provided.')
    #     print('Quality measure must be one of the following: default, ait h, mk, or wb.')
    #     exit(-1)

    if Uib.qualityMeasure == 0:
        # set default
        print('No quality measure given - quality measure set to default')
        quality_measure = 'default'
    elif Uib.qualityMeasure == 1:
        quality_measure = 'default'
        print('Quality measure: ', quality_measure)
    elif Uib.qualityMeasure == 2:
        quality_measure = 'ait_h'
        print('Quality measure: ', quality_measure)
    elif Uib.qualityMeasure == 3:
        quality_measure = 'mk'
        print('Quality measure: ', quality_measure)
    elif Uib.qualityMeasure == 4:
        quality_measure = 'wb'
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
        cwd = os.getcwd()
        codepoint_path = os.path.join(cwd, 'codepo_gb')
        # print('codepoint path1: ', codepoint_path)
        codepoint_directory = codepoint_path
        print('Code-Point Open directory: ', codepoint_directory)
        # print('[ERROR]: No codepoint directory given.')
        # print('Exit program.')
        # exit(-1)
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
        # print(type(date_selected))

    print('--------------------\n')

    return area, max_time_seconds, tw_interval, wb_coeff, maxtravel_coeff, maxwaiting_coeff, quality_measure, create_html_website, create_python_plots, codepoint_directory, input_filename, date_selected
### --- End def get_user_input_variables --- ###
