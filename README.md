## Subtitle Composer
[![Build Status](https://build.kde.org/job/Extragear/job/subtitlecomposer/job/kf5-qt5%20SUSEQt5.12/badge/icon)](https://build.kde.org/job/Extragear/job/subtitlecomposer/)
[![Localization](https://d322cqt584bo4o.cloudfront.net/subtitlecomposer/localized.svg)](https://l10n.kde.org/stats/gui/trunk-kf5/po/subtitlecomposer.po/)

An open source text-based subtitle editor that supports basic and advanced editing operations, aiming to become an improved version of Subtitle Workshop for every platform supported by Plasma Frameworks.

### FEATURES
  - Open/Save **Text Subtitle Formats**
    - SubRip/SRT, MicroDVD, SSA/ASS, MPlayer, TMPlayer and YouTube captions
  - Open/OCR **Graphics Subtitle Formats**
    - VobSub (.idx/.sub/.rar), BluRay/PGS (*.sup), formats supported by ffmpeg (DVD/Vob, DVB, XSUB, HDMV-PGS)
  - **Demux Graphics/Text Subtitle Stream** from video file
    - SRT, SSA/ASS, MOV text, MicroDVD, Graphic formats supported by ffmpeg (DVD/Vob, DVB, XSUB, HDMV-PGS)
  - **Speech Recognition** from audio/video file using PocketSphinx
  - Smart **language/text encoding** detection
  - Live preview of subtitles in **integrated video player** (MPV, GStreamer, MPlayer, Xine, Phonon) w/ audio stream selection
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
#### Linux
  - AppImage - downloadable from [releases](https://github.com/maxrd2/subtitlecomposer/releases) page
  - Arch
    - unofficial [repository](https://wiki.archlinux.org/index.php/unofficial_user_repositories#subtitlecomposer) with [subtitlecomposer-git](https://smoothware.net/subtitlecomposer/x86_64/) package
    - AUR - packages [subtitlecomposer](https://aur.archlinux.org/packages/subtitlecomposer) or [subtitlecomposer-git](https://aur.archlinux.org/packages/subtitlecomposer-git)
  - Ubuntu
    - official [subtitlecomposer](https://packages.ubuntu.com/subtitlecomposer) package
    - unofficial [repository](https://launchpad.net/~subtitlecomposer) - packages [subtitlecomposer](https://launchpad.net/~subtitlecomposer/+archive/ubuntu/subtitlecomposer-git-stable) or [subtitlecomposer-git](https://code.launchpad.net/~subtitlecomposer/+archive/ubuntu/subtitlecomposer-git)
  - Debian
    - official [subtitlecomposer](https://packages.debian.org/subtitlecomposer) package
  - OpenSUSE
    - official [subtitlecomposer](https://software.opensuse.org/package/subtitlecomposer) package

#### Windows
  - Installer downloadable from [releases](https://github.com/maxrd2/subtitlecomposer/releases) page

### BUILD
Instructions for building from sources can be found on [wiki page][build instructions]

### CONTRIBUTING
Submit bug reports or feature requests to the [official issue tracker][bugs].

Video tutorials are very welcome as are any kind of documentation/tutorials/examples, please let us know if you make some.

Feedback and/or ideas on how to make Subtitle Composer better are welcome and appreciated.

Pull requests and patches are welcome. Please follow the [coding style](README.CodingStyle.md).

### LICENSE

Subtitle Composer is released under [GNU General Public License v2.0](LICENSE)


[bugs]: https://invent.kde.org/kde/subtitlecomposer/issues "Issue Tracker"
[milestones]: https://invent.kde.org/kde/subtitlecomposer/-/milestones "Milestones"
[coding style]: https://invent.kde.org/kde/subtitlecomposer/blob/master/README.CodingStyle.md "Coding Style"
[build instructions]: https://invent.kde.org/kde/subtitlecomposer/wikis/Building-from-sources "Build Instructions"
