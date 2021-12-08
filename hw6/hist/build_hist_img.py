#!/usr/bin/env python3

import argparse

import matplotlib.pyplot as plt


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)

    args = parser.parse_args()

    with open(args.input, 'r') as fin:
        data = fin.readlines()

    for i in range(len(data)):
        data[i] = list(map(int, data[i].strip().split(' ')))

    fig, ax_arr = plt.subplots(3, 1, figsize=(4, 12))

    for i in range(3):
        ax_arr[i].bar(
            list(range(256)), data[i],
            width=1, edgecolor='none', color='black')

        ax_arr[i].axis('off')
        ax_arr[i].set_xlim([0, 255])

    fig.savefig(args.output)


if __name__ == "__main__":
    main()
