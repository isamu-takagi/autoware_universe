import json

import jsonschema
import yaml

from .common.case import TestResult
from .common.path import FilePath


def validate(schema_path, params_path):
    details = (
        ("schema", str(schema_path)),
        ("params", str(params_path)),
    )
    with schema_path.path.open() as fp:
        schema = json.load(fp)
    with params_path.path.open() as fp:
        target = yaml.safe_load(fp)
    try:
        jsonschema.validate(target, schema)
        return TestResult.Success("OK", details)
    except jsonschema.ValidationError as error:
        return TestResult.Failure(error.message, details)


def check(data: dict):
    try:
        schema = FilePath.Parse(data["schema"])
        params = FilePath.Parse(data["params"])
        return validate(schema, params)
    except Exception as error:
        return TestResult.Error(repr(error), "")


def main():
    print("param-check")
