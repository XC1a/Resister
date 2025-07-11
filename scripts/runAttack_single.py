import os
import argparse
 
parser = argparse.ArgumentParser()
parser.add_argument('-e', '--exeM', help='current execution matchine')
parser.add_argument('-p', '--prefix', help='current execution matchine')
args = parser.parse_args()

nowExe = ""
nowExeP = ""
if args.exeM == 's':
    nowExe = "-s3"
    nowExeP = "-s"
else:
    nowExe = "-pc"
    nowExeP = ""
print(nowExe,nowExeP)

# os.system("cd ../ ; "+"./build.sh --policy ori "+nowExe)
# os.system("./runAttack.sh --routing 1 --prefix "+ args.prefix + "Ori- " + nowExeP)

# os.system("cd ../ ; " + "./build.sh --policy valiant "+nowExe)
os.system("./runAttack.sh --routing 7 --prefix "+ args.prefix + "valiant- --defend " + nowExeP)

# os.system("cd .. ; " + "./build.sh --policy TDMA "+nowExe)
# os.system("./runAttack.sh --routing 1 --prefix "+ args.prefix + "TDMA- --defend " + nowExeP)

# percent = [1.0, 0.75, 0.5, 0.4, 0.3, 0.25, 0.2, 0.15]
# for p in percent:
#     os.system("cd .. ; " + "./build.sh --policy resister "+"--percent "+str(p)+" "+nowExe)
#     os.system("./runAttack.sh --routing 6 --prefix "+ (args.prefix + str(p*100)) + "Detour- " + " --defend " + nowExeP)
