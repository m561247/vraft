deps: gtest leveldb libuv

gtest:
	rm -rf third_party/googletest.v1.14.0/build
	mkdir -p third_party/googletest.v1.14.0/build
	cd third_party/googletest.v1.14.0/build
	cmake .. && make -j4
	cd -

leveldb:

libuv:

