Orbit Profiler - Pierric Gimmig

Thank you for using Orbit.

A quick introduction video can be found here:
http://telescopp.com/howto

A more in-depth video can be found here:
www.telescopp.com


Workflow
1. Select a process in the list of currently running processes in the "Home" tab
2. The list of loaded modules will appear on the bottom of the "Home" tab.  If a .pdb file was found for a module, it will appear in blue
3. Right click on the module(s) for wich you want to load debug information and select "Load Pdb".  Wait a little bit for Orbit to parse the pdb.  Once its done, The "Functions", "Types" and "Globals" tabs will get populated.
4. Select functions you wish to profile in the "Functions" tab by right clicking and choosing "Select"
5. In the "Capture" tab, start profiling by pressing 'X'.  To stop profiling, press 'X' again.  You can zoom time using the scroll wheel.  To zoom vertically, hold 'CTRL' while scrolling.  You can also right-click and drag to zoom time.  Press A to Zoom All.
6. When you select a function in the "Capture" view, the full callstack will be available in the "Callstack" tab.  You can select functions to be profiled in the callstack tab as well.  Also, if code is available on your machine, it will be displayed in the "Code" tab.

Sessions
Once you have loaded the debug information for your modules and have chosen functions of interest, you can save your profiling session so that you won't have to do this manually again.  To save a session, go to "File"->"Save Session"

Questions and comments are more than welcome, please send them directly to pierric.gimmig@gmail.com.