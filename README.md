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

First, make sure you have docker installed and working.  If you don't
belong to the 'docker' group, these scripts will prefix commands with
'sudo' and if you haven't enabled passwordless sudo, you'll need to
enter your password.

Next:

    git clone git@github.com/nrdvana/makerware-docker
    cd makerware-docker
    ./makerware-docker

This should "Just Work".  After the first run, you have a docker image
named "makerware:latest" and you can run the makerware-docker script
from any other directory you like, or install it in ~/bin or whatever.

If you don't belong to the 'docker' group and don't want to enable
passwordless sudo and don't want to have to enter a password every
time you launch makerware, you can use the script

    ./install-setgid

to compile a C program that does mostly the same thing as the
`makerware-docker` script and install it to ~/bin as set-gid for
group `docker`.  This script also installs a desktop menu item (complete
with icon) into `~/.local/share`, so from then on you can just run it
from your window manager's programs menu.

Note that on first launch, the Makerware software gives you a screen where
they want you to log in.  You can skip that and click on the middle icon
at the top to get to the screen where you load your models and export them.

Operation
=========

In short, makerware-docker launches a one-shot docker instance and
sets up X11 permissions so makerware can display its window, mounts
your current $HOME into the docker image, and passes your current
UID/GID/supplemental group list to the docker entry point,
`run-conveyor-and-makerware`.  That script then takes the user/group
information, applies it to the docker system, and then starts both the
conveyor service and the makerware app each as the proper user.
The makerware app should then run with a view of your actual home
directory with all the same permissions as your normal user account,
giving a seamless experience like you had run it on the host itself.
