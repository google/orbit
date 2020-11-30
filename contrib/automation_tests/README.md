This directory contains end-to-end tests for Orbit.

# Installing the prerequesits

We use pywinauto to remote control the Orbit UI. For each test there is a script $TestName.py in this folder.  
You need a python environment to run the automation scripts. 

Note that the use of virtual environments is recommended: https://packaging.python.org/guides/installing-using-pip-and-virtual-environments/

To setup a virtual environment in Python3, run the following inside this folder:

```
python -m venv env
```

where `env` will be the name of your virtual environment and can be chosen freely. 
Next, activate the virtual environment by running `.\env\Scripts\activate` on Windows, or `source ./env/bin/activate` on Linux.
Once you've activated the virtual environment, install the dependencies:

```
pip install absl-py
pip install pywinauto
```

The script should run with python 2 and 3 so just use what you have installed. Make sure you are running a 64 bit version of python.
The Orbit UI is 64 bit only and pywinauto requires the bitness of python to match the bitness of the program under test.

# Running the tests locally

Before running an E2E test, the following conditions need to be met: 
* There needs to be a gamelet reserved 
* "hello_ggp_standalone" has to be started on the gamelet
* Orbit must be started on the local machine - either via F5 in Visual Studio or with a double click on Orbit.exe

To run a test, e.g. orbit_instrument_function, execute

`python orbit_instrument_function.py`

Besides the test scripts, the folder contains the needed test data to set the test up and a folder "flags". For each script $TestName.py there is an empty file flags/$TestName.
If this file is not present the test is considered disabled and will not be executed during the nightly runs - it will silently be considered to have passed.