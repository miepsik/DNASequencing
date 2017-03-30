from lxml.cssselect import CSSSelector
from lxml.html import fromstring
from lxml.etree import tostring
from urllib.request import urlopen
import os
from tqdm import tqdm

def getURLs(url):
	#otwiera stronę
	h = fromstring(urlopen(url+"testy.html").read())

	dict={}
	#pobiera komórki tabel (#oligonukleotydów, długość oligonukleotydu, #błędów)
	sel = CSSSelector('tr td')
	for e in sel(h):
		if e.sourceline not in dict:
			dict[e.sourceline]=[]
		if e.text is not None:
			dict[e.sourceline].append(int(e.text))#.decode('iso-8859-2')
			
	#pobiera url zbiorów testowych
	sel = CSSSelector('tr td a')
	for e in sel(h):
		dict[e.sourceline].append(url+e.get('href'))#.decode('iso-8859-2')

	#pobiera typy zbiorów
	# dict2={}
	# sel = CSSSelector('h4')
	# for e in sel(h):
		# if e.sourceline not in dict:
			# dict2[e.sourceline]=[]
		# if e.text is not None:
			# dict2[e.sourceline].append(e.text)#.decode('iso-8859-2')

	#dodaje typy zbiorów
	# for x in dict2:
		# for y in dict:
			# if len(dict[y])==4 and x<y:
				# dict[y].append(dict2[x][0])
				# dict[y].append(x)
			# elif len(dict[y])==6 and x>dict[y][5] and x<y:
				# dict[y][4]=dict2[x][0]
				# dict[y][5]=x
				
	#usuwa nadmiarowe dane
	# for x in dict:
		# dict[x]=dict[x][:-1]
		
	return dict;
	
def doFile(dir,nazwa,tab):
	if not os.path.exists(dir):
		os.makedirs(dir)
	f = open(dir+"/"+nazwa,'w')
	f.write(str(tab[0]) + " " + str(tab[1]) + " " + "nie pamiętam co tu powinno być\n")
	for line in urlopen(tab[3]).readlines():
		f.write(line.decode('iso-8859-2'))
	f.close()

	
	
if __name__=="__main__":
	dict = getURLs("http://www.cs.put.poznan.pl/mkasprzak/bio/");
	for i,d in enumerate(tqdm(dict)):
		doFile("datasets","plik"+str(i)+".txt",dict[d])
	
