do_compile_append () {
	# now generate script to drive testing
	echo "#!/usr/bin/env sh" >${B}/${HOST_PREFIX}testeglibc
	set >> ${B}/${HOST_PREFIX}testeglibc
	# prune out the unneeded vars
	sed -i -e "/^BASH/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^USER/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^OPT/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^DIRSTACK/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^EUID/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^FUNCNAME/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^GROUPS/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^HOST/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^HOME/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^IFS/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^LC_ALL/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^LOGNAME/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^MACHTYPE/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^OSTYPE/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^PIPE/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^SHELL/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^'/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^UID/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^TERM/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^PATCH_GET/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^PKG_/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^POSIXLY_/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^PPID/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^PS4/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^Q/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^SHLVL/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^STAGING/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^LD_LIBRARY_PATH/d" ${B}/${HOST_PREFIX}testeglibc
	sed -i -e "/^PSEUDO/d" ${B}/${HOST_PREFIX}testeglibc

	# point to real sysroot not the toolchain bootstrap sysroot
	sed -i -e "s/\-tcbootstrap//g" ${B}/${HOST_PREFIX}testeglibc

	# use the final cross-gcc to test since some tests need libstdc++
	sed -i -e "s/^PATH=.*\.gcc-cross-intermediate\:/PATH=/g" ${B}/${HOST_PREFIX}testeglibc

	# append execution part script
cat >> ${B}/${HOST_PREFIX}testeglibc << STOP
target="\$1"
if [ "x\$target" = "x" ]
then
	echo "Please specify the target machine and remote user in form of user@target"
	exit 1;
fi
ssh \$target ls \$PWD\  2>&1 > /dev/null
if [ "x\$?" != "x0" ]
then
	echo "Failed connecting to \$target it could be because of:"
	echo "1. You dont have passwordless ssh setup to access \$target"
	echo "2. NFS share on \$target is not mounted or if mounted then not matching the build tree layout."
	echo "   The tree should be accessible at same location on build host and target"
	echo "   You can add nfs-server to IMAGE_FEATURES to get the nfs client on target"
	echo "3. nfs server on build host is not running."
	echo "   Please make sure that you have 'no_root_squash' added in /etc/exports if you want"
	echo "   to test as root user on target (usually its recommended to create a non"
	echo "   root user."
	echo "   As a sanity check make sure that target can read/write to the eglibc build tree"
	echo "   Please refer to ${S}/EGLIBC.cross-testing for further instructions on setup"
	exit 1
fi
	echo "# we test using cross compiler from real sysroot therefore override the" > ${B}/configparms
	echo "# definitions that come from ${B}/config.make" >> ${B}/configparms

	fgrep tcbootstrap ${B}/config.make > ${B}/configparms
	sed -i -e "s/\-tcbootstrap//g" ${B}/configparms
wrapper="${S}/scripts/cross-test-ssh.sh \$target"
localedef="${STAGING_BINDIR_NATIVE}/cross-localedef --little-endian --uint32-align=4"
make tests-clean
make cross-localedef="\$localedef" cross-test-wrapper="\$wrapper" -k check
rm -rf ${B}/configparms
STOP

	chmod +x ${B}/${HOST_PREFIX}testeglibc
}
