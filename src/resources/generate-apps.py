import argparse
import json
import os
import pathlib
import shutil
import subprocess
import sys

DEFAULT_STACK_SIZE = 50
OVERLAY_SIZE = 2048

def main():
    ap = argparse.ArgumentParser(description='compile Pawn source files into binaries')
    ap.add_argument('config', type=str, help='config file to use')
    ap.add_argument('--pawncc', type=str, help='Path to "pawncc" executable', default="pawncc")
    ap.add_argument('--include', '-i', type=str, help='Path to Pawn include folder in InfiniTime sources', default="src/pawn/programs/include")
    args = ap.parse_args()

    with open(args.config, 'r') as fd:
        data = json.load(fd)

    for name, app in data.items():
        print("Compiling Pawn: " + name)

        source = app["source"]
        if not os.path.exists(source):
            source = os.path.join(os.path.dirname(sys.argv[0]), source)

        args = [args.pawncc, "-p" + os.path.join(args.include, "infinitime.inc"), "-i" + args.include]
        if not app.get("debug", False):
            args.append("-O3")
            args.append("-d0")
        if app.get("overlays", False):
            args.append("-V" + str(OVERLAY_SIZE))
        args.append("-S" + str(app.get("stack_size", DEFAULT_STACK_SIZE)))

        args.append(source)

        subprocess.check_call(args) # The output will be written to the current directory with the ".amx" extension

        sourcePath = pathlib.Path(source)
        shutil.move(sourcePath.stem + ".amx", sourcePath.stem + ".bin")

if __name__ == '__main__':
    main()
