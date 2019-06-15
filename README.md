## Subtitle Composer [![TravisCI build](https://travis-ci.org/maxrd2/subtitlecomposer.svg?branch=master)](https://travis-ci.org/maxrd2/subtitlecomposer)

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

![Main Window](http://maxrd2.github.io/subtitlecomposer/images/screenshot.png)

### INSTALL
#### Linux
  - AppImage - downloadable from [releases](https://github.com/maxrd2/subtitlecomposer/releases) page
  - Arch
    - Official [repository](https://wiki.archlinux.org/index.php/unofficial_user_repositories#subtitlecomposer) with subtitlecomposer-git package
    - AUR stable - package [subtitlecomposer](https://aur.archlinux.org/packages/subtitlecomposer)
    - AUR git - package [subtitlecomposer-git](https://aur.archlinux.org/packages/subtitlecomposer-git)
  - Ubuntu
    - stable - package [subtitlecomposer](https://launchpad.net/~subtitlecomposer/+archive/ubuntu/subtitlecomposer-git-stable)
    - git - package [subtitlecomposer](https://code.launchpad.net/~subtitlecomposer/+archive/ubuntu/subtitlecomposer-git)
    - oudated - official [subtitlecomposer](https://packages.ubuntu.com/subtitlecomposer) package
  - Debian
    - oudated [subtitlecomposer](https://packages.debian.org/subtitlecomposer) package
  - OpenSUSE
    - oudated [subtitlecomposer](https://software.opensuse.org/package/subtitlecomposer) package

### BUILD
Instructions for building from sources can be found on [wiki page](https://github.com/maxrd2/subtitlecomposer/wiki/Building-from-sources)

### CONTRIBUTING
Submit bug reports or feature requests to the [issue tracker on GitHub][bugs].

Video tutorials are very welcome as are any kind of documentation/tutorials/examples, please let us know if you make some.

Feedback and/or ideas on how to make Subtitle Composer better are welcome and appreciated.

Pull requests and patches are welcome. Please follow the [coding style](README.CodingStyle.md).

### LICENSE

Subtitle Composer is released under [GNU General Public License v2.0](LICENSE)


[bugs]: https://github.com/maxrd2/subtitlecomposer/issues "Issue Tracker"
[milestones]: https://github.com/maxrd2/subtitlecomposer/milestones "Milestones"
[coding style]: https://github.com/maxrd2/subtitlecomposer/blob/master/README.CodingStyle.md "Coding Style"
[authors]: https://github.com/maxrd2/subtitlecomposer/blob/master/AUTHORS "Authors / Contributors"
