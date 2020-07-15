import numpy as np
import sys

def main(n):
    s = np.random.randint(n)
    t = np.random.randint(n)
    graph = np.argwhere(np.random.rand(n, n) < 0.10)
    graph = graph[graph[:, 0] < graph[:, 1]]
    np.savetxt('input.txt', graph, fmt = '%d', comments = '', header = '{} {} {}'.format(n, s, t))

if __name__ == "__main__":
    main(int(sys.argv[1]))
