CFLAGS = $(shell pkg-config --cflags poppler-glib) -Wall -Wextra -std=c11
LDLIBS = $(shell pkg-config --libs poppler-glib)

all: fill-pdf
