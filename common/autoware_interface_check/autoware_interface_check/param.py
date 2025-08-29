import json

import jsonschema
import yaml

from .common.path import FilePath
from .common.target import TestCase
from .common.target import TestResult


def validate(schema_path, params_path):
    print(f"schema: {schema_path}")
    print(f"params: {params_path}")
    with schema_path.path.open() as fp:
        schema = json.load(fp)
    with params_path.path.open() as fp:
        target = yaml.safe_load(fp)
    try:
        jsonschema.validate(target, schema)
        print("OK")
    except jsonschema.ValidationError as error:
        print(error.message)


def check(case: TestCase):
    try:
        schema = FilePath(case.data["schema"])
        params = FilePath(case.data["params"])
        validate(schema, params)
    except Exception as error:
        case.result = TestResult.ERROR
        case.details = repr(error)


def main():
    print("param-check")
