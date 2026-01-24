.PHONY: render

render:
	cd build && \
	cmake .. \
	-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
	-DCMAKE_BUILD_TYPE=Release && \
	cmake --build . && \
	cd ..

run:
	source build/conanrun.sh && ./build/Hello