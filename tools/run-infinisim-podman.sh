#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -f "${SCRIPT_DIR}/../CMakeLists.txt" && -d "${SCRIPT_DIR}/../src" ]]; then
  ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
elif [[ -f "${SCRIPT_DIR}/CMakeLists.txt" && -d "${SCRIPT_DIR}/src" ]]; then
  ROOT_DIR="${SCRIPT_DIR}"
else
  echo "Error: could not locate repository root." >&2
  exit 1
fi
SIM_DIR="${INFINISIM_DIR:-${ROOT_DIR}/build_lv_sim/InfiniSim}"
if [[ "${SIM_DIR}" != /* ]]; then
  SIM_DIR="${ROOT_DIR}/${SIM_DIR}"
fi
IMAGE_NAME="${IMAGE_NAME:-infinisim-build}"
CONTAINER_RUNTIME="${CONTAINER_RUNTIME:-}"
BUILD_DIR="${BUILD_DIR:-build}"
INFINISIM_REPO="${INFINISIM_REPO:-https://github.com/InfiniTimeOrg/InfiniSim.git}"
INFINISIM_REF="${INFINISIM_REF:-main}"
UPDATE_INFINISIM="${UPDATE_INFINISIM:-0}"
REBUILD_IMAGE=0
RUN_BINARY=1
SIM_ARGS=()

usage() {
  cat <<'EOF'
Usage: tools/run-infinisim-podman.sh [options] [-- <infinisim-args>]

Options:
  --rebuild-image   Rebuild Podman image without cache
  --no-run          Only build, do not execute ./build/infinisim
  -h, --help        Show this help

Environment variables:
  IMAGE_NAME        Podman image name (default: infinisim-build)
  CONTAINER_RUNTIME Runtime to use (podman or docker, auto-detected if empty)
  INFINISIM_DIR     InfiniSim source directory (default: build_lv_sim/InfiniSim)
  INFINISIM_REPO    InfiniSim git repository URL
  INFINISIM_REF     InfiniSim git reference to clone (default: main)
  UPDATE_INFINISIM  Set to 1 to update existing clone to INFINISIM_REF
  BUILD_DIR         Build directory inside InfiniSim (default: build)

Examples:
  tools/run-infinisim-podman.sh
  tools/run-infinisim-podman.sh --rebuild-image
  tools/run-infinisim-podman.sh -- --hide-status
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild-image)
      REBUILD_IMAGE=1
      shift
      ;;
    --no-run)
      RUN_BINARY=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      SIM_ARGS=("$@")
      break
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ -z "${CONTAINER_RUNTIME}" ]]; then
  if command -v podman >/dev/null 2>&1; then
    CONTAINER_RUNTIME="podman"
  elif command -v docker >/dev/null 2>&1; then
    CONTAINER_RUNTIME="docker"
  else
    echo "Error: neither podman nor docker is installed or available in PATH." >&2
    exit 1
  fi
fi

if [[ ! -d "${SIM_DIR}" ]]; then
  echo "[prep] InfiniSim source not found at ${SIM_DIR}; cloning ${INFINISIM_REF}"
  mkdir -p "$(dirname "${SIM_DIR}")"
  git clone --depth 1 --branch "${INFINISIM_REF}" "${INFINISIM_REPO}" "${SIM_DIR}"
fi

if [[ ! -d "${SIM_DIR}/.git" ]]; then
  echo "Error: ${SIM_DIR} exists but is not a git checkout." >&2
  exit 1
fi

if [[ "${UPDATE_INFINISIM}" == "1" ]]; then
  echo "[prep] Updating InfiniSim checkout to ${INFINISIM_REF}"
  git -C "${SIM_DIR}" fetch --depth 1 origin "${INFINISIM_REF}"
  git -C "${SIM_DIR}" reset --hard FETCH_HEAD
fi

VOLUME_SUFFIX=""
if [[ "${CONTAINER_RUNTIME}" == "podman" ]]; then
  VOLUME_SUFFIX=":Z"
fi

cd "${SIM_DIR}"

PATCHED_CMAKELISTS=0
cleanup() {
  if [[ ${PATCHED_CMAKELISTS} -eq 1 && -f "${SIM_DIR}/CMakeLists.txt.copilot-backup" ]]; then
    mv "${SIM_DIR}/CMakeLists.txt.copilot-backup" "${SIM_DIR}/CMakeLists.txt"
  fi
}
trap cleanup EXIT

if [[ -f "${SIM_DIR}/CMakeLists.txt" ]] && [[ ! -f "${ROOT_DIR}/src/displayapp/localization/Localization.h" ]] && grep -q "displayapp/localization/Localization.h" "${SIM_DIR}/CMakeLists.txt"; then
  echo "[prep] Detected incompatible localization entries in InfiniSim/CMakeLists.txt; patching for current branch"
  cp "${SIM_DIR}/CMakeLists.txt" "${SIM_DIR}/CMakeLists.txt.copilot-backup"
  sed -i \
    -e '/displayapp\/localization\/Localization\.h/d' \
    -e '/displayapp\/localization\/Localization\.cpp/d' \
    "${SIM_DIR}/CMakeLists.txt"
  PATCHED_CMAKELISTS=1
fi

echo "[1/4] Initializing submodules"
git submodule update --init --recursive

if [[ ${REBUILD_IMAGE} -eq 1 ]]; then
  echo "[2/4] Rebuilding container image (no cache): ${IMAGE_NAME}"
  "${CONTAINER_RUNTIME}" build --no-cache -t "${IMAGE_NAME}" -f .devcontainer/Dockerfile .
else
  if ! "${CONTAINER_RUNTIME}" image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
    echo "[2/4] Building container image: ${IMAGE_NAME}"
    "${CONTAINER_RUNTIME}" build -t "${IMAGE_NAME}" -f .devcontainer/Dockerfile .
  else
    echo "[2/4] Container image already exists: ${IMAGE_NAME}"
  fi
fi

echo "[3/4] Building InfiniSim in container"
"${CONTAINER_RUNTIME}" run --rm -it \
  -v "${SIM_DIR}:/sources${VOLUME_SUFFIX}" \
  -v "${ROOT_DIR}:/sources/InfiniTime${VOLUME_SUFFIX}" \
  -e "BUILD_DIRECTORY=${BUILD_DIR}" \
  -w /sources \
  "${IMAGE_NAME}"

if [[ ${RUN_BINARY} -eq 1 ]]; then
  BIN_PATH="${SIM_DIR}/${BUILD_DIR}/infinisim"
  if [[ ! -x "${BIN_PATH}" ]]; then
    echo "Error: binary not found at ${BIN_PATH}" >&2
    exit 1
  fi

  echo "[4/4] Running simulator: ${BIN_PATH} ${SIM_ARGS[*]:-}"
  "${BIN_PATH}" "${SIM_ARGS[@]}"
else
  echo "[4/4] Build finished (execution skipped by --no-run)"
fi
