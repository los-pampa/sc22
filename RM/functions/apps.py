from classes.rm_classes import rmApps


def rmPrintApps(self):
    print("RM: List of Applications to be executed")
    for index in self:
        print("\t"+index.name+":"+index.path+" Priority: "+str(index.priority))

def rmInitializeApps(listApps):
    myFile = open("./input/bufferApps.txt", "r") # for reproducibility, change here to the input list that contains the applications
    Lines = myFile.readlines()
    count = 0
    for line in Lines:
        list = line.rsplit(", ")
        listObject = rmApps(" "+list[0], list[1], list[2])
        listApps.append(listObject)
        count = count + 1
    myFile.close()

    myFile = open("./input/bufferApps.txt", "w") # for reproducibility, change here to the input list that contains the applications
    myFile.close()
    if(count > 0):
        listApps.sort(key=lambda listApps: listApps.priority)



