This directory contains end-to-end tests for Orbit. 

We use pywinauto to remote control the Orbit UI. For each test there is a script $TestName.py in this folder.  
You need a python environment to run the automation scripts. Install the dependencies:

````
pip install absl-py
pip install pywinauto
````

The script should run with python 2 and 3 so just use what you have installed. Make sure you are running a 64 bit version of python.
The Orbit UI is 64 bit only and pywinauto requires the bitness of python to match the bitness of the program under test.

Besides that the folder contains the needed test data to set the test up and a folder "flags". For each script $TestName.py there is an empty file flags/$TestName.
If this file is not present the test is considered disabled and will not be executed during the nightly runs - it will silently be considered to have passed.