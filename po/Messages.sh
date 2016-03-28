#!/bin/sh

project="subtitlecomposer" # project name
bugaddr="https://github.com/maxrd2/subtitlecomposer/issues" # MSGID-Bugs
cd "$(dirname "$0")"
podir=`pwd`
srcdir="../src/" # root of translatable sources

cd $podir

echo "Preparing rc files"
# we use simple sorting to make sure the lines do not jump around too much from system to system
find "$srcdir" -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > "$podir/rcfiles.list"
xargs --arg-file="$podir/rcfiles.list" extractrc > "$podir/rc.cpp"
# additional string for KAboutData
echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> "$podir/rc.cpp"
echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> "$podir/rc.cpp"
echo "Done preparing rc files"


echo "Extracting messages"
# see above on sorting
find "$srcdir" -name '*.cpp' -o -name '*.h' -o -name '*.c' | sort > "$podir/infiles.list"
echo "rc.cpp" >> "$podir/infiles.list"
xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
	-kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
	--msgid-bugs-address="$bugaddr" \
	--files-from="$podir/infiles.list" -D "$srcdir" -D "$podir" -o "$podir/$project.pot" || { echo "error while calling xgettext. aborting."; exit 1; }
echo "Done extracting messages"


echo "Merging translations"
for cat in `find "$podir" -name '*.po'`; do
	echo "$cat"
	msgmerge -o "$cat.new" "$cat" "$podir/$project.pot"
	mv "$cat.new" "$cat"
	sed -r -i -e 's/\.\.\/src\//..\/..\/src\//g' "$cat"
done
echo "Done merging translations"


echo "Cleaning up"
rm "$podir/rcfiles.list"
rm "$podir/infiles.list"
rm "$podir/rc.cpp"
echo "Done"
