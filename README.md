![Alt text](logos/orbit_logo.png)

www.orbitprofiler.com

**Pierric Gimmig**
[ <img src="https://github.com/pierricgimmig/orbitprofiler/blob/master/logos/twitter.png">](https://twitter.com/pierricgimmig)[ <img src="https://github.com/pierricgimmig/orbitprofiler/blob/master/logos/linkedin.png">](https://www.linkedin.com/in/pgimmig/)[ <img src="https://github.com/pierricgimmig/orbitprofiler/blob/master/logos/mail.png">](mailto:pierric.gimmig@gmail.com)

Thank you for using Orbit.

A quick introduction video can be found here:
http://telescopp.com/howto

A more in-depth video can be found here:
www.telescopp.com

**Workflow**
1. Select a process in the list of currently running processes in the "Home" tab
2. The list of loaded modules will appear on the bottom of the "Home" tab.  If a .pdb file was found for a module, it will appear in blue
3. Right click on the module(s) for wich you want to load debug information and select "Load Pdb".  Wait a little bit for Orbit to parse the pdb.  Once its done, The "Functions", "Types" and "Globals" tabs will get populated.
4. Select functions you wish to profile in the "Functions" tab by right clicking and choosing "Select"
5. In the "Capture" tab, start profiling by pressing 'X'.  To stop profiling, press 'X' again.  You can zoom time using the scroll wheel.  To zoom vertically, hold 'CTRL' while scrolling.  You can also right-click and drag to zoom time.  Press A to Zoom All.
6. When you select a function in the "Capture" view, the full callstack will be available in the "Callstack" tab.  You can select functions to be profiled in the callstack tab as well.  Also, if code is available on your machine, it will be displayed in the "Code" tab.

Sessions
Once you have loaded the debug information for your modules and have chosen functions of interest, you can save your profiling session so that you won't have to do this manually again.  To save a session, go to "File"->"Save Session"

Questions and comments are more than welcome, please send them directly to pierric.gimmig@gmail.com.

License (BSD 2-clause)
======

<a href="http://opensource.org/licenses/BSD-2-Clause" target="_blank">
<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">
</a>

	Copyright (c) 2017 Pierric Gimmig. All rights reserved.
	
	https://github.com/pierricgimmig/orbitprofiler
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	
	   1. Redistributions of source code must retain the above copyright notice,
	      this list of conditions and the following disclaimer.
	
	   2. Redistributions in binary form must reproduce the above copyright
	      notice, this list of conditions and the following disclaimer in the
	      documentation and/or other materials provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
	EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
