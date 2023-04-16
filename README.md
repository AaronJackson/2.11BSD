# 2.11BSD

![Current Build](https://github.com/AaronJackson/2.11BSD/actions/workflows/main.yml/badge.svg)

This is my development fork of 2.11BSD UNIX for the PDP-11. However,
this fork is set up with GitHub Workflows so we can test our changes
to the kernel.

For more information about 2.11BSD, see
https://wfjm.github.io/home/211bsd/

This repo was forked from bsdimp's 2.11BSD repo - thanks to him for
making a tidy repo which I can work from.

Current modifications:

- Addition of IBV11-A driver for talking to GPIB devices. (major 27).


## Patching

This tree started out at patch level 450, but appeared to have a
couple of modifications to one or two files. I ended up merging my
IBV11 driver to make it even more of a mess. Since the lowest patch
level supported by my `2.11BSD-Action` Github Action is 457, the
patches were applied in the following way.

   1. A branch called `patch-base` was made from main, at `c507710` to create a temporary directory `usr` with symlinks to `src -> ../`	and `include -> ../include`. This allowed patches to be applied	from a modern PC.
   2. From `patch-base`, a new branch `451-457` was made which allowed me to apply patches 451 to 457, bringing the Git tree back into alignment with the lowest patch level available of my GitHub
       action. I was able to push this to GitHub and let the workflow run to verify that everything was compiling correctly.
   3. Each individual patch was made in a new branch, taken from the previous, and pushed to GitHub to verify it can be built (i.e. `458` was taken from `451-457`, and `459` was taken from `458`). In each case, the `patch_level` variable was also bumped in this repos GitHub workflow to ensure the two trees would be compatible.
   4. Once all patches were applied, I created a cleanup branch to remove the temporary directory `usr` to keep the tree clean.

There were a few patches which caused issues:

- `452` attempts to update the version header of `share/lint/llib-lc` from 1999-09-09 to 2001-02-22, but the copy in this tree was dated 2001-02-22.
- `459` attempts to update the contents of `usr/doc/...` which does not exist in this tree - these changes were removed from the patch file.
- `460` seems to break `f77` builds, but this is fixed in `462`, so I commented out the build for `f77` until then.
- `476` was a confusing one. Mounting the file system of a 2.11BSD pl 476 image shows zero changes to `sys/mdec/bruboot.s` between the patchset applied to this repo and the patchset applied to the image, yet building `bruboot.s` results `warning: unterminated literal` errors on line 51, 52 and 100.
  ```
  $ curl -o ci.dsk.gz https://asjackson-211bsd-ci.s3.fr-par.scw.cloud/211bsd-ci-476.dsk.gz
  $ gzip -d ci.dsk.gz
  $ mkdir bsd
  $ bsd211fs ci.dsk bsd
  $ diff bsd/usr/src/sys/mdec/bruboot.s git/2.11BSD/sys/mdec/bruboot.s
  $ echo $?    # same if 0
  0
  $ rsync -av ~/git/2.11BSD/ bsd/usr/src/
  $ umount bsd
  $ ./pdp      # my script for booting pdp from ci.dsk
  ...
     # cd /usr/src/sys/mdec
	 # make clean
	 # make
	 ...
	 # [ $? -eq 0 ] && echo works on my local machine
	 works on my local machine
  ```
  Eventually I realised mistake - like with `459`, I had to remove the changes to `use/doc/...` but while doing so, accidentally removed the changeset to the Makefile for `mdec`. I'm leaving this in here to document my stupidity while applying these patches :-)
  
## Related Stuff

I have also been working on the following related stuff:

- [2.11BSD-Distribution-Builder](https://github.com/AaronJackson/2.11BSD-Distribution-Builder) to produce installation / distribution files for the same set of patch levels.
- [2.11BSD-Action](https://github.com/AaronJackson/2.11BSD-Action) which allows GitHub workflows to perform testing against a 2.11BSD simulated PDP-11 - this also performs builds of all patch levels and provides .img files for each
- [bnlisp](https://github.com/AaronJackson/bnlisp) A small lisp written in K&R C for 2.11BSD, dedicated to my friend bnl, written with lots of help from stylewarning
