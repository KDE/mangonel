# Mangonel a simple application launcher for KDE4.

Mangonel is intended as a light weight replacement for the, in my view bloated and slow, standard KRunner. Sadly you can't actually get rid of KRunner because it's also responsible for your screensaver and power-management.

> ### Mangonel currently lets you do these things.
>
> * Start applications in the KDE menu.
> * Opening files and directories with their default application.
> * Execute shell commands, including arguments.
> * Use it as a calculator.

The default global shortcut to show Mangonel is `CTR+ALT+Space`, you can change this in the KDE4 system settings.

Mangonel came to be because I started to really mis [Katapult][]. I loved Katapult for its simplicity and speed. Those are two points I really can't find in KRunner.

# Installation.

Installing Mangonel from source is not difficult. First [download the source on github][1] or [clone the git repository][2].
Once you have the files cd into the directory containing the source and follow the steps below to build and install the application.

* Create a build directory and move into it.
* run: `cmake ../`
* run: `make`
* If all went well you now have an executable called mangonel in your build directory. This is the only file that needs to be installed. You can now copy this to the desired `bin` directory.
* Start it.
* Enjoy launching your favourite applications in a mater of milliseconds.

### Autostart.
To automatically start Mangonel when you log on to your desktop, follow the steps below.

* Open 'System Settings'.
* Go to 'Startup and Shutdown'.
* Click 'Add Program...'.
* Browse to the executable.


[Katapult]: http://katapult.kde.org/
[1]: https://github.com/tarmack/Mangonel/archives/master
[2]: https://github.com/tarmack/Mangonel
