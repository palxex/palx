# Introduction #

Hi, as you have reach here, I think I should assume that you have enough skill on make and its parters.

Maybe you have a question that why palx release win32 builds only. One reason is that I thought only windows user has experience about the original game, and this is importent for such test; the oher is that till recently I havent got a stable and usable linux platform. But even though, after made changes to meet linux order, I won't release binary packages. The reason is simple: there are too many distributions, I don't want to package for each other, and for eliminate flame war, I wont package for any.

The dependencies are boost, allegro, adplug, libc and freetype's devel package, pkgconfig and build essential packages, and if you doesn't installed chinese environment, ttf-arphic-uming font package. one can use his distribution's package manager to installthem. Many distributions doesn't have the boost related m4 files used in the automake, copy from the svn'd m4 directory or google the latest m44boost.
after
```
./autogen.sh
./configure
make
```
Gentoo specific:
put http://github.com/palxex/palxex-overlay/blob/e1172058907a241e5de69f4978a01891b1cf889a/games-rpg/palx/palx-9999.ebuild to your own overlay, and then emerge palx (with console USE flag to enable lua console).

copy src/palx to the pal game's directory, (note: use the follow script all file's filename!) and play now:)
```
for i in *;
do
	mv $i `echo $i|tr [:lower:] [:upper:]`
done;
```