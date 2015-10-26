

# Makefile for spec2root.C
# razvan.lica@cern.ch



    CC  = g++
CFLAGS  = `root-config --cflags` -w 
LIBS    = `root-config --glibs` -lSpectrum  
INCLUDE = -I./include/
SOURCE  = spec2root.cc
OBJECT  = spec2root.o
   EXE  = spec2root


$(EXE): $(OBJECT)
	$(CC) -o $(EXE)  $(OBJECT) $(LIBS)

$(OBJECT): $(SOURCE)
	$(CC) -c $(CFLAGS) -o $(OBJECT) $(INCLUDE) $(SOURCE) 

clean:
	rm -f $(OBJECT)



 
 
#  More details about compiling and linking c++ executables
#  http://www.hep.shef.ac.uk/teaching/phy6040/ROOT/ROOTseminars/Seminar_4.html
 
# Makefile taken from
# http://mrbook.org/tutorials/make/

