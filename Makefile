GLADE_SRCS = $(wildcard res/*.glade)
GLADE_OBJS = ${GLADE_SRCS:.glade=.o}
SRCS = $(wildcard src/*.cpp)
OBJS = ${SRCS:.cpp=.o}
gcalc_1.0-1_amd64.deb: gcalc_1.0-1_amd64/DEBIAN/control gcalc_1.0-1_amd64/usr/bin/gcalc
	dpkg-deb --build --root-owner-group gcalc_1.0-1_amd64
	rm -r gcalc_1.0-1_amd64
gcalc_1.0-1_amd64/DEBIAN/control:
	mkdir -p gcalc_1.0-1_amd64/DEBIAN
	cp control gcalc_1.0-1_amd64/DEBIAN/control
gcalc_1.0-1_amd64/usr/bin/gcalc: gcalc
	mkdir -p gcalc_1.0-1_amd64/usr/bin
	cp gcalc gcalc_1.0-1_amd64/usr/bin/gcalc
deploy: gcalc_1.0-1_amd64.deb

all: gcalc
	
gcalc: ${GLADE_OBJS} ${OBJS}
	g++ ${OBJS} ${GLADE_OBJS} -o gcalc `pkg-config gtk+-3.0 --libs` -lboost_regex -g
	rm -f ${OBJS} ${GLADE_OBJS}
clean:
	rm -f ${OBJS} ${GLADE_OBJS} gcalc
%.o: %.glade
	objcopy -I binary -O elf64-x86-64 $< $@ -g
%.o: %.cpp
	g++ -c $< -o $@ -Iinclude `pkg-config gtk+-3.0 --cflags` -g