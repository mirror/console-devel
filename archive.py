import glob
import os
import sys
import fnmatch
import pprint
import zipfile
import tarfile
import getopt
import time
import re
import stat
import StringIO


GENERAL_PATTERNS = [
    ".archive",
    "restore.py",

    # UltraEdit:
    "*.prj",
    "-*.pui",

    # Python:
    "-*.pyc",
    "-*.pyo",

    # Cvs:
    ".cvsignore",
    "-.#*",
    "-CVS",

    # Exe
    "!archive-as *.tab *.bat",
    "!archive-as *.dmc *.cmd",
    "!archive-as *.xex *.exe",
    "!archive-as *.lld *.dll",

    # Crap:
    "-*.old",
    "-*.bak",
    "-*.zip",
    "-*.rar",

    "-Copy*",
    "-Copia*",
    "-Copie*",
]

CPP_PATTERNS = [
    "*.asm",
    "*.c",
    "*.h",
    "*.cpp",
    "*.hpp",
    "*.cxx",
    "*.hxx",
    "*.g",
    "*.py",
    "*.rc",
    "*.rc2",
    "*.hm",
    "*.bmp",
    "*.ico",
    "*.cur",
    "*.avi",
    "*.mak",
    "*.dsp",
    "*.vcproj",
    "*.dsw",
    "*.sln",

    "res",

    "-*.ncb",
    "-*.aps",
    "-*.opt",
    "-*.suo",

    "-Release*",
    "-Debug*",
]

class File:
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.path = os.path.join(parent, name)
        self.archive_path = self.path

        self.isFile = os.path.isfile(self.path)
        self.isDir = os.path.isdir(self.path)

        if self.isDir:
            self.files = []
        self.status = '?'


class NameModifier:
    def __init__(self, name_modifier):
        self.extention = ""
        if name_modifier.startswith('*'):
            self.extention = name_modifier[1:]
        else:
            print "invalid name modifier: %s" % name_modifier

    def getModifiedName(self, name):
        name, ext = os.path.splitext(name)
        return name + self.extention

class FilePattern:
    indexes = [
        "RenamePattern",
        "FilterPattern",

    ]
    def __cmp__(self, rhs):
        lhs_index = self.indexes.index(self.__class__.__name__)
        rhs_index = self.indexes.index(rhs.__class__.__name__)

        return cmp(lhs_index, rhs_index)


class FilterPattern(FilePattern):
    def __init__(self, pattern, status = '+', recursive = False):
        self.pattern = pattern
        self.status = status
        #self.name_modifier = None
        self.recursive = recursive

    def __repr__(self):
        return repr((self.pattern, self.status))

    def apply(self, file):
        # Set the file.status to determine if the file will be archived or not.
        # Set the file.archive_path to decide under which name the file will achived.
        # For filter pattern the return value determine if the next filter pattern have to be applied (False to continue applying the next filter patterns, True to stop).

        if not fnmatch.fnmatch(file.name, self.pattern):
            return False

        file.status = self.status
        return True

    def getType(self):
        # Filter patterns are applied until one "match" return True to stop the next one to be applied
        # Rule patterns are all apllied (after the filter patterns but I'm not sure it is the perfect solution).
        return "filter"

    def isRecursive(self):
        # Recursive pattern I kept from parent to child.
        return self.recursive


class RenamePattern(FilePattern):
    def __init__(self, pattern, name_modifier, recursive = False):
        self.pattern = pattern
        self.name_modifier = name_modifier
        self.recursive = recursive

    def apply(self, file):
        if not fnmatch.fnmatch(file.name, self.pattern):
            return False

        modified_name = self.name_modifier.getModifiedName(file.name)
        file.archive_path = os.path.join(file.parent, modified_name)
        return True

    def getType(self):
        return "rule"

    def isRecursive(self):
        return self.recursive


