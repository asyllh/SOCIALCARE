from tkinter import *

class UserInputBox:
    def __init__(self, top):
        self.areaName = 'Hampshire'
        self.twInterval = 15
        self.wbBalance = 1
        self.qualityMeasure = 'paper'
        self.maxTimeSeconds = 60
        self.createHtmlWebsite = 'y'
        self.createPythonPlots = 'y'
        self.inputBox = Frame(top, width=300, height=300)
        self.inputBox.pack()
        self.t_areaName = StringVar()
        self.t_twInterval = IntVar()
        self.t_wbBalance = DoubleVar()
        self.t_qualityMeasure = StringVar()
        self.t_maxTimeSeconds = IntVar()
        self.t_createHtmlWebsite = StringVar()
        self.t_createPythonPlots = StringVar()

        title_label = Label(self.inputBox, text='User Input Variables')
        title_label.configure(font=('Helvetica', 12))
        title_label.place(x=75, y=15)

        area_label = Label(self.inputBox, text='Area name')
        area_label.configure(font=('Helvetica', 10))
        area_label.place(x=25, y=45)
        area_text = Entry(self.inputBox, textvariable=self.t_areaName, bg="white")
        area_text.place(x=170, y=47, width=100, height=17)

        tw_label = Label(self.inputBox, text='Time window interval')
        tw_label.configure(font=('Helvetica', 10))
        tw_label.place(x=25, y=70)
        tw_text = Entry(self.inputBox, textvariable=self.t_twInterval, bg="white")
        tw_text.place(x=170, y=72, width=100, height=17)

        wb_label = Label(self.inputBox, text='Workload balance')
        wb_label.configure(font=('Helvetica', 10))
        wb_label.place(x=25, y=95)
        wb_text = Entry(self.inputBox, textvariable=self.t_wbBalance, bg="white")
        wb_text.place(x=170, y=97, width=100, height=17)

        quality_label = Label(self.inputBox, text='Quality measure')
        quality_label.configure(font=('Helvetica', 10))
        quality_label.place(x=25, y=120)
        quality_text = Entry(self.inputBox, textvariable=self.t_qualityMeasure, bg="white")
        quality_text.place(x=170, y=122, width=100, height=17)

        maxtime_label = Label(self.inputBox, text='Time limit (s)')
        maxtime_label.configure(font=('Helvetica', 10))
        maxtime_label.place(x=25, y=145)
        maxtime_text = Entry(self.inputBox, textvariable=self.t_maxTimeSeconds, bg="white")
        maxtime_text.place(x=170, y=147, width=100, height=17)

        html_label = Label(self.inputBox, text='Create Website')
        html_label.configure(font=('Helvetica', 10))
        html_label.place(x=25, y=170)
        html_text = Entry(self.inputBox, textvariable=self.t_createHtmlWebsite, bg="white")
        html_text.place(x=170, y=172, width=100, height=17)

        plots_label = Label(self.inputBox, text='Create Plots')
        plots_label.configure(font=('Helvetica', 10))
        plots_label.place(x=25, y=195)
        plots_text = Entry(self.inputBox, textvariable=self.t_createPythonPlots, bg="white")
        plots_text.place(x=170, y=197, width=100, height=17)

        ok_button = Button(self.inputBox, text='OK', command=self.GetVals)
        ok_button.configure(font=('Helvetica', 10))
        ok_button.place(x=85, y=235, width=50, height=25)
        exit_button = Button(self.inputBox, text='Exit', command=self.ExitProgram)
        exit_button.configure(font=('Helvetica', 10))
        exit_button.place(x=155, y=235, width=50, height=25)


    def GetVals(self):
        self.areaName = self.t_areaName.get()
        self.twInterval = self.t_twInterval.get()
        self.wbBalance = self.t_wbBalance.get()
        self.qualityMeasure = self.t_qualityMeasure.get()
        self.maxTimeSeconds = self.t_maxTimeSeconds.get()
        self.createHtmlWebsite = self.t_createHtmlWebsite.get()
        self.createPythonPlots = self.t_createPythonPlots.get()
        # print('areaName: ', self.areaName)
        # print('twInterval: ', self.twInterval)
        # print('wbBalance: ', self.wbBalance)
        # print('qualityMeasure: ', self.qualityMeasure)
        # print('maxTimeSeconds: ', self.maxTimeSeconds)
        self.inputBox.quit()

    def ExitProgram(self):
        self.inputBox.quit()
        exit(-1)
### --- End UserInputBox class --- ###


# def create_tkinter_box():
#     root = Tk()
#     root.title('User Input Variables')

#     Uib = UserInputBox(root)

#     root.mainloop()

#     print('\n')
#     print('areaName retrieved: ', Uib.areaName)
#     print('twInterval retrieved: ', Uib.twInterval)
#     print('wbBalance retrieved: ', Uib.wbBalance)
#     print('qualityMeasure retrieved: ', Uib.qualityMeasure)
#     print('maxTimeSeconds retrieved: ', Uib.maxTimeSeconds)

#     return Uib.areaName