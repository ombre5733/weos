import fnmatch
import os
import Queue
import re
import serial
from string import Template
import subprocess
import threading
import time
import uuid

rootDir = os.path.dirname(os.path.abspath(__file__))
workingDir = os.getcwd()


# ----===================================================----
#     Site config
# ----===================================================----

toolchain = "armcc"

targetUart = {'baudRate': 115200,
              'rxPin': 'PD09',
              'txPin': 'PD08'
             }

asmExecutable = "armasm"
asmFlags = ["--cpu=Cortex-M4",
            "--apcs=interwork"]

cExecutable = "armcc"
cFlags = ["--cpu=Cortex-M4",
          "--apcs=interwork",
          "-D__ASSERT_MSG",
          "-Ospace",
          "-I.",
          "-Icmsis"]

cxxExecutable = "armcc"
cxxFlags = ["--cpu=Cortex-M4",
            "--apcs=interwork",
            "--exceptions",
            "--cpp11",
            "-D__ASSERT_MSG",
            "-Ospace",
            "-I.",
            "-Icmsis"]

linkerExecutable = "armlink"
linkerFlags = ["RTX_CM4.lib"]

def downloadExecutable(executable):
    from subprocess import PIPE

    print("Download " + executable)
    binFile = executable + ".bin"
    try:
        subprocess.check_call(["fromelf",
                               "--bin",
                               "--output",
                               binFile,
                               executable],
                              stdout=PIPE, stderr=PIPE)
        subprocess.check_call(["openocd",
                               "-f",
                               "/usr/share/openocd/scripts/board/stm32f4discovery.cfg",
                               "-c",
                               "init; reset halt; wait_halt; flash write_image erase %s 0x8000000; reset run; shutdown" % (binFile)],
                              stdout=PIPE, stderr=PIPE)
    except subprocess.CalledProcessError:
        return False

    return True


#"""
toolchain = "gcc"

targetUart = {'baudRate': 115200,
              'rxPin': 'PD09',
              'txPin': 'PD08'
             }

asmExecutable = "arm-none-eabi-gcc"
asmFlags = ["-mthumb",
            "-mcpu=cortex-m4",
            "-mfloat-abi=hard",
            "-mfpu=fpv4-sp-d16",
            "-fno-builtin"]

cExecutable = "arm-none-eabi-gcc"
cFlags = ["-mthumb",
          "-mcpu=cortex-m4",
          "-mfloat-abi=hard",
          "-mfpu=fpv4-sp-d16",
          "-fno-builtin",
          "-O2",
          "-I.",
          "-Icmsis"]

cxxExecutable = "arm-none-eabi-g++"
cxxFlags = ["-mthumb",
            "-mcpu=cortex-m4",
            "-mfloat-abi=hard",
            "-mfpu=fpv4-sp-d16",
            "-fno-builtin",
            "-std=c++11",
            "-O2",
            "-I.",
            "-Icmsis"]

linkerExecutable = "arm-none-eabi-g++"
linkerFlags = ["-mthumb",
               "-mcpu=cortex-m4",
               "-mfloat-abi=hard",
               "-mfpu=fpv4-sp-d16",
               "--specs=nano.specs",
               "-lc", "-lc", "-lnosys",
               "-Wl,--gc-sections",
               "-fno-builtin",
               "-L.", "-lRTX_CM4"]

def downloadExecutable(executable):
    from subprocess import PIPE

    print("Download " + executable)
    binFile = executable + ".bin"
    try:
        subprocess.check_call(["arm-none-eabi-objcopy",
                               "-O", "binary",
                               executable,
                               binFile],
                              stdout=PIPE, stderr=PIPE)
        subprocess.check_call(["openocd",
                               "-f",
                               "/usr/share/openocd/scripts/board/stm32f4discovery.cfg",
                               "-c",
                               "init; reset halt; wait_halt; flash write_image erase %s 0x8000000; reset run; shutdown" % (binFile)],
                              stdout=PIPE, stderr=PIPE)
    except subprocess.CalledProcessError:
        return False

    return True
#"""

# ----===================================================----
#     End of site config
# ----===================================================----

class ToolChain:
    ARMCC = 1
    GCC = 2

toolchain = toolchain.lower()
if toolchain == "armcc":
    toolchain = ToolChain.ARMCC
