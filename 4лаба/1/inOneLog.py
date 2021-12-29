readerLog = open("reader.log", "r")
writerLog = open("writer.log", "r")

actionList = []

for line in readerLog:
	time, logAction = line.split(" | ")
	actionList.append((time, logAction))
	
for line in writerLog:
	time, logAction = line.split(" | ")
	actionList.append((time, logAction))
	
readerLog.close()
writerLog.close()

actionList.sort()

outLog = open("actions.log", "w")
for action in actionList:
	outLog.write(action[0] + " | " + action[1])
outLog.close()