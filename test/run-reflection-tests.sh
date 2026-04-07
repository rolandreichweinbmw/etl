#!/bin/bash

set -e

if [ "$1" = "test" ] ; then
	COMPILER=$2
    STL_OPTION=$3
	export CMAKE_BUILD_PARALLEL_LEVEL=$(nproc)
	rm -rf test/build-make
	mkdir -p test/build-make
	cd test/build-make
	cmake -DCMAKE_CXX_COMPILER=${COMPILER} -DNO_STL=${STL_OPTION} -DETL_USE_TYPE_TRAITS_BUILTINS=ON \
		  -DETL_USER_DEFINED_TYPE_TRAITS=OFF -DETL_FORCE_TEST_CPP03_IMPLEMENTATION=OFF -DETL_OPTIMISATION=-O0 -DETL_CXX_STANDARD=26 \
		  -DETL_ENABLE_SANITIZER=Off -DETL_MESSAGES_ARE_NOT_VIRTUAL=OFF -DETL_USE_BUILTIN_MEM_FUNCTIONS=ON ..
	cmake --build .
	ctest -V
	exit 0
fi

while read i ; do
	CONTAINER=`echo $i | cut -d, -f1 | sed -e 's/^ *//' -e 's/ *$//'`
	CONTAINER_USER=`echo $i | cut -d, -f2 | sed -e 's/^ *//' -e 's/ *$//'`
	COMPILER=`echo $i | cut -d, -f3 | sed -e 's/^ *//' -e 's/ *$//'`
	STL_OPTION=`echo $i | cut -d, -f4 | sed -e 's/^ *//' -e 's/ *$//'`

    if [ "$CONTAINER" = "" ] || [ "${CONTAINER:0:1}" = "#" ] ; then
		continue
	fi

	echo "Testing configuration: $i"

	docker build -t $CONTAINER .devcontainer/$CONTAINER
	docker run --rm -v .:/etl -u $CONTAINER_USER -w /etl $CONTAINER /bin/bash -c "$0 test $COMPILER $STL_OPTION"
done <<-EOF
# Container, User  , Compiler, NO_STL
clang-p2996, ubuntu, clang++ , ON
clang-p2996, ubuntu, clang++ , OFF
gcc16      , debian, g++     , ON
gcc16      , debian, g++     , OFF
#clang22    , debian, clang++ , ON
#clang22    , debian, clang++ , OFF
EOF