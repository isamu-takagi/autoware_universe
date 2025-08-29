from enum import Enum
from pathlib import Path

import yaml


class TestResult(Enum):
    NONE = 1
    OK = 1
    FAILURE = 2
    ERROR = 3
    SKIPPED = 4


class TestCase:
    def __init__(self, data):
        self.result = TestResult.NONE
        self.data = data
        self.message = ""
        self.details = ""


class TestSuite:
    def __init__(self, cases):
        self.cases = cases

    @staticmethod
    def Load(path: Path | str):
        path = Path(path) if type(path) is str else path
        with path.open() as fp:
            suite = yaml.safe_load(fp)
        return TestSuite([TestCase(data) for data in suite.get("param-tests", [])])
