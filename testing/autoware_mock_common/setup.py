from setuptools import find_packages
from setuptools import setup

package_name = "autoware_mock_common"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    data_files=[
        ("share/ament_index/resource_index/packages", [f"resource/{package_name}"]),
        (f"share/{package_name}", ["package.xml"]),
        (f"lib/{package_name}", ["script/mock"]),
    ],
    install_requires=["setuptools"],
    extras_require={"test": ["pytest"]},
    zip_safe=True,
    maintainer="Takagi, Isamu",
    maintainer_email="isamu.takagi@tier4.jp",
    license="Apache License 2.0",
)
