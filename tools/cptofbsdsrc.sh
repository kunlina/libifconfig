#!/bin/sh
gitdir="${1}"
svndir="${2}"
patchdir="${3}"

if [ ! -d "${gitdir}" ]; then
    echo "Source not a directory"
    exit
fi

if [ ! -d "${svndir}" ]; then
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
echo "Reverting destination tree. Press any key to continue..."
read discardtext
svn revert -R -q "${svndir}"
svn remove --force -q "${svndir}/lib/libifconfig"
svn remove --force -q "${svndir}/share/examples/libifconfig"

echo "Copying things"
# Copy things!
cp -R "${gitdir}/src" "${svndir}/lib/libifconfig"

cp "${gitdir}/Makefile.base" "${svndir}/lib/libifconfig/Makefile"
# Ugly hack to not copy example_ executables.
make -C "${gitdir}/examples" clean cleandepend
cp -Rf "${gitdir}/examples" "${svndir}/share/examples/libifconfig"
# Remove Makefile, because it's tailored for standalone builds
#
# Should copy in a Makefile.base once libifconfig is no longer marked private 
# in base.
rm "${svndir}/share/examples/libifconfig/Makefile"

# 
# Only make patches if third parameter (patch directory) is provided
# Create one patch for libifconfig itself, and one for example files.
# A separate patch for manpage(s) should be made once manpages are a thing.
# 
if [ -d "${patchdir}" ]; then
    cd "${svndir}"
    echo "Creating libifconfig patch file for upstreaming"
    svn add "${svndir}/lib/libifconfig"
    svn diff --diff-cmd=diff -x -U999999 . > "${patchdir}/libifconfig.patch"
    svn revert -R "${svndir}/lib/libifconfig"

    echo "Creating libifconfig examples patch file for upstreaming"
    svn add "${svndir}/share/examples/libifconfig"
    svn diff --diff-cmd=diff -x -U999999 . > "${patchdir}/libifconfig_examples.patch"
    svn revert -R "${svndir}/share/examples/libifconfig"

    echo "End of script. Yay."
fi
