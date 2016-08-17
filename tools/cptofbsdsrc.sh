#!/bin/sh
gitdir="${1}"
svndir="${2}"

if [ ! -d "${1}" ]; then
    echo "Source not a directory"
    exit
fi

if [ ! -d "${2}" ]; then
    echo "Destination not a directory"
    exit
fi

stuff=`svn info "${svndir}" | grep UUID | cut -w -f 3`
expecteduuid="ccf9f872-aa2e-dd11-9fc8-001c23d0bc1f"

if [ "${stuff}" != "${expecteduuid}" ]; then
    echo "SVN repo UUID mismatch."
    exit
fi

# Clean things!
echo "Reverting destination tree"
svn revert -R -q "${svndir}"
rm -Rf "${svndir}/lib/libifc"
rm -Rf "${svndir}/share/examples/libifc"

echo "Copying things"
# Copy things!
cp -R "${gitdir}/src" "${svndir}/lib/libifc"
cp "${gitdir}/Makefile.base" "${svndir}/lib/libifc/Makefile"
# Ugly hack to not copy example_ executables.
cd "${gitdir}/examples"
make clean
cp -Rf  "${gitdir}/examples" "${svndir}/share/examples/libifc"

echo "Patching things"
patch -p0 -d "${svndir}" < "${gitdir}tools/bsdbase_lib.patch"
rm "${svndir}/share/mk/bsd.libnames.mk.orig"
rm "${svndir}/share/mk/src.libnames.mk.orig"
svn add "${svndir}/lib/libifc"
svn add "${svndir}/share/examples/libifc"

echo "Storing diff as ~/libifc.patch"
cd "${svndir}"
svn diff --diff-cmd=diff -x -U999999 . > ~/libifc.patch

echo "End of script. Yay."