def readPatterns(path):
    patterns = []
    f = open(path)
    for l in f:
        l = l.rstrip()

        if l.startswith("!archive-as"):
            v = l.split(" ", 2) # Three parts maximum (!archive-as, name_modifier, name)
            if len(v) < 3: # Three parts minimum :-)
                print "invalid pattern: %s" % l
                continue

            name_modifier, name = v[1:]
            patterns += [
                    FilterPattern(name, "+"),
                    RenamePattern(name, NameModifier(name_modifier), False),
                ]
            continue

        if l.startswith("!include"):
            recursive = False
            if l.startswith("!include-recursively"):
                recursive = True

            v = l.split(" ", 1)
            if len(v) < 2:
                print "invalid pattern: %s" % l
                continue

            name = v[1]
            pattern = FilterPattern(name, "+", recursive)
            patterns.append(pattern)
            continue

        if l.startswith("!exclude"):
            recursive = False
            if l.startswith("!exclude-recursively"):
                recursive = True

            v = l.split(" ", 1)
            if len(v) < 2:
                print "invalid pattern: %s" % l
                continue

            name = v[1]
            pattern = FilterPattern(name, "-", recursive)
            patterns.append(pattern)
            continue

        if l.startswith("!rename"):
            recursive = False
            if l.startswith("!rename-recursively"):
                recursive = True

            v = l.split(" ", 2)
            if len(v) < 3:
                print "invalid pattern: %s" % l
                continue

            name, name_modifier = v[1:]
            pattern = RenamePattern(name, NameModifier(name_modifier), recursive)
            patterns.append(pattern)
            continue

        status = "+"
        if l and l[0] in ['+', '-']:
            if l[0] == '-':
                status = '-'
            l = l[1:]
        patterns.append(FilterPattern(l, status))
    f.close()

    return patterns


def buildFileList(root, parent_patterns, cookie):
    patterns = list(cookie.patterns) # Global patterns last.

    # Local patterns:
    path = os.path.join(root, cookie.bot_file)
    if os.path.isfile(path):
        patterns += readPatterns(path)

    # Cvs patterns:
    if cookie.read_cvs_patterns:
        patterns.append(FilterPattern("CVS", '-'))
        patterns.append(FilterPattern(".#*", '-'))
        path = os.path.join(root, ".cvsignore")
        if os.path.isfile(path):
            f = open(path)
            for l in f:
                l = l.rstrip()
                l = l.replace("\\", "")
                patterns.append(FilterPattern(l, '-'))
            f.close()

    patterns += parent_patterns # Parent patterns next.

    patterns.sort()

    names = os.listdir(root)

    filters = [pattern for pattern in patterns if pattern.getType() == "filter"]
    rules = [pattern for pattern in patterns if pattern.getType() == "rule"]

    dirs = []
    files = []
    for name in names:
        file = File(root, name)

        # All the filters run on each file until one match (return True).
        for pattern in filters:
            if pattern.apply(file):
                break

        # All active rules run on each file.
        for pattern in rules:
            pattern.apply(file)

        if file.isFile:
            if cookie.datetime_limit >= 0 and file.name != "restore.py":
                stat_info = os.stat(file.path)
                m_time = stat_info[stat.ST_MTIME]

                if m_time >= cookie.datetime_limit:
                    files.append(file)

            else:
                files.append(file)

        elif file.isDir:
            parent_patterns = [pattern for pattern in patterns if pattern.isRecursive()]
            if file.status != '-':
                file.files = buildFileList(file.path, parent_patterns, cookie)

            dirs.append(file)

    return dirs + files


def getArchivePath(root, cookie):

    archive_extention = ".zip"
    if cookie.compression == "bz2":
        archive_extention = ".tar.bz2"

    # Build a default name for the archive:
    if cookie.name:
        path = cookie.name
        name, ext = os.path.splitext(path)
        if not ext:
            path += archive_extention
    else:
        name = os.path.split(os.path.normpath(os.path.join(os.getcwd(), root)))[1]
        if not name:
            name = "archive"
        path = "%s" % name + archive_extention

        if os.path.isfile(path):
            path = "%s-%s" % (name, time.strftime("%y%m%d")) + archive_extention

        if os.path.isfile(path):
            path = "%s-%s" % (name, time.strftime("%y%m%d-%Hh%M-%S")) + archive_extention

    return path


def getFileOfStatus(files, status):
    ret = []

    for file in files:
        if file.isFile:
            if file.status != status:
                continue

            ret.append(file)
        else:
            if file.status == status:
                ret.append(file)

            if file.status == '-':
                continue

            ret += getFileOfStatus(file.files, status)

    return ret


def ask(prompt, cookie):
    if cookie.quiet:
        return ""

    return raw_input(prompt)


