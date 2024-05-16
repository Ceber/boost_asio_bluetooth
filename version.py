import re


def get_version():
    f = open("CMakeLists.txt", "r")
    content = f.read()

    version_major = (
        re.search(r"[^\)]+VERSION_MAJOR (\d+)[^\)]*\)", content).group(1).strip()
    )
    version_minor = (
        re.search(r"[^\)]+VERSION_MINOR (\d+)[^\)]*\)", content).group(1).strip()
    )
    version_patch = (
        re.search(r"[^\)]+VERSION_PATCH (\d+)[^\)]*\)", content).group(1).strip()
    )

    version = "{}.{}.{}".format(version_major, version_minor, version_patch)

    return version


print(get_version())
