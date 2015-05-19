# this file is part of packlim.
#
# Copyright (c) 2015 Dima Krasner
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

package require packlim

proc main {} {
	if {2 > $::argc} {
		usage "update|available|installed|install|remove|lock|source|purge \[ARG\]..."
	}

	set env [env]
	file mkdir /var/packlim /var/packlim/installed

	# wait for other instances to terminate
	if {[lockf.locked /var/packlim/lock]} {
		packlim::log warn "another instance is running; waiting"
	}
	set lock [lockf.lock /var/packlim/lock]

	switch -exact [lindex $::argv 1] update {
		if {2 != $::argc} {
			usage update
		}

		packlim::update [curl] [get_repo $env]
	} available {
		if {2 != $::argc} {
			usage available
		}

		set packages [lindex [packlim::available [curl] [get_repo $env]] 0]
		foreach name [dict keys $packages] {
			set package $packages($name)
			puts "$package(name)|$package(version)|$package(description)"
		}
	} installed {
		if {2 != $::argc} {
			usage installed
		}

		set packages [packlim::installed]
		foreach name [dict keys $packages] {
			set package [lindex $packages($name) 0]
			puts "$package(name)|$package(version)|$package(description)"
		}
	} install {
		if {3 != $::argc} {
			usage "install NAME"
		}

		set path /etc/packlim/pub_key
		if {![file exists $path]} {
			packlim::log error "failed to read the public key"
			exit 1
		} else {
			packlim::with_file fp $path r {set key [$fp read -nonewline]}
		}

		set repo [get_repo $env]
		set curl [curl]
		set available [packlim::available $curl $repo]

		packlim::install $curl $repo {*}$available [lindex $::argv 2] user $key
	} remove {
		if {3 != $::argc} {
			usage "remove NAME"
		}

		packlim::remove [lindex $::argv 2] [packlim::installed]
	} lock {
		if {3 != $::argc} {
			usage "lock NAME"
		}

		packlim::lock [lindex $::argv 2]
	} source {
		if {3 != $::argc} {
			usage "source SCRIPT"
		}

		source [lindex $::argv 2]
	} purge {
		if {2 != $::argc} {
			usage "purge"
		}

		packlim::purge
	} default {
		usage "update|available|installed|install|remove|lock|source|purge \[ARG\]..."
	}
}

try {
	main
} on error {msg opts} {
	if {0 < [string length $msg]} {
		packlim::log error $msg
	} else {
		packlim::log bug "caught an unknown, unhandled exception"
	}
	exit 1
}
