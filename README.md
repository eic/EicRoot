# EicRoot

  EIC detector simulation software based on the FairRoot framework.

Docker container
================

  Connection to the container instance through SSH connection will be shown.

  Do not forget to prepend the below commands with 'sudo' under Linux.

  Choose a free port on your computer (5555 in the below example).

  In order to mount a local directory <my-scratch-directory> from inside the container:

```
    mkdir <my-scratch-directory>
    chown 11111.11111 <my-scratch-directory> # this is currently required under Linux
```

  Use an additional key like '-v <my-scratch-directory>:/scratch:z' to the 'docker run' command below.

  <my-scratch-directory> will be accessible as /scratch in the container.

  Now run the container:

```
docker run -it -p 127.0.0.1:5555:22 -t ayk1964/eicroot-yr:v04
  
```

  This command also gives root access to the running container. Ctrl-D in this window 
stops the container (and terminates all the connected SSH processes).

  Open a different terminal on your local host system, and connect to the container:

```
ssh -Y eic@127.0.0.1 -p 5555 # pwd 'test'

# Set up the environment;
cd /container/app
. root-v6.14.00.binary/bin/thisroot.sh
. geant4.10.05.p01.build/bin/geant4.sh
. EicRoot/build/config.sh

# 'cd' to a particular example directory;
cd EicRoot/examples/tracking/config.1

# From this point on the situation is similar to what is was in the old eicroot:r940;
# ignore eic-smear-related warnings and Error in <TList::Clear>, those will be 
# cleaned up in the nearest future;
root -l tracker.C
root -l simulation.C
root -l digitization.C
root -l reconstruction.C
root -l ../analysis.C

# One can optionally change "Geant3" string to "Geant4" in simulation.C and re-run the 
# rest of the above chain with the GEANT4 rather than GEANT3 transport.

# This is optional; consider 'FairEventManager -> Info -> Current Event' to browse events;
# DO NOT USE 'Browser -> Quit ROOT' (crashes), but rather use '.q' command in the terminal;
root -l eventDisplay.C
```

Local installation
==================

  One needs a reasonable base installation (like an older CERN CC7), with additional 
packages described in the Dockerfile. 


Pre-requisites
--------------

Any modern ROOT version (6.14.00 as included in the Docker container certainly works; 
6.20.04 was shown to work either).

GEANT4 version geant4.10.05.p01 is known to work. In order to use it one needs to 
install G4 VMC, like version geant4_vmc.4.0.p1 .

In order to use GEANT3 one has to intall a recent G3 VMC (like geant3+_vmc.2.7).

In order to use eic-smear (for instance to import either ASCII or ROOT files with 
the simulated events) one has to install this package, obviously.

Dockerfile contains enough information for an expert to install these packages.

Important: different to the older EicRoot r940 Docker container it is assumed that the 
environmet setup steps are performed "by hand", like
 
. root-v6.14.00.binary/bin/thisroot.sh
. geant4.10.05.p01.build/bin/geant4.sh

Compiling
---------

See CMakeLists.txt file. More detailed instructions will follow soon.

