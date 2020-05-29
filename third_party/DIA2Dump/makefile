!if "$(DEBUG)" == ""
DEBUG = 1
!endif

!if "$(DEBUG)" == "1"
ODIR = debug
CFLAGS = $(CFLAGS) -Od -D_DEBUG -ZI 
D = d
!else
ODIR = release
CFLAGS = $(CFLAGS) -O1 -DNDEBUG -Zi
!endif


LIBS = \
    $(LIBS) \
    ole32.lib \
    oleaut32.lib


CFLAGS = $(CFLAGS) -MD$(D) -I"$(VSINSTALLDIR)\DIA SDK\include"

PCHNAME  = $(ODIR)\stdafx.pch
PCHHEADER = stdafx.h
PCHFLAGS = -Yu$(PCHHEADER) -Fp$(PCHNAME)
CFLAGS   = $(CFLAGS)   -nologo -c -Fd$(ODIR)\ -W3
LFLAGS   = $(LFLAGS)   -map -debug -PDB:$(ODIR)\dia2dump.pdb "-libpath:$(VSINSTALLDIR)\DIA SDK\lib"


!if "$(VERBOSE)" == "1"
!message DEBUG=$(DEBUG)
!endif

OBJS = \
    $(ODIR)\dia2dump.obj    \
    $(ODIR)\regs.obj        \
    $(ODIR)\printsymbol.obj \
    $(ODIR)\stdafx.obj      


##### Inference Rules

all : $(ODIR)\dia2dump.exe

$(PCHNAME) $(ODIR)\stdafx.obj : $(PCHHEADER) stdafx.cpp dia2dump.h
    cl $(CFLAGS) $(PCHFLAGS:Yu=Yc) -Fo$(ODIR)\ -FR$(ODIR)\ stdafx.cpp

{}.cpp{$(ODIR)\}.obj::
    cl $(CFLAGS) $(MPBUILDFLAGS) $(PCHFLAGS) -Fo$(ODIR)\ -FR$(ODIR)\ $<

$(ODIR)\dia2dump.exe : $(ODIR) $(PCHNAME) $(OBJS)
    link -out:$(ODIR)\dia2dump.exe $(OBJS) $(LFLAGS) $(LIBS)

$(ODIR):
    -md $(ODIR)

clean :
    del /q $(ODIR)
