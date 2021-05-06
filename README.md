## Subtitle Composer
[![Linux Build](https://subtitlecomposer.smoothware.net/badge.php?p=job/kf5-qt5&os=suse&t=Linux+Build)](https://build.kde.org/job/Extragear/job/subtitlecomposer/)
[![FreeBSD Build](https://subtitlecomposer.smoothware.net/badge.php?p=job/kf5-qt5&os=bsd&t=FreeBSD+Build)](https://build.kde.org/job/Extragear/job/subtitlecomposer/)
[![TravisCI Build](https://img.shields.io/travis/maxrd2/subtitlecomposer/master.svg?label=Travis+Builds)](https://travis-ci.org/maxrd2/subtitlecomposer)
[![Localization](https://d322cqt584bo4o.cloudfront.net/subtitlecomposer/localized.svg)](https://l10n.kde.org/stats/gui/trunk-kf5/po/subtitlecomposer.po/)

An open source text-based subtitle editor that supports basic and advanced editing operations, aiming to become an improved version of Subtitle Workshop for every platform supported by Plasma Frameworks.

[Homepage][homepage] - [Downloads][downloads]

### FEATURES
  - Open/Save **Text Subtitle Formats**
    - SubRip/SRT, MicroDVD, SSA/ASS, MPlayer, TMPlayer and YouTube captions
  - Open/OCR **Graphics Subtitle Formats**
    - VobSub (.idx/.sub/.rar), BluRay/PGS (*.sup), formats supported by ffmpeg (DVD/Vob, DVB, XSUB, HDMV-PGS)
  - **Demux Graphics/Text Subtitle Stream** from video file
    - SRT, SSA/ASS, MOV text, MicroDVD, Graphic formats supported by ffmpeg (DVD/Vob, DVB, XSUB, HDMV-PGS)
  - **Speech Recognition** from audio/video file using PocketSphinx
  - Smart **language/text encoding** detection
  - Live preview of subtitles in **integrated video player** (FFmpeg) w/ audio stream selection
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

![Main Window](https://cdn.kde.org/screenshots/subtitlecomposer/mainwindow.png)

### INSTALL
Linux and Windows binaries downloadable from [downloads page][downloads]

### BUILD
Instructions for building from sources can be found on [wiki page][build instructions]

### CONTRIBUTING
Join friendly Matrix chat room [#subtitlecomposer][matrix-chat] and say hello or ask for help.

Feedback and ideas on how to make Subtitle Composer better are welcome and appreciated!
Let us know in [#subtitlecomposer chat][matrix-chat].

Submit bug reports or feature requests to the [official issue tracker][bugs].

Pull requests and patches are welcome. Please follow the [coding style][coding style].

### LICENSE

Subtitle Composer is released under [GNU General Public License v2.0](LICENSE)

[homepage]: https://subtitlecomposer.kde.org/
[matrix-chat]: https://webchat.kde.org/#/room/#subtitlecomposer:kde.org
[bugs]: https://invent.kde.org/kde/subtitlecomposer/issues "Issue Tracker"
[milestones]: https://invent.kde.org/kde/subtitlecomposer/-/milestones "Milestones"
[coding style]: https://invent.kde.org/kde/subtitlecomposer/blob/master/README.CodingStyle.md "Coding Style"
[build instructions]: https://invent.kde.org/kde/subtitlecomposer/wikis/Building-from-sources "Build Instructions"
[downloads]: https://subtitlecomposer.kde.org/download.html