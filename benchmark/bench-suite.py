import subprocess
from threading import Thread
import re
import sys

def main(num_process, num_fibers):
    stdouts = ["" for i in range(num_process)]
    processes = []
    threads = []
    timings = [0.0 for i in range(num_process)]

    def threaded_fun(tid, process):
        out, err = process.communicate()
        regex = r"Time to run do the work \(per-fiber\):[0-9 ^\.]*\.[0-9]*"
        found = re.search(regex,str(out))
        time = float(found.group(0).split(":")[1].strip())
        print("Process#%d time: %f" % (tid, time))
        timings[tid] = time
        return 

    for i in range(num_process):
        processes.append(subprocess.Popen(["./test", str(num_fibers)], stdout=subprocess.PIPE))
        threads.append(Thread(target=threaded_fun, args=[i, processes[i]]))
    for i in range(num_process):
        threads[i].start()
    for i in range(num_process):
        threads[i].join()

    average_time = sum(timings) / num_process
    print("Average fiber running time for %d processes and %d fibers is %f" % (num_process, num_fibers, average_time))

if __name__ == "__main__":
    if len(sys.argv) == 3:
        main(int(sys.argv[1]), int(sys.argv[2]))
    else:
        print("Usage: python bench-suite.py <num_processes> <num_fibers>")