def updateFilesStatus(files, cookie):
    if not files:
        return True

    bot_path = files[0].path
    bot_path, name = os.path.split(bot_path)
    bot_path = os.path.join(bot_path, cookie.bot_file)

    for file in files:
        if file.status == '?':

            print file.path
            while True:
                if not cookie.unkown_file_answer:
                    answer = ask("pass[p], add[a], remove[r], quit[q] [%s]:" % cookie.last_unkown_file_answer, cookie)
                else:
                    answer = cookie.unkown_file_answer

                if not answer:
                    answer = cookie.last_unkown_file_answer
                else:
                    cookie.last_unkown_file_answer = answer

                if answer == 'p':
                    break

                if answer == 'q':
                    return False

                if answer == 'd':
                    file.status = 'd'
                    os.remove(file.path)
                    break

                path, name = os.path.split(file.path)
                if answer == 'r':
                    file.status = '-'
                    g = open(bot_path, "a+")
                    g.write("-" + name + "\n")
                    g.close()
                    break

                if answer == 'a':
                    file.status = '+'
                    g = open(bot_path, "a+")
                    g.write("+" + name + "\n")
                    g.close()
                    break

        if not file.isFile and file.status == '+':
            ret = updateFilesStatus(file.files, cookie)
            if not ret:
                return False

    return True


def printHelp():
    print """archive.py [--help] [--version] (--quiet | [--archive <archive_name>] (--exclude <exclude_patern>)* ([--archive-as <extention_pattern>,<file_pattern>])* [--bot-name <bot_name>] [--last-changes dateorduration]
e.g.:
    archive.py

    archive.py --archive test.zip --bot-name .archive
        Archive the child file in a test.zip file using the .archive files to determine what to archive and what not to archive.

    archive.py --last-changes 7
        Archive the changes of the last 7 days.
"""


def printVersion():
    print "1.1"


class Cookie:
    def __init__(self):
        # By default archive.py archive all the files
        self.quiet = False

        self.read_cvs_patterns = False

        self.name = ""
        self.bot_file = ".archive"

        self.default_status = '+'
        self.patterns = []

        self.unkown_file_answer = ""
        self.last_unkown_file_answer = "p"

        self.datetime_limit = -1

        self.compression = "zip"


def parseDuration(v):
    m = re.match(r'(\d+)sec$', v)
    if m:
        return int(m.groups()[0])

    m = re.match(r'(\d+)min$', v)
    if m:
        return int(m.groups()[0]) * 60

    m = re.match(r'(\d+)h$', v)
    if m:
        return int(m.groups()[0]) * 3600

    m = re.match(r'(\d+) +(\d+):(\d+):(\d+)$', v)
    if m:
        return int(m.groups()[0]) * 24 * 3600 + int(m.groups()[1]) * 3600 + int(m.groups()[2]) * 60 + int(m.groups()[3])

    m = re.match(r'(\d+):(\d+):(\d+)$', v)
    if m:
        return int(m.groups()[0]) * 3600 + int(m.groups()[1]) * 60 + int(m.groups()[2])

    m = re.match(r'(\d+):(\d+)$', v)
    if m:
        return int(m.groups()[0]) * 60 + int(m.groups()[1])

    m = re.match(r'(\d+)$', v)
    if m:
        return int(m.groups()[0]) * 24 * 3600

    return -1


def parseDate(v):
    m = re.match(r'(\d+)/(\d+)/(\d+) (\d+):(\d+):(\d+)$', v)
    if m:
        t = time.strptime(v, "%Y/%m/%d %H:%M:%S")
        t = time.mktime(t)
        return t

    m = re.match(r'(\d+)/(\d+)/(\d+)$', v)
    if m:
        t = time.strptime(v, "%Y/%m/%d")
        t = time.mktime(t)
        return t

    return -1


def parseArguments(args):
    cookie = Cookie()

    opts, args = getopt.getopt(args, "hvq", ["help", "version", "quiet", "read-cvs-patterns", "archive=", "exclude=", "archive-as=", "bot-name=", "last-changes=", "compression="])
    for o, v in opts:
        if o in ["-h", "--help"]:
            printHelp()
            return ".", None

        if o in ["-v", "--version"]:
            printVersion()
            return  ".", None

        if o in ["-q", "--quiet"]:
            cookie.quiet = True

        if o in ["--read-cvs-patterns"]:
            cookie.read_cvs_patterns = True

        if o in ["--archive"]:
            cookie.name = v

        if o in ["--exclude"]:
            cookie.patterns.append(FilterPattern(v, '-'))

        if o in ["--archive-as"]:
            couple = v.split(",")
            if len(couple) < 2:
                print "invalid parameter: %s" % v
            name_modifier, name = couple
            cookie.patterns.append(RenamePattern(name, NameModifier(name_modifier)))

        if o in ["--bot-name"]:
            cookie.bot_file = v

        if o in ["--last-changes"]:
            d = parseDuration(v)
            if d >= 0:
                t = time.time() - d
                print "Get change since: '%s'" % time.strftime("%Y/%m/%d %H:%M:%S", time.localtime(t))
                cookie.datetime_limit = t

            t = parseDate(v)
            if t >= 0:
                print "Get change since: '%s'" % time.strftime("%Y/%m/%d %H:%M:%S", time.localtime(t))
                cookie.datetime_limit = t

        if o in ["--compression"]:
            cookie.compression = v

    path, ext = os.path.splitext(cookie.name)
    if ext == ".bz2":
        cookie.compression = "bz2"

    root = "" # Was "." but to avoid the "." folder in archive produced by Python 2.4 and Python 2.3
    if len(args) > 0:
        root = args[0]

    return root, cookie


