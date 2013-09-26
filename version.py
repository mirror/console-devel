import os
import shutil
import re
import zipfile

def buildDocumentation(version):
    os.system(r"help\build.bat")


def buildExecutables(version):
    os.system(r'devenv %s /build "%s|%s"' % ("Console2.sln", "Release", "Win32"))
    os.system(r'devenv %s /build "%s|%s"' % ("Console2.sln", "Release", "x64"))

    os.system(r'msbuild Console2.sln /p:Configuration=Release /p:Platform=Win32 /t:ConsoleWow')

    os.system(r'msbuild Console2.sln /p:Configuration=Release /p:Platform=Win32 /t:ConsoleHook')
    os.system(r'msbuild Console2.sln /p:Configuration=Release /p:Platform=x64 /t:ConsoleHook')

    os.system(r'msbuild Console2.sln /p:Configuration=Release /p:Platform=Win32 /t:ExplorerIntegration')
    os.system(r'msbuild Console2.sln /p:Configuration=Release /p:Platform=x64 /t:ExplorerIntegration')


def archive(path, files):
    archive = zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED)

    for file in files:
        folder, archive_file = os.path.split(file)
        archive_file = archive_file.decode("ISO8859-1")
        archive_file = archive_file.encode("CP437")
        try:
            # print  "Add the archive '%s' to the archive file '%s'" % (str(file), str(path))
            archive.write(file, archive_file)
        except Exception as e:
            print "Failed to archive the file '%s'" % str(file)
            print e


    archive.close()


def archiveFolder(path, folder_path):
    archive = zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED)

    folder_path = os.path.normpath(folder_path)
    for root, folders, files in os.walk(folder_path):
        for file_path in files:
            file_path = os.path.join(root, file_path)

            archive_file = file_path
            if folder_path:
                archive_file = archive_file[len(folder_path) + 1:]

            archive_file = archive_file.decode("ISO8859-1")
            archive_file = archive_file.encode("CP437")

            try:
                print  "Add the archive '%s' to the archive file '%s'" % (str(file_path), str(archive_file))
                archive.write(file_path, archive_file)
            except Exception as e:
                print "Failed to archive the file '%s'" % str(file)
                print e

    archive.close()


def prepareArchiveReleases(version):
    shutil.copyfile(r"setup\config\console.xml", r"bin\Win32\Release\console.xml")
    shutil.copyfile(r"setup\config\console.xml", r"bin\x64\Release\console.xml")

    shutil.copyfile(r"setup\dlls\FreeImage.dll", r"bin\Win32\Release\FreeImage.dll")
    shutil.copyfile(r"setup\dlls\FreeImagePlus.dll", r"bin\Win32\Release\FreeImagePlus.dll")

    shutil.copyfile(r"setup\dlls\x64\FreeImage.dll", r"bin\x64\Release\FreeImage.dll")
    shutil.copyfile(r"setup\dlls\x64\FreeImagePlus.dll", r"bin\x64\Release\FreeImagePlus.dll")

    files = [
        r"Console.chm",
        r"Console.exe",
        r"Console.xml",
        r"ConsoleHook.dll",
        r"ExplorerIntegration.dll",
        r"FreeImage.dll",
        r"FreeImagePlus.dll",
    ]

    archive_path = r"setup\Console %s win32.zip" % version
    archive_files = [os.path.join(r"bin\Win32\Release", x) for x in files]

    archive(archive_path, archive_files)

    files += [
        r"ConsoleWow.exe",
        r"ConsoleHook32.dll",
    ]
    archive_path = r"setup\Console %s x64.zip" % version
    archive_files = [os.path.join(r"bin\x64\Release", x) for x in files]

    archive(archive_path, archive_files)

    pass


def prepareSourceRelease(version):
    if os.path.isdir(r"..\deploy\console-devel"):
        shutil.rmtree(r"..\deploy\console-devel")
    os.system(r"hg archive ..\deploy\console-devel")
    archiveFolder(r"setup\Console %s src.zip" % version, r"..\deploy\console-devel")


def updateInnoSetupConfigFile(path, version):
    shutil.copyfile(path, "temp.iss")
    f = open("temp.iss")
    g = open(path, "w")
    for l in f:
        m = re.match("OutputBaseFilename=Console \S+ (\S+)", l)
        if m:
            l = "OutputBaseFilename=Console " + version + " " + m.groups()[0] + "\n"

        m = re.match("AppVersion=\S+", l)
        if m:
            l = "AppVersion=" + version + "\n"

        g.write(l)


    f.close()
    g.close()
    os.remove("temp.iss")



def prepareSetupReleases(version):
    updateInnoSetupConfigFile(r"setup\console-x64.iss", version)
    updateInnoSetupConfigFile(r"setup\console.iss", version)
    os.system(r"setup\build.bat")


def tagVersion(version):
    tag = version
    tag = tag.replace(".", "")

    os.system("hg tag %s" % tag)

    os.system('hg commit -m "tag %s"' % tag)


def main():
    version = "2.00b148d"

    buildDocumentation(version)

    buildExecutables(version)

    prepareArchiveReleases(version)

    prepareSourceRelease(version)

    prepareSetupReleases(version)

    #tagVersion(version)


if __name__ == "__main__":
    main()