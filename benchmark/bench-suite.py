import subprocess
from threading import Thread
import re
import sys

# open each process within a gdb shell
use_xterm = False

def do_bench(num_process, num_fibers, silent=True):
    print("Starting bench (%d,%d)" % (num_process, num_fibers))
    processes = []
    threads = []
    run_timings = [0.0 for i in range(num_process)]
    init_timings = [0.0 for i in range(num_process)]

    def threaded_fun(tid, process):
        out, err = process.communicate()
        # parse run time
        run_time_regex = r"Time to run do the work \(per-fiber\):[0-9 ^\.]*\.[0-9]*"
        run_time_found = re.search(run_time_regex, str(out))
        if run_time_found != None:
            run_time_line = run_time_found.group(0)
            run_time = float(run_time_line.split(":")[1].strip())
            #print("Process#%d time: %f" % (tid, run_time))
            run_timings[tid] = run_time
        else:
            print("Process#%d has no run time line!" % tid)
        # parse init time
        init_time_regex = r"Time to initialize fibers:[0-9 ^\.]*\.[0-9]*"
        init_time_found = re.search(init_time_regex, str(out))
        if init_time_found != None:
            init_time_line = init_time_found.group(0)
            init_time = float(init_time_line.split(":")[1].strip())
            #print("Process#%d init time: %f" % (tid, init_time))
            init_timings[tid] = init_time
        else:
            print("Process#%d has no init time line!" % tid)

        if not silent:
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
        if(use_xterm):
            xterm_arg = "gdb -ex=r --args ./test " + str(num_fibers)
            xterm_params = ["xterm", "-e", xterm_arg]
            processes.append(subprocess.Popen(xterm_params, stdout=subprocess.PIPE))
        else:
            simple_args = "./test " + str(num_fibers)
            processes.append(subprocess.Popen(simple_args, stdout=subprocess.PIPE, shell=True))
        threads.append(Thread(target=threaded_fun, args=[i, processes[i]]))

    for i in range(num_process):
        threads[i].start()
    for i in range(num_process):
        threads[i].join()

    print("Bench (%d,%d) terminated" % (num_process, num_fibers))

    average_run_time = sum(run_timings) / num_process
    averate_init_time = sum(init_timings) / num_process

    if not silent:
        print()
        print("==> Timings")
        print_timings()
        print()
        print("With %d processes and %d fibers:" % (num_process, num_fibers))
        print("- Average fiber run time is %f" % average_run_time)
        print("- Average initialization time is %f" % averate_init_time)

    print()
    return average_run_time

def run_suite(processes_from, processes_to, processes_step ,fibers_from, fibers_to, fibers_step):
    def print_m(mat):
        for row in mat:
            for col in row:
                print("%3.3f " % col, end="")
            print()
    
    res_matrix = []
    i = 0
    j = 0
    for pr_n in range(processes_from, processes_to + processes_step, processes_step):
        res_matrix.append([])
        for f_n in range(fibers_from, fibers_to +fibers_step, fibers_step):
            res_matrix[i].append(do_bench(pr_n, f_n))
            j =+1
        i+=1
    
    print_m(res_matrix)  

if __name__ == "__main__":
    if len(sys.argv) == 7:
        num_processes_from = int(sys.argv[1])
        num_processes_to = int(sys.argv[2])
        processes_step = int(sys.argv[3])
        num_fibers_from = int(sys.argv[4])
        num_fibers_to = int(sys.argv[5])
        fibers_step = int(sys.argv[6])
        if(num_processes_from == num_processes_to and num_fibers_from == num_fibers_to):
            # for a single bench
            do_bench(num_processes_from, num_fibers_from, False)
        else:
            run_suite(num_processes_from, num_processes_to, processes_step, num_fibers_from, num_fibers_to, fibers_step) 
    elif len(sys.argv) == 3:
         do_bench(int(sys.argv[1]), int(sys.argv[2]), False)
    else:
        print("Usage: python bench-suite.py <num_processes> <num_fibers>")
        print("Usage: python bench-suite.py <num_processes_from> <num_processes_to> <processes_step> <num_fibers_from> <num_fibers_to> <fibers_step>")
