# kiraskyler@163.com
# V0.0.3.20210623

CC :=gcc
CFLAGS :=-Wall -g -O0 -std=gnu99

BINS := template template_vals

template :template.o ksargv.o
	$(CC) $(CFLAGS) -o $@ $^

template_vals :template_vals.o ksargv.o
	$(CC) $(CFLAGS) -o $@ $^

template_vals.o :
template.o :
ksargv.o :

all: template template_vals

clean:
	rm -f $(BINS) *.o