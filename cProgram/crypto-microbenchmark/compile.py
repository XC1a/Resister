import os
 
file_names = [f for f in os.listdir('.') if os.path.isfile(f)]

for p in file_names:
    if p[-2:] == 'cc':
        os.system("g++ "+ p + " -O2 -o " + p[:-3])
        os.system("mv "+ p[:-3] +" build/")
        print(p+" compile done!")