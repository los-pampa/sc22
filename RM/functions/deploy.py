import os

from functions.mapping import *


def rmGenerateStringDocker(dockerRun, application, machine, mask):
    maskCPU = "--name "+application.name+" --cpuset-cpus "+mask+" "
    command = dockerRun.command + maskCPU + dockerRun.image + application.path
    return command

def rmDeployContainer(dockerRun, listApps, machine, threadMapping):
    if not listApps:
        print("RM: No Application in the Queue to be Executed")
    else:
        if(rmIsCoreAvailable(threadMapping) == True):
            application = listApps.pop()
            mask = rmDefineInitialMask(threadMapping, application);
            command = rmGenerateStringDocker(dockerRun, application, machine, mask)
            print(application.name)
            os.system(command)
        else:
            print("core not available")
