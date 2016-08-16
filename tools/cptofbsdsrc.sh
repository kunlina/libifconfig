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
rm -Rf "${svndir}/lib/libifconfig"
rm -Rf "${svndir}/share/examples/libifconfig"

echo "Copying things"
# Copy things!
cp -R "${gitdir}/src" "${svndir}/lib/libifconfig"
cp "${gitdir}/Makefile.base" "${svndir}/lib/libifconfig/Makefile"
cp -R "${gitdir}/examples" "${svndir}/share/examples/libifconfig"

echo "Patching things"
patch -p0 -d "${svndir}" < "${gitdir}tools/bsdbase_lib.patch"
rm "${svndir}/share/mk/bsd.libnames.mk.orig"
rm "${svndir}/share/mk/src.libnames.mk.orig"
svn add "${svndir}/lib/libifconfig"
svn add "${svndir}/share/examples/libifconfig"

echo "Storing diff as ~/libifconfig.patch"
cd "${svndir}"
svn diff --diff-cmd=diff -x -U999999 . > ~/libifconfig.patch

echo "End of script. Yay."