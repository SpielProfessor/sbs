SBS  -  THE SIMPLE BUILD SYSTEM
-------------------------------

USAGE:
    sbs    --new   / -n    <project-name>
    sbs    --init  / -i    <project-name>
    sbs    --build / -b    [release-mode]
    sbs    --run   / -r    [release-mode]
    sbs    --help  / -h
  WHERE:
    project-name: single-word project name
    release-mode: either `--debug` (-d) or `--release` (-R)

DESCRIPTION:
  SBS is a simple build system. It uses the `Buildconf` file in the current directory (or one of the parent directories) for configuration.
  SBS compiles all compatible source files in the `src/` folder into object files in the `build/debug/obj` or `build/release/obj` folder.
  SBS then links the objects to `build/debug/<project-name>` or `build/release/<project-name>`


SBS BUILDCONF
-------------

SYNTAX:
  <aspect>: <content>\n
  comments begin with a // and continue for the rest of the line; they may only be placed on lines by themselves.

  a full configuration can be seen at this project's Buildconf file.
