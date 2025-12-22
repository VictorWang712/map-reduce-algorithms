import os
import subprocess
import time
import json
from datetime import datetime

SRC = "../code/src"
BIN = "../code/bin"
INPUT = "../data/input/The_Divine_Comedy_by_Dante.txt"
OUTPUT = "../data/output"
LOG = "../benchmark/log"

os.makedirs(BIN, exist_ok=True)
os.makedirs(OUTPUT, exist_ok=True)
os.makedirs(LOG, exist_ok=True)

programs = {
    "serial": "serial_wordcount.c",
    "mapreduce": "mapreduce_wordcount.c"
}

results = {}

for name, src in programs.items():
    exe = f"{BIN}/{name}.exe"
    subprocess.run(["gcc", f"{SRC}/{src}", "-O2", "-pthread", "-o", exe])

    out = f"{OUTPUT}/{name}_output.txt"
    if not os.path.exists(out):
        file = open(out, "w")
        file.close()

    start = time.time()
    subprocess.run([exe, INPUT, out])
    end = time.time()

    with open(out) as f:
        preview = f.read().splitlines()[:10]

    results[name] = {
        "time_seconds": end - start,
        "output_preview": preview
    }

timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
logfile = f"{LOG}/benchmark_{timestamp}.json"

with open(logfile, "w") as f:
    json.dump({
        "input": INPUT,
        "results": results
    }, f, indent=4)

print("Benchmark completed.")
print(f"Log saved to {logfile}")
