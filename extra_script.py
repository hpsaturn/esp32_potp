Import("env")
import subprocess
import json

flags = env.ParseFlags(env['BUILD_FLAGS'])
src_flags = env.ParseFlags(env['SRC_BUILD_FLAGS'])
defines = {k: v for (k, v) in flags.get("CPPDEFINES")}
print ('==================================')
gitrevcount = subprocess.Popen(['git', 'rev-list', '--all', '--count', 'HEAD'], stdout=subprocess.PIPE).stdout.readline()
print ('==> GIT REVISION: ' + gitrevcount)
print ('==> CPPDEFINES: ' + json.dumps(defines))
# print ('==> WIFI_SSID: ' + src_flags.get("WIFI_SSID"))
print ('=================================')
env.Replace(SRC_REV="rev%s" % gitrevcount)
# env.Replace(PROGNAME="firmware_%s" % defines.get("VERSION"))
