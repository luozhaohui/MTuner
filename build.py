#!/usr/bin/python
from tnbuildtools import *


class Rmem(Component):

    baseRevision = '1.2.0-alpha'
    vendorVersion = '1.0.0'  # master: commit b313a2678a497457896e35ebfd2033b4fcc2ba30
    vendorSite = 'https://github.com/milostosic/rmem'

    setCmakePaths = True

    dependencies = [
        StaticLibraryDependency('Boost'),
        StaticLibraryDependency('Lz4'),
        StaticLibraryDependency('RobinHoodHashing'),
        StaticLibraryDependency('GPerftools', platform=['linux-ubuntu'])
    ]

    artifacts_platforms = ['osx', 'win64', 'linux-ubuntu', 'android']

    # artifacts = [
    #     FileArtifact('rmem/inc/tn_rmem.h', 'include/rmem/tn_rmem.h'),
    #     FileArtifact('rmem/inc/rmem.h', 'include/rmem/rmem.h',
    #                  platform=artifacts_platforms),
    #     FileArtifact('rmem/inc/rmem_entry.h', 'include/rmem/rmem_entry.h',
    #                  platform=artifacts_platforms),
    #     FileArtifact('mtuner/DIA/bin/x64/msdia140.dll',
    #                  'bin/msdia140.dll', platform=['win64']),

    #     StaticLibraryArtifact(
    #         'rmem', platform=artifacts_platforms),
    #     # StaticLibraryArtifact( 'rbase', platform = artifacts_platforms ),
    #     # StaticLibraryArtifact( 'rdebug', platform = artifacts_platforms ),

    #     ExecutableArtifact("mtuner", platform=[
    #                        'win64', 'linux-ubuntu'])
    # ]


if __name__ == '__main__':
    main()
