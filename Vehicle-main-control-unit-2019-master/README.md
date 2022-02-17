# Vehicle-main-control-unit-2019
TAURD drive by wire 
engineering graduate project - controling servo motors with a network of micro-controllers.
the micro controllers connected with CAN-BUS control each of the vehicle systems simultaneously (brakes, wheel, pedals).
a controling unit (computer or steering kit) is opertaing the whole network and connected with bluetooth to the vehicle network. 

This git contains the code for:

1. Two kind of microprocessors - main control unit and noumerous generic control units.

2. A python script for extracting live data from the steering kit (or simple computer) and communicating with the main control unit microprocessor

The following represents the final setup the we used for our project (can be modified with this git codes):

<img width="277" alt="drive-by-wire-graph" src="https://user-images.githubusercontent.com/81683689/154458980-2b6ed4d5-1fe1-4d47-a923-b4f21e35f312.png">
