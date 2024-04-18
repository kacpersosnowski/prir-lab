from multiprocessing.managers import BaseManager
import queue
import sys
import math

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

def my_print():
    print("test")


def main(ip, port):
    in_queue = queue.Queue()
    out_queue = queue.Queue()
    QueueManager.register('in_queue', callable=lambda:in_queue)
    QueueManager.register('out_queue', callable=lambda:out_queue)
    manager = QueueManager(address=(ip, int(port)), authkey=b'blah')
    server = manager.get_server()
    server.serve_forever()


if __name__ == '__main__':
    main(*sys.argv[1:])
