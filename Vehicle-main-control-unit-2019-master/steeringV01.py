#sagicohen8@gmaol.com
#rajwanraz@gmail.com
#reads logi - tech kit values and send them through serial PORT

# make sure matplotlib and pygame are installed
import pygame
import time
import serial
from numpy import *
from pyqtgraph import QtGui, QtCore
import pyqtgraph as pg

def init_pygame():
    # initialize pygame
    pygame.init()
    js = pygame.joystick.Joystick(0)  # initialzie joystick
    js.init()
    jsInit = js.get_init()
    jsId = js.get_id()
    print(js.get_name() + "Joystick ID: %d Init status: %s" % (jsId, jsInit))
    numAxes = js.get_numaxes()

    print("Please press A to start, press the xBox button to quit")
    # waiting for initial keys
    while (True):
        pygame.event.pump()
        keys = [js.get_button(i) for i in range(js.get_numbuttons())]
        if keys[0] != 0:
            break
        if keys[10] != 0:
            sys.exit();

    ### START QtApp ####
    app = QtGui.QApplication([])  # you MUST do this once (initialize things)
    ####################

    win = pg.GraphicsWindow(title="Realtime Signals from Driving Kit")  # creates a window
    p1 = win.addPlot(title="Steering Wheel Signal [radians]")  # creates empty space for the plot in the window
    curve1 = p1.plot(pen=pg.mkPen('r', width=1))                                         # create an empty "plot" (a curve to plot)
    p2 = win.addPlot(title="Throttle Signal [Fraction 0-1]")
    curve2 = p2.plot(pen=pg.mkPen('b', width=1))
    p3 = win.addPlot(title="Break Signal [Fraction 0-1]")
    curve3 = p3.plot(pen=pg.mkPen('g', width=1))

    windowWidth = 500                  # width of the window displaying the curve
    Xm1 = linspace(0, 0, windowWidth)  # create array that will contain the relevant time series
    ptr1 = -windowWidth                # set first x position
    Xm2 = linspace(0, 0, windowWidth)
    ptr2 = -windowWidth
    Xm3 = linspace(0, 0, windowWidth)
    ptr3 = -windowWidth

    # Serial Defining #
    ser = serial.Serial("COM7", 57600)

    t1 = time.time()
    count =0;
    t1 = time.time()
    while (True):

        Xm1[:-1] = Xm1[1:]  # shift data in the temporal mean 1 sample left
        Xm2[:-1] = Xm2[1:]
        Xm3[:-1] = Xm3[1:]


        pygame.event.pump()
        jsInputs = [float(js.get_axis(i)) for i in range(numAxes)]
        steerVal = float(round(round(jsInputs[0], 5) * 7.85398163,5))
        throtVal = float(round(1 - (((jsInputs[2] - (-1)) * (1.0 - 0)) / (1.0 - (-1.0))) + 0, 4))
        breakVal = float(round(1 - (((jsInputs[3] - (-1)) * (1.0 - 0)) / (1.0 - (-1.0))) + 0, 4))
        str_steerVal = 'S' + str(steerVal)
        str_throtVal = 'T'+ str(throtVal)
        str_breakVal = 'B' + str(breakVal)
        packet_steer = 'json:' + str_steerVal + "\r\n"
        packet_throt='json:' + str_throtVal + "\r\n"
        packet_break='json:' + str_breakVal + "\r\n"
        if ser.isOpen():

            ser.write(str.encode(packet_steer))
            #time.sleep(0.001)
            ser.write(str.encode(packet_throt))
            #time.sleep(0.001)
            ser.write(str.encode(packet_break))
            ##time.sleep(0.001)
            #ser.write(str.encode(packet))

            count=count+1;

        else:
            print ("Serial Communication Failed")
            sys.exit();

        #t2 = time.time()
        #dt = t2 - t1

        Xm1[-1] = float(steerVal) # vector containing the instantaneous values
        ptr1 += 1                 # update x position for displaying the curve
        curve1.setData(Xm1)       # set the curve with this data
        curve1.setPos(ptr1, 0)    # set x position in the graph to 0

        Xm2[-1] = float(throtVal)
        ptr2 += 1
        curve2.setData(Xm2)
        curve2.setPos(ptr2, 0)

        Xm3[-1] = float(breakVal)
        ptr3 += 1
        curve3.setData(Xm3)
        curve3.setPos(ptr3, 0)

        QtGui.QApplication.processEvents()  # you MUST process the plot now



        keys = [js.get_button(i) for i in range(js.get_numbuttons())]
        print("Steering : " + str(steerVal) + "[rad] Throttle: " + str(100*throtVal) + \
              "% break: " + str( 100*breakVal) + "%")
        if keys[10] != 0:
            t2 = time.time()
            dt=t2-t1
            print (count/dt)
            sys.exit();
            pg.QtGui.QApplication.exec_()




init_pygame()
