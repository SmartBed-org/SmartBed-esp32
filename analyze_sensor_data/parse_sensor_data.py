import csv

LINE_LENGTH = 37
SUBSTR_BEGIN = 16
VAL_SIZE = 6

LOAD_SENSOR = 'Load_cell output val: '
FSR1 = 'Flexi Force sensor1: '
FSR2 = 'Flexi Force sensor2: '
SPACE = ' '


def parse_file():
    with open("data.txt", "r") as myfile:
        data = myfile.read().splitlines()

    '''
    val_to_print = data[0][SUBSTR_BEGIN:LINE_LENGTH]
    print(val_to_print)

    val_to_print = data[0][len(data[0]) - VAL_SIZE:len(data[0])]
    if SPACE in val_to_print:
        val_to_print = val_to_print.replace(' ', '')

    print(val_to_print)
    '''

    values_weight_sensors, values_fsr1, values_fsr2, timeline_fsr, timeline_load_cell = [], [], [], [], []
    current_time = 0;

    for line in data:
        if len(line) < LINE_LENGTH:
            continue
        else:
            if line[SUBSTR_BEGIN:LINE_LENGTH] == FSR1:
                value = line[len(line)-VAL_SIZE:len(line)]
                if SPACE in value:
                    value = value.replace(' ', '')
                values_fsr1.append(value)
                continue
            if line[SUBSTR_BEGIN:LINE_LENGTH] == FSR2:
                timeline_fsr.append(current_time)
                timeline_load_cell.append(current_time)
                current_time = current_time + 1
                value = line[len(line)-VAL_SIZE:len(line)]
                if SPACE in value:
                    value = value.replace(' ', '')
                values_fsr2.append(value)
                values_weight_sensors.append(0)
                continue
            if len(line) > LINE_LENGTH + 1 and line[SUBSTR_BEGIN:LINE_LENGTH + 1] == LOAD_SENSOR:
                timeline_load_cell.append(current_time)
                current_time = current_time + 1
                value = line[len(line)-VAL_SIZE:len(line)]
                if SPACE in value:
                    value = value.replace(' ', '')
                values_weight_sensors.append(value)
                continue
            else:
                continue

    with open('fsr1.csv', 'w') as fsr1csv:
        writer = csv.writer(fsr1csv)
        writer.writerow(values_fsr1)

    with open('fsr2.csv', 'w') as fsr2csv:
        writer = csv.writer(fsr2csv)
        writer.writerow(values_fsr2)

    with open('load_cell.csv', 'w') as loadCellCsv:
        writer = csv.writer(loadCellCsv)
        writer.writerow(values_weight_sensors)

    with open('timeline_fsr.csv', 'w') as timelineFsrCsv:
        writer = csv.writer(timelineFsrCsv)
        writer.writerow(timeline_fsr)

    with open('timeline_load_cell.csv', 'w') as timelineLoadCellCsv:
        writer = csv.writer(timelineLoadCellCsv)
        writer.writerow(timeline_load_cell)


if __name__ == '__main__':
    parse_file()
