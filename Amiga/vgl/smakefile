#
# Makefile for VGL for Amiga
#

# User selectable compilation options
COPTIONS =

# Use the SAS/6.51 C/C++ compiler, ANSI
CC = sc
CFLAGS = $(COPTIONS)
LIBRARY_DIR = //linklib
#FIXCRLF = fixcrlf
#FIXFLAGS = -c

LIB_OBJS = defpal.o \
	dumb.o \
	wuline.o \
	dumbpoly.o \
	pixmap.o \
	dumbtext.o \
	dumbbitblt.o \
	color.o \
	fontsmall.o \
	clib.o \
#	queue.o
#	mmi_150.o \
#	mmi_250.o \
#	db_180.o \
#	db_180_draw.o \
#	modes.o \
#	boards.o \
#	vgl_os9.o \
#	vgl_test.o \
#	vgl_vogl.o \
#	vgboard.o \

DRIVER_OBJS = vgldriver.o \
	sys_os9.irq.o \
	mmi_150_isr.o \
	mmi_250_isr.o


LIB_TARGET = libvgl.a
DRIVER     = vgldriver

all: $(LIB_TARGET)
	echo Done making $(LIB_TARGET).

#complete: fixdist $(LIB_TARGET) $(DRIVER) lines perf
#	echo Done making COMPLETE.

clean:
	delete $(LIB_OBJS) $(LIB_TARGET)


$(LIB_TARGET): $(LIB_OBJS)
   failat 40
   # delete it without error messages.
	-delete libvgl.a
#	merge $(LIB_OBJS) >$(LIB_TARGET)
   oml $(LIB_TARGET) a $(LIB_OBJS)


#fixdist: fixcrlf
#	load -d $(FIXCRLF)
#	$(FIXCRLF) $(FIXFLAGS) ../../lib/*.c
#	$(FIXCRLF) $(FIXFLAGS) ../../lib/*.h
#	$(FIXCRLF) $(FIXFLAGS) ../common/*.c
#	$(FIXCRLF) $(FIXFLAGS) ../common/*.h
#	$(FIXCRLF) $(FIXFLAGS) ../common/*.s
#	$(FIXCRLF) $(FIXFLAGS) ../sampleapps/*.c


#fixcrlf: ../common/fixcrlf.c libsys_clib.a
#	$(CC) $(CFLAGS) ../common/$*.c -L . -L ../68030 -l libsys_clib.a -f fixcrlf


#lines: ../sampleapps/lines.c libsys_clib.a $(LIB_TARGET)
#	$(CC) $(CFLAGS) ../sampleapps/$*.c -Wl,-M=128 -L . -L ../68030 -l $(LIB_TARGET) -l libsys_clib.a -f lines


#perf: ../sampleapps/perf.c libsys_clib.a $(LIB_TARGET)
#	$(CC) $(CFLAGS) ../sampleapps/$*.c -Wl,-M=128 -L . -L ../68030 -l $(LIB_TARGET) -l libsys_clib.a -f perf


#kbtest: ../sampleapps/kbtest.c libsys_clib.a $(LIB_TARGET)
#	$(CC) $(CFLAGS) ../sampleapps/$*.c -Wl,-M=128 -L . -L ../68030 -l $(LIB_TARGET) -l libsys_clib.a -f kbtest


#libsys_clib.a: $(LIBRARY_DIR)/sys_clib.l
#        echo >+$@               # Make sure the file exists so we can
#        del -f $@               # delete it without an error.
#        copy $(LIBRARY_DIR)/sys_clib.l $@
#        attr -w $@


# These are extra dependancies in addition to the obvious source files:
#$(LIB_OBJS): //lib/vgl.h //lib/vgl_internals.h //lib/vgl_config.h
#mmi_150.c: mmi_150.h
#mmi_250.c: mmi_250.h
#db_180.c: db_180.h



# OS-9 make has some dangerous default dependancies/rules, so we have to 
# disable all of them with -b above, and then spell the normal ones out
# explicitly here:
#defpal.o: ../../lib/defpal.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#dumb.o: ../../lib/dumb.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#wuline.o: ../../lib/wuline.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#dumbpoly.o: ../../lib/dumbpoly.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#mmi_150.o: ../../lib/mmi_150.c ../../lib/vgl.h ../../lib/vgl_internals.h\
#               ../../lib/mmi_150.h ../../lib/vgl_config.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#mmi_250.o: ../../lib/mmi_250.c ../../lib/vgl.h ../../lib/vgl_internals.h\
#               ../../lib/mmi_250.h ../../lib/vgl_config.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#db_180.o: ../../lib/db_180.c ../../lib/vgl.h ../../lib/vgl_internals.h\
#              ../../lib/db_180.h ../../lib/vgl_config.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#db_180_draw.o: ../../lib/db_180_draw.c   ../../lib/vgl.h\
#                   ../../lib/vgl_internals.h ../../lib/db_180.h\
#                   ../../lib/vgl_config.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#modes.o: ../../lib/modes.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#boards.o: ../../lib/boards.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#pixmap.o: ../../lib/pixmap.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#vgl_os9.o: ../common/vgl_os9.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../common/$*.c -f $*.o
#
#vgl_test.o: ../../lib/vgl_test.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#vgl_vogl.o: ../../lib/vgl_vogl.c ../../lib/vgl.h ../../lib/vgl_internals.h\
#                ../../lib/vgl_config.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#vgboard.o: ../../lib/vgboard.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#dumbtext.o: ../../lib/dumbtext.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#dumbbitblt.o: ../../lib/dumbbitblt.c ../../lib/vgl.h \
#                  ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#color.o: ../../lib/color.c ../../lib/vgl.h ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#fontsmall.o: ../../lib/fontsmall.c ../../lib/vgl.h \
#                 ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#vgldriver.o: ../common/vgldriver.c ../../lib/vgl.h  ../../lib/vgl_internals.h \
#             ../common/sys_os9.h
#        $(CC) $(CFLAGS) -c ../common/$*.c -f $*.o
#
#sys_os9.irq.o: ../common/sys_os9.irq.s ../common/sys_os9.h
#        $(CC) $(CFLAGS) -c ../common/$*.s -f $*.o
#
#mmi_150_isr.o: ../common/mmi_150_isr.c ../../lib/vgl.h \
#               ../../lib/vgl_internals.h ../../lib/mmi_150.h
#        $(CC) $(CFLAGS) -c ../common/$*.c -f $*.o
#
#mmi_250_isr.o: ../common/mmi_250_isr.c ../../lib/vgl.h \
#               ../../lib/vgl_internals.h ../../lib/mmi_250.h
#        $(CC) $(CFLAGS) -c ../common/$*.c -f $*.o
#
#clib.o: ../../lib/clib.c ../../lib/vgl.h \
#        ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
#
#queue.o: ../../lib/queue.c ../../lib/queue.h ../../lib/vgl.h \
#        ../../lib/vgl_internals.h
#	$(CC) $(CFLAGS) -c ../../lib/$*.c -f $*.o
