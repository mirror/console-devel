import os

extentions = [
    ".aps",
    ".ncb",
    ".pyc",
    ".pyo",
    ".suo",
    ".exp",
    ".lib",
    ".map",
    ".pdb",
    ".ilk",
    ".bsc",
    ".obj",
    ".sbr",
    ".pch",
    ".res",
    ".idb",
]


def main():
    for root, folders, files in os.walk("."):
        for file in files:
            name, ext = os.path.splitext(file)
            if ext not in extentions:
                continue

            path = os.path.join(root, file)
            try:
                os.remove(path)
            except:
                print "fail to remove", file


if __name__ == "__main__":
    main()
