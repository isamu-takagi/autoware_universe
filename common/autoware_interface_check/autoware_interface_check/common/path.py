from pathlib import Path

from ament_index_python.packages import get_package_share_directory


class FilePath:
    def __init__(self, path):
        self.path = path

    def __str__(self):
        return str(self.path)

    @staticmethod
    def Parse(data: dict):
        # package not found
        file = data.get("file")
        if file is None:
            # throw ParseError
            return None
        pkg = data.get("package")
        if pkg is None:
            return FilePath(Path(file))
        else:
            pkg = get_package_share_directory(pkg)
            return FilePath(Path(pkg) / Path(file))
