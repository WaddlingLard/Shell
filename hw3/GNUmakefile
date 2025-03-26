prog=shell

ldflags:=-lreadline -lncurses

include ../GNUmakefile

shell: $(objs) libdeq.so
	gcc -o $@ $(objs) $(ldflags) -L. -ldeq -Wl,-rpath=.

try: $(objs) libdeq.so
	gcc -o $@ $(objs) $(ldflags) -L. -ldeq -Wl,-rpath=.

trytest: try
	Test/run

test: $(prog)
	Test/run
