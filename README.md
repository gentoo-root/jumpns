# jumpns

Allow unprivileged users to enter existing network namespaces, according to
configurable ACLs.

## Installation

    $ make
    # make install

`make install` installs to /usr/local/bin by default, it can be overridden by
setting `BINDIR=...`.

If you choose not to use `make install`, make sure to set the capabilities:

    # setcap CAP_SYS_ADMIN+ep jumpns

jumpns must not be granted SUID or SGID permissions, because it doesn't drop
privileges back to the regular user.

## Configuration

Create the directory for ACLs:

    # mkdir -p /etc/jumpns/net

When a user attempts to enter a namespace, jumpns checks whether the file named
the same as the namespace is readable by the user. It allows to use the
flexibility of the standard Unix permissions and file ACLs to control who can
enter which namespace. A few examples:

    Allow everyone to enter netns earth:
    # install -m 444 /dev/null /etc/jumpns/net/earth

    Allow group astronauts to enter netns mars:
    # install -m 040 -g astronauts /dev/null /etc/jumpns/net/mars

    Allow users alice and malice to enter netns remorse:
    # install -m 000 /dev/null /etc/jumpns/net/remorse
    # setfacl -m u:alice:r,u:malice:r /etc/jumpns/net/remorse

## Usage

    $ jumpns <netns name> <executable> [<parameters>...]

For example:

    $ jumpns otherside bash -l

## Implementation notes

A slave mount namespace is unshared, and /sys is remounted to expose the correct
netdevs under /sys/class/net et al., similar to `ip netns exec`. If /sys is not
mounted in the original namespace, it won't be mounted in the target namespace.
