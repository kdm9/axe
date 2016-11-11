# Copyright (c) 2014-2015 Kevin Murray <spam@kdmurray.id.au>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

INCLUDE(GetGitRevisionDescription)

function(GetGitSemVer _var)
	get_git_head_revision(rev hash)
	git_get_exact_tag(tag)

	IF(NOT "${tag}" MATCHES "^-")
		SET(vers "${tag}")
	ELSE()
		git_describe(gitdesc "--always")
		if("${gitdesc}" MATCHES "^.+-.+-.+$")
			STRING (REGEX REPLACE "-" " " gdlist ${gitdesc})
			SEPARATE_ARGUMENTS(gdlist)
			LIST(GET gdlist 0 tag)
			LIST(GET gdlist 1 cmts_since_tag)
			SET(vers "${tag}-${cmts_since_tag}-dirty")
		ELSE()
			SET(vers "dirty")
		ENDIF()
	ENDIF()

	IF (NOT "${hash}" STREQUAL "")
		STRING(SUBSTRING ${hash} 0 7 hash)
		set(vers "${vers}+git=${hash}")
	ENDIF()
	set(${_var} ${vers} PARENT_SCOPE)
endfunction()
