import sys
import time
import urllib2
import re
import os
import webcolors
rss_url = "http://www.mailinator.com/rss.jsp?email=" + sys.argv[1]
current = ['0','0','0']

while 1:
	feed = urllib2.urlopen(rss_url).read()
	matches = re.findall('<title>([^>]*)<\/title>', feed)
	if len(matches)>1:
		newest_subject = matches[len(matches)-1].strip()
		newest_subject = newest_subject.split(" ",3)
		if len(newest_subject) != 3:
			newest_subject = matches[len(matches)-1].strip()
			try:
				newest_subject = webcolors.name_to_rgb(newest_subject.lower())
				newest_subject = list(newest_subject)
			except ValueError:
				newest_subject = current

		if current != newest_subject:
			print newest_subject
			os.system('DigiRGB ' + str(newest_subject[0]) + ' ' + str(newest_subject[1]) + ' ' + str(newest_subject[2])) 

		current = newest_subject
	time.sleep(60)