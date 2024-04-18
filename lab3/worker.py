from multiprocessing.managers import BaseManager
import multiprocessing
import sys
import json

class QueueObject:
    def __init__(self, Ab, number):
        self.Ab = Ab
        self.number = number
    
class QueueOutput:
    def __init__(self, c, number):
        self.c = c
        self.number = number
    

class QueueManager(BaseManager):
    pass

def read(fname):
	f = open(fname, "r")
	nr = int(f.readline())
	nc = int(f.readline())

	A = [[0] * nc for x in range(nr)]
	r = 0
	c = 0
	for i in range(0,nr*nc):
		A[r][c] = float(f.readline())
		c += 1
		if c == nc:
			c = 0
			r += 1

	return A

def mult1(Ab, X):
    output = 0
    for index, value in enumerate(Ab):
        output += value * X[index][0]
    return output

def mult2(queue_objects, X):
    queue_outputs = []
    output = 0
    for queue_object in queue_objects:
        for index, value in enumerate(queue_object.Ab):
            output += value * X[index][0]
        queue_outputs.append(QueueOutput(output, queue_object.number))
        output = 0
    return queue_outputs

def worker(ip, port, mode):
    X = read('X.dat')
    QueueManager.register('in_queue')
    QueueManager.register('out_queue')
    manager = QueueManager(address=(ip, int(port)), authkey=b'blah')
    manager.connect()
    print("Połączono")
    queue = manager.in_queue()
    out = manager.out_queue()

    if mode == "1":
        while True:
            queue_object = queue.get()
            if queue_object is not None:
                output = mult1(queue_object.Ab, X)
                queue_output = QueueOutput(output, queue_object.number)
                out.put(queue_output)

    elif mode == "2":
        while True:
            queue_object_list = queue.get()
            if len(queue_object_list) != 0:
                queue_output_list = mult2(queue_object_list, X)
                out.put(queue_output_list)
                

def main():
    if len(sys.argv) < 4:
        print("Usage: python program.py ip_address port")
        return

    ip = sys.argv[1]
    port = sys.argv[2]
    mode = sys.argv[3]
    num_processes = multiprocessing.cpu_count()

    processes = []
    for _ in range(num_processes):
        p = multiprocessing.Process(target=worker, args=(ip, port, mode))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()

if __name__ == '__main__':
    main()
