import os
import commands
import sys

for i in range(20):
    cmd = "mkdir -p batch_nfv_exam/nfv_exam_%d" % i
    status, output = commands.getstatusoutput(cmd)

    cmd = "cp -r ./bin ./batch_nfv_exam/nfv_exam_%d" % i
    status, output = commands.getstatusoutput(cmd)

    cmd = "cp -r ./data ./batch_nfv_exam/nfv_exam_%d" % i
    status, output = commands.getstatusoutput(cmd)

    #cmd = "cd batch_nfv_exam/nfv_exam_%d" % i
    #status, output = commands.getstatusoutput(cmd)

    cmd1 = "touch batch_nfv_exam/nfv_exam_%d/run_%d.sh" % (i, i)
    cmd2 = "echo \"nohup ./bin/nfv_exam %d &\" >> batch_nfv_exam/nfv_exam_%d/run_%d.sh" % (i, i, i)
    print(cmd1)
    print(cmd2)
    status, output = commands.getstatusoutput(cmd1)
    status, output = commands.getstatusoutput(cmd2)

    #cmd = "cd ../.."
    #status, output = commands.getstatusoutput(cmd)

