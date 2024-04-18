from multiprocessing.managers import BaseManager
import queue
import sys
import json
import time

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


def main(ip, port, mode):
    object_amount = 50

    A = read("A.dat")
    ma = len(A[0])
    na = len(A)
    # if(ma != nx):
    #     print("Złe parametry danych wejściowych")
    #     return
    QueueManager.register('in_queue')
    QueueManager.register('out_queue')
    manager = QueueManager(address=(ip, int(port)), authkey=b'blah')
    manager.connect()
    queue = manager.in_queue()
    out = manager.out_queue()
    C = [0] * na

    start = time.time()
    if mode == "1":
        for i in range(na):
            queue_object = QueueObject(A[i], i)
            queue.put(queue_object)
        for i in range(na):
            queue_output = out.get()
            if queue_output is not None:
                C[queue_output.number] = queue_output.c

    elif mode == "2":
        for i in range(int(na / object_amount)):
            queue_object_list = []
            for j in range(object_amount):
                index = i * object_amount + j
                queue_object = QueueObject(A[index], index)
                queue_object_list.append(queue_object)
            queue.put(queue_object_list)
        for i in range(int(na / object_amount)):
            queue_output_list = out.get()
            if len(queue_output_list) != 0:
                for queue_output in queue_output_list:
                    if queue_output is not None:
                        C[queue_output.number] = queue_output.c
    
    end = time.time()
    print(C)
    print("Czas: ", str(end - start))

if __name__ == '__main__':
    main(*sys.argv[1:])