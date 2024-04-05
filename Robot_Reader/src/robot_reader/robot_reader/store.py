# Copyright © 2023 University of Turin, Daniele Bortoluzzi & Giordano Scarso. All Rights Reserved.

from .position import PositionDTO
from .battery  import BatteryDTO
from .goal     import GoalDTO

from . import config

class Store:
    """!
    Class used to store sensor data received from the robot before
    writing it into a feedback file.
    """
    def __init__(self):
        self._dictionary = dict()
        for r in config.ROBOTS:
            robot_name = r["name"]
            self._dictionary[robot_name] = dict()
            self._dictionary[robot_name]["position"]    = PositionDTO(robot_name)
            self._dictionary[robot_name]["battery"]     = BatteryDTO(robot_name)
            self._dictionary[robot_name]["goal"]        = GoalDTO(robot_name)

content = Store()