elif toolchain == "gcc":
    toolchain = ToolChain.GCC
else:
    raise Exception("Unknown toolchain")


def rootRel(file):
    return os.path.join(rootDir, file)

def cwdRel(file):
    return os.path.join(workingDir, file)

def flatten(*args):
    result = []
    for arg in args:
        if type(arg) is list:
            result.extend(arg)
        else:
            result.append(arg)
    return result

def execute(exe, inFiles, outFile, *args):
    from subprocess import PIPE

    print("Generate " + outFile)

    outDir = os.path.split(outFile)[0]
    if not os.path.exists(outDir):
        os.makedirs(outDir)
    try:
        os.remove(outFile)
    except OSError:
        pass

    argv = flatten(exe, inFiles, "-o", outFile, *args)
    subprocess.check_call(argv, stdout=PIPE, stderr=PIPE)


class Test:
    PASS = 1
    FAIL = 2
    XPASS = 3
    XFAIL = 4
    FATAL = 5

    def __init__(self, name, expected):
        self.name = name
        self.expected = expected
        self.actual = None
        self.reason = ""

def findTests():
    tests = []
    for root, dirnames, filenames in os.walk(rootDir):
        for filename in fnmatch.filter(filenames, 'tst_*.cpp'):
            name = os.path.relpath(os.path.join(root, filename),
                                   rootDir)
            if ".pass." in name:
                expected = Test.PASS
            elif '.fail.' in name:
                expected = Test.FAIL
            elif '.xpass.' in name:
                expected = Test.XPASS
            elif '.xfail.' in name:
                expected = Test.XFAIL
            else:
                raise Exception("Expected result is invalid")
            tests.append(Test(name, expected))
    return tests

class Monitor(threading.Thread):
    UNKNOWN = 0
    BEGIN = 1
    PASS = 2
    FAIL = 3

    stringToToken = { "BEGIN" : BEGIN, "PASS" : PASS, "FAIL" : FAIL}

    def __init__(self, tokenQueue):
        super(Monitor, self).__init__()
        self.stopRequest = threading.Event()
        self.uart = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
        self.tokenQueue = tokenQueue

    def run(self):
        while not self.stopRequest.isSet():
            line = self.uart.readline()
            m = re.match(r"\^\^\^FREMTESTER:(.*):(.*)\^\^\^", line)
            if m:
                self.tokenQueue.put(
                    (m.group(1),
                     Monitor.stringToToken.get(m.group(2), Monitor.UNKNOWN)))

    def join(self, timeout=None):
        self.stopRequest.set()
        super(Monitor, self).join(timeout)


if toolchain == ToolChain.ARMCC:
    execute(asmExecutable,
            rootRel("common/armcc-startup_stm32f4xx.s"),
            cwdRel("startup_stm32f4xx.o"),
            asmFlags)
    retargetSourceFile = rootRel("common/armcc-retarget.cpp.in")
elif toolchain == ToolChain.GCC:
    execute(asmExecutable,
            rootRel("common/gcc-startup_stm32f4xx.S"),
            cwdRel("startup_stm32f4xx.o"),
            asmFlags, "-c")
    retargetSourceFile = rootRel("common/gcc-retarget.cpp.in")

execute(cExecutable,
        rootRel("common/system_stm32f4xx.c"),
        cwdRel("system_stm32f4xx.o"),
        cFlags, "-c")
execute(cExecutable,
        rootRel("common/RTX_Conf_CM.c"),
        cwdRel("RTX_Conf_CM.o"),
        cFlags, "-c")
execute(cxxExecutable,
        rootRel("../src/weos.cpp"),
        cwdRel("weos.o"),
        "-I" + rootRel("../src/"),
        cxxFlags, "-c")

fixtureObjectFiles = [cwdRel("startup_stm32f4xx.o"),
                      cwdRel("system_stm32f4xx.o"),
                      cwdRel("RTX_Conf_CM.o"),
                      cwdRel("weos.o")]


testList = findTests()

tokenQueue = Queue.Queue()
monitor = Monitor(tokenQueue)
monitor.start()

