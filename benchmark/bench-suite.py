import subprocess
from threading import Thread
import re
import sys

def main(num_process, num_fibers):
    print("=== Fibers benchmark suite ===")
    processes = []
    threads = []
    run_timings = [0.0 for i in range(num_process)]
    init_timings = [0.0 for i in range(num_process)]

    def threaded_fun(tid, process):
        out, err = process.communicate()
        # run time
        run_time_regex = r"Time to run do the work \(per-fiber\):[0-9 ^\.]*\.[0-9]*"
        run_time_found = re.search(run_time_regex, str(out))
        run_time_line = run_time_found.group(0)
        if run_time_line != None:
            run_time = float(run_time_line.split(":")[1].strip())
            #print("Process#%d time: %f" % (tid, run_time))
            run_timings[tid] = run_time
        else:
            print("Process#%d has no run time line!" % tid)
        # Init time
        init_time_regex = r"Time to initialize fibers:[0-9 ^\.]*\.[0-9]*"
        init_time_found = re.search(init_time_regex, str(out))
        init_time_line = init_time_found.group(0)
        if init_time_line != None:
            init_time = float(init_time_line.split(":")[1].strip())
            #print("Process#%d init time: %f" % (tid, init_time))
            init_timings[tid] = init_time
        else:
            print("Process#%d has no init time line!" % tid)

        print("Process#%d terminated" % tid)
        return 
    
    def print_timings():
        # run time
        for i in range(num_process):
            print("Process#%d time: %f" % (i, run_timings[i]))
        # init time
        for i in range(num_process):
            print("Process#%d init time: %f" % (i, init_timings[i]))
        

    for i in range(num_process):
        processes.append(subprocess.Popen(["./test", str(num_fibers)], stdout=subprocess.PIPE))
        threads.append(Thread(target=threaded_fun, args=[i, processes[i]]))
    for i in range(num_process):
        threads[i].start()
    for i in range(num_process):
        threads[i].join()

    average_run_time = sum(run_timings) / num_process
    averate_init_time = sum(init_timings) / num_process

    print()
    print("==> Timings")
    print_timings()
    print()
    print("With %d processes and %d fibers:" % (num_process, num_fibers))
    print("- Average fiber run time is %f" % average_run_time)
    print("- Average initialization time is %f" % averate_init_time)

if __name__ == "__main__":
    if len(sys.argv) == 3:
        main(int(sys.argv[1]), int(sys.argv[2]))
    else:
        print("Usage: python bench-suite.py <num_processes> <num_fibers>")