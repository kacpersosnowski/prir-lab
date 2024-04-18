from mpi4py import MPI
import sys
from utils import *

def main(argv):
    amount_of_points = int(argv[0])
    start = float(argv[1])
    end = float(argv[2])
    comm = MPI.COMM_WORLD
    id = comm.Get_rank()            
    num_processes = comm.Get_size()  
    myHostName = MPI.Get_processor_name()

    if id==0:
        points = calculate_range(amount_of_points, start, end)
        ranges = calculate_individual_ranges(num_processes, amount_of_points)
        for i in range(1, num_processes):
            comm.send((points, ranges[i]), dest=i)
        result = integrate(ranges[id][0], ranges[id][1], points)
        for i in range(1, num_processes):
            result+=comm.recv(source=i)

        print(result)
    else:
        rec_points, rec_range = comm.recv(source=0)
        ind_result = integrate(rec_range[0], rec_range[1], rec_points)
        comm.send(ind_result, dest= 0)

if "__main__" == __name__:
    main(sys.argv[1:])