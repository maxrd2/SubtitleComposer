## SubtitleComposer ##

An open source text-based subtitle editor that supports basic operations as well as more advanced ones, aiming to become an improved version of Subtitle Workshop for every platform supported by KDE.

This is a continuation of Subtitle Composer by Sergio Pistone - https://sourceforge.net/projects/subcomposer/   
Since original author didn't have time anymore to continue development of this great app, I've took over in order to submit fixes and add more features in free time.

### FEATURES
 - Support for multiple formats, including SubRip, MicroDVD, SSA/ASS (without advanced styles), MPlayer, TMPlayer and YouTube captions.
 - Live preview of subtitles and video with support for multiple backends (GStreamer, MPlayer, Xine, Phonon), audio channel selection and full screen mode.
 - Time shifting and adjusting, lines duration calculation, synchronization with video, etc.
 - Working with original subtitle and translation.
 - Texts styles (italic, bold, underline, stroke), font color, spell checking, automatic translation (using Google services), etc.
 - Joinning and splitting of files.
 - Automatic detection of errors.
 - Editing of subtitles through scripting (Ruby, Python, JavaScript and other languages supported by Kross).

### BUGS
Please submit bug reports or feature requests to the [issue tracker on GitHub][bugs]. 
If you do not have a GitHub account and feel uncomfortable creating one then feel free to send an 
e-mail to &lt;max at smoothware dot net&gt; instead.

### TODO
 - code cleanup and bugfixes before releasing v1.0
 - add VobSub import
 - add Speech Recognition (through PocketSphinx http://cmusphinx.sourceforge.net/2010/03/pocketsphinx-0-6-release/) to automate original language subtitle synhronization
 - add Speech Recognition to create new subtitles in original language

Help and ideas are welcome.

### AUTHORS / CONTRIBUTORS
 - Sergio Pistone - original author and maintainer
 - Mladen Milinkovic - maintainer
 - wantilles - support for VDPAU decoding on the mplayer backend
 and many others


[bugs]: https://github.com/maxrd2/subtitlecomposer/issues "Issue Tracker"
