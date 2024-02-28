# Copyright © 2023 University of Turin, Daniele Bortoluzzi & Giordano Scarso. All Rights Reserved.

import string, time, csv
from datetime import datetime
from pathlib import Path
import os

STORAGE_BASE_PATH = "../../../../Storage/"
WEARABLE_PLACEHOLDER = "#WEARABLE_CODE"
WEARABLE_OUTPUT_PATH_TO_WEB_FEEDBACK = "from_wearable/#WEARABLE_CODE/to_web/feedback/"
WEARABLE_OUTPUT_PATH_TO_WEB_SOS = "from_wearable/#WEARABLE_CODE/to_web/sos/"
WEARABLE_OUTPUT_PATH_TO_AP = "from_wearable/#WEARABLE_CODE/to_ap/feedback/"
DELIMITER = ";"

def write_file_feedback(wearable:string, store, suffix=""):
    """!
    Write physically a file in the Storage with data coming from odometry and sensors.
    """

    dt_obj = datetime.now()  # input datetime object
    millis = int(float(dt_obj.strftime('%s.%f')) * 1e3)
    filename = "wearable_feedback-{}{}.txt".format(millis, suffix)

    path_web = STORAGE_BASE_PATH + WEARABLE_OUTPUT_PATH_TO_WEB_FEEDBACK.replace(WEARABLE_PLACEHOLDER, wearable) + filename
    path_ap = STORAGE_BASE_PATH + WEARABLE_OUTPUT_PATH_TO_AP.replace(WEARABLE_PLACEHOLDER, wearable) + filename

    data = [store["sensors"]['wearable_code'],
            store['position']['x'],
            store['position']['y'],
            store["sensors"]['heart_rate_per_minute'],
            store["sensors"]['body_temperature'],
            store["sensors"]['oxygenation'],
            store["sensors"]['goal_status'],
            store["sensors"]['goal_code'],
            millis]

    path_web_abs = Path(__file__).parent / path_web
    path_ap_abs = Path(__file__).parent / path_ap
    path_web_abs.parent.mkdir(exist_ok=True, parents=True)
    path_ap_abs.parent.mkdir(exist_ok=True, parents=True)

    with path_web_abs.with_suffix('.lock').open(mode="w") as file_to_web:
        writer = csv.writer(file_to_web, delimiter=DELIMITER, quoting=csv.QUOTE_NONE)
        writer.writerow(data)
    os.rename(path_web_abs.with_suffix('.lock'), path_web_abs)

    with path_ap_abs.with_suffix('.lock').open(mode="w") as file_to_ap:
        writer = csv.writer(file_to_ap, delimiter=DELIMITER, quoting=csv.QUOTE_NONE)
        writer.writerow(data)
    os.rename(path_ap_abs.with_suffix('.lock'), path_ap_abs)

    return filename

def write_file_sos(wearable:string, store, suffix=""):
    """!
    Write physically a file in the Storage with data coming from odometry and triggers SOS.
    """

    dt_obj = datetime.now()  # input datetime object
    millis = int(float(dt_obj.strftime('%s.%f')) * 1e3)
    filename = "wearable_sos-{}{}.txt".format(millis, suffix)

    path_web = STORAGE_BASE_PATH + WEARABLE_OUTPUT_PATH_TO_WEB_SOS.replace(WEARABLE_PLACEHOLDER, wearable) + filename

    data = [store["triggers"]['wearable_code'],
            store['position']['x'],
            store['position']['y'],
            store["triggers"]['priority'],
            store["triggers"]['heart_rate_per_minute'],
            store["triggers"]['body_temperature'],
            store["triggers"]['oxygenation'],
            store["triggers"]['accidental_fall_sos_triggered'],
            store["triggers"]['heart_rate_sos_triggered'],
            store["triggers"]['body_temperature_sos_triggered'],
            store["triggers"]['oxygenation_sos_triggered'],
            store["triggers"]['follow_triggered'],
            millis]

    path_web_abs = Path(__file__).parent / path_web
    path_web_abs.parent.mkdir(exist_ok=True, parents=True)

    with path_web_abs.with_suffix('.lock').open(mode="w") as file_to_web:
        writer = csv.writer(file_to_web, delimiter=DELIMITER, quoting=csv.QUOTE_NONE)
        writer.writerow(data)
    os.rename(path_web_abs.with_suffix('.lock'), path_web_abs)

    return filename
