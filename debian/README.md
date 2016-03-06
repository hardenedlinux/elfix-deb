# About

This is a Debian port of elfix tools [https://dev.gentoo.org/~blueness/elfix/].

# Layout of git branches

This git repo follows a common practice based on gbp
[http://honk.sigxcpu.org/projects/git-buildpackage/manual-html/gbp.html]. We
currently have three branches:

* master: which is the debian-branch.
* pristine-tar: which holds the upstream tarballs
* upstream/0.9.x: which is the `elfix-0.9.x` branch from upstream elfix.git
  [https://gitweb.gentoo.org/proj/elfix.git/].

# Build procedure

1. Install essential building environment:
<pre>
    aptitude install build-essential git-buildpackage
</pre>

2. Install other elfix build requirements: 
<pre>
    aptitude install libelf-dev libattr1-dev dh-autoreconf python-all python-setuptools python-all-dev
</pre>

3. clone git repo and get all branches to local repo
<pre>
    git clone https://github.com/hardenedlinux/elfix-deb.git
    git checkout pristine-tar
    git checkout upstream/0.9.x
    git checkout master
</pre>

4. Build binary package
<pre>
    gbp buildpackage
</pre>

Multiple produced files are then available in the parent directory including
the binary package. Error maybe occurs during the debsign step if you haven't
properly configured GPG. You can simply ignore this error if you just install
the binary package locally.

## Compile from souce code

Install the dependency packages:
<pre>
sudo apt-get install -y vim libc6-dev libelf-dev libattr1-dev
</pre>

We follow the [Gentoo build options](https://gitweb.gentoo.org/repo/gentoo.git/tree/sys-apps/elfix/elfix-0.9.2.ebuild):
<pre>
./configure --enable-ptpax --enable-xtpax --disable-tests
make && sudo make install
</pre>

# TODO

1. Multiple scripts provided by this package requires the `portage` Python
   module which is not available on Debian.
