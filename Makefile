########################
# Independent settings #
########################

DEST	      = /usr/local/bin

CFLAGS	      = -DPROFILE -O3 -march=native -pedantic -std=gnu11 -Wall -pg
CFLAGS	      = -DDEBUG -g -fomit-frame-pointer -march=native -pedantic -std=gnu11 -Wall
CFLAGS	      = -O3 -fomit-frame-pointer -march=native -pedantic -std=gnu11 -Wall

LDFLAGS	      = -pg
LDFLAGS	      = -g
LDFLAGS	      =

######################
# Dependent settings #
######################

DIR	      = $(shell pwd)

HDRS	      = $(shell ls *.h)

SRCS	      = $(shell ls *.c)

OBJS	      = $(filter-out main_%.o, $(SRCS:%.c=%.o))

MAIN	      = $(filter main_%.o, $(SRCS:%.c=%.o))

LIST	      = $(MAIN:main_%.o=%)

LD	      = $(CC)

LIBRARY	      = lib$(notdir $(DIR)).a

LIBS	      = $(LIBRARY)

LIBT	      = -lptk

##########
# Rules. #
##########

all:		$(LIST)
		echo MAIN = $(LIST)

clean:;		rm -f $(MAIN)
		rm -f $(OBJS)

clobber:	clean
		rm -f $(LIST)

%:		main_%.o $(LIBS)
		$(LD) $(LDFLAGS) $< $(OBJS) $(LIBS) $(LIBT) -o temp.$@
		if [ -x /sbin/paxctl ]; then /sbin/paxctl -c  temp.$@; fi
		if [ -x /sbin/paxctl ]; then /sbin/paxctl -v  temp.$@; fi
		if [ -x /sbin/paxctl ]; then /sbin/paxctl -mr temp.$@; fi
		if [ -x /sbin/paxctl ]; then /sbin/paxctl -v  temp.$@; fi
		if [ -x /usr/sbin/paxctl-ng ]; then /usr/sbin/paxctl-ng -v temp.$@; fi
		if [ -x /usr/sbin/paxctl-ng ]; then /usr/sbin/paxctl-ng -F temp.$@; fi
		if [ -x /usr/sbin/paxctl-ng ]; then /usr/sbin/paxctl-ng -v temp.$@; fi
		mv temp.$@ $@

$(LIBRARY):	$(SUBS)
		rm -f $@ temp.$@
		$(AR) crv temp.$@ $(SUBS)
		mv temp.$@ $@

install:	$(MAIN:main_%.o=$(DEST)/%)

update:		$(MAIN:main_%.o=$(DEST)/%)

$(SRCS:%.c=%.o): $(HDRS)

$(DEST)/%:	%
		sudo rm -f $@
		sudo install -v -s -m 0755 $(<F) $(@D)
