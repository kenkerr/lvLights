IDIR=../include
ODIR=obj

CC=g++
CFLAGS=-I$(IDIR)


DEPS=$(IDIR)/LV_Lights.h $(IDIR)/LvLightController.h $(IDIR)/nwComms.h $(IDIR)/relayControl.h $(IDIR)/modbusTransaction.h $(IDIR)/modbusRegisters.h $(IDIR)/dawnDusk.h
OBJS=$(ODIR)/LV_Lights.o $(ODIR)/nwComms.o $(ODIR)/relayControl.o $(ODIR)/modbusTransaction.o $(ODIR)/modbusRegisters.o $(ODIR)/dawnDusk.o

default:  LV_Lights

# Create the executable "lightController.exe"
#
LV_Lights: $(OBJS)
	$(CC) $(CFLAGS) -o lightController $(OBJS) -lwiringPi

# Create the necessary object files
#
$(ODIR)/LV_Lights.o:  LV_Lights.cpp $(DEPS)
	$(CC) $(CFLAGS) -I/$(IDIR) -c LV_Lights.cpp -o $(ODIR)/LV_Lights.o

$(ODIR)/nwComms.o:  nwComms.cpp $(DEPS)
	$(CC) $(CFLAGS) -I/$(IDIR) -c nwComms.cpp -o $(ODIR)/nwComms.o

$(ODIR)/relayControl.o:  relayControl.cpp $(DEPS)
	$(CC) $(CFLAGS) -I/$(IDIR) -c relayControl.cpp -o $(ODIR)/relayControl.o -lwiringPi

$(ODIR)/modbusTransaction.o:  modbusTransaction.cpp $(DEPS)
	$(CC) $(CFLAGS) -I/$(IDIR) -c modbusTransaction.cpp -o $(ODIR)/modbusTransaction.o

$(ODIR)/modbusRegisters.o:  modbusRegisters.cpp $(DEPS)
	$(CC) $(CFLAGS) -I/$(IDIR) -c modbusRegisters.cpp -o $(ODIR)/modbusRegisters.o

$(ODIR)/dawnDusk.o:  dawnDusk.cpp $(DEPS)
	$(CC) $(CFLAGS) -I/$(IDIR) -c dawnDusk.cpp -o $(ODIR)/dawnDusk.o

clean:
	rm -f $(ODIR)/*.o *~



#######################################
#   more ambitious version 
#
#$(ODIR)/%.o: %.cpp $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

#LV_Lights: $(OBJ) 
#	$(CC) -o $@ $^ $(CFLAGS)
#######################################


