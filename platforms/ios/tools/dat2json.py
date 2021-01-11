import sys

arguments = len(sys.argv)

if len(sys.argv) != 2:
    sys.exit("Need exactly one argument: path to file.")

file = sys.argv[1]

lines = []
with open(file) as f:
    lines = f.readlines()
    f.close() 

f = open(file + ".json", "w")
f.write("[\n")


parsing = False
for line in lines:
    l = line.strip()

    if l == "game (":
        parsing = True
        f.write("\t{\n")

    if l == ")":
        parsing = False
        f.write("\t},\n")

    if parsing:
        if l.startswith("name "):
            title = l[6:-1]
            f.write("\t\t\"title\": \"" + title + "\",\n")
        if l.startswith("rom ( "):
            crc_index = l.find("crc")
            crc = l[crc_index+4:crc_index+12]
            f.write("\t\t\"crc\": \"" + crc + "\"\n")

f.write("]\n")
f.close() 
