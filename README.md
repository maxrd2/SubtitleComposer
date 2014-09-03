## SubtitleComposer ##

An open source text-based subtitle editor that supports basic operations as well as more advanced ones, aiming to become an improved version of Subtitle Workshop for every platform supported by KDE.

This is a continuation of Subtitle Composer by Sergio Pistone - https://sourceforge.net/projects/subcomposer/   
Since original author didn't have time anymore to continue development of this great app, I've took over in order to submit fixes and add more features in free time.

### FEATURES
 - Support for multiple formats, including SubRip, MicroDVD, SSA/ASS (without advanced styles), MPlayer, TMPlayer and YouTube captions.
 - Live preview of subtitles and video with support for multiple backends (GStreamer, MPlayer/2, Xine, Phonon), audio channel selection and full screen mode.
 - Time shifting and adjusting, lines duration calculation, synchronization with video, etc.
 - Working with original subtitle and translation.
 - Texts styles (italic, bold, underline, stroke, color), spell checking, automatic translation (using Google services), etc.
 - Joining and splitting of files.
 - Automatic detection of errors.
 - Editing of subtitles through scripting (Ruby, Python, JavaScript and other languages supported by [Kross](http://techbase.kde.org/Development/Tutorials/Kross-Tutorial)).

### BUGS
Please submit bug reports or feature requests to the [issue tracker on GitHub][bugs]. 
If you do not have a GitHub account and feel uncomfortable creating one then feel free to send an 
e-mail to &lt;max at smoothware dot net&gt; instead.

### TODO
Please look at [Milestone list][milestones] and [issue tracker on GitHub][bugs] for todo list.

Help and ideas are welcome.   
If you would like to do some code changes, please check the new [coding style][coding style] or [wiki][coding style wiki].   

### AUTHORS / CONTRIBUTORS
 - Mladen Milinkovic - maintainer
 - Gogo - Croatian translation
 - Petar Toushkov - Bulgarian translation
 - Sergio Pistone - original author and maintainer
 - Petr Gadula (Goliash), Thomas Gastine, Panagiotis Papadopoulos, Alessandro Polverini, Tomasz Argasiński, Marcio P. Moraes,
 Alexander Antsev, Slobodan Simic, Yuri Chornoivan, Alexandros Perdikomatis, Barcza Károly - original translations
 - wantilles - support for VDPAU decoding on the mplayer backend
 - Martin Steghöfer
 - and many others


[bugs]: https://github.com/maxrd2/subtitlecomposer/issues "Issue Tracker"
[milestones]: https://github.com/maxrd2/subtitlecomposer/issues/milestones "Milestones"
[coding style]: https://github.com/maxrd2/subtitlecomposer/blob/master/README.CodingStyle.md "Coding Style"
[coding style wiki]: https://github.com/maxrd2/subtitlecomposer/wiki/Coding-Style "Coding Style - Wiki"
