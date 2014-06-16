from itertools import combinations
import sys

if __name__ == '__main__':
    n = int(sys.argv[1])
    r = int(sys.argv[2])
    for t in combinations(range(n), r):
        print t
