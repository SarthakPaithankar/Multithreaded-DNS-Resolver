MSRCS = multi-lookup.c array.c
MHDRS = multi-lookup.h array.h

CC = gcc
CFLAGS = -Wextra -Wall -g -std=gnu99
INCLUDES = 
LFLAGS = 
LIBS = -lpthread

MAIN = multi-lookup

SRCS = $(MSRCS) util.c
HDRS = $(MHDRS) util.h

OBJS = $(SRCS:.c=.o) 

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean: 
	$(RM) *.o *~ $(MAIN)

SUBMITFILES = $(MSRCS) $(MHDRS) Makefile README
submit: 
	@read -r -p "Enter your identikey username: " username; \
	echo; echo Bundling the following files for submission; \
	tar --transform "s|^|PA6-$$username/|" -cvf PA6-$$username.txt $(SUBMITFILES); \
	echo; echo Please upload the file PA6-$$username.txt to Canvas to complete your submission; echo
