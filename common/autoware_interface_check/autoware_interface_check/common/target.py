from pathlib import Path

import yaml


class TestCase:
    def __init__(self, data):
        self.data = data


class TestFile:
    def __init__(self, path):
        self.path = Path(path)

    def cases(self):
        with self.path.open() as fp:
            data = yaml.safe_load(fp)

        for case in data.get("param-tests", []):
            yield TestCase(case)
