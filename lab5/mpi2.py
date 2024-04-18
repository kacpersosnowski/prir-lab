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
    else:
        ranges = None
        points = calculate_range(amount_of_points, start, end)
    
    my_range = comm.scatter(ranges, root=0)
    ind_result = integrate(my_range[0], my_range[1], points)

    done_results = comm.gather(ind_result, root = 0)

    if id==0:
        result = 0
        for element in done_results:
            result+=element
        print(result)

if "__main__" == __name__:
    main(sys.argv[1:])