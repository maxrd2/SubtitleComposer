## SubtitleComposer ##

An open source text-based subtitle editor that supports basic operations as well as more advanced ones, aiming to become an improved version of Subtitle Workshop for every platform supported by KDE.

This is a continuation of Subtitle Composer by Sergio Pistone from sourceforget.net/projects/subcomposer/

### FEATURES
 - Load/save multiple formats - SubRip, MicroDVD, SSA/ASS, MPlayer, TMPlayer and YouTube captions
 - Smart language/text encoding selection
 - Live preview of subtitles in video player (GStreamer, MPlayer, MPV, Xine, Phonon) w/ audio channel selection
 - Live preview of subtitles on audio waveform w/ audio channel selection
 - Easy sync fixing with multiple anchors/graftpoints, time shifting and stretching, lines duration re-calculation, framerate conversion, etc.
 - Side by side subtitle translations
 - Texts styles (italic, bold, underline, stroke, color)
 - Spell checking
 - Joining and splitting of subtitle files
 - Detection of timing errors in subtitles
 - Scripting support (Ruby, Python, JavaScript and other languages supported by [Kross](http://techbase.kde.org/Development/Tutorials/Kross-Tutorial)).

<p align="center">
	<img src="https://raw.githubusercontent.com/maxrd2/subtitlecomposer/gh-pages/screenshots/screen-main.png" alt="Main Window"/>
	<img src="https://raw.githubusercontent.com/maxrd2/subtitlecomposer/gh-pages/screenshots/screen-actions.png" alt="Functions"/>
	<img src="https://raw.githubusercontent.com/maxrd2/subtitlecomposer/gh-pages/screenshots/screen-settings.png" alt="Settings Window"/>
</p>

### BUGS
Please submit bug reports or feature requests to the [issue tracker on GitHub][bugs]. 
If you do not have a GitHub account and feel uncomfortable creating one then feel free to send an 
e-mail to &lt;max at smoothware dot net&gt; instead.

### TODO
Please look at [Milestone list][milestones] and [issue tracker on GitHub][bugs] for todo list.

### CONTRIBUTING
Help and ideas are welcome.   
If you would like to do some code changes, please check the [wiki][coding style wiki].   

### AUTHORS / CONTRIBUTORS
 - Mladen Milinkovic - author/maintainer
 - @Martchus - code contributions, arch linux builds, German translation
 - Goran Vidovic (gogo) - Croatian translation
 - Petar Toushkov - Bulgarian translation
 - Sergio Pistone - original author
 - Petr Gadula (Goliash), Thomas Gastine, Panagiotis Papadopoulos, Alessandro Polverini, Tomasz Argasiński, Marcio P. Moraes,
 Alexander Antsev, Slobodan Simic, Yuri Chornoivan, Alexandros Perdikomatis, Barcza Károly - original translations
 - wantilles - support for VDPAU decoding on the mplayer backend
 - Martin Steghöfer
 - and many others


[bugs]: https://github.com/maxrd2/subtitlecomposer/issues "Issue Tracker"
[milestones]: https://github.com/maxrd2/subtitlecomposer/milestones "Milestones"
[coding style wiki]: https://github.com/maxrd2/subtitlecomposer/wiki/Coding-Style "Coding Style - Wiki"
