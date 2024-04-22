from setuptools import find_packages, setup
import os
from glob import glob

package_name = "crazyflie_controller"

setup(
    name=package_name,
    version="0.0.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        (
            os.path.join("share", package_name, package_name, "trajectory_data"),
            glob(os.path.join(package_name, "trajectory_data", "*")),
        ),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="valentino",
    maintainer_email="valentino.dicianni@gmail.com",
    description="TODO: Package description",
    license="TODO: License declaration",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "run_many = crazyflie_controller.crazyflie_controller:main"
        ],
    },
)
