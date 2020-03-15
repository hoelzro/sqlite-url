CFLAGS += -fPIC
LDFLAGS += -lcurl

all: url.so

%.so: %.o
	gcc -o $@ -shared $^ $(LDFLAGS)

test: all
	sqlite3 -batch ':memory:' '.read test.sql'

clean:
	rm -f *.o *.so
