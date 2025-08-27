import argparse

from .common.target import TestFile
from . import param


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("target")
    args = parser.parse_args()

    target = TestFile(args.target)
    for case in target.cases():
        param.check(case)
