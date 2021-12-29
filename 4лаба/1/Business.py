import sys

if len(sys.argv) < 2:
	raise Exception("Укажите число процессов (только читателей/писателей)")

iProcesses = int(sys.argv[1])

actionLog = open("actions.log", "r")
businessLog = open("business.m", "w", encoding="utf-8")

readerStart = int(iProcesses)*2 + 10 #220

minTime = -1
lastTime = 0
#last time
readers = [-1 for i in range(iProcesses+1)]
writers = [-1 for i in range(iProcesses+1)]

businessLog.write("figure;\nhold on;\n")
for line in actionLog:
	if "ready" not in line and "going down" not in line:
		time = int(line[:line.find(" ")])
		if minTime == -1:
			minTime = time
		time = time - minTime
		who = 0	#0 - writer, 1 - reader
		if("writer:" not in line):
			who = 1
		
		number = 0
		if who == 0:
			number = int(line[line.find("|")+2:line.find(" w")])
		else:
			number = int(line[line.find("|")+2:line.find(" r")])
		state = 0	#free
		if("waiting for" in line):	#wait
			state = 1
		elif("release reader's semaphore" in line or "read the page" in line or "going down" in line):	#busy
			state = 2
		
		if who == 0 and writers[number] == -1:
			writers[number] = time;
		elif who == 1 and readers[number] == -1:
			readers[number] = time;
		else:
		
			color = []
			if state == 0:
				color.append(0)
				color.append(1)
				color.append(0)
			elif state == 1:
				color.append(0)
				color.append(1)
				color.append(0)
			elif state == 2:
				color.append(1)
				color.append(0)
				color.append(0)
			if who == 0: #writer
				businessLog.write(f"rectangle('Position', [{writers[number]},{number*2}, {time-writers[number]}, 1], 'FaceColor', {color}, 'LineWidth', 1);\n")
				writers[number] = time
			else:
				businessLog.write(f"rectangle('Position', [{readers[number]},{readerStart+number*2}, {time-readers[number]}, 1], 'FaceColor', {color}, 'LineWidth', 1);\n")
				readers[number] = time

#rectangle('Position', [1,1,2,1], 'FaceColor', [1, 0, 0], 'EdgeColor', [1, 0, 0], 'LineWidth', 1);
businessLog.write("xlabel(\"Время, мс\");\nhold off;\n")
actionLog.close()
businessLog.close()