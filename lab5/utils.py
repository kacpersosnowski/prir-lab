import math


def normal_round(n):
    if n - math.floor(n) < 0.5:
        return math.floor(n)
    return math.ceil(n)

def fun(x):
    return pow(x, 2)

def calculate_range(amount_of_points, start, end):
    points = []
    current = start
    for i in range(amount_of_points):
        points.append(current)
        current+=(end-start)/(amount_of_points-1)
    return points

def calculate_individual_ranges(num_processes, amount_of_points):
    ids = []
    divisor = float((amount_of_points -1) / num_processes)
    for i in range(num_processes):
        ind_id = []
        start = int(normal_round(i * divisor))
        end = int(normal_round((i+1.0) *divisor - 1.0))
        ind_id.append(start)
        ind_id.append(end)
        ids.append(ind_id)
    return ids

def integrate(start, end, points):
    result = 0
    for i in range(start, end+1):
        result+=((fun(points[i]) + fun(points[i+1])) * (points[i+1] - points[i])/2)
    return result
