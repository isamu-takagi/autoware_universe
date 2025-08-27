import json
import sys

import jsonschema
import yaml

from .common.config import TestConfig


def validate(schema_path, target_path):
    print(f"schema: {schema_path}")
    print(f"target: {target_path}")
    with open(schema_path) as fp:
        schema = json.load(fp)
    with open(target_path) as fp:
        target = yaml.safe_load(fp)
    try:
        jsonschema.validate(target, schema)
    except jsonschema.ValidationError as error:
        print(error.message)


def main():
    print("Test Tool")
    print(TestConfig)
    validate(*sys.argv[1:])
