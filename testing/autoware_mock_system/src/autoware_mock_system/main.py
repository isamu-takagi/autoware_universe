from autoware_mock_common.main import run

from . import NODE_NAME
from . import SPEC
from . import default_plugins


def main(args=None):
    run(spec=SPEC, default_plugins=default_plugins(), node_name=NODE_NAME, args=args)


if __name__ == "__main__":
    main()
