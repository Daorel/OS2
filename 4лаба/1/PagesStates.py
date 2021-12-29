pageLog = open("pages.log", "r")
parsedPageLog = open("octavePagesBusiness.m", "w", encoding="utf-8")

pagesStatus = [0 for i in range(20)]
timeOctave = [0]

pagesRead = []
pagesWrite = []
pagesFree = [i for i in range(20)]

pagesReadOctave = [0]
pagesWriteOctave = [0]
pagesFreeOctave = [20]

minTime = -1

for line in pageLog:
	time, page, status = int(line[:line.find(" ")]), int(line[line.find("|")+1:line.find(":")]), line[line.find(":")+2:]
	if(minTime == -1):
		minTime = time-10
	time = time - minTime
	#parsedPageLog.write(f"Time: {time}, page: {page}, status: {status}")
	if("writing in" in status):
		if page in pagesRead:
			pagesRead.remove(page)
		elif page in pagesFree:
			pagesFree.remove(page)
		pagesWrite.append(page)
	elif("is being read" in status):
		if page in pagesWrite:
			pagesWrite.remove(page)
		elif page in pagesFree:
			pagesFree.remove(page)
		pagesRead.append(page)
	else:
		if page in pagesWrite:
			pagesWrite.remove(page)
		elif page in pagesRead:
			pagesRead.remove(page)
		if page not in pagesFree:
			pagesFree.append(page)
	
	timeOctave.append(time)
	pagesReadOctave.append(len(pagesRead))
	pagesWriteOctave.append(len(pagesWrite))
	pagesFreeOctave.append(len(pagesFree))

parsedPageLog.write(f"time = {timeOctave};\nread = {pagesReadOctave};\nwrite = {pagesWriteOctave};\nfree = {pagesFreeOctave};\n")
parsedPageLog.write("figure;\nhold on;\nplot(time, read, \"r\");\nplot(time, write, \"b\");\nplot(time, free, \"g\");\nxlabel(\"Время, мс\")\nhold off;")
pageLog.close()
parsedPageLog.close()