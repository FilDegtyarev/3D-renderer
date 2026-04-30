BINARY_NAME = Hello
BUILD_DIR_RELEASE = build/Release
BUILD_DIR_DEBUG = build/Debug

GENERATOR = Ninja

.PHONY: all render debug run run-debug clean help

all: render

$(BUILD_DIR_RELEASE)/build.ninja: CMakeLists.txt
	@mkdir -p $(BUILD_DIR_RELEASE)
	cmake -G $(GENERATOR) -S . -B $(BUILD_DIR_RELEASE) -DCMAKE_BUILD_TYPE=Release


render: $(BUILD_DIR_RELEASE)/build.ninja
	cmake --build $(BUILD_DIR_RELEASE) --parallel


$(BUILD_DIR_DEBUG)/build.ninja: CMakeLists.txt
	@mkdir -p $(BUILD_DIR_DEBUG)
	cmake -G $(GENERATOR) -S . -B $(BUILD_DIR_DEBUG) -DCMAKE_BUILD_TYPE=Debug

debug: $(BUILD_DIR_DEBUG)/build.ninja
	cmake --build $(BUILD_DIR_DEBUG) --parallel


run: render
	./$(BUILD_DIR_RELEASE)/$(BINARY_NAME) $(ARGS)

run-debug: debug
	./$(BUILD_DIR_DEBUG)/$(BINARY_NAME)

clean:
	rm -rf build
