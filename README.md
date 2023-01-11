# Fuzzing OpenSSL

This repository has a companion blog post titled "Finding CVE-2022-3786 (openssl) with Mayhem" at https://www.seandeaton.com.

## tl;dr

All of this is taken care of for you with the included Dockerfile (also on DockerHub). You can run it like so:

```shell
# Build the container
docker build --tag openssl-cve-2022-3768 .
# Or if you just want to pull down the existing one:
TODO
# Ensure that you're in this project's root directory (ie you can see ./output/)
# Mount the ./input/ directory to the containers /input. This is for fuzz input.
# This is Linux specific, Windows I think has %CD% in lieu of $(pwd)?
docker run --interactive --tty --volume $(pwd)/input:/input
```

The entrypoint of the container is to just run `afl` so you can get started
fuzzing immediately. To override this behavior, append `/bin/bash` to the end
of the `docker run` line.

## Getting a Vulnerable Version

The last commit that includes the vulnerability is commit SHA `3b421ebc64c7b52f1b9feb3812bdc7781c784332` from November 1st, 2022. It was fixed in commit SHA `680e65b94c916af259bfdc2e25f1ab6e0c7a97d6`. We can get the vulnerable version easily with `git`:

```shell
# Clone the repository.
git clone git://git.openssl.org/openssl.git
# Change into the working directory.
cd openssl
# Detach HEAD from origin to examine the code as it was when it was vulnerable.
git checkout 3b421ebc64c7b52f1b9feb3812bdc7781c784332
```

## Compiling

For compilation, we use AFL's gcc compiler (because I kept getting undefined
references with `clang`). Because of the small buffer overflow
offset, we also want to use address sanitization (ASAN), enabled with AFL's
environment variable `AFL_USE_ASAN`. Given ASAN's use of large amounts of
memory, we also need to restrict the address space which we can do by compiling
the program for a 32-bit architecture. More detail [here][afl-asan].

OpenSSL's configuration for 32-bit takes in the flags `-m32` and
`linux-generic32`. The `compile.sh` script does this for you.

```shell
# Configuration
AFL_USE_ASAN=1 CC=afl-gcc-fast CXX=afl-g++-fast ./Configure -m32 linux-generic32
# Make
AFL_USE_ASAN=1 CC=afl-gcc-fast CXX=afl-g++-fast CFLAGS="-m32" CXXFLAGS="-m32" make
```

This could take awhile given your system's resources. After compilation, we need
to compile our harness. A Makefile is given.

```shell
# Compile the harness.
$ make harness
# Run the harness.
$ ./harness input/seed0.txt
ossl_a2ulabel returned: 1
```

And there you go, you can get started fuzzing the `ossl_a2ulabel` in `openssl`.
With AFL the command looks something like the following (or just use the
included `run.sh` script).

```shell
afl-fuzz -i /input -o /output /harness/harness @@
```

[afl-asan]: https://afl-1.readthedocs.io/en/latest/notes_for_asan.html
