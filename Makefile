CFLAGS += -fPIC
LDFLAGS += -lcurl

all: url.so

%.so: %.o
	gcc -o $@ -shared $^ $(LDFLAGS)

test:
	sqlite3 -batch ':memory:' '.read test.sql'

clean:
	rm -f *.o *.so
