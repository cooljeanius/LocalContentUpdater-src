
README.TXT for Local Content Updater

Before using the Macromedia Flash Local Content Updater, be sure to
read the article on Flash Player 8 security at
http://www.macromedia.com/go/flash8security .

The Macromedia Flash Local Content Updater is a command-line tool for
managing the security privileges of Flash movies (SWFs) that are
intended to run as local files.  Most SWFs are usually run from HTTP
servers, but SWFs may also be run from local filesystems.

In the Flash Player 8 browser players and standalones, the security
model for local SWFs has changed.  These changes do not affect
projectors, or local non-browser applications embedding the Flash
Player ActiveX control.

In this enhanced Flash Player 8 local security model, local SWFs (of
any version) may either read from local resources, or communicate with
the Internet, but not both.  In previous versions of the Flash Player,
all local SWFs could do both of these things.

In Flash Player 8, by default, local SWFs are allowed to read from
local resources, but not communicate with the Internet.  You can
change this default for a local SWF by inserting a flag into the SWF
that requests network privileges.  When this flag is present, Flash
Player 8 will allow the local SWF to communicate with the Internet,
but not to read from local resources.

Note that there are several other ways to work around the restrictions
of the Flash Player 8 security model; the network flag is only one of
several options.  Be sure to refer to the Flash Player 8 security
article referenced at the top of this file.

Macromedia Flash 8 has a new publish option called "local playback
security" that enables you to include network privileges in SWFs that
you publish (by choosing "access network only").  However, it is
possible that some Flash authors (or other parties that maintain or
distribute Flash content) may wish to add network privileges to their
SWFs, but may not own Flash 8, or may not wish to re-publish their
SWFs.  For this audience, Macromedia has developed the Local Content
Updater, with which you can easily inspect, add, or remove network
privileges in one or many SWFs.

Macromedia freely distributes the Local Content Updater for Windows,
Mac OS X, and Linux.  The Local Content Updater is also available as
C++ source code, which should allow developers to port it to other
platforms if needed.  If you have questions or comments about the
Local Content Updater, or wish to submit ports or other contributions,
please e-mail local_content_updater@macromedia.com .

To run the tool:

1. Unzip the tool to a directory on your hard drive.

2. Open a command prompt. 

3. Type the following command, including the full path to where you
installed the tool:

  LocalContentUpdater

The updater prints a usage message that describes what arguments you
can use.  You can check for, add, or remove network privileges, and
run the tool on single files, multiple files, or recursively on
directories.

4. Change to the directory that includes the SWF files you need to
update.

5. Run the command again, including the full path to where you
installed the tool and the appropriate arguments.

(You can avoid typing the full path to the tool by ensuring that its
location is included in your PATH environment variable.  Refer to your
operating system documentation for details on how to do this.)
