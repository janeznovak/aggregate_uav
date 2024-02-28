export const getGoalStatusDescription = (code: string): string => {
    switch (code) {
        case "REACHED":
            return "COMPLETATO"
        case "RUNNING":
            return "IN CORSO"
        case "ABORTED":
            return "INTERROTTO"
        case "FAILED":
            return "FALLITO"
        case "UNKNOWN":
            return "ERRORE SCONOSCIUTO"
        case "NO_GOAL":
            return "NON PRESENTE"
        default:
          break;
      }
};

export const getGoalStatusIcon = (code: string): string => {
    switch (code) {
        case "REACHED":
          return 'nb-checkmark-circle,green'
        case "RUNNING":
          return 'nb-loop,orange'
        case "ABORTED":
          return 'nb-close-circled,red'
        case "FAILED":
          return 'nb-alert,red'
        case "UNKNOWN":
          return 'nb-fold,grey'
        case "NO_GOAL":
          return 'nb-foldnb-fold,grey'
        default:
          return 'nb-fold'
    }
};

export const getSubCodeDescription = (code: string): string => {
  switch (code) {
      case "BODY_TEMPERATURE":
          return "media temperatura corporea"
      case "NONE":
          return "chiamata manuale"
      case "ACCIDENTAL_FALL":
          return "caduta accidentale"
      case "HEART":
          return "problema cardiaco"
      case "OXYGENATION":
          return "media ossigenazione bassa"
        break;
    }
};

export const getPatientNameFromCode = (wearableCode: string): string => {
  const match = wearableCode.match(/_(\d+)$/);
  const numberWearable = match ? parseInt(match[1], 10) : null;
  return 'Paziente ' + numberWearable
}

export const getRobotName = (nodeId: number): string => {
  return 'Robot ' + nodeId
}

export const getWearableName = (wearableId: number): string => {
  return 'Paziente ' + wearableId
}
