OBJS += \
  build/cn-cbor.o \
  build/cn-create.o \
  build/cn-encoder.o \
  build/cn-error.o \
  build/cn-get.o \
  build/CborArray.o \
  build/CborBuffer.o \
  build/CborObject.o \
  build/CborVariant.o \
  build/test.o 

LIBS += \
  -lgtest \
  -lpthread

build/%.o: src/cn-cbor/%.c
	@mkdir -p build
	gcc $(CCFLAGS) -c -o "$@" "$<" -I src

build/%.o: src/%.cpp
	@mkdir -p build
	g++ $(CCFLAGS) -c -o "$@" "$<"

build/%.o: extra/test/%.cpp
	@mkdir -p build
	g++ $(CCFLAGS) -c -o "$@" "$<" -I src

all: test

test: $(OBJS)
	g++ -o gtest $(OBJS) $(LIBS)
	./gtest

clean:
	@rm -rf build
	@rm -f test
