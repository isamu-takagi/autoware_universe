from pathlib import Path

from ament_index_python.packages import get_package_share_directory


class FilePath:
    def __init__(self, data):
        self.data = data
        self.path = self.resolve(data)

    def __str__(self):
        return str(self.path)

    @staticmethod
    def resolve(data: str | dict):
        if type(data) is str:
            return Path(data)
        package = data.get("package")
        path = data["path"]
        if package is None:
            return Path(path)
        package = get_package_share_directory(package)
        return Path(package) / Path(path)
