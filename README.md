## SubtitleComposer ##

An open source text-based subtitle editor that supports basic operations as well as more advanced ones, aiming to become an improved version of Subtitle Workshop for every platform supported by Plasma Framework.

This is a continuation of Subtitle Composer by Sergio Pistone from sourceforget.net/projects/subcomposer/

### FEATURES
 - Open/Save **Text Subtitle Formats**
    - SubRip/SRT, MicroDVD, SSA/ASS, MPlayer, TMPlayer and YouTube captions
 - Open/OCR **Graphics Subtitle Formats**
    - VobSub (.idx/.sub/.rar), formats supported by ffmpeg (DVD/Vob, DVB, XSUB, HDMV-PGS)
 - **Demux/OCR/Import** Graphics/Text Subtitle Formats directly **from video**
    - SRT, SSA/ASS, MOV text, MicroDVD, Graphic formats supported by ffmpeg (DVD/Vob, DVB, XSUB, HDMV-PGS)
 - **Speech Recognition** from audio/video file using PocketSphinx
 - Smart **language/text encoding** detection
 - Live preview of subtitles in **integrated video player** (GStreamer, MPlayer, MPV, Xine, Phonon) w/ audio stream selection
 - Preview/editing of subtitles on **audio waveform** w/ audio stream selection
 - Quick and **easy subtitle sync**:
    - Dragging several anchors/graftpoints and stretching timeline
    - Time shifting and scaling, lines duration re-calculation, framerate conversion, etc.
    - Joining and splitting of subtitle files
 - Side by side subtitle **translations**
 - Text **styles** (italic, bold, underline, stroke, color)
 - **Spell** checking
 - Detection of timing errors in subtitles
 - **Scripting** (JavaScript, Python, Ruby and other languages supported by [Kross](http://techbase.kde.org/Development/Tutorials/Kross-Tutorial)).

![Main Window](https://raw.githubusercontent.com/maxrd2/subtitlecomposer/gh-pages/screenshots/screen-main.png)

![Functions](https://raw.githubusercontent.com/maxrd2/subtitlecomposer/gh-pages/screenshots/screen-actions.png)

![Settings Window](https://raw.githubusercontent.com/maxrd2/subtitlecomposer/gh-pages/screenshots/screen-settings.png)

### BUILD / INSTALL
Instructions can be found on ['Building from sources' wiki page](https://github.com/maxrd2/subtitlecomposer/wiki/Building-from-sources)

### BUGS / FEATURES / IDEAS / QUESTIONS
Please submit bug reports or feature requests to the [issue tracker on GitHub][bugs].  
If you do not have a GitHub account and feel uncomfortable creating one then feel free to send me an
email to max at smoothware dot net instead.

### TODO
Please look at [Milestone list][milestones] and [issue tracker on GitHub][bugs] for todo list.

### CONTRIBUTING
Help and ideas are welcome.   
If you would like to do some code changes, please check the [coding style doc][coding style].   

### AUTHORS / CONTRIBUTORS
Please look in [AUTHORS file][authors] for (incomplete) list.


[bugs]: https://github.com/maxrd2/subtitlecomposer/issues "Issue Tracker"
[milestones]: https://github.com/maxrd2/subtitlecomposer/milestones "Milestones"
[coding style]: https://github.com/maxrd2/subtitlecomposer/blob/master/README.CodingStyle.md "Coding Style"
[authors]: https://github.com/maxrd2/subtitlecomposer/blob/master/AUTHORS "Authors / Contributors"
