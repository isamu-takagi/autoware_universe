import argparse
import time
import xml.etree.ElementTree as ET

from .common.target import TestSuite
from . import param


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("target")
    parser.add_argument("--xunit-file")
    parser.add_argument("--xunit-name")
    args = parser.parse_args()

    start = time.time()

    suite = TestSuite.Load(args.target)
    for case in suite.cases:
        param.check(case)

    duration = time.time() - start

    for case in suite.cases:
        print(case.result)
        print(case.details)

    if args.xunit_file:
        path = args.xunit_file
        name = args.xunit_name
        generate_xunit(path, name, duration)


def generate_xunit(path, name, duration):
    root = ET.Element("testsuite")
    root.set("name", name)
    root.set("tests", "1")
    root.set("errors", "0")
    root.set("failures", "0")
    root.set("time", f"{duration:.3f}")

    tree = ET.ElementTree(root)
    tree.write(path, xml_declaration=True, encoding="UTF-8")