substitutions = {}
for testIdx in xrange(len(testList)):
    testFile = testList[testIdx].name
    print("*" * 75)
    print(testFile)
    print("*" * 75)

    testFileBaseName = os.path.splitext(testFile)[0]
    substitutions["TESTID"] = str(uuid.uuid4())
    with open(retargetSourceFile, 'r') as sourceFile:
        src = Template(sourceFile.read())
        result = src.substitute(substitutions)
        outFileName = cwdRel(testFileBaseName + "-retarget.cpp")
        outDir = os.path.split(outFileName)[0]
        if not os.path.exists(outDir):
            os.makedirs(outDir)
        with open(outFileName, 'w') as outFile:
            outFile.write(result)

    retargetObjectFile = cwdRel(testFileBaseName + "-retarget.o")
    execute(cxxExecutable,
            cwdRel(testFileBaseName + "-retarget.cpp"),
            retargetObjectFile,
            cxxFlags, "-c", "-I" + rootRel("common/"))

    objectFile = cwdRel(testFileBaseName + '.o')
    exeFile = cwdRel(testFileBaseName + '.axf')
    try:
        execute(cxxExecutable,
                rootRel(testFile),
                objectFile,
                cxxFlags, "-c")
        if toolchain == ToolChain.ARMCC:
            execute(linkerExecutable,
                    objectFile,
                    exeFile,
                    fixtureObjectFiles,
                    retargetObjectFile,
                    linkerFlags,
                    "--scatter", rootRel("common/armcc-linker.sct"))
        elif toolchain == ToolChain.GCC:
            execute(linkerExecutable,
                    objectFile,
                    exeFile,
                    fixtureObjectFiles,
                    retargetObjectFile,
                    linkerFlags,
                    "-T" + rootRel("common/gcc-stm32f4xx.ld"))
    except subprocess.CalledProcessError:
        testList[testIdx].actual = Test.FAIL
        continue

    try:
        if not downloadExecutable(exeFile):
            testList[testIdx].actual = Test.FATAL
            testList[testIdx].reason = "download failed"
            continue
    except:
        testList[testIdx].actual = Test.FATAL
        testList[testIdx].reason = "download failed"
        continue

    startTime = time.time()
    started = False
    while time.time() - startTime < 3:
        try:
            token = tokenQueue.get(True, 0.1)
            if token[0] == substitutions["TESTID"] and token[1] == Monitor.BEGIN:
                started = True
                break
        except Queue.Empty:
            continue
    if not started:
        testList[testIdx].actual = Test.FATAL
        testList[testIdx].reason = "test not started"
        continue

    startTime = time.time()
    TestTimeout = 30
    while time.time() - startTime < TestTimeout:
        try:
            token = tokenQueue.get(True, 0.1)
            if token[0] != substitutions["TESTID"]:
                continue
            if token[1] == Monitor.PASS:
                testList[testIdx].actual = Test.PASS
            elif token[1] == Monitor.FAIL:
                testList[testIdx].actual = Test.FAIL
            else:
                testList[testIdx].actual = Test.FATAL
                testList[testIdx].reason = "unknown test token"

            if testList[testIdx].actual is not None:
                break
        except Queue.Empty:
            continue

    if testList[testIdx].actual is None:
        testList[testIdx].actual = Test.FATAL
        testList[testIdx].reason = "timeout"


monitor.join()


print("")
print("*" * 75)
print("Summary")
print("*" * 75)

resultString = { Test.PASS : "PASS", Test.FAIL : "FAIL", Test.FATAL : "FATAL"}

nameLength = max([len(test.name) for test in testList])
formatStr = "{:%d}" % (nameLength + 3)
goodStr =  formatStr + "[\033[92m{:5}\033[0m]"
warnStr =  formatStr + "[\033[93m{:5}\033[0m]"
errorStr = formatStr + "[\033[91m{:5}\033[0m] !"
fatalStr = formatStr + "[\033[91m{:5}\033[0m] <{}>"

for test in testList:
    if test.actual == Test.FATAL:
        print(fatalStr.format(test.name, resultString[test.actual],
                              test.reason))
        continue

    if test.actual == test.expected:
        fmt = goodStr
    elif ((test.expected == Test.XPASS and test.actual == Test.FAIL) or
          (test.expected == Test.XFAIL and test.actual == Test.PASS)):
        fmt = warnStr
    else:
        fmt = errorStr
    print(fmt.format(test.name, resultString[test.actual]))

