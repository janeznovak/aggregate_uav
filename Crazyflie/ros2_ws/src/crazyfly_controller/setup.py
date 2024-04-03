from setuptools import find_packages, setup
import os
from glob import glob

package_name = 'crazyfly_controller'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='valentino',
    maintainer_email='valentino.dicianni@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            "run_many = crazyfly_controller.crazyfly_controller:main"
        ],
    },
)
