import matplotlib.pyplot as plt
import numpy as np


def plot(x, y):
    fig, ax = plt.subplots()
    ax.plot(x, y)
    plt.show()


def readFile(name):
    with open(name) as file:
        m = []
        loss_rate = []
        lines = file.readlines()
        lines = [line.rstrip().split(':') for line in lines]
        for i in lines:
            m.append(i[0])
            loss_rate.append(i[1])
        return np.array(m), np.array(loss_rate)


def main():
    m, loss = readFile('loss_rate.txt')
    plot(m, loss)


if __name__ == '__main__':
    main()