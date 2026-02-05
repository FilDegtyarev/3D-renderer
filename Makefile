.PHONY: render debug run clean

render:
	conan install . -s build_type=Release --build=missing
	cmake --preset conan-release
	cmake --build --preset conan-release

debug:
	conan install . -s build_type=Debug --build=missing
	cmake --preset conan-debug
	cmake --build --preset conan-debug

run:
	./build/Release/Hello $(ARGS)

run-debug:
	./build/Debug/Hello