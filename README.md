# passkey

### Password Manager

This is a simple local password manager for all your master keys or for personal use. I used the `openssl` command line tool, i did not implement the encryption & decryption functions.

### Install `openssl`

```bash
dnf install openssl
```
I use fedora so i use `dnf`. Figure out how to install `openssl` on your distro. It's fairly simple.

### Steps to setup vault

Step 1: Generate `key.bin` and `iv.bin`.

These are essential for encrypting and decrypting files that are used to store passwords.

```bash
# Generate a 256-bit (32-byte) key
openssl rand -out key.bin 32

# Generate a 128-bit (16-byte) IV
openssl rand -out iv.bin 16
```

Store both these files in any folder and `export` the folder path. Use `credentials_path` as the name for env.

```bash
export credentials_path="/path/to/folder/"
```

Use the `/` at the end of the path as mentioned above.

### Build

Clone the repo.

```bash
git clone https://github.com/horriedhenry/passkey.git
```
Build

```bash
cd passkey
mkdir build
cd build
```

Inside the build directory 

```bash
cmake ..
make
```
This will produce an executable named `passkey`. And here is the usage for the executable.

```bash
USAGE :
 passkey option [args]

options :
a, add        add new entry to vault
g, get        get entry from vault
d, delete     delete entry from vault

args :
a,            [sitename email password]
g,            [sitename]
d,            [sitename]

credentials :
credentials folder should have 'key.bin' and 'iv.bin' files
```

Example usage. Assuming you are in `build` folder.

```bash

./passkey a google.com some@gmail.com pwd@1234#
./passkey g google.com
./passkey d google.com

```

You can add `passkey` executable to your path or move it to `/usr/local/bin/` or anywhere you want. And then you can use `passkey` command from anywhere in cmd line.
