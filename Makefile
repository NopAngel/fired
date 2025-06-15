GPP=g++

build:
	$(GPP) ./src/main.cc -o ./bin/bld.exe
	./bin/bld.exe

clean:
	rm ./bin/bld.exe