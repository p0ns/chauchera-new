Release Process
====================

Before every release candidate:

* Update translations (ping wumpus on IRC) see [translation_process.md](https://github.com/bitcoin/bitcoin/blob/master/doc/translation_process.md#synchronising-translations).

Before every minor and major release:

* Update [bips.md](bips.md) to account for changes since the last release.
* Update version in sources (see below)
* Write release notes (see below)
* Update `src/chainparams.cpp` nMinimumChainWork with information from the getblockchaininfo rpc.

Before every major release:

* Update hardcoded [seeds](/contrib/seeds/README.md), see [this pull request](https://github.com/bitcoin/bitcoin/pull/7415) for an example.

### First time / New builders

Check out the source code in the following directory hierarchy.

    cd /path/to/your/toplevel/build
    git clone https://github.com/chaucha-project/gitian.sigs.ltc.git
    git clone https://github.com/chaucha-project/chaucha-detached-sigs.git
    git clone https://github.com/devrandom/gitian-builder.git
    git clone https://github.com/chaucha-project/chaucha.git

### Chaucha maintainers/release engineers, update version in sources

Update the following:

- `configure.ac`:
    - `_CLIENT_VERSION_MAJOR`
    - `_CLIENT_VERSION_MINOR`
    - `_CLIENT_VERSION_REVISION`
    - Don't forget to set `_CLIENT_VERSION_IS_RELEASE` to `true`
- `src/clientversion.h`: (this mirrors `configure.ac` - see issue #3539)
    - `CLIENT_VERSION_MAJOR`
    - `CLIENT_VERSION_MINOR`
    - `CLIENT_VERSION_REVISION`
    - Don't forget to set `CLIENT_VERSION_IS_RELEASE` to `true`
- `doc/README.md` and `doc/README_windows.txt`
- `doc/Doxyfile`: `PROJECT_NUMBER` contains the full version
- `contrib/gitian-descriptors/*.yml`: usually one'd want to do this on master after branching off the release - but be sure to at least do it before a new major release

Write release notes. git shortlog helps a lot, for example:

    git shortlog --no-merges v(current version, e.g. 0.7.2)..v(new version, e.g. 0.8.0)

(or ping @wumpus on IRC, he has specific tooling to generate the list of merged pulls
and sort them into categories based on labels)

Generate list of authors:

    git log --format='%aN' "$*" | sort -ui | sed -e 's/^/- /'

Tag version (or release candidate) in git

    git tag -s v(new version, e.g. 0.8.0)

### Setup and perform Gitian builds

Setup Gitian descriptors:

    pushd ./chaucha
    export SIGNER=(your Gitian key, ie bluematt, sipa, etc)
    export VERSION=(new version, e.g. 0.8.0)
    git fetch
    git checkout v${VERSION}
    popd

Ensure your gitian.sigs.ltc are up-to-date if you wish to gverify your builds against other Gitian signatures.

    pushd ./gitian.sigs.ltc
    git pull
    popd

Ensure gitian-builder is up-to-date:

    pushd ./gitian-builder
    git pull
    popd

### Fetch and create inputs: (first time, or when dependency versions change)

    pushd ./gitian-builder
    mkdir -p inputs
    wget -P inputs https://bitcoincore.org/cfields/osslsigncode-Backports-to-1.7.1.patch
    wget -P inputs http://downloads.sourceforge.net/project/osslsigncode/osslsigncode/osslsigncode-1.7.1.tar.gz
    popd

Create the OS X SDK tarball, see the [OS X readme](README_osx.md) for details, and copy it into the inputs directory.

### Optional: Seed the Gitian sources cache and offline git repositories

By default, Gitian will fetch source files as needed. To cache them ahead of time:

    pushd ./gitian-builder
    make -C ../chaucha/depends download SOURCES_PATH=`pwd`/cache/common
    popd

Only missing files will be fetched, so this is safe to re-run for each build.

NOTE: Offline builds must use the --url flag to ensure Gitian fetches only from local URLs. For example:

    pushd ./gitian-builder
    ./bin/gbuild --url chaucha=/path/to/chaucha,signature=/path/to/sigs {rest of arguments}
    popd

The gbuild invocations below <b>DO NOT DO THIS</b> by default.

### Build and sign Chauchera for Linux, Windows, and OS X:

    pushd ./gitian-builder
    ./bin/gbuild --memory 3000 --commit chaucha=v${VERSION} ../chaucha/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-linux --destination ../gitian.sigs.ltc/ ../chaucha/contrib/gitian-descriptors/gitian-linux.yml
    mv build/out/chaucha-*.tar.gz build/out/src/chaucha-*.tar.gz ../

    ./bin/gbuild --memory 3000 --commit chaucha=v${VERSION} ../chaucha/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-win-unsigned --destination ../gitian.sigs.ltc/ ../chaucha/contrib/gitian-descriptors/gitian-win.yml
    mv build/out/chaucha-*-win-unsigned.tar.gz inputs/chaucha-win-unsigned.tar.gz
    mv build/out/chaucha-*.zip build/out/chaucha-*.exe ../

    ./bin/gbuild --memory 3000 --commit chaucha=v${VERSION} ../chaucha/contrib/gitian-descriptors/gitian-osx.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-osx-unsigned --destination ../gitian.sigs.ltc/ ../chaucha/contrib/gitian-descriptors/gitian-osx.yml
    mv build/out/chaucha-*-osx-unsigned.tar.gz inputs/chaucha-osx-unsigned.tar.gz
    mv build/out/chaucha-*.tar.gz build/out/chaucha-*.dmg ../
    popd

Build output expected:

  1. source tarball (`chaucha-${VERSION}.tar.gz`)
  2. linux 32-bit and 64-bit dist tarballs (`chaucha-${VERSION}-linux[32|64].tar.gz`)
  3. windows 32-bit and 64-bit unsigned installers and dist zips (`chaucha-${VERSION}-win[32|64]-setup-unsigned.exe`, `chaucha-${VERSION}-win[32|64].zip`)
  4. OS X unsigned installer and dist tarball (`chaucha-${VERSION}-osx-unsigned.dmg`, `chaucha-${VERSION}-osx64.tar.gz`)
  5. Gitian signatures (in `gitian.sigs.ltc/${VERSION}-<linux|{win,osx}-unsigned>/(your Gitian key)/`)

### Verify other gitian builders signatures to your own. (Optional)

Add other gitian builders keys to your gpg keyring

    gpg --import chaucha/contrib/gitian-keys/*.pgp

Verify the signatures

    pushd ./gitian-builder
    ./bin/gverify -v -d ../gitian.sigs.ltc/ -r ${VERSION}-linux ../chaucha/contrib/gitian-descriptors/gitian-linux.yml
    ./bin/gverify -v -d ../gitian.sigs.ltc/ -r ${VERSION}-win-unsigned ../chaucha/contrib/gitian-descriptors/gitian-win.yml
    ./bin/gverify -v -d ../gitian.sigs.ltc/ -r ${VERSION}-osx-unsigned ../chaucha/contrib/gitian-descriptors/gitian-osx.yml
    popd

### Next steps:

Commit your signature to gitian.sigs.ltc:

    pushd gitian.sigs.ltc
    git add ${VERSION}-linux/${SIGNER}
    git add ${VERSION}-win-unsigned/${SIGNER}
    git add ${VERSION}-osx-unsigned/${SIGNER}
    git commit -a
    git push  # Assuming you can push to the gitian.sigs.ltc tree
    popd

Wait for Windows/OS X detached signatures:

- Once the Windows/OS X builds each have 3 matching signatures, they will be signed with their respective release keys.
- Detached signatures will then be committed to the [chaucha-detached-sigs](https://github.com/chaucha-project/chaucha-detached-sigs) repository, which can be combined with the unsigned apps to create signed binaries.

Create (and optionally verify) the signed OS X binary:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSION} ../chaucha/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-osx-signed --destination ../gitian.sigs.ltc/ ../chaucha/contrib/gitian-descriptors/gitian-osx-signer.yml
    ./bin/gverify -v -d ../gitian.sigs.ltc/ -r ${VERSION}-osx-signed ../chaucha/contrib/gitian-descriptors/gitian-osx-signer.yml
    mv build/out/chaucha-osx-signed.dmg ../chaucha-${VERSION}-osx.dmg
    popd

Create (and optionally verify) the signed Windows binaries:

    pushd ./gitian-builder
    ./bin/gbuild -i --commit signature=v${VERSION} ../chaucha/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gsign --signer $SIGNER --release ${VERSION}-win-signed --destination ../gitian.sigs.ltc/ ../chaucha/contrib/gitian-descriptors/gitian-win-signer.yml
    ./bin/gverify -v -d ../gitian.sigs.ltc/ -r ${VERSION}-win-signed ../chaucha/contrib/gitian-descriptors/gitian-win-signer.yml
    mv build/out/chaucha-*win64-setup.exe ../chaucha-${VERSION}-win64-setup.exe
    mv build/out/chaucha-*win32-setup.exe ../chaucha-${VERSION}-win32-setup.exe
    popd

Commit your signature for the signed OS X/Windows binaries:

    pushd gitian.sigs.ltc
    git add ${VERSION}-osx-signed/${SIGNER}
    git add ${VERSION}-win-signed/${SIGNER}
    git commit -a
    git push  # Assuming you can push to the gitian.sigs.ltc tree
    popd

### After 3 or more people have gitian-built and their results match:

- Create `SHA256SUMS.asc` for the builds, and GPG-sign it:

```bash
sha256sum * > SHA256SUMS
```

The list of files should be:
```
chaucha-${VERSION}-aarch64-linux-gnu.tar.gz
chaucha-${VERSION}-arm-linux-gnueabihf.tar.gz
chaucha-${VERSION}-i686-pc-linux-gnu.tar.gz
chaucha-${VERSION}-x86_64-linux-gnu.tar.gz
chaucha-${VERSION}-osx64.tar.gz
chaucha-${VERSION}-osx.dmg
chaucha-${VERSION}.tar.gz
chaucha-${VERSION}-win32-setup.exe
chaucha-${VERSION}-win32.zip
chaucha-${VERSION}-win64-setup.exe
chaucha-${VERSION}-win64.zip
```
The `*-debug*` files generated by the gitian build contain debug symbols
for troubleshooting by developers. It is assumed that anyone that is interested
in debugging can run gitian to generate the files for themselves. To avoid
end-user confusion about which file to pick, as well as save storage
space *do not upload these to the chaucha.org server, nor put them in the torrent*.

- GPG-sign it, delete the unsigned file:
```
gpg --digest-algo sha256 --clearsign SHA256SUMS # outputs SHA256SUMS.asc
rm SHA256SUMS
```
(the digest algorithm is forced to sha256 to avoid confusion of the `Hash:` header that GPG adds with the SHA256 used for the files)
Note: check that SHA256SUMS itself doesn't end up in SHA256SUMS, which is a spurious/nonsensical entry.

- Upload zips and installers, as well as `SHA256SUMS.asc` from last step, to the chaucha.org server.

- Update chaucha.org version

- Announce the release:

  - chaucha-dev mailing list

  - Chauchera announcements list https://groups.google.com/forum/#!forum/chaucha-dev

  - blog.chaucha.org blog post

  - chauchatalk.io forum announcement

  - Update title of #chaucha on Freenode IRC

  - Optionally twitter, reddit /r/Chaucha, ... but this will usually sort out itself

  - Add release notes for the new version to the directory `doc/release-notes` in git master

  - Celebrate
