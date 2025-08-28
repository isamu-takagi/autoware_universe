import argparse

from .common.target import TestFile
from . import param


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("target")
    parser.add_argument("--xunit-file")
    args = parser.parse_args()

    target = TestFile(args.target)
    for case in target.cases():
        param.check(case)

    from pathlib import Path

    if args.xunit_file:
        xunit = Path(args.xunit_file)
        print(xunit)
        with xunit.open("w") as fp:
            fp.write(generate_xunit())


# def generate_xunit(report, testname, elapsed):
def generate_xunit(report, testname, elapsed):
    return ""
