SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit

mkdir -p Build && cd Build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
