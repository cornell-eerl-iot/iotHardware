import csv

with open('blinks.csv') as file:
    readCSV = csv.reader(file, delimiter=',')
    for row in readCSV:
        print(row)