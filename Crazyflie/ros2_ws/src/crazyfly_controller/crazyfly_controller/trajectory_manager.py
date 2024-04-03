import csv
import os

STORAGE_BASE_PATH           = os.getcwd() +"/src/Storage/"
ROBOT_PLACEHOLDER           = "#ROBOT"
TRAJECTORY_PATH             = "trajectory/#ROBOT/"

class TrajectoryManager:
    def __init__(self, cfname):
        self.robot_name = cfname


    def write_waypoint_file(self, data):        
        waypoint_path = STORAGE_BASE_PATH + TRAJECTORY_PATH.replace(ROBOT_PLACEHOLDER, self.robot_name) 

        if not os.path.exists(str(waypoint_path)):
            os.makedirs(str(waypoint_path))
                
        with open(waypoint_path + "waypoint.csv", mode='w', newline='') as file_csv:
                writer = csv.writer(file_csv)
                writer.writerows(data)
        
            
    def generate_trajectory(self):
        waypoint_path = STORAGE_BASE_PATH + TRAJECTORY_PATH.replace(ROBOT_PLACEHOLDER, self.robot_name) 
        if not os.path.exists(str(waypoint_path)):
            return ""

        os.chdir(STORAGE_BASE_PATH + "trajectory/")
        os.system(f"./genTrajectory -i {self.robot_name}/waypoint.csv -o {self.robot_name}/traj.csv")
        return STORAGE_BASE_PATH + TRAJECTORY_PATH.replace(ROBOT_PLACEHOLDER, self.robot_name) + "traj.csv"
