# Mangonel, a simple application launcher

Mangonel is intended as a light weight replacement for the standard Plasma KRunner.
The main difference is that Mangonel sacrifices the number of things it can do for speed and simplicity.

And Mangonel will never re-arrange results, so you don't have to worry about another item being selected right before you hit enter.

> ### Mangonel currently lets you do these things.
>
> * Start applications.
> * Opening files and directories with their default application.
> * Execute shell commands, including arguments.
> * Use it as a calculator.
> * Convert between values in different units.

The default global shortcut to show Mangonel is `CTR+ALT+Space`, you can change this in the Plasma system settings, or by right clicking on it when it is visible.

Mangonel is inspired by the classic [Katapult][] application launcher.

# Installation.

Installing Mangonel from source is not difficult. So far it is only available from [the Git repository][1].
Once you have the files cd into the directory containing the source and follow the steps below to build and install the application.

* Create a build directory and move into it.
* run: `cmake ../`
* run: `make`
* As root, run: `make install`
* Start mangonel.
* Enjoy launching your favourite applications in a matter of milliseconds.

### Autostart.
To automatically start Mangonel when you log on to your desktop, follow these steps.

* Open the Plasma System Settings.
* Go to 'Startup and Shutdown'.
* Click 'Add Program...'.
* Browse to the executable.


[Katapult]: http://katapult.kde.org/
[1]: https://invent.kde.org/utilities/mangonel