def createZipArchiveFile(archive_path, archive_files):
    path, ext = os.path.splitext(archive_path)
    if ext == "":
        archive_path += ".zip"

    # Build the archive and put the file into it:
    archive = zipfile.ZipFile(archive_path, "w", zipfile.ZIP_DEFLATED)
    manifest = []
    for file in archive_files:
        if not file.isFile:
            continue

        archive_path = file.archive_path
        archive_path = archive_path.decode("ISO8859-1")
        archive_path = archive_path.encode("CP437")
        archive.write(file.path, archive_path)

        if file.path != file.archive_path:
            manifest.append((file.path, file.archive_path))

    if manifest:
        body = ""
        for file_archive in manifest:
            body += 'archive-as\t%s\t%s\n' % file_archive

        m = zipfile.ZipInfo(".manifest", time.localtime()[:6])
        m.compress_type = zipfile.ZIP_DEFLATED
        archive.writestr(m, body)

    archive.close()


def createGZipArchiveFile(archive_path, archive_files):
    path, ext = os.path.splitext(archive_path)
    if ext == "":
        archive_path += ".gzip"

    # Build the archive and put the file into it:
    archive = tarfile.TarFileCompat(archive_path, "w", tarfile.TAR_GZIPPED)
    manifest = []
    for file in archive_files:
        if not file.isFile:
            continue

        archive_path = file.archive_path
        archive_path = archive_path.decode("ISO8859-1")
        archive_path = archive_path.encode("CP437")
        archive.write(file.path, archive_path)

        if file.path != file.archive_path:
            manifest.append((file.path, file.archive_path))

    if manifest:
        body = ""
        for file_archive in manifest:
            body += 'archive-as\t%s\t%s\n' % file_archive

        m = tarfile.TarInfo(".manifest", time.localtime()[:6])
        archive.writestr(m, body)

    archive.close()


def createBz2ArchiveFile(archive_path, archive_files):
    path, ext = os.path.splitext(archive_path)
    if ext == "":
        archive_path += ".tar.bz2"

    # Build the archive and put the file into it:
    archive = tarfile.open(archive_path, "w:bz2")
    manifest = []
    for file in archive_files:
        if not file.isFile:
            continue

        archive_path = file.archive_path
#       archive_path = archive_path.decode("ISO8859-1")
#       archive_path = archive_path.encode("CP437")
        archive.add(file.path, archive_path)

        if file.path != file.archive_path:
            manifest.append((file.path, file.archive_path))

    if manifest:
        body = ""
        for file_archive in manifest:
            body += 'archive-as\t%s\t%s\n' % file_archive

        m = tarfile.TarInfo(name = ".manifest")
        m.mtime = time.time()
        m.size = len(body)
        f = StringIO.StringIO(body)
        archive.addfile(m, f)

    archive.close()


def main(args):
    root, cookie = parseArguments(args)
    if not cookie:
        return

    # Build of files to include in the archive:
    files = buildFileList(root, [], cookie)

    archive_path = getArchivePath(root, cookie)

    while True:
        unknown_files = getFileOfStatus(files, "?")
        if not unknown_files:
            break

        for file in unknown_files:
            print "? " + file.path

        answer = ask("continue[c], quit[q], add-all[a], review[r], [c]:", cookie)

        if not answer or answer == 'c':
            break

        if answer == 'q':
            return

        if answer == 'a':
            cookie.unkown_file_answer = "a"
            updateFilesStatus(files, cookie)

        if answer == 'r':
            updateFilesStatus(files, cookie)

    archive_files = getFileOfStatus(files, "+")

    if not archive_files:
        print "nothing to archive"
        return

    archive_paths = [os.path.normpath(file.path) for file in archive_files if file.isFile]

    # If the produced file is among the file to compress remove it:
    if archive_path in archive_paths:
        i = files.index(archive_path)
        del files[i]

    if cookie.compression == "bz2":
        createBz2ArchiveFile(archive_path, archive_files)
    else:
        createZipArchiveFile(archive_path, archive_files)


if __name__ == "__main__":
    main(sys.argv[1:])
