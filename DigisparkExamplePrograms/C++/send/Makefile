
ifeq ($(shell uname), Linux)
	USBFLAGS = `libusb-config --cflags`
	USBLIBS = `libusb-config --libs`
	EXE_SUFFIX =
	OSFLAG = -D LINUX
else ifeq ($(shell uname), Darwin)
	USBFLAGS = `libusb-config --cflags`
	USBLIBS = `libusb-config --libs`
	EXE_SUFFIX =
	OSFLAG = -D MAC_OS -D_DARWIN_USE_64_BIT_INODE=1
else
	USBFLAGS = -I C:\MinGW\include
	USBLIBS = -L C:\MinGW\lib -lusb
	EXE_SUFFIX = .exe
	OSFLAG = -D WIN -static-libstdc++ -static-libgcc
endif

LIBS    = $(USBLIBS)
CFLAGS  = $(USBFLAGS) $(LIBS) -O -g $(OSFLAG)

CC=	g++
PROGRAM= send$(EXE_SUFFIX)
SOURCE= send.cpp

all: $(PROGRAM)

$(PROGRAM): $(SOURCE)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(SOURCE) $(USBLIBS)

clean:
	rm -f $(PROGRAM) *.o
