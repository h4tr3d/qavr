#!/bin/bash

if [ -z "$2" ]; then
  echo "Use: $0 <src.c> <hdr.h>"
  exit 1
fi

git log > /dev/null 2>&1
is_git=$?

revision=
[ $is_git -eq 0 ] && \
revision=`LANG=C git log --date=rfc | grep Date: | head -1 | sed 's|Date:||' | xargs -I{} date -d "{}" +%Y%m%d%H%M`

if [ -z "$revision" ]; then
  revision="_unknown"
fi

# gitinfo.c
cat >  $1 << _EOF_
const char *git_revision = "$revision";
_EOF_

# gitinfo.h
cat > $2 << _EOF_
#ifndef _GITINFO_H_
#define _GITINFO_H_

#define GIT_REVISION "$revision"
extern const char *git_revision;

#endif /* _GITINFO_H_ */
_EOF_
