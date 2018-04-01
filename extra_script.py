Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}
# print defines

import subprocess
gitrevcount = subprocess.Popen(['git', 'rev-list', '--all', '--count', 'HEAD'], stdout=subprocess.PIPE).stdout.readline()
print ('git revision: '+gitrevcount)
env.Replace(VERSION="rev%s" % gitrevcount)
# env.Replace(PROGNAME="firmware_%s" % defines.get("VERSION"))
