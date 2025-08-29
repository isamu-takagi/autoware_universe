import argparse
import time
import xml.etree.ElementTree as ET

from .common.case import TestStatus
from .common.case import TestSuite
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
        case.result = param.check(case.data)

    duration = time.time() - start

    for index, case in enumerate(suite.cases):
        print(f"Test #{index} ({case.result.status.name})")
        print("  message:", case.result.message)
        print("  details:")
        for key, value in case.result.details:
            print(f"    {key}: {value}")
        print()

    print("Summary")
    print("  all    :", suite.count())
    print("  success:", suite.count(TestStatus.Success))
    print("  failure:", suite.count(TestStatus.Failure))
    print("  errors :", suite.count(TestStatus.Error))

    if args.xunit_file:
        path = args.xunit_file
        name = args.xunit_name
        generate_xunit(path, suite, name, duration)


def generate_xunit(path, suite, name, duration):
    root = ET.Element("testsuite")
    root.set("name", name)
    root.set("tests", str(suite.count()))
    root.set("errors", str(suite.count(TestStatus.Error)))
    root.set("failures", str(suite.count(TestStatus.Failure)))
    root.set("time", f"{duration:.3f}")

    tree = ET.ElementTree(root)
    tree.write(path, xml_declaration=True, encoding="UTF-8")
