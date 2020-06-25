import altiwx
import subprocess
import os

temp_file = altiwx.filename + ".wav"
command = "sox -t raw -e floating-point -b 32 -c 2 -r " + altiwx.samplerate + " '" + altiwx.input_file + "' -t wav -e signed-integer -b 16 -c 2 -r " + altiwx.samplerate + " '" + temp_file + "'"
altiwx.debug(command)
subprocess.Popen([command], shell=1).wait()

lrpt_file = altiwx.filename + ".lrpt"
command = "meteor_demod -q -B -s " + altiwx.samplerate + " '" + temp_file + "' -o '" + lrpt_file + "'"
altiwx.debug(command)
subprocess.Popen([command], shell=1).wait()

os.remove(temp_file)

output_file = altiwx.filename + ".png"
command = "medet '" + lrpt_file + "' '" + altiwx.filename + "' -r 65 -g 65 -b 64 > /dev/null"
altiwx.debug(command)
subprocess.Popen([command], shell=1).wait()