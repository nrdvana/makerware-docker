Makerware for Linux, via Docker
===============================

Makerbot's lineup of software has changed over the years, and their
latest main printing software "Makerbot Print" no longer supports
the first generation Replicator.  To print on a first-gen Replicator,
you have to stick with their Makerware Desktop, which is no longer
supported, and the last version was targeted toward Ubuntu 14.04.

You might not want to run Ubuntu 14.04 as a desktop anymore, but it
works great as a docker image.  The catch is that running desktop
software in docker can be hard, especially since Makerware needs a
system service running in order to function.

This project consists of a Dockerfile to get the basic Makerware
software installed, and two scripts that help bridge the gap between
the inside and outside of the docker container.

Usage
=====

First, make sure you have docker installed and working.

Next:

  git clone git@github.com/nrdvana/makerware-docker
  cd makerware-docker
  ./makerware-docker

This should "Just Work".  After the first run, you have a docker image
named "makerware:latest" and you can run the makerware-docker script
from any other directory you like, or install it in ~/bin or whatever.

Operation
=========

In short, makerware-docker launches a one-shot docker instance and
sets up X11 permissions so makerware can display its window, mounts
your current $HOME into the docker image, and passes your current
UID/GID/supplemental group list to the docker entry point,
run-conveyor-and-makerware.  That script then takes the user/group
information, applies it to the docker system, and then starts both the
conveyor service and the makerware app each as the proper user.
The makerware app should be running with a view of your actual home
directory with all the same permissions as your normal user account.
