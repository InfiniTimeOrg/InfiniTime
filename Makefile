.PHONY: check-runtime ensure-firmware-image build build-infinisim run-infinisim clean help

# Default target
.DEFAULT_GOAL := build

# Build directories
BUILD_DIR ?= build
INFINISIM_DIR ?= build_lv_sim/InfiniSim
INFINISIM_BUILD_DIR ?= $(INFINISIM_DIR)/build

# Container configuration
CONTAINER_RUNTIME ?= $(shell if command -v podman >/dev/null 2>&1; then echo podman; elif command -v docker >/dev/null 2>&1; then echo docker; fi)
CONTAINER_VOLUME_SUFFIX := $(shell if [ "$(CONTAINER_RUNTIME)" = "podman" ]; then echo ":Z"; fi)
FIRMWARE_IMAGE_NAME ?= infinitime-build
INFINISIM_IMAGE_NAME ?= infinisim-build
REBUILD_IMAGE ?= 0

check-runtime:
	@if [ -z "$(CONTAINER_RUNTIME)" ]; then \
		echo "Error: neither podman nor docker is installed." >&2; \
		echo "Install one container runtime and try again." >&2; \
		echo "Examples:" >&2; \
		echo "  Fedora: sudo dnf install podman" >&2; \
		echo "  Debian/Ubuntu: sudo apt install podman" >&2; \
		echo "  Docker alternative: https://docs.docker.com/engine/install/" >&2; \
		echo "Then verify with: podman --version or docker --version" >&2; \
		exit 1; \
	fi

ensure-firmware-image: check-runtime
	@if [ "$(REBUILD_IMAGE)" = "1" ] || ! $(CONTAINER_RUNTIME) image inspect "$(FIRMWARE_IMAGE_NAME)" >/dev/null 2>&1; then \
		echo "[1/2] Building firmware container image: $(FIRMWARE_IMAGE_NAME)"; \
		$(CONTAINER_RUNTIME) build -t "$(FIRMWARE_IMAGE_NAME)" -f docker/Dockerfile docker; \
	else \
		echo "[1/2] Firmware container image already exists: $(FIRMWARE_IMAGE_NAME)"; \
	fi

help:
	@echo "InfiniTime Build System"
	@echo "======================="
	@echo ""
	@echo "Targets:"
	@echo "  make build              Build the InfiniTime firmware in container (default)"
	@echo "  make build-infinisim    Build InfiniSim simulator in container"
	@echo "  make run-infinisim      Run the InfiniSim simulator"
	@echo "  make clean              Clean build directories"
	@echo ""
	@echo "Environment Variables:"
	@echo "  CONTAINER_RUNTIME      Container runtime (auto: podman, fallback: docker)"
	@echo "  BUILD_DIR              Firmware build directory (default: build)"
	@echo "  INFINISIM_DIR          InfiniSim source directory (default: build_lv_sim/InfiniSim)"
	@echo "  INFINISIM_BUILD_DIR    Simulator build directory (default: build_lv_sim/InfiniSim/build)"
	@echo "  FIRMWARE_IMAGE_NAME    Firmware image name (default: infinitime-build)"
	@echo "  INFINISIM_IMAGE_NAME   Simulator image name (default: infinisim-build)"
	@echo "  REBUILD_IMAGE          Rebuild container image when set to 1"
	@echo ""
	@echo "Examples:"
	@echo "  make build              # Build firmware in container"
	@echo "  make build REBUILD_IMAGE=1"
	@echo "  make build-infinisim    # Build simulator in container"
	@echo "  make run-infinisim      # Run simulator"

# Build firmware in container
build: ensure-firmware-image
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		if ! grep -q '^CMAKE_HOME_DIRECTORY:INTERNAL=/sources$$' "$(BUILD_DIR)/CMakeCache.txt"; then \
			echo "[prep] Removing incompatible CMake cache in $(BUILD_DIR) for container build"; \
			rm -rf "$(BUILD_DIR)"; \
		fi; \
	fi
	@echo "[2/2] Building firmware..."
	@$(CONTAINER_RUNTIME) run --rm -it \
		-v "$(CURDIR):/sources$(CONTAINER_VOLUME_SUFFIX)" \
		-e "BUILD_DIR=/sources/$(BUILD_DIR)" \
		-e "OUTPUT_DIR=/sources/$(BUILD_DIR)/output" \
		"$(FIRMWARE_IMAGE_NAME)"

# Build InfiniSim simulator in container
build-infinisim: check-runtime
	@echo "[1/3] Preparing InfiniSim build..."
	@if [ ! -x "tools/run-infinisim-podman.sh" ]; then \
		echo "Error: tools/run-infinisim-podman.sh is missing or not executable" >&2; \
		exit 1; \
	fi
	@INFINISIM_DIR="$(INFINISIM_DIR)" CONTAINER_RUNTIME="$(CONTAINER_RUNTIME)" IMAGE_NAME="$(INFINISIM_IMAGE_NAME)" REBUILD_IMAGE="$(REBUILD_IMAGE)" tools/run-infinisim-podman.sh --no-run

# Run the InfiniSim simulator
run-infinisim: check-runtime
	@if [ ! -x "tools/run-infinisim-podman.sh" ]; then \
		echo "Error: tools/run-infinisim-podman.sh is missing or not executable" >&2; \
		exit 1; \
	fi
	@echo "[3/3] Starting simulator..."
	@INFINISIM_DIR="$(INFINISIM_DIR)" CONTAINER_RUNTIME="$(CONTAINER_RUNTIME)" IMAGE_NAME="$(INFINISIM_IMAGE_NAME)" REBUILD_IMAGE="$(REBUILD_IMAGE)" tools/run-infinisim-podman.sh

# Clean build directories
clean:
	@echo "Cleaning build directories..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(INFINISIM_BUILD_DIR)
	@echo "Clean complete"

# Phony targets
.PHONY: check-runtime ensure-firmware-image build build-infinisim run-infinisim clean help
