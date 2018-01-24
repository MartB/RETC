# Installer builder script for RETC
import sys
import os
import distutils.dir_util
import shutil

CUE_SDK_DL_URL = "http://downloads.corsair.com/download?item=Files/CUE/CUESDK_2.18.127.zip"
CUE_SDK_DLL_NAME_32 = "CUESDK_2015.dll"
CUE_SDK_DLL_NAME_64 = "CUESDK.x64_2015.dll"

# NSSM Binary
NSSM_BIN_NAME = "nssm.exe"

SERVER_BUILD_PATH_32 = '../out/x86/Release/Server/'
SERVER_BUILD_PATH_64 = '../out/x64/Release/Server/'

CLIENT_BUILD_PATH_32 = '../out/x86/Release/ShimDLL/'
CLIENT_BUILD_PATH_64 = '../out/x64/Release/ShimDLL/'

THIRD_PARTY_INC_PATH = '../3rdparty/inc/'

FILE_PATH = 'files/'
FILE_PATH_32 = FILE_PATH + 'win32/'
FILE_PATH_64 = FILE_PATH + 'win64/'
TMP_PATH = FILE_PATH + 'tmp/'

def checkFiles(list):
	for file in list:
		if not os.path.isfile(file):
			print("FileCheck: could not find",file)
			return False
			
	return True

def checkFileAlternatives(list):
	for file in list:
		if os.path.isfile(file):
			return file
			
	return ""
	
	
def findInList(list, string):
	return [s for s in list if string.lower() in s.lower()]
	
class zipopfailed(Exception): pass
def downloadAndExtractZipFile(url, ziplist):
	import urllib.request, urllib.error, urllib.parse
	import zipfile
	try:
		try:
			response = urllib.request.urlopen(url)
		except urllib.error.HTTPError as e:
			print("Download: http error",e.reason," url(",url,")")
			return False
		except urllib.error.URLError as e:
			print("Download: url error",e.reason," url(",url,")")
			return False
			
		data = response.read()
		fileName = url.split('/')[-1]
		print("Downloading: ",fileName)
		distutils.dir_util.mkpath(TMP_PATH)
		with open(TMP_PATH+fileName , 'wb') as file:
			file.write(data)
			
		with zipfile.ZipFile(TMP_PATH + fileName, 'r') as zip:
			fileList = zip.namelist()
			
			for entry in ziplist:
				print("Extracting: ", entry[0])
				zipFileEntry = findInList(fileList, entry[0])
				if not zipFileEntry:
					print("Did not find",entry[0],"in zip")
					raise zipopfailed()
					
				with open(entry[1], 'wb') as file: 
					try:
						file.write(zip.read(zipFileEntry[0]))
					except IOError as e:
						print(e)
						raise zipopfailed()
	except zipopfailed:
		os.remove(TMP_PATH+fileName)
		return False
		
	os.remove(TMP_PATH+fileName)
	return True
	
	
def downloadCueSdk():
	return downloadAndExtractZipFile(CUE_SDK_DL_URL, [
		(CUE_SDK_DLL_NAME_32, FILE_PATH_32 + CUE_SDK_DLL_NAME_32),
		(CUE_SDK_DLL_NAME_64, FILE_PATH_64 + CUE_SDK_DLL_NAME_64)
	])


def copyBuildFiles():
	buildFiles = [
		("../",FILE_PATH,"LICENSE"),
		("../",FILE_PATH,"README.md"),
		(SERVER_BUILD_PATH_32, FILE_PATH_32, "retc-rpc-server-32.exe"),
		(SERVER_BUILD_PATH_64, FILE_PATH_64, "retc-rpc-server-64.exe"),
		(CLIENT_BUILD_PATH_32, FILE_PATH_32, "RzChromaSDK.dll"),
		(CLIENT_BUILD_PATH_64, FILE_PATH_64, "RzChromaSDK64.dll"),
	]
	
	copyFiles(buildFiles)

def copyLicenses():
		licenses = [
			(THIRD_PARTY_INC_PATH + "spdlog/LICENSE",FILE_PATH + "SPDLOG_LICENSE"),
			(THIRD_PARTY_INC_PATH + "SimpleIni/LICENCE.txt",FILE_PATH + "SIMPLEINI_LICENSE"),
		]

		copyFiles(licenses)


def copyFiles(buildFiles):
		 for file in buildFiles:
				if len(file) > 2:
					shutil.copy2(file[0]+file[2], file[1]+file[2])
				else:
					shutil.copy2(file[0], file[1])

def cleanBuildFiles():
	print("Cleaning build files")
	try:
		shutil.rmtree(TMP_PATH)
	except OSError:
		pass
	else:
		print("Deleting", TMP_PATH)
		
	for root, dirs, files in os.walk(FILE_PATH):
		for file in files:
			if file.endswith((".dll",".exe","LICENSE", ".md")):
				filePath = os.path.join(root,file)
				print("Deleting", filePath)
				os.remove(filePath)
				
def main(argv):
	if findInList(argv, "clean"):
		cleanBuildFiles()
		return
		
	nssmExists = checkFiles([
		FILE_PATH_32 + NSSM_BIN_NAME,
		FILE_PATH_64 + NSSM_BIN_NAME
	])
	
	cueSDKExists = checkFiles([
		FILE_PATH_32 + CUE_SDK_DLL_NAME_32,
		FILE_PATH_64 + CUE_SDK_DLL_NAME_64
	])
	
	if not nssmExists:
		print("Could not find nssm (please compile it yourself)")
		return
		
	if not cueSDKExists and not downloadCueSdk():
		print("Could not download cue sdk")
		return
	
	copyBuildFiles()
	copyLicenses()
	
	nsisExecutable = checkFileAlternatives([
		"makensis.exe",
		os.environ["ProgramFiles"] + "\\NSIS\\makensis.exe",
		os.environ["ProgramFiles(x86)"] + "\\NSIS\\makensis.exe"
	])
	
	if nsisExecutable == "":
		print("NSIS Not found please install it from here: https://sourceforge.net/projects/nsis/")
		print("You also need the moreinfo plugin from here: http://nsis.sourceforge.net/MoreInfo_plug-in")
		return

	from subprocess import call
	call([nsisExecutable, "RETC_Installer.nsi"])
		
if __name__ == "__main__":
	main(sys.argv)
